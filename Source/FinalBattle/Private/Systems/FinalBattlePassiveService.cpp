#include "Systems/FinalBattlePassiveService.h"

#include "Battle/Definitions/FinalPassiveDefinition.h"
#include "Queries/FinalBattleQueryTypes.h"
#include "Runtime/FinalBattlePassiveInstance.h"
#include "Runtime/FinalBattleState.h"

namespace
{
int32 ResolveInitialRemainingDuration(
	const UFinalPassiveDefinition* PassiveDefinition,
	const int32 DurationOverride)
{
	if (PassiveDefinition == nullptr)
	{
		return 0;
	}

	if (PassiveDefinition->DurationType == EFinalPassiveDurationType::Battle)
	{
		return 0;
	}

	return DurationOverride > 0 ? DurationOverride : 1;
}

}

int32 FFinalBattlePassiveService::ApplyPassive(
	FFinalBattleState& BattleState,
	const FName OwnerUnitId,
	const FName SourceUnitId,
	const FFinalPassiveId& PassiveId,
	const UFinalPassiveDefinition* PassiveDefinition,
	const int32 StacksToAdd,
	const int32 DurationOverride) const
{
	if (OwnerUnitId.IsNone() || !PassiveId.IsValid() || PassiveDefinition == nullptr || StacksToAdd <= 0)
	{
		return 0;
	}

	FFinalBattlePassiveInstance* ExistingInstance = FindPassiveInstance(BattleState, OwnerUnitId, PassiveId);
	if (PassiveDefinition->StackPolicy == EFinalPassiveStackPolicy::RefreshExisting && ExistingInstance != nullptr)
	{
		const int32 PreviousStacks = ExistingInstance->CurrentStacks;
		ExistingInstance->CurrentStacks = PassiveDefinition->MaxStacks > 0
			? FMath::Min(ExistingInstance->CurrentStacks + StacksToAdd, PassiveDefinition->MaxStacks)
			: ExistingInstance->CurrentStacks + StacksToAdd;
		ExistingInstance->SourceUnitId = SourceUnitId;
		ExistingInstance->DisplayId = PassiveDefinition->DisplayId;
		ExistingInstance->DisplayName = PassiveDefinition->DisplayName;
		ExistingInstance->DurationType = PassiveDefinition->DurationType;
		ExistingInstance->RemainingDuration = FMath::Max(
			ExistingInstance->RemainingDuration,
			ResolveInitialRemainingDuration(PassiveDefinition, DurationOverride));
		return FMath::Max(ExistingInstance->CurrentStacks - PreviousStacks, 0);
	}

	FFinalBattlePassiveInstance& PassiveInstance = BattleState.PassiveInstances.AddDefaulted_GetRef();
	PassiveInstance.PassiveInstanceId = FGuid::NewGuid();
	PassiveInstance.PassiveId = PassiveId;
	PassiveInstance.DisplayId = PassiveDefinition->DisplayId;
	PassiveInstance.DisplayName = PassiveDefinition->DisplayName;
	PassiveInstance.OwnerUnitId = OwnerUnitId;
	PassiveInstance.SourceUnitId = SourceUnitId;
	PassiveInstance.CurrentStacks = PassiveDefinition->MaxStacks > 0
		? FMath::Min(StacksToAdd, PassiveDefinition->MaxStacks)
		: StacksToAdd;
	PassiveInstance.DurationType = PassiveDefinition->DurationType;
	PassiveInstance.RemainingDuration = ResolveInitialRemainingDuration(PassiveDefinition, DurationOverride);
	PassiveInstance.AppliedSequence = BattleState.LastEventSequence + BattleState.PassiveInstances.Num();
	for (const FFinalRuntimeTriggerDefinition& TriggerDefinition : PassiveDefinition->RuntimeTriggers)
	{
		if (TriggerDefinition.Domain != EFinalRuntimeTriggerDomain::Battle
			|| TriggerDefinition.Window == EFinalRuntimeTriggerWindow::None
			|| TriggerDefinition.Effects.IsEmpty())
		{
			continue;
		}

		FFinalBattleRuntimeTriggerState& TriggerState = PassiveInstance.TriggerStates.AddDefaulted_GetRef();
		TriggerState.TriggerDefinition = TriggerDefinition;
	}

	return PassiveInstance.CurrentStacks;
}

void FFinalBattlePassiveService::ResetPlayerTurnTriggerCounts(FFinalBattleState& BattleState) const
{
	for (FFinalBattlePassiveInstance& PassiveInstance : BattleState.PassiveInstances)
	{
		for (FFinalBattleRuntimeTriggerState& TriggerState : PassiveInstance.TriggerStates)
		{
			if (TriggerState.TriggerDefinition.Limit == EFinalRuntimeTriggerLimit::OncePerPlayerTurn)
			{
				TriggerState.TriggeredCountThisPlayerTurn = 0;
			}
		}
	}
}

void FFinalBattlePassiveService::ResolvePlayerTurnEndPassives(FFinalBattleState& BattleState) const
{
	for (int32 PassiveIndex = BattleState.PassiveInstances.Num() - 1; PassiveIndex >= 0; --PassiveIndex)
	{
		FFinalBattlePassiveInstance& PassiveInstance = BattleState.PassiveInstances[PassiveIndex];
		if (PassiveInstance.DurationType == EFinalPassiveDurationType::PlayerTurns)
		{
			PassiveInstance.RemainingDuration = FMath::Max(PassiveInstance.RemainingDuration - 1, 0);
			if (PassiveInstance.RemainingDuration <= 0)
			{
				BattleState.PassiveInstances.RemoveAt(PassiveIndex);
			}
		}
	}
}

void FFinalBattlePassiveService::BuildPassiveSnapshotData(
	const FFinalBattleState& BattleState,
	TArray<FFinalBattlePassiveViewData>& OutPassives) const
{
	for (const FFinalBattlePassiveInstance& PassiveInstance : BattleState.PassiveInstances)
	{
		FFinalBattlePassiveViewData PassiveView;
		PassiveView.PassiveInstanceId = PassiveInstance.PassiveInstanceId;
		PassiveView.PassiveId = PassiveInstance.PassiveId;
		PassiveView.DisplayId = PassiveInstance.DisplayId;
		PassiveView.DisplayName = PassiveInstance.DisplayName.IsEmpty()
			? FText::FromName(PassiveInstance.PassiveId.Value)
			: PassiveInstance.DisplayName;
		PassiveView.OwnerUnitId = PassiveInstance.OwnerUnitId;
		PassiveView.SourceUnitId = PassiveInstance.SourceUnitId;
		PassiveView.CurrentStacks = PassiveInstance.CurrentStacks;
		PassiveView.RemainingDuration = PassiveInstance.RemainingDuration;
		PassiveView.DurationType = PassiveInstance.DurationType;
		OutPassives.Add(MoveTemp(PassiveView));
	}
}

const FFinalBattlePassiveInstance* FFinalBattlePassiveService::FindPassiveInstance(
	const FFinalBattleState& BattleState,
	const FName OwnerUnitId,
	const FFinalPassiveId& PassiveId) const
{
	return BattleState.PassiveInstances.FindByPredicate(
		[&OwnerUnitId, &PassiveId](const FFinalBattlePassiveInstance& Candidate)
		{
			return Candidate.OwnerUnitId == OwnerUnitId && Candidate.PassiveId == PassiveId;
		});
}

FFinalBattlePassiveInstance* FFinalBattlePassiveService::FindPassiveInstance(
	FFinalBattleState& BattleState,
	const FName OwnerUnitId,
	const FFinalPassiveId& PassiveId) const
{
	return BattleState.PassiveInstances.FindByPredicate(
		[&OwnerUnitId, &PassiveId](const FFinalBattlePassiveInstance& Candidate)
		{
			return Candidate.OwnerUnitId == OwnerUnitId && Candidate.PassiveId == PassiveId;
		});
}
