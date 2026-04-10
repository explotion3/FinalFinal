#include "Systems/FinalEnemyIntentService.h"

#include "Battle/Definitions/FinalEnemyIntentDefinition.h"
#include "Runtime/FinalBattleEnemyState.h"

namespace
{
bool PassesStaticRequirements(const FFinalBattleEnemyState& EnemyState, const FFinalBattleEnemyIntentRuntimeState& IntentState)
{
	if (IntentState.Definition == nullptr)
	{
		return false;
	}

	if (!IntentState.Definition->RequiredEnemyRoleTags.IsEmpty()
		&& !EnemyState.RoleTags.HasAll(IntentState.Definition->RequiredEnemyRoleTags))
	{
		return false;
	}

	if (IntentState.Definition->UseLimitPerBattle > 0
		&& IntentState.UseCount >= IntentState.Definition->UseLimitPerBattle)
	{
		return false;
	}

	if (IntentState.Definition->PhaseTags.Num() > 0)
	{
		if (EnemyState.CurrentPhaseTag == NAME_None || !IntentState.Definition->PhaseTags.Contains(EnemyState.CurrentPhaseTag))
		{
			return false;
		}
	}

	return true;
}

void RefreshPhaseState(FFinalBattleEnemyState& EnemyState)
{
	if (EnemyState.PhaseSequence.Num() == 0 || EnemyState.MaxHP <= 0)
	{
		EnemyState.CurrentPhaseIndex = INDEX_NONE;
		EnemyState.CurrentPhaseTag = NAME_None;
		return;
	}

	const float CurrentHpPercent = FMath::Clamp(static_cast<float>(EnemyState.CurrentHP) / static_cast<float>(EnemyState.MaxHP), 0.0f, 1.0f);
	int32 ResolvedPhaseIndex = INDEX_NONE;

	for (int32 PhaseIndex = 0; PhaseIndex < EnemyState.PhaseSequence.Num(); ++PhaseIndex)
	{
		if (CurrentHpPercent <= EnemyState.PhaseSequence[PhaseIndex].MaxHpPercent)
		{
			ResolvedPhaseIndex = PhaseIndex;
		}
	}

	if (ResolvedPhaseIndex == INDEX_NONE)
	{
		ResolvedPhaseIndex = 0;
	}

	if (EnemyState.CurrentPhaseIndex != INDEX_NONE)
	{
		ResolvedPhaseIndex = FMath::Max(EnemyState.CurrentPhaseIndex, ResolvedPhaseIndex);
	}

	if (EnemyState.CurrentPhaseIndex != ResolvedPhaseIndex)
	{
		EnemyState.CurrentIntentIndex = INDEX_NONE;
	}

	EnemyState.CurrentPhaseIndex = ResolvedPhaseIndex;
	EnemyState.CurrentPhaseTag = EnemyState.PhaseSequence.IsValidIndex(ResolvedPhaseIndex)
		? EnemyState.PhaseSequence[ResolvedPhaseIndex].PhaseTag
		: NAME_None;
}

bool IsIntentEligible(const FFinalBattleEnemyState& EnemyState, const FFinalBattleEnemyIntentRuntimeState& IntentState, const int32 PreviewRound)
{
	return PassesStaticRequirements(EnemyState, IntentState)
		&& PreviewRound >= IntentState.NextAvailableRound;
}

void ApplySelection(FFinalBattleEnemyState& EnemyState, const int32 SelectedIndex)
{
	if (!EnemyState.IntentRuntimeStates.IsValidIndex(SelectedIndex))
	{
		EnemyState.CurrentIntentDefinition = nullptr;
		EnemyState.CurrentIntentIndex = INDEX_NONE;
		EnemyState.CurrentIntentId = NAME_None;
		EnemyState.CurrentIntentText = FText::Format(
			NSLOCTEXT("FinalEnemyIntentService", "FallbackEnemyIntent", "Attack {0}"),
			FText::AsNumber(EnemyState.RuntimeDamagePower));
		return;
	}

	EnemyState.CurrentIntentIndex = SelectedIndex;
	EnemyState.CurrentIntentDefinition = EnemyState.IntentRuntimeStates[SelectedIndex].Definition;
	EnemyState.CurrentIntentId = EnemyState.IntentRuntimeStates[SelectedIndex].IntentId;
	EnemyState.CurrentIntentText = EnemyState.CurrentIntentDefinition != nullptr && !EnemyState.CurrentIntentDefinition->PreviewText.IsEmpty()
		? EnemyState.CurrentIntentDefinition->PreviewText
		: (EnemyState.CurrentIntentDefinition != nullptr
			? EnemyState.CurrentIntentDefinition->DisplayName
			: FText::Format(
				NSLOCTEXT("FinalEnemyIntentService", "FallbackEnemyIntent", "Attack {0}"),
				FText::AsNumber(EnemyState.RuntimeDamagePower)));
}

int32 SelectFallbackIndex(const FFinalBattleEnemyState& EnemyState)
{
	for (int32 Index = 0; Index < EnemyState.IntentRuntimeStates.Num(); ++Index)
	{
		if (PassesStaticRequirements(EnemyState, EnemyState.IntentRuntimeStates[Index]))
		{
			return Index;
		}
	}

	for (int32 Index = 0; Index < EnemyState.IntentRuntimeStates.Num(); ++Index)
	{
		if (EnemyState.IntentRuntimeStates[Index].Definition != nullptr)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

int32 SelectCycleIndex(const FFinalBattleEnemyState& EnemyState, const int32 PreviewRound)
{
	if (EnemyState.IntentRuntimeStates.Num() == 0)
	{
		return INDEX_NONE;
	}

	const int32 StartIndex = EnemyState.CurrentIntentIndex == INDEX_NONE
		? 0
		: (EnemyState.CurrentIntentIndex + 1) % EnemyState.IntentRuntimeStates.Num();

	for (int32 Offset = 0; Offset < EnemyState.IntentRuntimeStates.Num(); ++Offset)
	{
		const int32 CandidateIndex = (StartIndex + Offset) % EnemyState.IntentRuntimeStates.Num();
		if (IsIntentEligible(EnemyState, EnemyState.IntentRuntimeStates[CandidateIndex], PreviewRound))
		{
			return CandidateIndex;
		}
	}

	return INDEX_NONE;
}

int32 SelectWeightedRandomIndex(const FFinalBattleEnemyState& EnemyState, const int32 PreviewRound)
{
	int32 TotalWeight = 0;
	TArray<int32, TInlineAllocator<8>> EligibleIndices;
	TArray<int32, TInlineAllocator<8>> EligibleWeights;

	for (int32 Index = 0; Index < EnemyState.IntentRuntimeStates.Num(); ++Index)
	{
		const FFinalBattleEnemyIntentRuntimeState& IntentState = EnemyState.IntentRuntimeStates[Index];
		if (!IsIntentEligible(EnemyState, IntentState, PreviewRound))
		{
			continue;
		}

		const int32 IntentWeight = FMath::Max(IntentState.Definition->Weight, 1);
		EligibleIndices.Add(Index);
		EligibleWeights.Add(IntentWeight);
		TotalWeight += IntentWeight;
	}

	if (TotalWeight <= 0)
	{
		return INDEX_NONE;
	}

	int32 WeightRoll = FMath::RandRange(1, TotalWeight);
	for (int32 EntryIndex = 0; EntryIndex < EligibleIndices.Num(); ++EntryIndex)
	{
		WeightRoll -= EligibleWeights[EntryIndex];
		if (WeightRoll <= 0)
		{
			return EligibleIndices[EntryIndex];
		}
	}

	return EligibleIndices.Last();
}
}

void FFinalEnemyIntentService::RefreshIntent(FFinalBattleEnemyState& EnemyState, const int32 PreviewRound) const
{
	RefreshPhaseState(EnemyState);

	if (EnemyState.IntentRuntimeStates.Num() == 0)
	{
		ApplySelection(EnemyState, INDEX_NONE);
		return;
	}

	int32 SelectedIndex = INDEX_NONE;

	switch (EnemyState.IntentSelectRule)
	{
	case EFinalIntentSelectRule::WeightedRandom:
		SelectedIndex = SelectWeightedRandomIndex(EnemyState, PreviewRound);
		break;

	case EFinalIntentSelectRule::Cycle:
	case EFinalIntentSelectRule::PhaseSequence:
	case EFinalIntentSelectRule::Scripted:
	default:
		SelectedIndex = SelectCycleIndex(EnemyState, PreviewRound);
		break;
	}

	if (SelectedIndex == INDEX_NONE)
	{
		SelectedIndex = SelectFallbackIndex(EnemyState);
	}

	ApplySelection(EnemyState, SelectedIndex);
}

void FFinalEnemyIntentService::CommitCurrentIntentExecution(FFinalBattleEnemyState& EnemyState, const int32 CurrentRound) const
{
	if (!EnemyState.IntentRuntimeStates.IsValidIndex(EnemyState.CurrentIntentIndex))
	{
		return;
	}

	FFinalBattleEnemyIntentRuntimeState& IntentState = EnemyState.IntentRuntimeStates[EnemyState.CurrentIntentIndex];
	if (IntentState.Definition == nullptr)
	{
		return;
	}

	++IntentState.UseCount;
	IntentState.NextAvailableRound = CurrentRound + FMath::Max(IntentState.Definition->CooldownTurns, 0) + 1;
}
