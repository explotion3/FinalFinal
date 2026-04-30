#pragma once

#include "CoreMinimal.h"
#include "Battle/Definitions/FinalStatusDefinition.h"
#include "Types/FinalCoreTypes.h"
#include "Ids/FinalIds.h"

struct FFinalBattleStatusRuntimeModifierInstance
{
	int32 OutgoingDamagePercentPerStack = 0;
	bool bOnlyAffectAttackCards = false;
	int32 IncomingTeamHealthDamageReductionPercentPerStack = 0;
	bool bConsumeOnSuccessfulOwnerDamage = false;
	bool bConsumeOnPreventedTeamHealthDamage = false;
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

struct FFinalBattleStatusInstance
{
	FGuid StatusInstanceId;
	FFinalStatusId StatusId;
	FName OwnerUnitId = NAME_None;
	FName SourceUnitId = NAME_None;
	FText DisplayName;
	int32 CurrentStacks = 0;
	int32 RemainingDuration = 0;
	bool bIsResourceStatus = false;
	EFinalStatusResourceBehavior ResourceBehavior = EFinalStatusResourceBehavior::None;
	bool bAutoAffectBattleRules = false;
	bool bAutoProjectToCards = false;
	TArray<FFinalBattleStatusRuntimeModifierInstance> RuntimeModifiers;
	TArray<FFinalBattleStatusProjectedCardModifierInstance> ProjectedCardModifiers;
	int32 OutgoingDamagePercentPerStack = 0;
	bool bExpireAtPlayerTurnEnd = false;
	bool bConsumeOnSuccessfulOwnerDamage = false;
	bool bOnlyAffectAttackCards = false;
	int32 IncomingTeamHealthDamageReductionPercentPerStack = 0;
	bool bConsumeOnPreventedTeamHealthDamage = false;
	bool bProjectToOwnedHandCards = false;
	EFinalCardType ProjectedCardTypeFilter = EFinalCardType::Attack;
	int32 ProjectedOutgoingDamagePercentPerStack = 0;
};
