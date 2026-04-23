#include "Bootstrap/FinalPrototypeContentBootstrapEffectUtils.h"

#include "Battle/Conditions/FinalBattleConditionStatusChanged.h"
#include "Battle/Conditions/FinalBattleConditionHandCard.h"
#include "Battle/Conditions/FinalBattleConditionMovedCards.h"
#include "Battle/Conditions/FinalBattleConditionResolvedCard.h"
#include "Battle/Conditions/FinalBattleConditionTargetState.h"
#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalEnemyDefinition.h"
#include "Battle/Definitions/FinalEnemyIntentDefinition.h"
#include "Battle/Definitions/FinalStatusDefinition.h"
#include "Battle/Definitions/FinalUltimateDefinition.h"
#include "Battle/Effects/FinalBattleEffectApplyStatus.h"
#include "Battle/Effects/FinalBattleEffectBonusBreak.h"
#include "Battle/Effects/FinalBattleEffectDamage.h"
#include "Battle/Effects/FinalBattleEffectDrawCards.h"
#include "Battle/Effects/FinalBattleEffectGainAP.h"
#include "Battle/Effects/FinalBattleEffectGenerateCard.h"
#include "Battle/Effects/FinalBattleEffectGainShield.h"
#include "Battle/Effects/FinalBattleEffectHeal.h"
#include "Battle/Effects/FinalBattleEffectMoveCards.h"
#include "Battle/Effects/FinalBattleEffectRemoveStatus.h"
#include "Battle/Conditions/Requirements/FinalBattleTargetStateRequirement.h"
#include "Run/Definitions/FinalPrototypeBootstrapDefinition.h"
#include "Run/Definitions/FinalRelicDefinition.h"
#include "Run/Definitions/FinalRunNodeDefinition.h"
#include "Run/Definitions/FinalRunRouteDefinition.h"
#include "Run/Rewards/FinalRunRewardTypes.h"

namespace FinalPrototypeContentBootstrap
{
	FGameplayTag GetRetainKeyword()
	{
		return FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Retain"));
	}

	FGameplayTag GetExpendKeyword()
	{
		return FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Expend"));
	}

	FGameplayTag GetSwordArrayKeyword()
	{
		return FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.SwordArray"));
	}

	FFinalRunRewardEntry MakeBaseRewardEntry(
		const FName RewardId,
		const EFinalRunRewardType RewardType,
		const int32 Value,
		const FName DisplayId,
		const FText& DisplayName)
	{
		FFinalRunRewardEntry Entry;
		Entry.RewardId = RewardId;
		Entry.RewardType = RewardType;
		Entry.Value = Value;
		Entry.DisplayId = DisplayId;
		Entry.DisplayName = DisplayName;
		return Entry;
	}

	FFinalRunRewardEntry MakeRelicRewardEntry(const FName RewardId, const FFinalRelicId& RelicId, const FText& DisplayName)
	{
		FFinalRunRewardEntry Entry = MakeBaseRewardEntry(
			RewardId,
			EFinalRunRewardType::RelicGrant,
			1,
			RelicId.Value,
			DisplayName);
		Entry.GrantedRelicId = RelicId;
		return Entry;
	}

	FFinalRunRewardEntry MakeCardRewardEntry(const FName RewardId, const FFinalCardId& CardId, const FText& DisplayName)
	{
		FFinalRunRewardEntry Entry = MakeBaseRewardEntry(
			RewardId,
			EFinalRunRewardType::CardGrant,
			1,
			CardId.Value,
			DisplayName);
		Entry.GrantedCardId = CardId;
		return Entry;
	}

	FFinalRunRewardEntry MakeGrowthRewardEntry(
		const FName RewardId,
		const FFinalCharacterId& TargetCharacterId,
		const EFinalRunGrowthEffectType GrowthEffectType,
		const int32 Value,
		const FName DisplayId,
		const FText& DisplayName)
	{
		FFinalRunRewardEntry Entry = MakeBaseRewardEntry(
			RewardId,
			EFinalRunRewardType::Growth,
			Value,
			DisplayId,
			DisplayName);
		Entry.GrowthTargetCharacterId = TargetCharacterId;
		Entry.GrowthEffectType = GrowthEffectType;
		return Entry;
	}

	FFinalRunRewardEntry MakeRemoveCardRewardEntry(const FName RewardId, const FFinalCardId& RemovedCardId, const FText& DisplayName)
	{
		FFinalRunRewardEntry Entry = MakeBaseRewardEntry(
			RewardId,
			EFinalRunRewardType::RemoveCard,
			1,
			RemovedCardId.Value,
			DisplayName);
		Entry.RemovedCardId = RemovedCardId;
		return Entry;
	}

	FFinalRunRewardEntry MakeUpgradeCardRewardEntry(
		const FName RewardId,
		const FFinalCardId& UpgradeFromCardId,
		const FFinalCardId& UpgradeToCardId,
		const FText& DisplayName)
	{
		FFinalRunRewardEntry Entry = MakeBaseRewardEntry(
			RewardId,
			EFinalRunRewardType::UpgradeCard,
			1,
			UpgradeToCardId.Value,
			DisplayName);
		Entry.UpgradeFromCardId = UpgradeFromCardId;
		Entry.UpgradeToCardId = UpgradeToCardId;
		return Entry;
	}

	FFinalInitialLoadoutCardEntry MakeLoadoutEntry(const FFinalCardId& CardId, const int32 Count, const EFinalLoadoutRole LoadoutRole)
	{
		FFinalInitialLoadoutCardEntry Entry;
		Entry.CardId = CardId;
		Entry.Count = Count;
		Entry.LoadoutRole = LoadoutRole;
		return Entry;
	}

	FFinalPrototypeBootstrapCharacterState MakeBootstrapCharacterState(const FFinalCharacterId& CharacterId, const int32 CurrentStress)
	{
		FFinalPrototypeBootstrapCharacterState CharacterState;
		CharacterState.CharacterId = CharacterId;
		CharacterState.CurrentStress = CurrentStress;
		CharacterState.bCollapsed = false;
		CharacterState.CurrentAwakenCount = 0;
		CharacterState.CollapseCount = 0;
		return CharacterState;
	}

	UFinalBattleConditionTargetState* AddTargetStateCondition(UFinalBattleEffectDefinition* Effect, const FFinalBattleTargetStateRequirement& Requirement)
	{
		if (Effect == nullptr)
		{
			return nullptr;
		}

		UFinalBattleConditionTargetState* Condition = NewObject<UFinalBattleConditionTargetState>(Effect);
		Condition->ConditionId = TEXT("condition.target_state");
		Condition->Requirement = Requirement;
		Effect->Conditions.Add(Condition);
		return Condition;
	}

	UFinalBattleConditionStatusChanged* AddStatusChangedCondition(
		UFinalBattleEffectDefinition* Effect,
		const FFinalStatusId& StatusId,
		const int32 MinimumStacks)
	{
		if (Effect == nullptr)
		{
			return nullptr;
		}

		UFinalBattleConditionStatusChanged* Condition = NewObject<UFinalBattleConditionStatusChanged>(Effect);
		Condition->ConditionId = TEXT("condition.status_changed");
		Condition->Requirement.ChangeKind = EFinalBattleStatusChangeKind::Removed;
		Condition->Requirement.RequiredStatusId = StatusId;
		Condition->Requirement.MinimumStacks = FMath::Max(MinimumStacks, 1);
		Effect->Conditions.Add(Condition);
		return Condition;
	}

	UFinalBattleConditionHandCard* AddHandCardCondition(
		UFinalBattleEffectDefinition* Effect,
		const FGameplayTag& RequiredKeyword,
		const int32 MinimumCount,
		const bool bGeneratedOnly)
	{
		if (Effect == nullptr)
		{
			return nullptr;
		}

		UFinalBattleConditionHandCard* Condition = NewObject<UFinalBattleConditionHandCard>(Effect);
		Condition->ConditionId = TEXT("condition.hand_card");
		Condition->Requirement.bRequireInHand = true;
		Condition->Requirement.RequiredKeyword = RequiredKeyword;
		Condition->Requirement.MinimumCount = FMath::Max(MinimumCount, 1);
		Condition->Requirement.bGeneratedOnly = bGeneratedOnly;
		Effect->Conditions.Add(Condition);
		return Condition;
	}

	UFinalBattleConditionResolvedCard* AddResolvedCardCondition(
		UObject* Owner,
		TArray<TObjectPtr<UFinalBattleConditionDefinition>>& Conditions,
		const FFinalBattleResolvedCardRequirement& Requirement)
	{
		if (Owner == nullptr)
		{
			return nullptr;
		}

		UFinalBattleConditionResolvedCard* Condition = NewObject<UFinalBattleConditionResolvedCard>(Owner);
		Condition->ConditionId = TEXT("condition.resolved_card");
		Condition->Requirement = Requirement;
		Conditions.Add(Condition);
		return Condition;
	}

	UFinalBattleConditionMovedCards* AddMovedCardsCondition(
		UFinalBattleEffectDefinition* Effect,
		const FGameplayTag& RequiredKeyword,
		const int32 MinimumCount,
		const bool bGeneratedOnly,
		const bool bRequireSourceZone,
		const EFinalBattleCardZoneRule SourceZone,
		const bool bRequireDestinationZone,
		const EFinalBattleCardZoneRule DestinationZone)
	{
		if (Effect == nullptr)
		{
			return nullptr;
		}

		UFinalBattleConditionMovedCards* Condition = NewObject<UFinalBattleConditionMovedCards>(Effect);
		Condition->ConditionId = TEXT("condition.moved_cards");
		Condition->Requirement.RequiredKeyword = RequiredKeyword;
		Condition->Requirement.MinimumCount = FMath::Max(MinimumCount, 1);
		Condition->Requirement.bGeneratedOnly = bGeneratedOnly;
		Condition->Requirement.bRequireSourceZone = bRequireSourceZone;
		Condition->Requirement.SourceZone = SourceZone;
		Condition->Requirement.bRequireDestinationZone = bRequireDestinationZone;
		Condition->Requirement.DestinationZone = DestinationZone;
		Effect->Conditions.Add(Condition);
		return Condition;
	}

	UFinalBattleEffectDamage* AddDamageEffect(
		UObject* Owner,
		TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects,
		const FName EffectId,
		const EFinalBattleUnitTargetRule TargetRule,
		const float BaseValue,
		const EFinalBattleScalarMode ScaleMode,
		const EFinalBattleSourceStat SourceStat,
		const int32 HitCount,
		const FText& Notes,
		const FFinalBattleTargetStateRequirement* TargetStateRequirement)
	{
		UFinalBattleEffectDamage* DamageEffect = NewObject<UFinalBattleEffectDamage>(Owner);
		DamageEffect->EffectId = EffectId;
		DamageEffect->UnitTargetRule = TargetRule;
		DamageEffect->Scalar.BaseValue = BaseValue;
		DamageEffect->Scalar.ScaleMode = ScaleMode;
		DamageEffect->Scalar.SourceStat = SourceStat;
		DamageEffect->HitCount = HitCount;
		DamageEffect->Notes = Notes;
		if (TargetStateRequirement != nullptr)
		{
			AddTargetStateCondition(DamageEffect, *TargetStateRequirement);
		}
		Effects.Add(DamageEffect);
		return DamageEffect;
	}

	UFinalBattleEffectGainShield* AddShieldEffect(
		UObject* Owner,
		TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects,
		const FName EffectId,
		const EFinalBattleUnitTargetRule TargetRule,
		const float BaseValue,
		const EFinalBattleScalarMode ScaleMode,
		const EFinalBattleSourceStat SourceStat,
		const FText& Notes)
	{
		UFinalBattleEffectGainShield* ShieldEffect = NewObject<UFinalBattleEffectGainShield>(Owner);
		ShieldEffect->EffectId = EffectId;
		ShieldEffect->UnitTargetRule = TargetRule;
		ShieldEffect->Scalar.BaseValue = BaseValue;
		ShieldEffect->Scalar.ScaleMode = ScaleMode;
		ShieldEffect->Scalar.SourceStat = SourceStat;
		ShieldEffect->Notes = Notes;
		Effects.Add(ShieldEffect);
		return ShieldEffect;
	}

	UFinalBattleEffectDrawCards* AddDrawEffect(
		UObject* Owner,
		TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects,
		const FName EffectId,
		const int32 DrawCount,
		const FText& Notes)
	{
		UFinalBattleEffectDrawCards* DrawEffect = NewObject<UFinalBattleEffectDrawCards>(Owner);
		DrawEffect->EffectId = EffectId;
		DrawEffect->DrawCount = DrawCount;
		DrawEffect->Notes = Notes;
		Effects.Add(DrawEffect);
		return DrawEffect;
	}

	UFinalBattleEffectHeal* AddHealEffect(
		UObject* Owner,
		TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects,
		const FName EffectId,
		const EFinalBattleUnitTargetRule TargetRule,
		const float BaseValue,
		const EFinalBattleScalarMode ScaleMode,
		const EFinalBattleSourceStat SourceStat,
		const FText& Notes)
	{
		UFinalBattleEffectHeal* HealEffect = NewObject<UFinalBattleEffectHeal>(Owner);
		HealEffect->EffectId = EffectId;
		HealEffect->UnitTargetRule = TargetRule;
		HealEffect->Scalar.BaseValue = BaseValue;
		HealEffect->Scalar.ScaleMode = ScaleMode;
		HealEffect->Scalar.SourceStat = SourceStat;
		HealEffect->Notes = Notes;
		Effects.Add(HealEffect);
		return HealEffect;
	}

	UFinalBattleEffectApplyStatus* AddApplyStatusEffect(
		UObject* Owner,
		TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects,
		const FName EffectId,
		const EFinalBattleUnitTargetRule TargetRule,
		UFinalStatusDefinition* StatusDefinition,
		const int32 Stacks,
		const FText& Notes)
	{
		UFinalBattleEffectApplyStatus* ApplyStatusEffect = NewObject<UFinalBattleEffectApplyStatus>(Owner);
		ApplyStatusEffect->EffectId = EffectId;
		ApplyStatusEffect->UnitTargetRule = TargetRule;
		ApplyStatusEffect->StatusDefinition = StatusDefinition;
		ApplyStatusEffect->StatusId = StatusDefinition ? StatusDefinition->StatusId : FFinalStatusId();
		ApplyStatusEffect->Stacks = FMath::Max(Stacks, 1);
		ApplyStatusEffect->Notes = Notes;
		Effects.Add(ApplyStatusEffect);
		return ApplyStatusEffect;
	}

	UFinalBattleEffectRemoveStatus* AddRemoveStatusEffect(
		UObject* Owner,
		TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects,
		const FName EffectId,
		const EFinalBattleUnitTargetRule TargetRule,
		UFinalStatusDefinition* StatusDefinition,
		const int32 Stacks,
		const FText& Notes)
	{
		UFinalBattleEffectRemoveStatus* RemoveStatusEffect = NewObject<UFinalBattleEffectRemoveStatus>(Owner);
		RemoveStatusEffect->EffectId = EffectId;
		RemoveStatusEffect->UnitTargetRule = TargetRule;
		RemoveStatusEffect->StatusDefinition = StatusDefinition;
		RemoveStatusEffect->StatusId = StatusDefinition ? StatusDefinition->StatusId : FFinalStatusId();
		RemoveStatusEffect->Stacks = FMath::Max(Stacks, 1);
		RemoveStatusEffect->Notes = Notes;
		Effects.Add(RemoveStatusEffect);
		return RemoveStatusEffect;
	}

	UFinalBattleEffectGainAP* AddGainApEffect(
		UObject* Owner,
		TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects,
		const FName EffectId,
		const int32 GainValue,
		const FText& Notes)
	{
		UFinalBattleEffectGainAP* GainApEffect = NewObject<UFinalBattleEffectGainAP>(Owner);
		GainApEffect->EffectId = EffectId;
		GainApEffect->GainValue = FMath::Max(GainValue, 0);
		GainApEffect->Notes = Notes;
		Effects.Add(GainApEffect);
		return GainApEffect;
	}

	UFinalBattleEffectBonusBreak* AddBonusBreakEffect(
		UObject* Owner,
		TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects,
		const FName EffectId,
		const EFinalBattleUnitTargetRule TargetRule,
		const float BaseValue,
		const EFinalBattleScalarMode ScaleMode,
		const EFinalBattleSourceStat SourceStat,
		const FText& Notes)
	{
		UFinalBattleEffectBonusBreak* BonusBreakEffect = NewObject<UFinalBattleEffectBonusBreak>(Owner);
		BonusBreakEffect->EffectId = EffectId;
		BonusBreakEffect->UnitTargetRule = TargetRule;
		BonusBreakEffect->Scalar.BaseValue = BaseValue;
		BonusBreakEffect->Scalar.ScaleMode = ScaleMode;
		BonusBreakEffect->Scalar.SourceStat = SourceStat;
		BonusBreakEffect->Notes = Notes;
		Effects.Add(BonusBreakEffect);
		return BonusBreakEffect;
	}

	UFinalBattleEffectGenerateCard* AddGenerateCardEffect(
		UObject* Owner,
		TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects,
		const FName EffectId,
		UFinalCardDefinition* GeneratedCardDefinition,
		const TArray<UFinalCardDefinition*>& CandidateCardDefinitions,
		const int32 GenerateCount,
		const bool bChooseRandomCandidate,
		const FText& Notes)
	{
		UFinalBattleEffectGenerateCard* GenerateCardEffect = NewObject<UFinalBattleEffectGenerateCard>(Owner);
		GenerateCardEffect->EffectId = EffectId;
		GenerateCardEffect->GeneratedCardDefinition = GeneratedCardDefinition;
		GenerateCardEffect->GeneratedCardId = GeneratedCardDefinition != nullptr
			? GeneratedCardDefinition->CardId
			: FFinalCardId();
		GenerateCardEffect->CandidateCardDefinitions = CandidateCardDefinitions;
		GenerateCardEffect->GenerateCount = FMath::Max(GenerateCount, 1);
		GenerateCardEffect->bChooseRandomCandidate = bChooseRandomCandidate;
		GenerateCardEffect->bGeneratedCard = true;
		GenerateCardEffect->bTemporaryCard = true;
		GenerateCardEffect->Notes = Notes;
		Effects.Add(GenerateCardEffect);
		return GenerateCardEffect;
	}

	UFinalBattleEffectMoveCards* AddMoveCardsEffect(
		UObject* Owner,
		TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects,
		const FName EffectId,
		const EFinalBattleCardZoneRule SourceZone,
		const EFinalBattleCardZoneRule DestinationZone,
		const FFinalCardId& RequiredCardId,
		const FGameplayTag& RequiredKeyword,
		const int32 MoveCount,
		const bool bGeneratedOnly,
		const bool bRecordMovedCards,
		const FText& Notes)
	{
		UFinalBattleEffectMoveCards* MoveCardsEffect = NewObject<UFinalBattleEffectMoveCards>(Owner);
		MoveCardsEffect->EffectId = EffectId;
		MoveCardsEffect->SourceZone = SourceZone;
		MoveCardsEffect->DestinationZone = DestinationZone;
		MoveCardsEffect->RequiredCardId = RequiredCardId;
		MoveCardsEffect->RequiredKeyword = RequiredKeyword;
		MoveCardsEffect->MoveCount = FMath::Max(MoveCount, 1);
		MoveCardsEffect->bGeneratedOnly = bGeneratedOnly;
		MoveCardsEffect->bRecordMovedCards = bRecordMovedCards;
		MoveCardsEffect->Notes = Notes;
		Effects.Add(MoveCardsEffect);
		return MoveCardsEffect;
	}
}
