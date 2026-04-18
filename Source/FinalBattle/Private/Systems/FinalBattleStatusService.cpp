#include "Systems/FinalBattleStatusService.h"

#include "Battle/Definitions/FinalStatusDefinition.h"
#include "Queries/FinalBattleQueryTypes.h"
#include "Runtime/FinalBattleCharacterState.h"
#include "Runtime/FinalBattleState.h"
#include "Runtime/FinalBattleStatusInstance.h"

namespace
{
const FName TeamPlayerUnitId(TEXT("team_player"));

bool IsPlayerOwnedStatus(const FFinalBattleState& BattleState, const FName OwnerUnitId)
{
	if (OwnerUnitId == TeamPlayerUnitId)
	{
		return true;
	}

	return BattleState.Characters.ContainsByPredicate(
		[&OwnerUnitId](const FFinalBattleCharacterState& Candidate)
		{
			return Candidate.RuntimeUnitId == OwnerUnitId;
		});
}

bool IsOutgoingDamageModifierApplicable(const FFinalBattleStatusInstance& StatusInstance, const bool bIsAttackCardDamage)
{
	if (StatusInstance.OutgoingDamagePercentPerStack == 0)
	{
		return false;
	}

	if (StatusInstance.bOnlyAffectAttackCards && !bIsAttackCardDamage)
	{
		return false;
	}

	return true;
}
}

void FFinalBattleStatusService::ResolvePlayerTurnEndStatuses(FFinalBattleState& BattleState) const
{
	for (int32 StatusIndex = BattleState.StatusInstances.Num() - 1; StatusIndex >= 0; --StatusIndex)
	{
		FFinalBattleStatusInstance& StatusInstance = BattleState.StatusInstances[StatusIndex];
		if (!StatusInstance.bExpireAtPlayerTurnEnd || !IsPlayerOwnedStatus(BattleState, StatusInstance.OwnerUnitId))
		{
			continue;
		}

		StatusInstance.RemainingDuration = FMath::Max(StatusInstance.RemainingDuration - 1, 0);
		if (StatusInstance.RemainingDuration <= 0)
		{
			BattleState.StatusInstances.RemoveAt(StatusIndex);
		}
	}
}

int32 FFinalBattleStatusService::AddStatusStacks(
	FFinalBattleState& BattleState,
	const FName OwnerUnitId,
	const FName SourceUnitId,
	const FFinalStatusId& StatusId,
	const UFinalStatusDefinition* StatusDefinition,
	const int32 StacksToAdd,
	const int32 DurationOverride) const
{
	if (OwnerUnitId.IsNone() || !StatusId.IsValid() || StacksToAdd <= 0)
	{
		return 0;
	}

	FFinalBattleStatusInstance* ExistingInstance = FindStatusInstance(BattleState, OwnerUnitId, StatusId);
	const int32 MaxStacks = StatusDefinition ? StatusDefinition->MaxStacks : 0;
	int32 BaseDuration = DurationOverride > 0
		? DurationOverride
		: (StatusDefinition ? StatusDefinition->DefaultDuration : 0);
	if (StatusDefinition != nullptr && StatusDefinition->bExpireAtPlayerTurnEnd && BaseDuration <= 0)
	{
		BaseDuration = 1;
	}

	if (ExistingInstance == nullptr)
	{
		FFinalBattleStatusInstance& NewInstance = BattleState.StatusInstances.AddDefaulted_GetRef();
		NewInstance.StatusInstanceId = FGuid::NewGuid();
		NewInstance.StatusId = StatusId;
		NewInstance.OwnerUnitId = OwnerUnitId;
		NewInstance.SourceUnitId = SourceUnitId;
		NewInstance.DisplayName = StatusDefinition && !StatusDefinition->DisplayName.IsEmpty()
			? StatusDefinition->DisplayName
			: FText::FromName(StatusId.Value);
		NewInstance.CurrentStacks = MaxStacks > 0 ? FMath::Min(StacksToAdd, MaxStacks) : StacksToAdd;
		NewInstance.RemainingDuration = BaseDuration;
		NewInstance.OutgoingDamagePercentPerStack = StatusDefinition ? StatusDefinition->OutgoingDamagePercentPerStack : 0;
		NewInstance.bExpireAtPlayerTurnEnd = StatusDefinition ? StatusDefinition->bExpireAtPlayerTurnEnd : false;
		NewInstance.bConsumeOnSuccessfulOwnerDamage = StatusDefinition ? StatusDefinition->bConsumeOnSuccessfulOwnerDamage : false;
		NewInstance.bOnlyAffectAttackCards = StatusDefinition ? StatusDefinition->bOnlyAffectAttackCards : false;
		return NewInstance.CurrentStacks;
	}

	const int32 PreviousStacks = ExistingInstance->CurrentStacks;
	ExistingInstance->CurrentStacks = MaxStacks > 0
		? FMath::Min(ExistingInstance->CurrentStacks + StacksToAdd, MaxStacks)
		: ExistingInstance->CurrentStacks + StacksToAdd;
	ExistingInstance->SourceUnitId = SourceUnitId;
	if (StatusDefinition && !StatusDefinition->DisplayName.IsEmpty())
	{
		ExistingInstance->DisplayName = StatusDefinition->DisplayName;
	}
	if (StatusDefinition != nullptr)
	{
		ExistingInstance->OutgoingDamagePercentPerStack = StatusDefinition->OutgoingDamagePercentPerStack;
		ExistingInstance->bExpireAtPlayerTurnEnd = StatusDefinition->bExpireAtPlayerTurnEnd;
		ExistingInstance->bConsumeOnSuccessfulOwnerDamage = StatusDefinition->bConsumeOnSuccessfulOwnerDamage;
		ExistingInstance->bOnlyAffectAttackCards = StatusDefinition->bOnlyAffectAttackCards;
	}
	if (BaseDuration > 0)
	{
		ExistingInstance->RemainingDuration = FMath::Max(ExistingInstance->RemainingDuration, BaseDuration);
	}
	return FMath::Max(ExistingInstance->CurrentStacks - PreviousStacks, 0);
}

int32 FFinalBattleStatusService::GetOutgoingDamageModifierPercent(
	const FFinalBattleState& BattleState,
	const FName OwnerUnitId,
	const bool bIsAttackCardDamage) const
{
	int32 TotalModifierPercent = 0;

	for (const FFinalBattleStatusInstance& StatusInstance : BattleState.StatusInstances)
	{
		if (StatusInstance.OwnerUnitId != OwnerUnitId
			|| !IsOutgoingDamageModifierApplicable(StatusInstance, bIsAttackCardDamage))
		{
			continue;
		}

		TotalModifierPercent += StatusInstance.OutgoingDamagePercentPerStack * StatusInstance.CurrentStacks;
	}

	return TotalModifierPercent;
}

int32 FFinalBattleStatusService::ConsumeOutgoingDamageModifierStacks(
	FFinalBattleState& BattleState,
	const FName OwnerUnitId,
	const bool bIsAttackCardDamage) const
{
	int32 TotalRemovedStacks = 0;

	for (int32 StatusIndex = BattleState.StatusInstances.Num() - 1; StatusIndex >= 0; --StatusIndex)
	{
		FFinalBattleStatusInstance& StatusInstance = BattleState.StatusInstances[StatusIndex];
		if (StatusInstance.OwnerUnitId != OwnerUnitId
			|| !StatusInstance.bConsumeOnSuccessfulOwnerDamage
			|| !IsOutgoingDamageModifierApplicable(StatusInstance, bIsAttackCardDamage))
		{
			continue;
		}

		StatusInstance.CurrentStacks = FMath::Max(StatusInstance.CurrentStacks - 1, 0);
		++TotalRemovedStacks;
		if (StatusInstance.CurrentStacks <= 0)
		{
			BattleState.StatusInstances.RemoveAt(StatusIndex);
		}
	}

	return TotalRemovedStacks;
}

int32 FFinalBattleStatusService::RemoveStatusStacks(
	FFinalBattleState& BattleState,
	const FName OwnerUnitId,
	const FFinalStatusId& StatusId,
	const int32 StacksToRemove) const
{
	if (OwnerUnitId.IsNone() || !StatusId.IsValid() || StacksToRemove <= 0)
	{
		return 0;
	}

	const int32 StatusIndex = BattleState.StatusInstances.IndexOfByPredicate(
		[&OwnerUnitId, &StatusId](const FFinalBattleStatusInstance& Candidate)
		{
			return Candidate.OwnerUnitId == OwnerUnitId && Candidate.StatusId == StatusId;
		});
	if (StatusIndex == INDEX_NONE)
	{
		return 0;
	}

	FFinalBattleStatusInstance& StatusInstance = BattleState.StatusInstances[StatusIndex];
	const int32 RemovedStacks = FMath::Min(StatusInstance.CurrentStacks, StacksToRemove);
	StatusInstance.CurrentStacks = FMath::Max(StatusInstance.CurrentStacks - RemovedStacks, 0);
	if (StatusInstance.CurrentStacks <= 0)
	{
		BattleState.StatusInstances.RemoveAt(StatusIndex);
	}
	return RemovedStacks;
}

int32 FFinalBattleStatusService::GetStatusStacks(
	const FFinalBattleState& BattleState,
	const FName OwnerUnitId,
	const FFinalStatusId& StatusId) const
{
	if (const FFinalBattleStatusInstance* StatusInstance = FindStatusInstance(BattleState, OwnerUnitId, StatusId))
	{
		return StatusInstance->CurrentStacks;
	}

	return 0;
}

const FFinalBattleStatusInstance* FFinalBattleStatusService::FindStatusInstance(
	const FFinalBattleState& BattleState,
	const FName OwnerUnitId,
	const FFinalStatusId& StatusId) const
{
	return BattleState.StatusInstances.FindByPredicate(
		[&OwnerUnitId, &StatusId](const FFinalBattleStatusInstance& Candidate)
		{
			return Candidate.OwnerUnitId == OwnerUnitId && Candidate.StatusId == StatusId;
		});
}

FFinalBattleStatusInstance* FFinalBattleStatusService::FindStatusInstance(
	FFinalBattleState& BattleState,
	const FName OwnerUnitId,
	const FFinalStatusId& StatusId) const
{
	return BattleState.StatusInstances.FindByPredicate(
		[&OwnerUnitId, &StatusId](const FFinalBattleStatusInstance& Candidate)
		{
			return Candidate.OwnerUnitId == OwnerUnitId && Candidate.StatusId == StatusId;
		});
}

void FFinalBattleStatusService::BuildStatusSnapshotData(
	const FFinalBattleState& BattleState,
	TArray<FFinalBattleCharacterStatusesViewData>& OutCharacterStatuses,
	TArray<FFinalBattleStatusViewData>& OutTeamStatuses,
	TArray<FFinalBattleStatusViewData>& OutStatuses) const
{
	for (const FFinalBattleCharacterState& CharacterState : BattleState.Characters)
	{
		FFinalBattleCharacterStatusesViewData CharacterStatusEntry;
		CharacterStatusEntry.OwnerUnitId = CharacterState.RuntimeUnitId;
		CharacterStatusEntry.CharacterId = CharacterState.CharacterId;
		OutCharacterStatuses.Add(MoveTemp(CharacterStatusEntry));
	}

	for (const FFinalBattleStatusInstance& StatusInstance : BattleState.StatusInstances)
	{
		FFinalBattleStatusViewData StatusView;
		StatusView.StatusInstanceId = StatusInstance.StatusInstanceId;
		StatusView.StatusId = StatusInstance.StatusId;
		StatusView.OwnerUnitId = StatusInstance.OwnerUnitId;
		StatusView.SourceUnitId = StatusInstance.SourceUnitId;
		StatusView.DisplayName = StatusInstance.DisplayName.IsEmpty()
			? FText::FromName(StatusInstance.StatusId.Value)
			: StatusInstance.DisplayName;
		StatusView.CurrentStacks = StatusInstance.CurrentStacks;
		StatusView.RemainingDuration = StatusInstance.RemainingDuration;

		if (StatusInstance.OwnerUnitId == TeamPlayerUnitId)
		{
			OutTeamStatuses.Add(StatusView);
		}
		else if (FFinalBattleCharacterStatusesViewData* CharacterStatuses = OutCharacterStatuses.FindByPredicate(
			[&StatusInstance](const FFinalBattleCharacterStatusesViewData& Candidate)
			{
				return Candidate.OwnerUnitId == StatusInstance.OwnerUnitId;
			}))
		{
			CharacterStatuses->StatusEntries.Add(StatusView);
		}

		OutStatuses.Add(MoveTemp(StatusView));
	}
}
