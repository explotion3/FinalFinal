#include "Systems/FinalBattleStatusService.h"

#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalStatusDefinition.h"
#include "Queries/FinalBattleQueryTypes.h"
#include "Runtime/FinalBattleCardInstance.h"
#include "Runtime/FinalBattleCharacterState.h"
#include "Runtime/FinalBattleEnemyState.h"
#include "Runtime/FinalBattleState.h"
#include "Runtime/FinalBattleStatusInstance.h"
#include "Systems/FinalBattleCardService.h"
#include "Systems/FinalBattleEffectExecutionService.h"
#include "Systems/FinalBattleTriggerService.h"
#include "Systems/FinalBattleUnitService.h"

namespace
{
const FName TeamPlayerUnitId(TEXT("team_player"));
const TCHAR* DerivedStatusHandProjectionPrefix = TEXT("status.hand_projection.");

bool UsesStructuredRuntimeModifiers(const FFinalBattleStatusInstance& StatusInstance)
{
	return StatusInstance.RuntimeModifiers.Num() > 0;
}

bool UsesStructuredProjectedCardModifiers(const FFinalBattleStatusInstance& StatusInstance)
{
	return StatusInstance.ProjectedCardModifiers.Num() > 0;
}

bool IsResourceStatus(const FFinalBattleStatusInstance& StatusInstance)
{
	return StatusInstance.bIsResourceStatus
		&& StatusInstance.ResourceBehavior == EFinalStatusResourceBehavior::AccumulateAndConsume;
}

bool IsResourceStatus(const UFinalStatusDefinition* StatusDefinition)
{
	return StatusDefinition != nullptr
		&& StatusDefinition->bIsResourceStatus
		&& StatusDefinition->ResourceBehavior == EFinalStatusResourceBehavior::AccumulateAndConsume;
}

bool IsDamageOverTimeStatus(const FFinalBattleStatusInstance& StatusInstance)
{
	return StatusInstance.bIsDamageOverTime
		&& StatusInstance.DamageOverTimeTickWindow != EFinalStatusDamageOverTimeTickWindow::None
		&& StatusInstance.DamageOverTimeAttackPowerPercentPerStack > 0;
}

bool IsDamageOverTimeStatus(const UFinalStatusDefinition* StatusDefinition)
{
	return StatusDefinition != nullptr
		&& StatusDefinition->bIsDamageOverTime
		&& StatusDefinition->DamageOverTimeTickWindow != EFinalStatusDamageOverTimeTickWindow::None
		&& StatusDefinition->DamageOverTimeAttackPowerPercentPerStack > 0;
}

bool ShouldExpireAtPlayerTurnEnd(const UFinalStatusDefinition* StatusDefinition)
{
	if (StatusDefinition == nullptr)
	{
		return false;
	}

	return StatusDefinition->DurationType == EFinalStatusDurationType::PlayerTurns
		&& StatusDefinition->ExpireWindow == EFinalStatusExpireWindow::PlayerTurnEnd;
}

void BuildStructuredRuntimeModifiers(
	const UFinalStatusDefinition* StatusDefinition,
	TArray<FFinalBattleStatusRuntimeModifierInstance>& OutRuntimeModifiers)
{
	OutRuntimeModifiers.Reset();
	if (StatusDefinition == nullptr)
	{
		return;
	}

	for (const FFinalStatusRuntimeModifierDefinition& ModifierDefinition : StatusDefinition->RuntimeModifiers)
	{
		if (ModifierDefinition.OutgoingDamagePercentPerStack == 0
			&& ModifierDefinition.IncomingDamagePercentPerStack == 0
			&& ModifierDefinition.IncomingTeamHealthDamageReductionPercentPerStack == 0
			&& !ModifierDefinition.bOnlyAffectAttackCards)
		{
			continue;
		}

		FFinalBattleStatusRuntimeModifierInstance& ModifierInstance = OutRuntimeModifiers.AddDefaulted_GetRef();
		ModifierInstance.OutgoingDamagePercentPerStack = ModifierDefinition.OutgoingDamagePercentPerStack;
		ModifierInstance.IncomingDamagePercentPerStack = ModifierDefinition.IncomingDamagePercentPerStack;
		ModifierInstance.bOnlyAffectAttackCards = ModifierDefinition.bOnlyAffectAttackCards;
		ModifierInstance.IncomingTeamHealthDamageReductionPercentPerStack = ModifierDefinition.IncomingTeamHealthDamageReductionPercentPerStack;
	}
}

void BuildStructuredProjectedCardModifiers(
	const UFinalStatusDefinition* StatusDefinition,
	TArray<FFinalBattleStatusProjectedCardModifierInstance>& OutProjectedCardModifiers)
{
	OutProjectedCardModifiers.Reset();
	if (StatusDefinition == nullptr)
	{
		return;
	}

	for (const FFinalStatusProjectedCardModifierDefinition& ModifierDefinition : StatusDefinition->ProjectedCardModifiers)
	{
		if (ModifierDefinition.TargetSource == EFinalTriggeredCardModifierTargetSource::None)
		{
			continue;
		}

		if (ModifierDefinition.CostDeltaAPPerStack == 0
			&& ModifierDefinition.OutgoingDamagePercentPerStack == 0)
		{
			continue;
		}

		FFinalBattleStatusProjectedCardModifierInstance& ModifierInstance = OutProjectedCardModifiers.AddDefaulted_GetRef();
		ModifierInstance.TargetSource = ModifierDefinition.TargetSource;
		ModifierInstance.bRequireCardType = ModifierDefinition.bRequireCardType;
		ModifierInstance.RequiredCardType = ModifierDefinition.RequiredCardType;
		ModifierInstance.CostDeltaAPPerStack = ModifierDefinition.CostDeltaAPPerStack;
		ModifierInstance.OutgoingDamagePercentPerStack = ModifierDefinition.OutgoingDamagePercentPerStack;
		ModifierInstance.LifetimePolicy = ModifierDefinition.LifetimePolicy;
		ModifierInstance.bExpireAtPlayerTurnEnd = ModifierDefinition.bExpireAtPlayerTurnEnd;
	}
}

void BuildConsumptionRules(
	const UFinalStatusDefinition* StatusDefinition,
	TArray<FFinalBattleStatusConsumptionRuleInstance>& OutConsumptionRules)
{
	OutConsumptionRules.Reset();
	if (StatusDefinition == nullptr)
	{
		return;
	}

	for (const FFinalStatusConsumptionRuleDefinition& RuleDefinition : StatusDefinition->ConsumptionRules)
	{
		if (RuleDefinition.Window == EFinalStatusConsumptionWindow::None || RuleDefinition.StacksToConsume <= 0)
		{
			continue;
		}

		FFinalBattleStatusConsumptionRuleInstance& RuleInstance = OutConsumptionRules.AddDefaulted_GetRef();
		RuleInstance.Window = RuleDefinition.Window;
		RuleInstance.StacksToConsume = RuleDefinition.StacksToConsume;
		RuleInstance.bRequireAttackCardDamage = RuleDefinition.bRequireAttackCardDamage;
	}
}

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

bool IsEnemyOwnedStatus(const FFinalBattleState& BattleState, const FName OwnerUnitId)
{
	return BattleState.Enemies.ContainsByPredicate(
		[&OwnerUnitId](const FFinalBattleEnemyState& Candidate)
		{
			return Candidate.RuntimeUnitId == OwnerUnitId;
		});
}

bool IsStatusOwnerAllowedByAppliesTo(
	const FFinalBattleState& BattleState,
	const FName OwnerUnitId,
	const EFinalStatusAppliesTo AppliesTo)
{
	if (OwnerUnitId.IsNone())
	{
		return false;
	}

	const bool bIsPlayerOwned = IsPlayerOwnedStatus(BattleState, OwnerUnitId);
	const bool bIsEnemyOwned = IsEnemyOwnedStatus(BattleState, OwnerUnitId);
	if (!bIsPlayerOwned && !bIsEnemyOwned)
	{
		return false;
	}

	switch (AppliesTo)
	{
	case EFinalStatusAppliesTo::Shared:
		return true;
	case EFinalStatusAppliesTo::PlayerOnly:
		return bIsPlayerOwned;
	case EFinalStatusAppliesTo::EnemyOnly:
		return bIsEnemyOwned;
	default:
		return false;
	}
}

const FFinalBattleStatusInstance* FindStatusInstanceForAdd(
	const FFinalBattleState& BattleState,
	const FName OwnerUnitId,
	const FName SourceUnitId,
	const FFinalStatusId& StatusId,
	const UFinalStatusDefinition* StatusDefinition)
{
	const EFinalStatusStackKeyPolicy StackKeyPolicy = StatusDefinition != nullptr
		? StatusDefinition->StackKeyPolicy
		: EFinalStatusStackKeyPolicy::ByOwner;

	return BattleState.StatusInstances.FindByPredicate(
		[&OwnerUnitId, &SourceUnitId, &StatusId, StackKeyPolicy](const FFinalBattleStatusInstance& Candidate)
		{
			if (Candidate.OwnerUnitId != OwnerUnitId || Candidate.StatusId != StatusId)
			{
				return false;
			}

			switch (StackKeyPolicy)
			{
			case EFinalStatusStackKeyPolicy::ByOwner:
				return true;
			case EFinalStatusStackKeyPolicy::ByOwnerAndSource:
				return Candidate.SourceUnitId == SourceUnitId;
			case EFinalStatusStackKeyPolicy::SeparateInstances:
			default:
				return false;
			}
		});
}

FFinalBattleStatusInstance* FindStatusInstanceForAdd(
	FFinalBattleState& BattleState,
	const FName OwnerUnitId,
	const FName SourceUnitId,
	const FFinalStatusId& StatusId,
	const UFinalStatusDefinition* StatusDefinition)
{
	return BattleState.StatusInstances.FindByPredicate(
		[&OwnerUnitId, &SourceUnitId, &StatusId, StatusDefinition](const FFinalBattleStatusInstance& Candidate)
		{
			if (Candidate.OwnerUnitId != OwnerUnitId || Candidate.StatusId != StatusId)
			{
				return false;
			}

			const EFinalStatusStackKeyPolicy StackKeyPolicy = StatusDefinition != nullptr
				? StatusDefinition->StackKeyPolicy
				: EFinalStatusStackKeyPolicy::ByOwner;
			switch (StackKeyPolicy)
			{
			case EFinalStatusStackKeyPolicy::ByOwner:
				return true;
			case EFinalStatusStackKeyPolicy::ByOwnerAndSource:
				return Candidate.SourceUnitId == SourceUnitId;
			case EFinalStatusStackKeyPolicy::SeparateInstances:
			default:
				return false;
			}
		});
}

int32 ResolveSourceAttackPower(
	const FFinalBattleState& BattleState,
	const FFinalBattleUnitService& UnitService,
	const FName SourceUnitId)
{
	if (SourceUnitId.IsNone())
	{
		return 0;
	}

	if (const FFinalBattleCharacterState* SourceCharacter = UnitService.FindCharacterState(BattleState, SourceUnitId))
	{
		return SourceCharacter->RuntimeAttack;
	}

	if (const FFinalBattleEnemyState* SourceEnemy = UnitService.FindEnemyState(BattleState, SourceUnitId))
	{
		return SourceEnemy->RuntimeDamagePower;
	}

	return 0;
}

bool IsStructuredOutgoingDamageModifierApplicable(
	const FFinalBattleStatusRuntimeModifierInstance& RuntimeModifier,
	const bool bIsAttackCardDamage)
{
	if (RuntimeModifier.OutgoingDamagePercentPerStack == 0)
	{
		return false;
	}

	if (RuntimeModifier.bOnlyAffectAttackCards && !bIsAttackCardDamage)
	{
		return false;
	}

	return true;
}

bool IsOutgoingDamageModifierApplicable(const FFinalBattleStatusInstance& StatusInstance, const bool bIsAttackCardDamage)
{
	if (IsResourceStatus(StatusInstance) && !StatusInstance.bAutoAffectBattleRules)
	{
		return false;
	}

	if (!UsesStructuredRuntimeModifiers(StatusInstance))
	{
		return false;
	}

	for (const FFinalBattleStatusRuntimeModifierInstance& RuntimeModifier : StatusInstance.RuntimeModifiers)
	{
		if (IsStructuredOutgoingDamageModifierApplicable(RuntimeModifier, bIsAttackCardDamage))
		{
			return true;
		}
	}

	return false;
}

bool IsProjectedHandCardModifierApplicable(const FFinalBattleStatusInstance& StatusInstance, const bool bIsAttackCardDamage)
{
	if (IsResourceStatus(StatusInstance) && !StatusInstance.bAutoProjectToCards)
	{
		return false;
	}

	if (StatusInstance.CurrentStacks <= 0)
	{
		return false;
	}

	for (const FFinalBattleStatusProjectedCardModifierInstance& ModifierInstance : StatusInstance.ProjectedCardModifiers)
	{
		if (ModifierInstance.TargetSource != EFinalTriggeredCardModifierTargetSource::CurrentOwnedHandCards
			|| ModifierInstance.OutgoingDamagePercentPerStack == 0)
		{
			continue;
		}

		if (ModifierInstance.bRequireCardType
			&& ModifierInstance.RequiredCardType == EFinalCardType::Attack
			&& !bIsAttackCardDamage)
		{
			continue;
		}

		return true;
	}

	return false;
}

bool IsDerivedStatusHandProjectionModifier(const FFinalBattleCardModifierRecord& ModifierRecord)
{
	return ModifierRecord.SourceType == EFinalBattleCardModifierSourceType::Status
		&& ModifierRecord.ModifierId.ToString().StartsWith(DerivedStatusHandProjectionPrefix);
}

FName BuildDerivedStatusHandProjectionModifierId(const FGuid& StatusInstanceId, const FGuid& CardInstanceId)
{
	return FName(*FString::Printf(
		TEXT("%s%s_%s"),
		DerivedStatusHandProjectionPrefix,
		*StatusInstanceId.ToString(EGuidFormats::Digits),
		*CardInstanceId.ToString(EGuidFormats::Digits)));
}

bool IsIncomingTeamHealthDamageProtectionApplicable(const FFinalBattleStatusInstance& StatusInstance)
{
	if (IsResourceStatus(StatusInstance) && !StatusInstance.bAutoAffectBattleRules)
	{
		return false;
	}

	return StatusInstance.OwnerUnitId == TeamPlayerUnitId
		&& StatusInstance.CurrentStacks > 0
		&& StatusInstance.RuntimeModifiers.ContainsByPredicate(
			[](const FFinalBattleStatusRuntimeModifierInstance& RuntimeModifier)
			{
				return RuntimeModifier.IncomingTeamHealthDamageReductionPercentPerStack > 0;
			});
}
}

void FFinalBattleStatusService::ResolvePlayerTurnEndStatuses(FFinalBattleState& BattleState) const
{
	for (int32 StatusIndex = BattleState.StatusInstances.Num() - 1; StatusIndex >= 0; --StatusIndex)
	{
		FFinalBattleStatusInstance& StatusInstance = BattleState.StatusInstances[StatusIndex];
		const bool bExpiresAtPlayerTurnEnd =
			StatusInstance.DurationType == EFinalStatusDurationType::PlayerTurns
			&& StatusInstance.ExpireWindow == EFinalStatusExpireWindow::PlayerTurnEnd;
		if (!bExpiresAtPlayerTurnEnd
			|| IsDamageOverTimeStatus(StatusInstance)
			|| !IsPlayerOwnedStatus(BattleState, StatusInstance.OwnerUnitId))
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

	if (!IsStatusOwnerAllowedByAppliesTo(
		BattleState,
		OwnerUnitId,
		StatusDefinition != nullptr ? StatusDefinition->AppliesTo : EFinalStatusAppliesTo::Shared))
	{
		return 0;
	}

	FFinalBattleStatusInstance* ExistingInstance = FindStatusInstanceForAdd(
		BattleState,
		OwnerUnitId,
		SourceUnitId,
		StatusId,
		StatusDefinition);
	const int32 MaxStacks = StatusDefinition ? StatusDefinition->MaxStacks : 0;
	int32 BaseDuration = DurationOverride > 0
		? DurationOverride
		: (StatusDefinition ? StatusDefinition->DefaultDuration : 0);
	const bool bExpireAtPlayerTurnEnd = ShouldExpireAtPlayerTurnEnd(StatusDefinition);
	if (bExpireAtPlayerTurnEnd && BaseDuration <= 0)
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
		NewInstance.StackKeyPolicy = StatusDefinition != nullptr ? StatusDefinition->StackKeyPolicy : EFinalStatusStackKeyPolicy::ByOwner;
		NewInstance.DurationType = StatusDefinition != nullptr ? StatusDefinition->DurationType : EFinalStatusDurationType::PlayerTurns;
		NewInstance.ExpireWindow = StatusDefinition != nullptr ? StatusDefinition->ExpireWindow : EFinalStatusExpireWindow::None;
		NewInstance.bIsResourceStatus = IsResourceStatus(StatusDefinition);
		NewInstance.ResourceBehavior = StatusDefinition != nullptr ? StatusDefinition->ResourceBehavior : EFinalStatusResourceBehavior::None;
		NewInstance.bAutoAffectBattleRules = StatusDefinition != nullptr && StatusDefinition->bAutoAffectBattleRules;
		NewInstance.bAutoProjectToCards = StatusDefinition != nullptr && StatusDefinition->bAutoProjectToCards;
		NewInstance.bIsDamageOverTime = IsDamageOverTimeStatus(StatusDefinition);
		NewInstance.DamageOverTimeTickWindow = StatusDefinition != nullptr ? StatusDefinition->DamageOverTimeTickWindow : EFinalStatusDamageOverTimeTickWindow::None;
		NewInstance.DamageOverTimeAttackPowerPercentPerStack = StatusDefinition != nullptr ? StatusDefinition->DamageOverTimeAttackPowerPercentPerStack : 0;
		BuildStructuredRuntimeModifiers(StatusDefinition, NewInstance.RuntimeModifiers);
		BuildStructuredProjectedCardModifiers(StatusDefinition, NewInstance.ProjectedCardModifiers);
		BuildConsumptionRules(StatusDefinition, NewInstance.ConsumptionRules);
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
		ExistingInstance->StackKeyPolicy = StatusDefinition->StackKeyPolicy;
		ExistingInstance->DurationType = StatusDefinition->DurationType;
		ExistingInstance->ExpireWindow = StatusDefinition->ExpireWindow;
		ExistingInstance->bIsResourceStatus = IsResourceStatus(StatusDefinition);
		ExistingInstance->ResourceBehavior = StatusDefinition->ResourceBehavior;
		ExistingInstance->bAutoAffectBattleRules = StatusDefinition->bAutoAffectBattleRules;
		ExistingInstance->bAutoProjectToCards = StatusDefinition->bAutoProjectToCards;
		ExistingInstance->bIsDamageOverTime = IsDamageOverTimeStatus(StatusDefinition);
		ExistingInstance->DamageOverTimeTickWindow = StatusDefinition->DamageOverTimeTickWindow;
		ExistingInstance->DamageOverTimeAttackPowerPercentPerStack = StatusDefinition->DamageOverTimeAttackPowerPercentPerStack;
		BuildStructuredRuntimeModifiers(StatusDefinition, ExistingInstance->RuntimeModifiers);
		BuildStructuredProjectedCardModifiers(StatusDefinition, ExistingInstance->ProjectedCardModifiers);
		BuildConsumptionRules(StatusDefinition, ExistingInstance->ConsumptionRules);
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
		if (StatusInstance.OwnerUnitId != OwnerUnitId || StatusInstance.CurrentStacks <= 0)
		{
			continue;
		}

		if (IsResourceStatus(StatusInstance) && !StatusInstance.bAutoAffectBattleRules)
		{
			continue;
		}

		if (UsesStructuredRuntimeModifiers(StatusInstance))
		{
			for (const FFinalBattleStatusRuntimeModifierInstance& RuntimeModifier : StatusInstance.RuntimeModifiers)
			{
				if (!IsStructuredOutgoingDamageModifierApplicable(RuntimeModifier, bIsAttackCardDamage))
				{
					continue;
				}

				TotalModifierPercent += RuntimeModifier.OutgoingDamagePercentPerStack * StatusInstance.CurrentStacks;
			}
		}
	}

	return TotalModifierPercent;
}

int32 FFinalBattleStatusService::GetIncomingDamageModifierPercent(
	const FFinalBattleState& BattleState,
	const FName OwnerUnitId) const
{
	int32 TotalModifierPercent = 0;

	for (const FFinalBattleStatusInstance& StatusInstance : BattleState.StatusInstances)
	{
		if (StatusInstance.OwnerUnitId != OwnerUnitId || StatusInstance.CurrentStacks <= 0)
		{
			continue;
		}

		if (IsResourceStatus(StatusInstance) && !StatusInstance.bAutoAffectBattleRules)
		{
			continue;
		}

		if (!UsesStructuredRuntimeModifiers(StatusInstance))
		{
			continue;
		}

		for (const FFinalBattleStatusRuntimeModifierInstance& RuntimeModifier : StatusInstance.RuntimeModifiers)
		{
			if (RuntimeModifier.IncomingDamagePercentPerStack == 0)
			{
				continue;
			}

			TotalModifierPercent += RuntimeModifier.IncomingDamagePercentPerStack * StatusInstance.CurrentStacks;
		}
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
		if (StatusInstance.OwnerUnitId != OwnerUnitId)
		{
			continue;
		}

		if (IsResourceStatus(StatusInstance) && !StatusInstance.bAutoAffectBattleRules)
		{
			continue;
		}

		int32 StacksToConsume = 0;
		for (const FFinalBattleStatusConsumptionRuleInstance& ConsumptionRule : StatusInstance.ConsumptionRules)
		{
			if (ConsumptionRule.Window != EFinalStatusConsumptionWindow::SuccessfulOwnerDamage
				|| ConsumptionRule.StacksToConsume <= 0)
			{
				continue;
			}

			if (ConsumptionRule.bRequireAttackCardDamage && !bIsAttackCardDamage)
			{
				continue;
			}

			const bool bHasApplicableBenefit =
				IsOutgoingDamageModifierApplicable(StatusInstance, bIsAttackCardDamage)
				|| IsProjectedHandCardModifierApplicable(StatusInstance, bIsAttackCardDamage);
			if (!bHasApplicableBenefit)
			{
				continue;
			}

			StacksToConsume = FMath::Max(StacksToConsume, ConsumptionRule.StacksToConsume);
		}

		if (StacksToConsume <= 0)
		{
			continue;
		}

		const int32 RemovedStacks = FMath::Min(StatusInstance.CurrentStacks, StacksToConsume);
		StatusInstance.CurrentStacks = FMath::Max(StatusInstance.CurrentStacks - RemovedStacks, 0);
		TotalRemovedStacks += RemovedStacks;
		if (StatusInstance.CurrentStacks <= 0)
		{
			BattleState.StatusInstances.RemoveAt(StatusIndex);
		}
	}

	return TotalRemovedStacks;
}

void FFinalBattleStatusService::ResyncProjectedHandCardModifiers(
	FFinalBattleState& BattleState,
	const FFinalBattleCardService& CardService,
	const FName OwnerUnitId) const
{
	if (OwnerUnitId.IsNone())
	{
		return;
	}

	TArray<FGuid> CardInstanceIdsToReproject;
	for (FFinalBattleCardInstance& CardInstance : BattleState.CardInstances)
	{
		if (CardInstance.RuntimeOwnerUnitId != OwnerUnitId)
		{
			continue;
		}

		const int32 RemovedModifierCount = CardInstance.ModifierRecords.RemoveAll(
			[](const FFinalBattleCardModifierRecord& ModifierRecord)
			{
				return IsDerivedStatusHandProjectionModifier(ModifierRecord);
			});
		if (RemovedModifierCount > 0)
		{
			CardInstanceIdsToReproject.AddUnique(CardInstance.CardInstanceId);
		}
	}

	for (const FFinalBattleStatusInstance& StatusInstance : BattleState.StatusInstances)
	{
		if (StatusInstance.OwnerUnitId != OwnerUnitId
			|| StatusInstance.CurrentStacks <= 0
			|| !UsesStructuredProjectedCardModifiers(StatusInstance))
		{
			continue;
		}

		if (IsResourceStatus(StatusInstance) && !StatusInstance.bAutoProjectToCards)
		{
			continue;
		}

		if (UsesStructuredProjectedCardModifiers(StatusInstance))
		{
			for (const FFinalBattleStatusProjectedCardModifierInstance& ProjectedModifier : StatusInstance.ProjectedCardModifiers)
			{
				if (ProjectedModifier.TargetSource != EFinalTriggeredCardModifierTargetSource::CurrentOwnedHandCards)
				{
					continue;
				}

				if (ProjectedModifier.CostDeltaAPPerStack == 0
					&& ProjectedModifier.OutgoingDamagePercentPerStack == 0)
				{
					continue;
				}

				for (const FGuid& HandCardInstanceId : BattleState.DeckState.HandCardInstanceIds)
				{
					FFinalBattleCardInstance* CardInstance = CardService.FindCardInstance(BattleState, HandCardInstanceId);
					if (CardInstance == nullptr || CardInstance->RuntimeOwnerUnitId != OwnerUnitId)
					{
						continue;
					}

					const UFinalCardDefinition* EffectiveCardDefinition = CardInstance->ProjectedDefinition != nullptr
						? CardInstance->ProjectedDefinition
						: CardInstance->BaseDefinition;
					if (EffectiveCardDefinition == nullptr)
					{
						continue;
					}

					if (ProjectedModifier.bRequireCardType && EffectiveCardDefinition->CardType != ProjectedModifier.RequiredCardType)
					{
						continue;
					}

					FFinalBattleCardModifierRecord ModifierRecord;
					ModifierRecord.ModifierId = BuildDerivedStatusHandProjectionModifierId(StatusInstance.StatusInstanceId, CardInstance->CardInstanceId);
					ModifierRecord.SourceType = EFinalBattleCardModifierSourceType::Status;
					ModifierRecord.DurationPolicy = EFinalBattleCardModifierDuration::ManualClear;
					ModifierRecord.ApplyOrder = 1000;
					ModifierRecord.CostDeltaAP = StatusInstance.CurrentStacks * ProjectedModifier.CostDeltaAPPerStack;
					ModifierRecord.OutgoingDamagePercentDelta = StatusInstance.CurrentStacks * ProjectedModifier.OutgoingDamagePercentPerStack;
					CardInstance->ModifierRecords.Add(MoveTemp(ModifierRecord));
					CardInstanceIdsToReproject.AddUnique(CardInstance->CardInstanceId);
				}
			}

		}
	}

	for (const FGuid& CardInstanceId : CardInstanceIdsToReproject)
	{
		CardService.ReprojectCardInstance(BattleState, CardInstanceId, BattleState.RuntimeProjectionOwner);
	}
}

int32 FFinalBattleStatusService::GetIncomingTeamHealthDamageReductionPercent(const FFinalBattleState& BattleState) const
{
	int32 TotalReductionPercent = 0;

	for (const FFinalBattleStatusInstance& StatusInstance : BattleState.StatusInstances)
	{
		if (!IsIncomingTeamHealthDamageProtectionApplicable(StatusInstance))
		{
			continue;
		}

		for (const FFinalBattleStatusRuntimeModifierInstance& RuntimeModifier : StatusInstance.RuntimeModifiers)
		{
			if (RuntimeModifier.IncomingTeamHealthDamageReductionPercentPerStack <= 0)
			{
				continue;
			}

			TotalReductionPercent += RuntimeModifier.IncomingTeamHealthDamageReductionPercentPerStack * StatusInstance.CurrentStacks;
		}
	}

	return FMath::Clamp(TotalReductionPercent, 0, 100);
}

int32 FFinalBattleStatusService::ApplyIncomingTeamHealthDamageProtection(
	FFinalBattleState& BattleState,
	const int32 IncomingHealthDamage) const
{
	const int32 ClampedIncomingHealthDamage = FMath::Max(IncomingHealthDamage, 0);
	if (ClampedIncomingHealthDamage <= 0)
	{
		return 0;
	}

	const int32 ReductionPercent = GetIncomingTeamHealthDamageReductionPercent(BattleState);
	if (ReductionPercent <= 0)
	{
		return ClampedIncomingHealthDamage;
	}

	const int32 PreventedHealthDamage = FMath::Clamp(
		FMath::RoundToInt(static_cast<float>(ClampedIncomingHealthDamage) * static_cast<float>(ReductionPercent) / 100.0f),
		0,
		ClampedIncomingHealthDamage);
	if (PreventedHealthDamage <= 0)
	{
		return ClampedIncomingHealthDamage;
	}

	for (int32 StatusIndex = BattleState.StatusInstances.Num() - 1; StatusIndex >= 0; --StatusIndex)
	{
		FFinalBattleStatusInstance& StatusInstance = BattleState.StatusInstances[StatusIndex];
		if (!IsIncomingTeamHealthDamageProtectionApplicable(StatusInstance))
		{
			continue;
		}

		int32 StacksToConsume = 0;
		for (const FFinalBattleStatusConsumptionRuleInstance& ConsumptionRule : StatusInstance.ConsumptionRules)
		{
			if (ConsumptionRule.Window != EFinalStatusConsumptionWindow::PreventedTeamHealthDamage
				|| ConsumptionRule.StacksToConsume <= 0)
			{
				continue;
			}

			StacksToConsume = FMath::Max(StacksToConsume, ConsumptionRule.StacksToConsume);
		}

		if (StacksToConsume <= 0)
		{
			continue;
		}

		StatusInstance.CurrentStacks = FMath::Max(StatusInstance.CurrentStacks - StacksToConsume, 0);
		if (StatusInstance.CurrentStacks <= 0)
		{
			BattleState.StatusInstances.RemoveAt(StatusIndex);
		}
	}

	return FMath::Max(ClampedIncomingHealthDamage - PreventedHealthDamage, 0);
}

FFinalBattleDamageOverTimeResult FFinalBattleStatusService::ResolveDamageOverTimeAtTickWindow(
	FFinalBattleState& BattleState,
	const EFinalStatusDamageOverTimeTickWindow TickWindow,
	const FFinalBattleUnitService& UnitService,
	const FFinalBattleTriggerService& TriggerService,
	const FFinalBattleEffectExecutionService& EffectExecutionService) const
{
	FFinalBattleDamageOverTimeResult Result;

	for (int32 StatusIndex = BattleState.StatusInstances.Num() - 1; StatusIndex >= 0; --StatusIndex)
	{
		FFinalBattleStatusInstance& StatusInstance = BattleState.StatusInstances[StatusIndex];
		if (!IsDamageOverTimeStatus(StatusInstance)
			|| StatusInstance.DamageOverTimeTickWindow != TickWindow
			|| StatusInstance.CurrentStacks <= 0)
		{
			continue;
		}

		const int32 SourceAttackPower = ResolveSourceAttackPower(BattleState, UnitService, StatusInstance.SourceUnitId);
		const int32 BaseDamage = FMath::Max(
			FMath::RoundToInt(static_cast<float>(SourceAttackPower) * static_cast<float>(StatusInstance.DamageOverTimeAttackPowerPercentPerStack) / 100.0f),
			0);
		const int32 TotalIncomingDamage = FMath::Max(BaseDamage * StatusInstance.CurrentStacks, 0);
		if (TotalIncomingDamage > 0)
		{
			FFinalBattleEffectExecutionSummary Summary;
			if (StatusInstance.OwnerUnitId == TeamPlayerUnitId)
			{
				const int32 HpDamage = EffectExecutionService.ApplyTeamIncomingDamageAndTriggers(
					BattleState,
					TotalIncomingDamage,
					UnitService,
					TriggerService,
					Summary);
				Result.TotalDamageToTeam += HpDamage;
			}
			else if (UnitService.FindEnemyState(BattleState, StatusInstance.OwnerUnitId) != nullptr)
			{
				const int32 HpDamage = EffectExecutionService.ApplyEnemyIncomingDamage(
					BattleState,
					StatusInstance.OwnerUnitId,
					TotalIncomingDamage,
					UnitService,
					Summary);
				Result.TotalDamageToEnemies += HpDamage;
				Result.TotalEnemiesDefeated += Summary.TotalEnemiesDefeated;
			}
		}

		if (StatusInstance.RemainingDuration > 0)
		{
			StatusInstance.RemainingDuration = FMath::Max(StatusInstance.RemainingDuration - 1, 0);
			if (StatusInstance.RemainingDuration <= 0)
			{
				BattleState.StatusInstances.RemoveAt(StatusIndex);
			}
		}
	}

	return Result;
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

bool FFinalBattleStatusService::CanConsumeStatusResource(
	const FFinalBattleState& BattleState,
	const FName OwnerUnitId,
	const FFinalStatusId& StatusId,
	const int32 StacksToConsume) const
{
	const FFinalBattleStatusInstance* StatusInstance = FindStatusInstance(BattleState, OwnerUnitId, StatusId);
	return StatusInstance != nullptr
		&& IsResourceStatus(*StatusInstance)
		&& StacksToConsume > 0
		&& StatusInstance->CurrentStacks >= StacksToConsume;
}

int32 FFinalBattleStatusService::ConsumeStatusResource(
	FFinalBattleState& BattleState,
	const FName OwnerUnitId,
	const FFinalStatusId& StatusId,
	const int32 StacksToConsume) const
{
	if (!CanConsumeStatusResource(BattleState, OwnerUnitId, StatusId, StacksToConsume))
	{
		return 0;
	}

	return RemoveStatusStacks(BattleState, OwnerUnitId, StatusId, StacksToConsume);
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
