#include "Systems/FinalEnemyIntentService.h"

#include "Battle/Definitions/FinalEnemyIntentDefinition.h"
#include "Runtime/FinalBattleEnemyState.h"

namespace
{
float GetEnemyHpPercent(const FFinalBattleEnemyState& EnemyState)
{
	if (EnemyState.MaxHP <= 0)
	{
		return 0.0f;
	}

	return FMath::Clamp(static_cast<float>(EnemyState.CurrentHP) / static_cast<float>(EnemyState.MaxHP), 0.0f, 1.0f);
}

bool PassesDefinitionRequirements(const FFinalBattleEnemyState& EnemyState, const FFinalBattleEnemyIntentRuntimeState& IntentState)
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

	return true;
}

bool PassesPhaseRequirements(const FFinalBattleEnemyState& EnemyState, const FFinalBattleEnemyIntentRuntimeState& IntentState)
{
	if (IntentState.Definition == nullptr)
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

bool PassesDynamicRequirements(const FFinalBattleEnemyState& EnemyState, const FFinalBattleEnemyIntentRuntimeState& IntentState, const int32 PreviewRound)
{
	if (IntentState.Definition == nullptr)
	{
		return false;
	}

	if (PreviewRound < FMath::Max(IntentState.Definition->MinPreviewRound, 1))
	{
		return false;
	}

	if (IntentState.Definition->MaxPreviewRound > 0 && PreviewRound > IntentState.Definition->MaxPreviewRound)
	{
		return false;
	}

	const float CurrentHpPercent = GetEnemyHpPercent(EnemyState);
	if (CurrentHpPercent < IntentState.Definition->MinEnemyHpPercent || CurrentHpPercent > IntentState.Definition->MaxEnemyHpPercent)
	{
		return false;
	}

	if (IntentState.Definition->bDisallowRepeatLastIntent && EnemyState.LastExecutedIntentId == IntentState.IntentId)
	{
		return false;
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

	const float CurrentHpPercent = GetEnemyHpPercent(EnemyState);
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

bool IsIntentEligible(
	const FFinalBattleEnemyState& EnemyState,
	const FFinalBattleEnemyIntentRuntimeState& IntentState,
	const int32 PreviewRound,
	const bool bRespectCooldown,
	const bool bRespectDynamicRequirements,
	const bool bRespectPhaseRequirements)
{
	if (!PassesDefinitionRequirements(EnemyState, IntentState))
	{
		return false;
	}

	if (bRespectPhaseRequirements && !PassesPhaseRequirements(EnemyState, IntentState))
	{
		return false;
	}

	if (bRespectDynamicRequirements && !PassesDynamicRequirements(EnemyState, IntentState, PreviewRound))
	{
		return false;
	}

	if (bRespectCooldown && PreviewRound < IntentState.NextAvailableRound)
	{
		return false;
	}

	return true;
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
	EnemyState.LastSelectedIntentId = EnemyState.CurrentIntentId;
	EnemyState.CurrentIntentText = EnemyState.CurrentIntentDefinition != nullptr && !EnemyState.CurrentIntentDefinition->PreviewText.IsEmpty()
		? EnemyState.CurrentIntentDefinition->PreviewText
		: (EnemyState.CurrentIntentDefinition != nullptr
			? EnemyState.CurrentIntentDefinition->DisplayName
			: FText::Format(
				NSLOCTEXT("FinalEnemyIntentService", "FallbackEnemyIntent", "Attack {0}"),
				FText::AsNumber(EnemyState.RuntimeDamagePower)));
}

int32 SelectFallbackIndex(const FFinalBattleEnemyState& EnemyState, const int32 PreviewRound)
{
	for (int32 Index = 0; Index < EnemyState.IntentRuntimeStates.Num(); ++Index)
	{
		if (IsIntentEligible(
			EnemyState,
			EnemyState.IntentRuntimeStates[Index],
			PreviewRound,
			false,
			true,
			true))
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

int32 SelectCycleIndex(
	const FFinalBattleEnemyState& EnemyState,
	const int32 PreviewRound,
	const bool bRespectPhaseRequirements = true)
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
		if (IsIntentEligible(EnemyState, EnemyState.IntentRuntimeStates[CandidateIndex], PreviewRound, true, true, bRespectPhaseRequirements))
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
		if (!IsIntentEligible(EnemyState, IntentState, PreviewRound, true, true, true))
		{
			continue;
		}

		const int32 IntentWeight = FMath::Max(IntentState.Definition->Weight, 0);
		if (IntentWeight <= 0)
		{
			continue;
		}

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

int32 FindIntentIndexById(const FFinalBattleEnemyState& EnemyState, const FName IntentId)
{
	if (IntentId == NAME_None)
	{
		return INDEX_NONE;
	}

	for (int32 Index = 0; Index < EnemyState.IntentRuntimeStates.Num(); ++Index)
	{
		if (EnemyState.IntentRuntimeStates[Index].IntentId == IntentId)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

int32 SelectPhaseSequenceIndex(const FFinalBattleEnemyState& EnemyState, const int32 PreviewRound)
{
	if (EnemyState.CurrentPhaseTag != NAME_None)
	{
		const int32 StartIndex = EnemyState.CurrentIntentIndex == INDEX_NONE
			? 0
			: (EnemyState.CurrentIntentIndex + 1) % EnemyState.IntentRuntimeStates.Num();

		for (int32 Offset = 0; Offset < EnemyState.IntentRuntimeStates.Num(); ++Offset)
		{
			const int32 CandidateIndex = (StartIndex + Offset) % EnemyState.IntentRuntimeStates.Num();
			const FFinalBattleEnemyIntentRuntimeState& IntentState = EnemyState.IntentRuntimeStates[CandidateIndex];
			if (IntentState.Definition == nullptr || !IntentState.Definition->PhaseTags.Contains(EnemyState.CurrentPhaseTag))
			{
				continue;
			}

			if (IsIntentEligible(EnemyState, IntentState, PreviewRound, true, true, true))
			{
				return CandidateIndex;
			}
		}
	}

	return SelectCycleIndex(EnemyState, PreviewRound, false);
}

int32 SelectScriptedIndex(const FFinalBattleEnemyState& EnemyState, const int32 PreviewRound)
{
	if (EnemyState.ScriptedIntentSequence.Num() == 0)
	{
		return INDEX_NONE;
	}

	const int32 StepIndex = EnemyState.IntentExecutionCount;
	const FFinalBattleEnemyScriptedIntentRuntimeStep* ScriptedStep = nullptr;
	if (EnemyState.ScriptedIntentSequence.IsValidIndex(StepIndex))
	{
		ScriptedStep = &EnemyState.ScriptedIntentSequence[StepIndex];
	}
	else
	{
		const FFinalBattleEnemyScriptedIntentRuntimeStep& LastStep = EnemyState.ScriptedIntentSequence.Last();
		if (LastStep.bRepeatLastStep)
		{
			ScriptedStep = &LastStep;
		}
	}

	if (ScriptedStep == nullptr)
	{
		return INDEX_NONE;
	}

	if (ScriptedStep->PhaseTag != NAME_None && ScriptedStep->PhaseTag != EnemyState.CurrentPhaseTag)
	{
		return INDEX_NONE;
	}

	const int32 IntentIndex = FindIntentIndexById(EnemyState, ScriptedStep->IntentId);
	if (!EnemyState.IntentRuntimeStates.IsValidIndex(IntentIndex))
	{
		return INDEX_NONE;
	}

	return IsIntentEligible(EnemyState, EnemyState.IntentRuntimeStates[IntentIndex], PreviewRound, true, true, true)
		? IntentIndex
		: INDEX_NONE;
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
		SelectedIndex = SelectCycleIndex(EnemyState, PreviewRound);
		break;

	case EFinalIntentSelectRule::PhaseSequence:
		SelectedIndex = SelectPhaseSequenceIndex(EnemyState, PreviewRound);
		break;

	case EFinalIntentSelectRule::Scripted:
		SelectedIndex = SelectScriptedIndex(EnemyState, PreviewRound);
		break;

	default:
		SelectedIndex = SelectCycleIndex(EnemyState, PreviewRound);
		break;
	}

	if (SelectedIndex == INDEX_NONE)
	{
		SelectedIndex = SelectFallbackIndex(EnemyState, PreviewRound);
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

	if (EnemyState.LastExecutedIntentId == IntentState.IntentId)
	{
		++EnemyState.ConsecutiveIntentUseCount;
	}
	else
	{
		EnemyState.ConsecutiveIntentUseCount = 1;
	}

	EnemyState.LastExecutedIntentId = IntentState.IntentId;
	++EnemyState.IntentExecutionCount;
}
