#include "Systems/FinalBattleStatusService.h"

#include "Battle/Definitions/FinalStatusDefinition.h"
#include "Queries/FinalBattleQueryTypes.h"
#include "Runtime/FinalBattleCharacterState.h"
#include "Runtime/FinalBattleState.h"
#include "Runtime/FinalBattleStatusInstance.h"

namespace
{
const FName TeamPlayerUnitId(TEXT("team_player"));
}

void FFinalBattleStatusService::TickStatusWindows(FFinalBattleState& BattleState) const
{
	// Status timing windows are still intentionally minimal at this stage.
	// The service exists to keep tick ownership out of the main resolver.
	(void)BattleState;
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
	const int32 BaseDuration = DurationOverride > 0
		? DurationOverride
		: (StatusDefinition ? StatusDefinition->DefaultDuration : 0);

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
	if (BaseDuration > 0)
	{
		ExistingInstance->RemainingDuration = FMath::Max(ExistingInstance->RemainingDuration, BaseDuration);
	}
	return FMath::Max(ExistingInstance->CurrentStacks - PreviousStacks, 0);
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
