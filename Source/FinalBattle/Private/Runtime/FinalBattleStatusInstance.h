#pragma once

#include "CoreMinimal.h"
#include "Battle/Definitions/FinalStatusDefinition.h"
#include "Types/FinalCoreTypes.h"
#include "Ids/FinalIds.h"

struct FFinalBattleStatusRuntimeModifierInstance
{
	int32 OutgoingDamagePercentPerStack = 0;
	int32 IncomingDamagePercentPerStack = 0;
	bool bOnlyAffectAttackCards = false;
	int32 IncomingTeamHealthDamageReductionPercentPerStack = 0;
};

struct FFinalBattleStatusProjectedCardModifierInstance
{
	EFinalTriggeredCardModifierTargetSource TargetSource = EFinalTriggeredCardModifierTargetSource::CurrentOwnedHandCards;
	bool bRequireCardType = false;
	EFinalCardType RequiredCardType = EFinalCardType::Attack;
	int32 CostDeltaAPPerStack = 0;
	int32 OutgoingDamagePercentPerStack = 0;
	EFinalStatusProjectedCardModifierLifetimePolicy LifetimePolicy = EFinalStatusProjectedCardModifierLifetimePolicy::WhileStatusActive;
	bool bExpireAtPlayerTurnEnd = false;
};

struct FFinalBattleStatusConsumptionRuleInstance
{
	EFinalStatusConsumptionWindow Window = EFinalStatusConsumptionWindow::None;
	int32 StacksToConsume = 1;
	bool bRequireAttackCardDamage = false;
};

struct FFinalBattleStatusInstance
{
	FGuid StatusInstanceId;
	FFinalStatusId StatusId;
	FName OwnerUnitId = NAME_None;
	FName SourceUnitId = NAME_None;
	FText DisplayName;
	int32 CurrentStacks = 0;
	int32 RemainingDuration = 0;
	EFinalStatusStackKeyPolicy StackKeyPolicy = EFinalStatusStackKeyPolicy::ByOwner;
	EFinalStatusDurationType DurationType = EFinalStatusDurationType::PlayerTurns;
	EFinalStatusExpireWindow ExpireWindow = EFinalStatusExpireWindow::None;
	bool bIsResourceStatus = false;
	EFinalStatusResourceBehavior ResourceBehavior = EFinalStatusResourceBehavior::None;
	bool bAutoAffectBattleRules = false;
	bool bAutoProjectToCards = false;
	bool bIsDamageOverTime = false;
	EFinalStatusDamageOverTimeTickWindow DamageOverTimeTickWindow = EFinalStatusDamageOverTimeTickWindow::None;
	int32 DamageOverTimeAttackPowerPercentPerStack = 0;
	TArray<FFinalBattleStatusRuntimeModifierInstance> RuntimeModifiers;
	TArray<FFinalBattleStatusProjectedCardModifierInstance> ProjectedCardModifiers;
	TArray<FFinalBattleStatusConsumptionRuleInstance> ConsumptionRules;
};
