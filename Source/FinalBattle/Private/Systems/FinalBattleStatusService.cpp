#include "Systems/FinalBattleStatusService.h"

#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalStatusDefinition.h"
#include "Queries/FinalBattleQueryTypes.h"
#include "Runtime/FinalBattleCardInstance.h"
#include "Runtime/FinalBattleCharacterState.h"
#include "Runtime/FinalBattleState.h"
#include "Runtime/FinalBattleStatusInstance.h"
#include "Systems/FinalBattleCardService.h"

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

bool UsesStructuredRuntimeModifiers(const UFinalStatusDefinition* StatusDefinition)
{
	return StatusDefinition != nullptr && StatusDefinition->RuntimeModifiers.Num() > 0;
}

bool UsesStructuredProjectedCardModifiers(const UFinalStatusDefinition* StatusDefinition)
{
	return StatusDefinition != nullptr && StatusDefinition->ProjectedCardModifiers.Num() > 0;
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

bool ShouldExpireAtPlayerTurnEnd(const UFinalStatusDefinition* StatusDefinition)
{
	if (StatusDefinition == nullptr)
	{
		return false;
	}

	if (UsesStructuredRuntimeModifiers(StatusDefinition))
	{
		return StatusDefinition->DurationType == EFinalStatusDurationType::PlayerTurns
			&& StatusDefinition->ExpireWindow == EFinalStatusExpireWindow::PlayerTurnEnd;
	}

	return StatusDefinition->bExpireAtPlayerTurnEnd;
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
			&& ModifierDefinition.IncomingTeamHealthDamageReductionPercentPerStack == 0
			&& !ModifierDefinition.bConsumeOnSuccessfulOwnerDamage
			&& !ModifierDefinition.bConsumeOnPreventedTeamHealthDamage
			&& !ModifierDefinition.bOnlyAffectAttackCards)
		{
			continue;
		}

		FFinalBattleStatusRuntimeModifierInstance& ModifierInstance = OutRuntimeModifiers.AddDefaulted_GetRef();
		ModifierInstance.OutgoingDamagePercentPerStack = ModifierDefinition.OutgoingDamagePercentPerStack;
		ModifierInstance.bOnlyAffectAttackCards = ModifierDefinition.bOnlyAffectAttackCards;
		ModifierInstance.IncomingTeamHealthDamageReductionPercentPerStack = ModifierDefinition.IncomingTeamHealthDamageReductionPercentPerStack;
		ModifierInstance.bConsumeOnSuccessfulOwnerDamage = ModifierDefinition.bConsumeOnSuccessfulOwnerDamage;
		ModifierInstance.bConsumeOnPreventedTeamHealthDamage = ModifierDefinition.bConsumeOnPreventedTeamHealthDamage;
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

	if (UsesStructuredRuntimeModifiers(StatusInstance)
		|| StatusInstance.bProjectToOwnedHandCards
		|| StatusInstance.OutgoingDamagePercentPerStack == 0)
	{
		return false;
	}

	if (StatusInstance.bOnlyAffectAttackCards && !bIsAttackCardDamage)
	{
		return false;
	}

	return true;
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

	if (UsesStructuredProjectedCardModifiers(StatusInstance))
	{
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

	if (!StatusInstance.bProjectToOwnedHandCards
		|| StatusInstance.ProjectedOutgoingDamagePercentPerStack == 0)
	{
		return false;
	}

	if (StatusInstance.ProjectedCardTypeFilter == EFinalCardType::Attack && !bIsAttackCardDamage)
	{
		return false;
	}

	return true;
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

	if (UsesStructuredRuntimeModifiers(StatusInstance))
	{
		const bool bHasApplicableModifier = StatusInstance.RuntimeModifiers.ContainsByPredicate(
			[](const FFinalBattleStatusRuntimeModifierInstance& RuntimeModifier)
			{
				return RuntimeModifier.IncomingTeamHealthDamageReductionPercentPerStack > 0;
			});
		return StatusInstance.OwnerUnitId == TeamPlayerUnitId
			&& StatusInstance.CurrentStacks > 0
			&& bHasApplicableModifier;
	}

	return StatusInstance.OwnerUnitId == TeamPlayerUnitId
		&& StatusInstance.CurrentStacks > 0
		&& StatusInstance.IncomingTeamHealthDamageReductionPercentPerStack > 0;
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
		NewInstance.bIsResourceStatus = IsResourceStatus(StatusDefinition);
		NewInstance.ResourceBehavior = StatusDefinition != nullptr ? StatusDefinition->ResourceBehavior : EFinalStatusResourceBehavior::None;
		NewInstance.bAutoAffectBattleRules = StatusDefinition != nullptr && StatusDefinition->bAutoAffectBattleRules;
		NewInstance.bAutoProjectToCards = StatusDefinition != nullptr && StatusDefinition->bAutoProjectToCards;
		BuildStructuredRuntimeModifiers(StatusDefinition, NewInstance.RuntimeModifiers);
		BuildStructuredProjectedCardModifiers(StatusDefinition, NewInstance.ProjectedCardModifiers);
		NewInstance.OutgoingDamagePercentPerStack = UsesStructuredRuntimeModifiers(NewInstance) ? 0 : (StatusDefinition ? StatusDefinition->OutgoingDamagePercentPerStack : 0);
		NewInstance.bExpireAtPlayerTurnEnd = bExpireAtPlayerTurnEnd;
		NewInstance.bConsumeOnSuccessfulOwnerDamage = UsesStructuredRuntimeModifiers(NewInstance) ? false : (StatusDefinition ? StatusDefinition->bConsumeOnSuccessfulOwnerDamage : false);
		NewInstance.bOnlyAffectAttackCards = UsesStructuredRuntimeModifiers(NewInstance) ? false : (StatusDefinition ? StatusDefinition->bOnlyAffectAttackCards : false);
		NewInstance.IncomingTeamHealthDamageReductionPercentPerStack = UsesStructuredRuntimeModifiers(NewInstance) ? 0 : (StatusDefinition ? StatusDefinition->IncomingTeamHealthDamageReductionPercentPerStack : 0);
		NewInstance.bConsumeOnPreventedTeamHealthDamage = UsesStructuredRuntimeModifiers(NewInstance) ? false : (StatusDefinition ? StatusDefinition->bConsumeOnPreventedTeamHealthDamage : false);
		NewInstance.bProjectToOwnedHandCards = UsesStructuredProjectedCardModifiers(NewInstance) ? false : (StatusDefinition ? StatusDefinition->bProjectToOwnedHandCards : false);
		NewInstance.ProjectedCardTypeFilter = UsesStructuredProjectedCardModifiers(NewInstance) ? EFinalCardType::Attack : (StatusDefinition ? StatusDefinition->ProjectedCardTypeFilter : EFinalCardType::Attack);
		NewInstance.ProjectedOutgoingDamagePercentPerStack = UsesStructuredProjectedCardModifiers(NewInstance) ? 0 : (StatusDefinition ? StatusDefinition->ProjectedOutgoingDamagePercentPerStack : 0);
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
		ExistingInstance->bIsResourceStatus = IsResourceStatus(StatusDefinition);
		ExistingInstance->ResourceBehavior = StatusDefinition->ResourceBehavior;
		ExistingInstance->bAutoAffectBattleRules = StatusDefinition->bAutoAffectBattleRules;
		ExistingInstance->bAutoProjectToCards = StatusDefinition->bAutoProjectToCards;
		BuildStructuredRuntimeModifiers(StatusDefinition, ExistingInstance->RuntimeModifiers);
		BuildStructuredProjectedCardModifiers(StatusDefinition, ExistingInstance->ProjectedCardModifiers);
		ExistingInstance->OutgoingDamagePercentPerStack = UsesStructuredRuntimeModifiers(*ExistingInstance) ? 0 : StatusDefinition->OutgoingDamagePercentPerStack;
		ExistingInstance->bExpireAtPlayerTurnEnd = bExpireAtPlayerTurnEnd;
		ExistingInstance->bConsumeOnSuccessfulOwnerDamage = UsesStructuredRuntimeModifiers(*ExistingInstance) ? false : StatusDefinition->bConsumeOnSuccessfulOwnerDamage;
		ExistingInstance->bOnlyAffectAttackCards = UsesStructuredRuntimeModifiers(*ExistingInstance) ? false : StatusDefinition->bOnlyAffectAttackCards;
		ExistingInstance->IncomingTeamHealthDamageReductionPercentPerStack = UsesStructuredRuntimeModifiers(*ExistingInstance) ? 0 : StatusDefinition->IncomingTeamHealthDamageReductionPercentPerStack;
		ExistingInstance->bConsumeOnPreventedTeamHealthDamage = UsesStructuredRuntimeModifiers(*ExistingInstance) ? false : StatusDefinition->bConsumeOnPreventedTeamHealthDamage;
		ExistingInstance->bProjectToOwnedHandCards = UsesStructuredProjectedCardModifiers(*ExistingInstance) ? false : StatusDefinition->bProjectToOwnedHandCards;
		ExistingInstance->ProjectedCardTypeFilter = UsesStructuredProjectedCardModifiers(*ExistingInstance) ? EFinalCardType::Attack : StatusDefinition->ProjectedCardTypeFilter;
		ExistingInstance->ProjectedOutgoingDamagePercentPerStack = UsesStructuredProjectedCardModifiers(*ExistingInstance) ? 0 : StatusDefinition->ProjectedOutgoingDamagePercentPerStack;
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
			continue;
		}

		if (!IsOutgoingDamageModifierApplicable(StatusInstance, bIsAttackCardDamage))
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
		if (StatusInstance.OwnerUnitId != OwnerUnitId)
		{
			continue;
		}

		if (IsResourceStatus(StatusInstance) && !StatusInstance.bAutoAffectBattleRules)
		{
			continue;
		}

		bool bShouldConsumeStack = false;
		if (UsesStructuredRuntimeModifiers(StatusInstance))
		{
			bShouldConsumeStack = StatusInstance.RuntimeModifiers.ContainsByPredicate(
				[bIsAttackCardDamage](const FFinalBattleStatusRuntimeModifierInstance& RuntimeModifier)
				{
					return RuntimeModifier.bConsumeOnSuccessfulOwnerDamage
						&& IsStructuredOutgoingDamageModifierApplicable(RuntimeModifier, bIsAttackCardDamage);
				});
		}
		else
		{
			bShouldConsumeStack = StatusInstance.bConsumeOnSuccessfulOwnerDamage
				&& (IsOutgoingDamageModifierApplicable(StatusInstance, bIsAttackCardDamage)
					|| IsProjectedHandCardModifierApplicable(StatusInstance, bIsAttackCardDamage));
		}

		if (!bShouldConsumeStack)
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
			|| (!StatusInstance.bProjectToOwnedHandCards && !UsesStructuredProjectedCardModifiers(StatusInstance)))
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

			continue;
		}

		if (StatusInstance.ProjectedOutgoingDamagePercentPerStack == 0)
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
			if (EffectiveCardDefinition == nullptr || EffectiveCardDefinition->CardType != StatusInstance.ProjectedCardTypeFilter)
			{
				continue;
			}

			FFinalBattleCardModifierRecord ModifierRecord;
			ModifierRecord.ModifierId = BuildDerivedStatusHandProjectionModifierId(StatusInstance.StatusInstanceId, CardInstance->CardInstanceId);
			ModifierRecord.SourceType = EFinalBattleCardModifierSourceType::Status;
			ModifierRecord.DurationPolicy = EFinalBattleCardModifierDuration::ManualClear;
			ModifierRecord.ApplyOrder = 1000;
			ModifierRecord.OutgoingDamagePercentDelta = StatusInstance.CurrentStacks * StatusInstance.ProjectedOutgoingDamagePercentPerStack;
			CardInstance->ModifierRecords.Add(MoveTemp(ModifierRecord));
			CardInstanceIdsToReproject.AddUnique(CardInstance->CardInstanceId);
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

		if (UsesStructuredRuntimeModifiers(StatusInstance))
		{
			for (const FFinalBattleStatusRuntimeModifierInstance& RuntimeModifier : StatusInstance.RuntimeModifiers)
			{
				if (RuntimeModifier.IncomingTeamHealthDamageReductionPercentPerStack <= 0)
				{
					continue;
				}

				TotalReductionPercent += RuntimeModifier.IncomingTeamHealthDamageReductionPercentPerStack * StatusInstance.CurrentStacks;
			}
			continue;
		}

		TotalReductionPercent += StatusInstance.IncomingTeamHealthDamageReductionPercentPerStack * StatusInstance.CurrentStacks;
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

		bool bShouldConsumeStack = false;
		if (UsesStructuredRuntimeModifiers(StatusInstance))
		{
			bShouldConsumeStack = StatusInstance.RuntimeModifiers.ContainsByPredicate(
				[](const FFinalBattleStatusRuntimeModifierInstance& RuntimeModifier)
				{
					return RuntimeModifier.IncomingTeamHealthDamageReductionPercentPerStack > 0
						&& RuntimeModifier.bConsumeOnPreventedTeamHealthDamage;
				});
		}
		else
		{
			bShouldConsumeStack = StatusInstance.bConsumeOnPreventedTeamHealthDamage;
		}

		if (!bShouldConsumeStack)
		{
			continue;
		}

		StatusInstance.CurrentStacks = FMath::Max(StatusInstance.CurrentStacks - 1, 0);
		if (StatusInstance.CurrentStacks <= 0)
		{
			BattleState.StatusInstances.RemoveAt(StatusIndex);
		}
	}

	return FMath::Max(ClampedIncomingHealthDamage - PreventedHealthDamage, 0);
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
