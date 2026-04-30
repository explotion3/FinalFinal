#pragma once

#include "CoreMinimal.h"
#include "Battle/Conditions/Requirements/FinalBattleResolvedCardRequirement.h"
#include "Battle/Definitions/FinalRuntimeTriggerDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Conditions/Requirements/FinalBattleTargetStateRequirement.h"
#include "Ids/FinalIds.h"
#include "Run/Definitions/FinalPrototypeBootstrapDefinition.h"
#include "Run/Rewards/FinalRunRewardTypes.h"

class UFinalBattleEffectApplyStatus;
class UFinalBattleEffectApplyPassive;
class UFinalBattleEffectApplyCardModifiers;
class UFinalBattleEffectBonusBreak;
class UFinalBattleEffectDamage;
class UFinalBattleEffectDefinition;
class UFinalBattleEffectDrawCards;
class UFinalBattleEffectGainAP;
class UFinalBattleEffectGainShield;
class UFinalBattleEffectGenerateCard;
class UFinalBattleEffectHeal;
class UFinalBattleEffectMoveCards;
class UFinalBattleEffectConsumeStatusResource;
class UFinalBattleEffectRemoveStatus;
class UFinalBattleConditionMovedCards;
class UFinalBattleConditionResourceConsumed;
class UFinalBattleConditionStatusChanged;
class UFinalBattleConditionHandCard;
class UFinalBattleConditionDefinition;
class UFinalBattleConditionResolvedCard;
class UFinalBattleConditionTargetState;
class UFinalCardDefinition;
class UFinalPassiveDefinition;
class UFinalStatusDefinition;

namespace FinalPrototypeContentBootstrap
{
	FGameplayTag GetRetainKeyword();
	FGameplayTag GetExpendKeyword();
	FGameplayTag GetSwordArrayKeyword();

	FFinalRunRewardEntry MakeBaseRewardEntry(const FName RewardId, EFinalRunRewardType RewardType, int32 Value, FName DisplayId, const FText& DisplayName);
	FFinalRunRewardEntry MakeRelicRewardEntry(FName RewardId, const FFinalRelicId& RelicId, const FText& DisplayName);
	FFinalRunRewardEntry MakeCardRewardEntry(FName RewardId, const FFinalCardId& CardId, const FText& DisplayName);
	FFinalRunRewardEntry MakeGrowthRewardEntry(FName RewardId, const FFinalCharacterId& TargetCharacterId, EFinalRunGrowthEffectType GrowthEffectType, int32 Value, FName DisplayId, const FText& DisplayName);
	FFinalRunRewardEntry MakeRemoveCardRewardEntry(FName RewardId, const FFinalCardId& RemovedCardId, const FText& DisplayName);
	FFinalRunRewardEntry MakeUpgradeCardRewardEntry(FName RewardId, const FFinalCardId& UpgradeFromCardId, const FFinalCardId& UpgradeToCardId, const FText& DisplayName);
	FFinalInitialLoadoutCardEntry MakeLoadoutEntry(const FFinalCardId& CardId, int32 Count, EFinalLoadoutRole LoadoutRole);
	FFinalPrototypeBootstrapCharacterState MakeBootstrapCharacterState(
		const FFinalCharacterId& CharacterId,
		int32 CurrentStress,
		int32 Level = 1,
		int32 BreakthroughValue = 0,
		int32 BreakthroughRequiredValue = 0,
		int32 RootBone = 0,
		int32 Insight = 0,
		int32 KillingIntent = 0);

	UFinalBattleConditionTargetState* AddTargetStateCondition(UFinalBattleEffectDefinition* Effect, const FFinalBattleTargetStateRequirement& Requirement);
	UFinalBattleConditionStatusChanged* AddStatusChangedCondition(UFinalBattleEffectDefinition* Effect, const FFinalStatusId& StatusId, int32 MinimumStacks);
	UFinalBattleConditionResourceConsumed* AddResourceConsumedCondition(UFinalBattleEffectDefinition* Effect, const FFinalStatusId& StatusId, int32 MinimumStacks);
	UFinalBattleConditionHandCard* AddHandCardCondition(UFinalBattleEffectDefinition* Effect, const FGameplayTag& RequiredKeyword, int32 MinimumCount, bool bGeneratedOnly);
	UFinalBattleConditionResolvedCard* AddResolvedCardCondition(UObject* Owner, TArray<TObjectPtr<UFinalBattleConditionDefinition>>& Conditions, const FFinalBattleResolvedCardRequirement& Requirement);
	UFinalBattleConditionMovedCards* AddMovedCardsCondition(UFinalBattleEffectDefinition* Effect, const FGameplayTag& RequiredKeyword, int32 MinimumCount, bool bGeneratedOnly = true, bool bRequireSourceZone = false, EFinalBattleCardZoneRule SourceZone = EFinalBattleCardZoneRule::Hand, bool bRequireDestinationZone = false, EFinalBattleCardZoneRule DestinationZone = EFinalBattleCardZoneRule::ConsumePile);

	UFinalBattleEffectDamage* AddDamageEffect(UObject* Owner, TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects, FName EffectId, EFinalBattleUnitTargetRule TargetRule, float BaseValue, EFinalBattleScalarMode ScaleMode, EFinalBattleSourceStat SourceStat, int32 HitCount = 1, const FText& Notes = FText::GetEmpty(), const FFinalBattleTargetStateRequirement* TargetStateRequirement = nullptr);
	UFinalBattleEffectGainShield* AddShieldEffect(UObject* Owner, TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects, FName EffectId, EFinalBattleUnitTargetRule TargetRule, float BaseValue, EFinalBattleScalarMode ScaleMode, EFinalBattleSourceStat SourceStat = EFinalBattleSourceStat::None, const FText& Notes = FText::GetEmpty());
	UFinalBattleEffectDrawCards* AddDrawEffect(UObject* Owner, TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects, FName EffectId, int32 DrawCount, const FText& Notes = FText::GetEmpty());
	UFinalBattleEffectHeal* AddHealEffect(UObject* Owner, TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects, FName EffectId, EFinalBattleUnitTargetRule TargetRule, float BaseValue, EFinalBattleScalarMode ScaleMode, EFinalBattleSourceStat SourceStat = EFinalBattleSourceStat::None, const FText& Notes = FText::GetEmpty());
	UFinalBattleEffectApplyStatus* AddApplyStatusEffect(UObject* Owner, TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects, FName EffectId, EFinalBattleUnitTargetRule TargetRule, UFinalStatusDefinition* StatusDefinition, int32 Stacks, const FText& Notes = FText::GetEmpty());
	UFinalBattleEffectApplyPassive* AddApplyPassiveEffect(UObject* Owner, TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects, FName EffectId, EFinalBattleUnitTargetRule TargetRule, UFinalPassiveDefinition* PassiveDefinition, int32 Stacks, int32 DurationOverride = 0, const FText& Notes = FText::GetEmpty());
	UFinalBattleEffectRemoveStatus* AddRemoveStatusEffect(UObject* Owner, TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects, FName EffectId, EFinalBattleUnitTargetRule TargetRule, UFinalStatusDefinition* StatusDefinition, int32 Stacks, const FText& Notes = FText::GetEmpty());
	UFinalBattleEffectConsumeStatusResource* AddConsumeStatusResourceEffect(UObject* Owner, TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects, FName EffectId, EFinalBattleUnitTargetRule TargetRule, UFinalStatusDefinition* StatusDefinition, int32 StacksToConsume, const FText& Notes = FText::GetEmpty());
	UFinalBattleEffectApplyCardModifiers* AddApplyCardModifiersEffect(UObject* Owner, TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects, FName EffectId, const TArray<FFinalTriggeredCardModifierDefinition>& CardModifiers, const FText& Notes = FText::GetEmpty());
	UFinalBattleEffectGainAP* AddGainApEffect(UObject* Owner, TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects, FName EffectId, int32 GainValue, const FText& Notes = FText::GetEmpty());
	UFinalBattleEffectBonusBreak* AddBonusBreakEffect(UObject* Owner, TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects, FName EffectId, EFinalBattleUnitTargetRule TargetRule, float BaseValue, EFinalBattleScalarMode ScaleMode, EFinalBattleSourceStat SourceStat = EFinalBattleSourceStat::None, const FText& Notes = FText::GetEmpty());
	UFinalBattleEffectGenerateCard* AddGenerateCardEffect(UObject* Owner, TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects, FName EffectId, UFinalCardDefinition* GeneratedCardDefinition, const TArray<UFinalCardDefinition*>& CandidateCardDefinitions, int32 GenerateCount, bool bChooseRandomCandidate, const FText& Notes = FText::GetEmpty());
	UFinalBattleEffectMoveCards* AddMoveCardsEffect(UObject* Owner, TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects, FName EffectId, EFinalBattleCardZoneRule SourceZone, EFinalBattleCardZoneRule DestinationZone, const FFinalCardId& RequiredCardId, const FGameplayTag& RequiredKeyword, int32 MoveCount, bool bGeneratedOnly, bool bRecordMovedCards, const FText& Notes = FText::GetEmpty());
}
