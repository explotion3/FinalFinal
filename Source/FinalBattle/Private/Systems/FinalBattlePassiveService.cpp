#include "Systems/FinalBattlePassiveService.h"

#include "Battle/Definitions/FinalPassiveDefinition.h"
#include "Queries/FinalBattleQueryTypes.h"
#include "Runtime/FinalBattleCharacterState.h"
#include "Runtime/FinalBattleEnemyState.h"
#include "Runtime/FinalBattlePassiveInstance.h"
#include "Runtime/FinalBattleState.h"

namespace
{
const FName TeamPlayerUnitId(TEXT("team_player"));
const FName PassiveRemovedExpiredReasonTag(TEXT("passive.removed.expired"));

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

bool IsPlayerOwnedPassive(const FFinalBattleState& BattleState, const FName OwnerUnitId)
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

bool IsEnemyOwnedPassive(const FFinalBattleState& BattleState, const FName OwnerUnitId)
{
	return BattleState.Enemies.ContainsByPredicate(
		[&OwnerUnitId](const FFinalBattleEnemyState& Candidate)
		{
			return Candidate.RuntimeUnitId == OwnerUnitId;
		});
}

bool IsPassiveOwnerAllowedByAppliesTo(
	const FFinalBattleState& BattleState,
	const FName OwnerUnitId,
	const EFinalPassiveAppliesTo AppliesTo)
{
	if (OwnerUnitId.IsNone())
	{
		return false;
	}

	const bool bIsPlayerOwned = IsPlayerOwnedPassive(BattleState, OwnerUnitId);
	const bool bIsEnemyOwned = IsEnemyOwnedPassive(BattleState, OwnerUnitId);
	if (!bIsPlayerOwned && !bIsEnemyOwned)
	{
		return false;
	}

	switch (AppliesTo)
	{
	case EFinalPassiveAppliesTo::Shared:
		return true;
	case EFinalPassiveAppliesTo::PlayerOnly:
		return bIsPlayerOwned;
	case EFinalPassiveAppliesTo::EnemyOnly:
		return bIsEnemyOwned;
	default:
		return false;
	}
}

}

FFinalBattlePassiveApplyResult FFinalBattlePassiveService::ApplyPassive(
	FFinalBattleState& BattleState,
	const FName OwnerUnitId,
	const FName SourceUnitId,
	const FFinalPassiveId& PassiveId,
	const UFinalPassiveDefinition* PassiveDefinition,
	const int32 StacksToAdd,
	const int32 DurationOverride) const
{
	FFinalBattlePassiveApplyResult Result;
	if (OwnerUnitId.IsNone() || !PassiveId.IsValid() || PassiveDefinition == nullptr || StacksToAdd <= 0)
	{
		return Result;
	}

	if (!IsPassiveOwnerAllowedByAppliesTo(BattleState, OwnerUnitId, PassiveDefinition->AppliesTo))
	{
		return Result;
	}

	FFinalBattlePassiveInstance* ExistingInstance = FindPassiveInstance(BattleState, OwnerUnitId, PassiveId);
	if (PassiveDefinition->StackPolicy == EFinalPassiveStackPolicy::RefreshExisting && ExistingInstance != nullptr)
	{
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
		Result.bApplied = true;
		Result.bCreatedNewInstance = false;
		Result.PassiveInstanceId = ExistingInstance->PassiveInstanceId;
		Result.PassiveId = ExistingInstance->PassiveId;
		Result.DisplayName = ExistingInstance->DisplayName.IsEmpty() ? FText::FromName(PassiveId.Value) : ExistingInstance->DisplayName;
		Result.OwnerUnitId = ExistingInstance->OwnerUnitId;
		Result.SourceUnitId = ExistingInstance->SourceUnitId;
		Result.CurrentStacks = ExistingInstance->CurrentStacks;
		Result.RemainingDuration = ExistingInstance->RemainingDuration;
		return Result;
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
			|| (TriggerDefinition.Effects.IsEmpty() && TriggerDefinition.TriggeredCardModifiers.IsEmpty()))
		{
			continue;
		}

		FFinalBattleRuntimeTriggerState& TriggerState = PassiveInstance.TriggerStates.AddDefaulted_GetRef();
		TriggerState.TriggerDefinition = TriggerDefinition;
	}

	Result.bApplied = true;
	Result.bCreatedNewInstance = true;
	Result.PassiveInstanceId = PassiveInstance.PassiveInstanceId;
	Result.PassiveId = PassiveInstance.PassiveId;
	Result.DisplayName = PassiveInstance.DisplayName.IsEmpty() ? FText::FromName(PassiveId.Value) : PassiveInstance.DisplayName;
	Result.OwnerUnitId = PassiveInstance.OwnerUnitId;
	Result.SourceUnitId = PassiveInstance.SourceUnitId;
	Result.CurrentStacks = PassiveInstance.CurrentStacks;
	Result.RemainingDuration = PassiveInstance.RemainingDuration;
	return Result;
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

TArray<FFinalBattlePassiveRemovalResult> FFinalBattlePassiveService::ResolvePlayerTurnEndPassives(FFinalBattleState& BattleState) const
{
	TArray<FFinalBattlePassiveRemovalResult> RemovalResults;
	for (int32 PassiveIndex = BattleState.PassiveInstances.Num() - 1; PassiveIndex >= 0; --PassiveIndex)
	{
		FFinalBattlePassiveInstance& PassiveInstance = BattleState.PassiveInstances[PassiveIndex];
		if (PassiveInstance.DurationType == EFinalPassiveDurationType::PlayerTurns)
		{
			const int32 RemainingDurationBeforeTick = PassiveInstance.RemainingDuration;
			PassiveInstance.RemainingDuration = FMath::Max(PassiveInstance.RemainingDuration - 1, 0);
			if (PassiveInstance.RemainingDuration <= 0)
			{
				FFinalBattlePassiveRemovalResult& RemovalResult = RemovalResults.AddDefaulted_GetRef();
				RemovalResult.PassiveInstanceId = PassiveInstance.PassiveInstanceId;
				RemovalResult.PassiveId = PassiveInstance.PassiveId;
				RemovalResult.DisplayName = PassiveInstance.DisplayName.IsEmpty()
					? FText::FromName(PassiveInstance.PassiveId.Value)
					: PassiveInstance.DisplayName;
				RemovalResult.OwnerUnitId = PassiveInstance.OwnerUnitId;
				RemovalResult.SourceUnitId = PassiveInstance.SourceUnitId;
				RemovalResult.CurrentStacksBeforeRemoval = PassiveInstance.CurrentStacks;
				RemovalResult.RemainingDurationBeforeRemoval = RemainingDurationBeforeTick;
				RemovalResult.RemovalReasonTag = PassiveRemovedExpiredReasonTag;
				BattleState.PassiveInstances.RemoveAt(PassiveIndex);
			}
		}
	}

	return RemovalResults;
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
