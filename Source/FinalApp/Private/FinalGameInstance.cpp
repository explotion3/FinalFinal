#include "App/FinalGameInstance.h"

#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalEnemyDefinition.h"
#include "Battle/Definitions/FinalEnemyIntentDefinition.h"
#include "Battle/Effects/FinalBattleEffectDamage.h"
#include "Battle/Effects/FinalBattleEffectDrawCards.h"
#include "Battle/Effects/FinalBattleEffectGainShield.h"
#include "Facade/FinalBattleSession.h"
#include "Facade/FinalRunSession.h"
#include "Queries/FinalDataRegistry.h"
#include "Run/Definitions/FinalRunNodeDefinition.h"
#include "Run/Definitions/FinalRunRouteDefinition.h"
#include "Run/Rewards/FinalRunRewardTypes.h"
#include "Runtime/FinalRunPersistentCharacterState.h"
#include "Subsystems/FinalGameFlowSubsystem.h"
#include "Subsystems/FinalRunFlowSubsystem.h"

#if __has_include("Run/Definitions/FinalRelicDefinition.h")
#include "Run/Definitions/FinalRelicDefinition.h"
#define FINALAPP_HAS_TEST_RELIC_DEFINITION 1
#else
#define FINALAPP_HAS_TEST_RELIC_DEFINITION 0
#endif

DEFINE_LOG_CATEGORY_STATIC(LogFinalGameInstance, Log, All);

namespace FinalTestBootstrap
{
	struct FPrototypeRunRouteBuildArgs
	{
		FName RouteId = NAME_None;
		FText RouteDisplayName;
		FName OpeningBattleNodeId = NAME_None;
		FName RewardNodeId = NAME_None;
		FName EventNodeId = NAME_None;
		FName ShopNodeId = NAME_None;
		FName FollowupBattleNodeId = NAME_None;
		FFinalEncounterId EncounterId;
		FFinalRuleConfigId RuleConfigId;
		FFinalCardId RewardRemovedCardId;
		FFinalCardId EventUpgradeFromCardId;
		FFinalCardId EventUpgradeToCardId;
		FFinalCardId ShopGrantedCardId;
		FFinalRelicId RewardGrantedRelicId;
		FFinalRelicId ShopGrantedRelicId;
		FFinalCharacterId GrowthTargetCharacterId;
		EFinalRunGrowthEffectType GrowthEffectType = EFinalRunGrowthEffectType::None;
		int32 GrowthValue = 0;
	};

	struct FResolvedPrototypeDefinitions
	{
		UFinalBattleRuleConfig* RuleConfig = nullptr;
		UFinalBattleEncounterDefinition* EncounterDefinition = nullptr;
		UFinalCharacterDefinition* GuardianDefinition = nullptr;
		UFinalCharacterDefinition* SupportDefinition = nullptr;
		UFinalCardDefinition* GuardianStrikeCard = nullptr;
		UFinalCardDefinition* GuardianGuardCard = nullptr;
		UFinalCardDefinition* SupportShotCard = nullptr;
		UFinalCardDefinition* SupportFocusCard = nullptr;
#if FINALAPP_HAS_TEST_RELIC_DEFINITION
		UFinalRelicDefinition* RewardCharmRelic = nullptr;
		UFinalRelicDefinition* ShopRepairKitRelic = nullptr;
#endif
		UFinalRunRouteDefinition* PrototypeRunRoute = nullptr;

		bool HasRequiredDefinitions() const
		{
			const bool bHasCoreDefinitions =
				RuleConfig != nullptr
				&& EncounterDefinition != nullptr
				&& GuardianDefinition != nullptr
				&& SupportDefinition != nullptr
				&& GuardianStrikeCard != nullptr
				&& GuardianGuardCard != nullptr
				&& SupportShotCard != nullptr
				&& SupportFocusCard != nullptr
				&& PrototypeRunRoute != nullptr;

#if FINALAPP_HAS_TEST_RELIC_DEFINITION
			return bHasCoreDefinitions && RewardCharmRelic != nullptr && ShopRepairKitRelic != nullptr;
#else
			return bHasCoreDefinitions;
#endif
		}
	};

	const FName RuleConfigId(TEXT("rule.test.bootstrap"));
	const FName EncounterId(TEXT("encounter.test.bootstrap"));
	const FName GuardianCharacterId(TEXT("character.test.guardian"));
	const FName SupportCharacterId(TEXT("character.test.support"));
	const FName EnemyId(TEXT("enemy.test.raider"));
	const FName GuardianStrikeCardId(TEXT("card.test.guardian.strike"));
	const FName GuardianGuardCardId(TEXT("card.test.guardian.guard"));
	const FName SupportShotCardId(TEXT("card.test.support.shot"));
	const FName SupportFocusCardId(TEXT("card.test.support.focus"));
	const FName RewardCharmRelicId(TEXT("relic.test.charm"));
	const FName ShopRepairKitRelicId(TEXT("relic.test.repair_kit"));
	const FName PhaseOneTag(TEXT("phase.one"));
	const FName PhaseTwoTag(TEXT("phase.two"));
	const FName OpeningBattleNodeId(TEXT("run.test.node.battle.opening"));
	const FName RewardNodeId(TEXT("run.test.node.reward.cache"));
	const FName EventNodeId(TEXT("run.test.node.event.crossroads"));
	const FName ShopNodeId(TEXT("run.test.node.shop.supply"));
	const FName FollowupBattleNodeId(TEXT("run.test.node.battle.followup"));
	const FName PrototypeRouteId(TEXT("run.route.test.prototype"));

	bool ResolvePrototypeDefinitionsFromRegistry(UFinalDataRegistry* DataRegistry, FResolvedPrototypeDefinitions& OutDefinitions)
	{
		if (DataRegistry == nullptr)
		{
			return false;
		}

		OutDefinitions.RuleConfig = DataRegistry->FindRuleConfig(FFinalRuleConfigId(RuleConfigId));
		OutDefinitions.EncounterDefinition = DataRegistry->FindEncounterDefinition(FFinalEncounterId(EncounterId));
		OutDefinitions.GuardianDefinition = DataRegistry->FindCharacterDefinition(FFinalCharacterId(GuardianCharacterId));
		OutDefinitions.SupportDefinition = DataRegistry->FindCharacterDefinition(FFinalCharacterId(SupportCharacterId));
		OutDefinitions.GuardianStrikeCard = DataRegistry->FindCardDefinition(FFinalCardId(GuardianStrikeCardId));
		OutDefinitions.GuardianGuardCard = DataRegistry->FindCardDefinition(FFinalCardId(GuardianGuardCardId));
		OutDefinitions.SupportShotCard = DataRegistry->FindCardDefinition(FFinalCardId(SupportShotCardId));
		OutDefinitions.SupportFocusCard = DataRegistry->FindCardDefinition(FFinalCardId(SupportFocusCardId));
#if FINALAPP_HAS_TEST_RELIC_DEFINITION
		OutDefinitions.RewardCharmRelic = DataRegistry->FindRelicDefinition(FFinalRelicId(RewardCharmRelicId));
		OutDefinitions.ShopRepairKitRelic = DataRegistry->FindRelicDefinition(FFinalRelicId(ShopRepairKitRelicId));
#endif
		OutDefinitions.PrototypeRunRoute = DataRegistry->FindRunRouteDefinition(PrototypeRouteId);
		return OutDefinitions.HasRequiredDefinitions();
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

	UFinalRunRouteDefinition* CreateTransientPrototypeRunRouteDefinition(
		UObject* Outer,
		const FPrototypeRunRouteBuildArgs& Args)
	{
		if (Outer == nullptr || Args.RouteId.IsNone() || Args.OpeningBattleNodeId.IsNone())
		{
			return nullptr;
		}

		UFinalRunRouteDefinition* RouteDefinition = NewObject<UFinalRunRouteDefinition>(Outer);
		RouteDefinition->RouteId = Args.RouteId;
		RouteDefinition->DisplayName = Args.RouteDisplayName;
		RouteDefinition->EntryNodeId = Args.OpeningBattleNodeId;

		FFinalRunNodeDefinition OpeningBattleNode;
		OpeningBattleNode.NodeId = Args.OpeningBattleNodeId;
		OpeningBattleNode.NodeType = EFinalRunNodeType::Battle;
		OpeningBattleNode.DisplayName = FText::FromString(TEXT("外环巡逻"));
		OpeningBattleNode.DisplayLabel = TEXT("RunNode.Test.OpeningBattle");
		OpeningBattleNode.ChapterIndex = 1;
		OpeningBattleNode.FloorIndex = 1;
		OpeningBattleNode.EncounterId = Args.EncounterId;
		OpeningBattleNode.RuleConfigId = Args.RuleConfigId;
		OpeningBattleNode.NextNodeIds.Add(Args.RewardNodeId);

		FFinalRunNodeDefinition RewardNode;
		RewardNode.NodeId = Args.RewardNodeId;
		RewardNode.NodeType = EFinalRunNodeType::Reward;
		RewardNode.DisplayName = FText::FromString(TEXT("战利品分拣"));
		RewardNode.DisplayLabel = TEXT("RunNode.Test.Reward");
		RewardNode.ChapterIndex = 1;
		RewardNode.FloorIndex = 2;
		RewardNode.NextNodeIds.Add(Args.EventNodeId);
		RewardNode.RewardContent.Title = FText::FromString(TEXT("战利品分拣"));
		RewardNode.RewardContent.Summary = FText::FromString(TEXT("用于验证奖励节点页。确认后会补入少量金币与一件试作遗物。"));
		RewardNode.RewardContent.RewardEntries.Add(MakeBaseRewardEntry(
			TEXT("reward.node.cache.gold"),
			EFinalRunRewardType::Gold,
			12,
			TEXT("Currency.Gold"),
			FText::FromString(TEXT("节点金币"))));
		RewardNode.RewardContent.RewardEntries.Add(MakeRelicRewardEntry(
			TEXT("reward.node.cache.relic"),
			Args.RewardGrantedRelicId,
			FText::FromString(TEXT("试作护符"))));
		RewardNode.RewardContent.RewardEntries.Add(MakeRemoveCardRewardEntry(
			TEXT("reward.node.cache.remove_guardian_strike"),
			Args.RewardRemovedCardId,
			FText::FromString(TEXT("移除一张试作斩击"))));

		FFinalRunNodeDefinition EventNode;
		EventNode.NodeId = Args.EventNodeId;
		EventNode.NodeType = EFinalRunNodeType::Event;
		EventNode.DisplayName = FText::FromString(TEXT("岔路告示"));
		EventNode.DisplayLabel = TEXT("RunNode.Test.Event");
		EventNode.ChapterIndex = 1;
		EventNode.FloorIndex = 3;
		EventNode.NextNodeIds.Add(Args.ShopNodeId);
		EventNode.EventContent.Title = FText::FromString(TEXT("岔路告示"));
		EventNode.EventContent.Summary = FText::FromString(TEXT("用于验证事件节点页。至少有一个可选项、一个禁用项，并带有结果摘要。"));

		FFinalRunEventOptionDefinition ReadNoticeOption;
		ReadNoticeOption.OptionId = TEXT("event.option.read_notice");
		ReadNoticeOption.DisplayText = FText::FromString(TEXT("查看告示"));
		ReadNoticeOption.OutcomeSummary = FText::FromString(TEXT("整理出一些情报，额外获得 6 金币。"));
		ReadNoticeOption.RewardEntries.Add(MakeBaseRewardEntry(
			TEXT("reward.event.notice.gold"),
			EFinalRunRewardType::Gold,
			6,
			TEXT("Currency.Gold"),
			FText::FromString(TEXT("情报赏金"))));
		ReadNoticeOption.RewardEntries.Add(MakeUpgradeCardRewardEntry(
			TEXT("reward.event.notice.upgrade_support_shot"),
			Args.EventUpgradeFromCardId,
			Args.EventUpgradeToCardId,
			FText::FromString(TEXT("试作速射 -> 试作整备"))));
		EventNode.EventContent.Options.Add(ReadNoticeOption);

		FFinalRunEventOptionDefinition ForceDoorOption;
		ForceDoorOption.OptionId = TEXT("event.option.force_door");
		ForceDoorOption.DisplayText = FText::FromString(TEXT("强行破门"));
		ForceDoorOption.OutcomeSummary = FText::FromString(TEXT("当前原型不开放这条支线。"));
		ForceDoorOption.bStartsDisabled = true;
		ForceDoorOption.DisabledReason = FText::FromString(TEXT("测试原型里暂未开放这条事件分支。"));
		EventNode.EventContent.Options.Add(ForceDoorOption);

		FFinalRunEventOptionDefinition TakeRestOption;
		TakeRestOption.OptionId = TEXT("event.option.take_rest");
		TakeRestOption.DisplayText = FText::FromString(TEXT("原地整备"));
		TakeRestOption.OutcomeSummary = FText::FromString(TEXT("测试策应压力 -1，用于验证最小 Growth 落地。"));
		TakeRestOption.RewardEntries.Add(MakeGrowthRewardEntry(
			TEXT("reward.event.take_rest.growth"),
			Args.GrowthTargetCharacterId,
			Args.GrowthEffectType,
			Args.GrowthValue,
			TEXT("Growth.Test.Support.ReduceStress"),
			FText::FromString(TEXT("测试策应·减压"))));
		EventNode.EventContent.Options.Add(TakeRestOption);

		FFinalRunNodeDefinition ShopNode;
		ShopNode.NodeId = Args.ShopNodeId;
		ShopNode.NodeType = EFinalRunNodeType::Shop;
		ShopNode.DisplayName = FText::FromString(TEXT("流动补给摊"));
		ShopNode.DisplayLabel = TEXT("RunNode.Test.Shop");
		ShopNode.ChapterIndex = 1;
		ShopNode.FloorIndex = 4;
		ShopNode.NextNodeIds.Add(Args.FollowupBattleNodeId);
		ShopNode.ShopContent.Title = FText::FromString(TEXT("流动补给摊"));
		ShopNode.ShopContent.Summary = FText::FromString(TEXT("用于验证商店节点页。准备了一件可买商品和一件当前买不起的商品。"));

		FFinalRunShopOfferDefinition RepairKitOffer;
		RepairKitOffer.OfferId = TEXT("shop.offer.repair_kit");
		RepairKitOffer.DisplayId = TEXT("Shop.Test.RepairKit");
		RepairKitOffer.DisplayName = FText::FromString(TEXT("试作修理包"));
		RepairKitOffer.Description = FText::FromString(TEXT("用于验证商店页的最小购买流。"));
		RepairKitOffer.Price = 10;
		RepairKitOffer.RewardEntries.Add(MakeRelicRewardEntry(
			TEXT("reward.shop.repair_kit"),
			Args.ShopGrantedRelicId,
			FText::FromString(TEXT("修理包"))));
		ShopNode.ShopContent.Offers.Add(RepairKitOffer);

		FFinalRunShopOfferDefinition PremiumBundleOffer;
		PremiumBundleOffer.OfferId = TEXT("shop.offer.premium_bundle");
		PremiumBundleOffer.DisplayId = TEXT("Shop.Test.PremiumBundle");
		PremiumBundleOffer.DisplayName = FText::FromString(TEXT("高价整备箱"));
		PremiumBundleOffer.Description = FText::FromString(TEXT("价格故意偏高，用于验证不可购买状态。"));
		PremiumBundleOffer.Price = 40;
		PremiumBundleOffer.RewardEntries.Add(MakeCardRewardEntry(
			TEXT("reward.shop.premium_bundle"),
			Args.ShopGrantedCardId,
			FText::FromString(TEXT("试作整备"))));
		ShopNode.ShopContent.Offers.Add(PremiumBundleOffer);

		FFinalRunNodeDefinition FollowupBattleNode;
		FollowupBattleNode.NodeId = Args.FollowupBattleNodeId;
		FollowupBattleNode.NodeType = EFinalRunNodeType::EliteBattle;
		FollowupBattleNode.DisplayName = FText::FromString(TEXT("巷战回响"));
		FollowupBattleNode.DisplayLabel = TEXT("RunNode.Test.FollowupBattle");
		FollowupBattleNode.ChapterIndex = 1;
		FollowupBattleNode.FloorIndex = 5;
		FollowupBattleNode.EncounterId = Args.EncounterId;
		FollowupBattleNode.RuleConfigId = Args.RuleConfigId;

		RouteDefinition->NodeDefinitions = {
			OpeningBattleNode,
			RewardNode,
			EventNode,
			ShopNode,
			FollowupBattleNode
		};

		return RouteDefinition;
	}

	UFinalBattleRuleConfig* RegisterTransientTestRuleConfig(
		UFinalDataRegistry* DataRegistry,
		UObject* Outer,
		TArray<TObjectPtr<UObject>>& RuntimeAssets)
	{
		if (DataRegistry == nullptr || Outer == nullptr)
		{
			return nullptr;
		}

		UFinalBattleRuleConfig* RuleConfig = NewObject<UFinalBattleRuleConfig>(Outer, TEXT("DA_TestBattleRuleConfig"));
		RuleConfig->RuleConfigId = FFinalRuleConfigId(RuleConfigId);
		RuleConfig->InitialAP = 3;
		RuleConfig->InitialHandSize = 5;
		RuleConfig->HandLimit = 10;
		RuleConfig->MaxEP = 70;
		RuleConfig->EndTurnEpGain = 3;
		RuleConfig->OnHitEpGain = 4;
		RuleConfig->BaseCardEpGain = 1;
		RuleConfig->BreakRewardAP = 1;
		RuleConfig->NormalCardInitiativeEventCount = 1;
		RuleConfig->CollapsedCardInitiativeEventCount = 1;
		RuleConfig->StressHpLossPerPoint = 5;
		RuleConfig->StressHealPerPoint = 8;
		RuleConfig->MinStressChangePerEvent = 1;
		RuleConfig->MaxStressGainPerHit = 3;
		RuleConfig->StressRandomProtectionCount = 2;
		RuleConfig->DamageToBreakCap = 6;
		RuntimeAssets.Add(RuleConfig);
		DataRegistry->RegisterRuleConfig(RuleConfig);
		return RuleConfig;
	}

	UFinalCharacterDefinition* RegisterTransientTestCharacterDefinition(
		UFinalDataRegistry* DataRegistry,
		UObject* Outer,
		TArray<TObjectPtr<UObject>>& RuntimeAssets,
		const TCHAR* ObjectName,
		const FFinalCharacterId& CharacterId,
		const FText& DisplayName,
		const int32 BaseVitalShare,
		const int32 BaseStressCap,
		const int32 BaseAttack,
		const int32 BaseDefense,
		const float BaseBreakRate,
		const float BaseCritChance,
		const float BaseCritDamage,
		const int32 EpGainPerAP)
	{
		if (DataRegistry == nullptr || Outer == nullptr || !CharacterId.IsValid())
		{
			return nullptr;
		}

		UFinalCharacterDefinition* CharacterDefinition = NewObject<UFinalCharacterDefinition>(Outer, ObjectName);
		CharacterDefinition->CharacterId = CharacterId;
		CharacterDefinition->DisplayName = DisplayName;
		CharacterDefinition->BaseVitalShare = BaseVitalShare;
		CharacterDefinition->BaseStressCap = BaseStressCap;
		CharacterDefinition->BaseAttack = BaseAttack;
		CharacterDefinition->BaseDefense = BaseDefense;
		CharacterDefinition->BaseBreakRate = BaseBreakRate;
		CharacterDefinition->BaseCritChance = BaseCritChance;
		CharacterDefinition->BaseCritDamage = BaseCritDamage;
		CharacterDefinition->EpGainPerAP = EpGainPerAP;
		RuntimeAssets.Add(CharacterDefinition);
		DataRegistry->RegisterCharacterDefinition(CharacterDefinition);
		return CharacterDefinition;
	}

	UFinalCardDefinition* RegisterTransientTestGuardianStrikeCard(
		UFinalDataRegistry* DataRegistry,
		UObject* Outer,
		TArray<TObjectPtr<UObject>>& RuntimeAssets,
		UFinalCharacterDefinition* OwnerCharacterDefinition)
	{
		if (DataRegistry == nullptr || Outer == nullptr || OwnerCharacterDefinition == nullptr)
		{
			return nullptr;
		}

		UFinalCardDefinition* CardDefinition = NewObject<UFinalCardDefinition>(Outer, TEXT("DA_TestGuardianStrikeCard"));
		CardDefinition->CardId = FFinalCardId(GuardianStrikeCardId);
		CardDefinition->OwnerUnitId = OwnerCharacterDefinition->CharacterId.Value;
		CardDefinition->DisplayName = FText::FromString(TEXT("试作斩击"));
		CardDefinition->CardType = EFinalCardType::Attack;
		CardDefinition->Rarity = EFinalRarity::Common;
		CardDefinition->BaseCostAP = 1;
		CardDefinition->RulesText = FText::FromString(TEXT("测试用普通攻击牌。"));
		UFinalBattleEffectDamage* DamageEffect = NewObject<UFinalBattleEffectDamage>(CardDefinition);
		DamageEffect->EffectId = TEXT("effect.test.guardian.strike.damage");
		DamageEffect->Scalar.BaseValue = 1.0f;
		DamageEffect->Scalar.ScaleMode = EFinalBattleScalarMode::SourceStatMultiplier;
		DamageEffect->Scalar.SourceStat = EFinalBattleSourceStat::Attack;
		CardDefinition->Effects.Add(DamageEffect);
		RuntimeAssets.Add(CardDefinition);
		DataRegistry->RegisterCardDefinition(CardDefinition);
		return CardDefinition;
	}

	UFinalCardDefinition* RegisterTransientTestGuardianGuardCard(
		UFinalDataRegistry* DataRegistry,
		UObject* Outer,
		TArray<TObjectPtr<UObject>>& RuntimeAssets,
		UFinalCharacterDefinition* OwnerCharacterDefinition)
	{
		if (DataRegistry == nullptr || Outer == nullptr || OwnerCharacterDefinition == nullptr)
		{
			return nullptr;
		}

		UFinalCardDefinition* CardDefinition = NewObject<UFinalCardDefinition>(Outer, TEXT("DA_TestGuardianGuardCard"));
		CardDefinition->CardId = FFinalCardId(GuardianGuardCardId);
		CardDefinition->OwnerUnitId = OwnerCharacterDefinition->CharacterId.Value;
		CardDefinition->DisplayName = FText::FromString(TEXT("试作格挡"));
		CardDefinition->CardType = EFinalCardType::Skill;
		CardDefinition->Rarity = EFinalRarity::Common;
		CardDefinition->BaseCostAP = 1;
		CardDefinition->RulesText = FText::FromString(TEXT("测试用防御牌。"));
		UFinalBattleEffectGainShield* ShieldEffect = NewObject<UFinalBattleEffectGainShield>(CardDefinition);
		ShieldEffect->EffectId = TEXT("effect.test.guardian.guard.shield");
		ShieldEffect->Scalar.BaseValue = 1.0f;
		ShieldEffect->Scalar.ScaleMode = EFinalBattleScalarMode::SourceStatMultiplier;
		ShieldEffect->Scalar.SourceStat = EFinalBattleSourceStat::Defense;
		CardDefinition->Effects.Add(ShieldEffect);
		RuntimeAssets.Add(CardDefinition);
		DataRegistry->RegisterCardDefinition(CardDefinition);
		return CardDefinition;
	}

	UFinalCardDefinition* RegisterTransientTestSupportShotCard(
		UFinalDataRegistry* DataRegistry,
		UObject* Outer,
		TArray<TObjectPtr<UObject>>& RuntimeAssets,
		UFinalCharacterDefinition* OwnerCharacterDefinition)
	{
		if (DataRegistry == nullptr || Outer == nullptr || OwnerCharacterDefinition == nullptr)
		{
			return nullptr;
		}

		UFinalCardDefinition* CardDefinition = NewObject<UFinalCardDefinition>(Outer, TEXT("DA_TestSupportShotCard"));
		CardDefinition->CardId = FFinalCardId(SupportShotCardId);
		CardDefinition->OwnerUnitId = OwnerCharacterDefinition->CharacterId.Value;
		CardDefinition->DisplayName = FText::FromString(TEXT("试作速射"));
		CardDefinition->CardType = EFinalCardType::Attack;
		CardDefinition->Rarity = EFinalRarity::Common;
		CardDefinition->BaseCostAP = 1;
		CardDefinition->RulesText = FText::FromString(TEXT("测试用远程攻击牌。"));
		UFinalBattleEffectDamage* DamageEffect = NewObject<UFinalBattleEffectDamage>(CardDefinition);
		DamageEffect->EffectId = TEXT("effect.test.support.shot.damage");
		DamageEffect->Scalar.BaseValue = 1.0f;
		DamageEffect->Scalar.ScaleMode = EFinalBattleScalarMode::SourceStatMultiplier;
		DamageEffect->Scalar.SourceStat = EFinalBattleSourceStat::Attack;
		CardDefinition->Effects.Add(DamageEffect);
		RuntimeAssets.Add(CardDefinition);
		DataRegistry->RegisterCardDefinition(CardDefinition);
		return CardDefinition;
	}

	UFinalCardDefinition* RegisterTransientTestSupportFocusCard(
		UFinalDataRegistry* DataRegistry,
		UObject* Outer,
		TArray<TObjectPtr<UObject>>& RuntimeAssets,
		UFinalCharacterDefinition* OwnerCharacterDefinition)
	{
		if (DataRegistry == nullptr || Outer == nullptr || OwnerCharacterDefinition == nullptr)
		{
			return nullptr;
		}

		UFinalCardDefinition* CardDefinition = NewObject<UFinalCardDefinition>(Outer, TEXT("DA_TestSupportFocusCard"));
		CardDefinition->CardId = FFinalCardId(SupportFocusCardId);
		CardDefinition->OwnerUnitId = OwnerCharacterDefinition->CharacterId.Value;
		CardDefinition->DisplayName = FText::FromString(TEXT("试作整备"));
		CardDefinition->CardType = EFinalCardType::Skill;
		CardDefinition->Rarity = EFinalRarity::Common;
		CardDefinition->BaseCostAP = 1;
		CardDefinition->RulesText = FText::FromString(TEXT("测试用辅助牌。"));
		UFinalBattleEffectDrawCards* DrawEffect = NewObject<UFinalBattleEffectDrawCards>(CardDefinition);
		DrawEffect->EffectId = TEXT("effect.test.support.focus.draw");
		DrawEffect->DrawCount = 2;
		CardDefinition->Effects.Add(DrawEffect);
		RuntimeAssets.Add(CardDefinition);
		DataRegistry->RegisterCardDefinition(CardDefinition);
		return CardDefinition;
	}

	UFinalEnemyDefinition* ResolveOrRegisterTransientTestEnemyDefinition(
		UFinalDataRegistry* DataRegistry,
		UObject* Outer,
		TArray<TObjectPtr<UObject>>& RuntimeAssets)
	{
		if (DataRegistry == nullptr || Outer == nullptr)
		{
			return nullptr;
		}

		if (UFinalEnemyDefinition* EnemyDefinition = DataRegistry->FindEnemyDefinition(FFinalEnemyId(EnemyId)))
		{
			return EnemyDefinition;
		}

		UFinalEnemyIntentDefinition* AttackIntent = DataRegistry->FindEnemyIntentDefinition(TEXT("intent.test.enemy.attack"));
		if (AttackIntent == nullptr)
		{
			AttackIntent = NewObject<UFinalEnemyIntentDefinition>(Outer, TEXT("DA_TestEnemyAttackIntent"));
			AttackIntent->IntentId = TEXT("intent.test.enemy.attack");
			AttackIntent->DisplayName = FText::FromString(TEXT("试作劈砍"));
			AttackIntent->IntentType = EFinalIntentType::Attack;
			AttackIntent->PreviewText = FText::FromString(TEXT("劈砍 6"));
			AttackIntent->PhaseTags.Add(PhaseOneTag);
			UFinalBattleEffectDamage* AttackEffect = NewObject<UFinalBattleEffectDamage>(AttackIntent);
			AttackEffect->EffectId = TEXT("effect.test.enemy.attack.damage");
			AttackEffect->UnitTargetRule = EFinalBattleUnitTargetRule::TeamPlayer;
			AttackEffect->Scalar.BaseValue = 1.0f;
			AttackEffect->Scalar.ScaleMode = EFinalBattleScalarMode::SourceStatMultiplier;
			AttackEffect->Scalar.SourceStat = EFinalBattleSourceStat::BaseDamagePower;
			AttackIntent->Effects.Add(AttackEffect);
			RuntimeAssets.Add(AttackIntent);
			DataRegistry->RegisterEnemyIntentDefinition(AttackIntent);
		}

		UFinalEnemyIntentDefinition* GuardIntent = DataRegistry->FindEnemyIntentDefinition(TEXT("intent.test.enemy.guard"));
		if (GuardIntent == nullptr)
		{
			GuardIntent = NewObject<UFinalEnemyIntentDefinition>(Outer, TEXT("DA_TestEnemyGuardIntent"));
			GuardIntent->IntentId = TEXT("intent.test.enemy.guard");
			GuardIntent->DisplayName = FText::FromString(TEXT("试作整备"));
			GuardIntent->IntentType = EFinalIntentType::Defense;
			GuardIntent->PreviewText = FText::FromString(TEXT("获得 4 护盾"));
			GuardIntent->PhaseTags.Add(PhaseOneTag);
			GuardIntent->CooldownTurns = 1;
			GuardIntent->UseLimitPerBattle = 2;
			UFinalBattleEffectGainShield* GuardEffect = NewObject<UFinalBattleEffectGainShield>(GuardIntent);
			GuardEffect->EffectId = TEXT("effect.test.enemy.guard.shield");
			GuardEffect->UnitTargetRule = EFinalBattleUnitTargetRule::Self;
			GuardEffect->Scalar.BaseValue = 4.0f;
			GuardEffect->Scalar.ScaleMode = EFinalBattleScalarMode::Flat;
			GuardIntent->Effects.Add(GuardEffect);
			RuntimeAssets.Add(GuardIntent);
			DataRegistry->RegisterEnemyIntentDefinition(GuardIntent);
		}

		UFinalEnemyIntentDefinition* EnrageIntent = DataRegistry->FindEnemyIntentDefinition(TEXT("intent.test.enemy.enrage"));
		if (EnrageIntent == nullptr)
		{
			EnrageIntent = NewObject<UFinalEnemyIntentDefinition>(Outer, TEXT("DA_TestEnemyEnrageIntent"));
			EnrageIntent->IntentId = TEXT("intent.test.enemy.enrage");
			EnrageIntent->DisplayName = FText::FromString(TEXT("试作狂斩"));
			EnrageIntent->IntentType = EFinalIntentType::Attack;
			EnrageIntent->PreviewText = FText::FromString(TEXT("狂斩 10"));
			EnrageIntent->PhaseTags.Add(PhaseTwoTag);
			UFinalBattleEffectDamage* EnrageEffect = NewObject<UFinalBattleEffectDamage>(EnrageIntent);
			EnrageEffect->EffectId = TEXT("effect.test.enemy.enrage.damage");
			EnrageEffect->UnitTargetRule = EFinalBattleUnitTargetRule::TeamPlayer;
			EnrageEffect->Scalar.BaseValue = 10.0f;
			EnrageEffect->Scalar.ScaleMode = EFinalBattleScalarMode::Flat;
			EnrageIntent->Effects.Add(EnrageEffect);
			RuntimeAssets.Add(EnrageIntent);
			DataRegistry->RegisterEnemyIntentDefinition(EnrageIntent);
		}

		UFinalEnemyDefinition* EnemyDefinition = NewObject<UFinalEnemyDefinition>(Outer, TEXT("DA_TestEnemyDefinition"));
		EnemyDefinition->EnemyId = FFinalEnemyId(EnemyId);
		EnemyDefinition->DisplayName = FText::FromString(TEXT("测试劫匪"));
		EnemyDefinition->MaxHP = 36;
		EnemyDefinition->MaxBreakValue = 12;
		EnemyDefinition->BaseDamagePower = 6;
		EnemyDefinition->InitialInitiativeValue = 2;
		EnemyDefinition->InitiativeResponse = 1;
		EnemyDefinition->IntentSelectRule = EFinalIntentSelectRule::PhaseSequence;

		FFinalEnemyPhaseDefinition& PhaseOneDefinition = EnemyDefinition->PhaseSequence.AddDefaulted_GetRef();
		PhaseOneDefinition.PhaseTag = PhaseOneTag;
		PhaseOneDefinition.MaxHpPercent = 1.0f;

		FFinalEnemyPhaseDefinition& PhaseTwoDefinition = EnemyDefinition->PhaseSequence.AddDefaulted_GetRef();
		PhaseTwoDefinition.PhaseTag = PhaseTwoTag;
		PhaseTwoDefinition.MaxHpPercent = 0.5f;

		EnemyDefinition->IntentPool.Add(AttackIntent);
		EnemyDefinition->IntentPool.Add(GuardIntent);
		EnemyDefinition->IntentPool.Add(EnrageIntent);
		RuntimeAssets.Add(EnemyDefinition);
		DataRegistry->RegisterEnemyDefinition(EnemyDefinition);
		return EnemyDefinition;
	}

	UFinalBattleEncounterDefinition* RegisterTransientTestEncounterDefinition(
		UFinalDataRegistry* DataRegistry,
		UObject* Outer,
		TArray<TObjectPtr<UObject>>& RuntimeAssets,
		UFinalBattleRuleConfig* RuleConfig,
		UFinalEnemyDefinition* EnemyDefinition)
	{
		if (DataRegistry == nullptr || Outer == nullptr || RuleConfig == nullptr || EnemyDefinition == nullptr)
		{
			return nullptr;
		}

		UFinalBattleEncounterDefinition* EncounterDefinition = NewObject<UFinalBattleEncounterDefinition>(Outer, TEXT("DA_TestEncounterDefinition"));
		EncounterDefinition->EncounterId = FFinalEncounterId(EncounterId);
		EncounterDefinition->DisplayName = FText::FromString(TEXT("测试遭遇"));
		EncounterDefinition->RuleConfig = RuleConfig;

		FFinalEnemyRosterEntry EnemyRosterEntry;
		EnemyRosterEntry.EnemyDefinition = EnemyDefinition;
		EnemyRosterEntry.PositionIndex = 0;
		EnemyRosterEntry.SpawnWave = 1;
		EncounterDefinition->EnemyRoster.Add(EnemyRosterEntry);
		RuntimeAssets.Add(EncounterDefinition);
		DataRegistry->RegisterEncounterDefinition(EncounterDefinition);
		return EncounterDefinition;
	}

#if FINALAPP_HAS_TEST_RELIC_DEFINITION
	void RegisterTransientTestRelicDefinition(
		UFinalDataRegistry* DataRegistry,
		UObject* Outer,
		TArray<TObjectPtr<UObject>>& RuntimeAssets,
		const FFinalRelicId& RelicId,
		const FName DisplayId,
		const FText& DisplayName,
		const TArray<FFinalRelicBattleStartEffectDefinition>& BattleStartEffects,
		const TArray<FFinalRelicPlayerTurnStartEffectDefinition>& PlayerTurnStartEffects)
	{
		if (DataRegistry == nullptr || !RelicId.IsValid())
		{
			return;
		}

		UFinalRelicDefinition* RelicDefinition = NewObject<UFinalRelicDefinition>(Outer);
		RelicDefinition->RelicId = RelicId;
		RelicDefinition->DisplayId = DisplayId;
		RelicDefinition->DisplayName = DisplayName;
		RelicDefinition->BattleStartEffects = BattleStartEffects;
		RelicDefinition->PlayerTurnStartEffects = PlayerTurnStartEffects;
		RuntimeAssets.Add(RelicDefinition);
		DataRegistry->RegisterRelicDefinition(RelicDefinition);
	}
#endif

	void EnsureTransientPrototypeFallbackDefinitions(
		UFinalDataRegistry* DataRegistry,
		UObject* Outer,
		TArray<TObjectPtr<UObject>>& RuntimeAssets,
		FResolvedPrototypeDefinitions& InOutDefinitions)
	{
		if (DataRegistry == nullptr || Outer == nullptr)
		{
			return;
		}

		if (InOutDefinitions.RuleConfig == nullptr)
		{
			InOutDefinitions.RuleConfig = RegisterTransientTestRuleConfig(DataRegistry, Outer, RuntimeAssets);
		}

		if (InOutDefinitions.GuardianDefinition == nullptr)
		{
			InOutDefinitions.GuardianDefinition = RegisterTransientTestCharacterDefinition(
				DataRegistry,
				Outer,
				RuntimeAssets,
				TEXT("DA_TestGuardianCharacter"),
				FFinalCharacterId(GuardianCharacterId),
				FText::FromString(TEXT("测试先锋")),
				24,
				12,
				7,
				3,
				1.2f,
				0.05f,
				1.5f,
				1);
		}

		if (InOutDefinitions.SupportDefinition == nullptr)
		{
			InOutDefinitions.SupportDefinition = RegisterTransientTestCharacterDefinition(
				DataRegistry,
				Outer,
				RuntimeAssets,
				TEXT("DA_TestSupportCharacter"),
				FFinalCharacterId(SupportCharacterId),
				FText::FromString(TEXT("测试策应")),
				18,
				14,
				5,
				2,
				1.0f,
				0.08f,
				1.5f,
				1);
		}

		if (InOutDefinitions.GuardianStrikeCard == nullptr)
		{
			InOutDefinitions.GuardianStrikeCard = RegisterTransientTestGuardianStrikeCard(
				DataRegistry,
				Outer,
				RuntimeAssets,
				InOutDefinitions.GuardianDefinition);
		}

		if (InOutDefinitions.GuardianGuardCard == nullptr)
		{
			InOutDefinitions.GuardianGuardCard = RegisterTransientTestGuardianGuardCard(
				DataRegistry,
				Outer,
				RuntimeAssets,
				InOutDefinitions.GuardianDefinition);
		}

		if (InOutDefinitions.SupportShotCard == nullptr)
		{
			InOutDefinitions.SupportShotCard = RegisterTransientTestSupportShotCard(
				DataRegistry,
				Outer,
				RuntimeAssets,
				InOutDefinitions.SupportDefinition);
		}

		if (InOutDefinitions.SupportFocusCard == nullptr)
		{
			InOutDefinitions.SupportFocusCard = RegisterTransientTestSupportFocusCard(
				DataRegistry,
				Outer,
				RuntimeAssets,
				InOutDefinitions.SupportDefinition);
		}

		if (InOutDefinitions.EncounterDefinition == nullptr)
		{
			UFinalEnemyDefinition* EnemyDefinition = ResolveOrRegisterTransientTestEnemyDefinition(
				DataRegistry,
				Outer,
				RuntimeAssets);
			InOutDefinitions.EncounterDefinition = RegisterTransientTestEncounterDefinition(
				DataRegistry,
				Outer,
				RuntimeAssets,
				InOutDefinitions.RuleConfig,
				EnemyDefinition);
		}

#if FINALAPP_HAS_TEST_RELIC_DEFINITION
		TArray<FFinalRelicBattleStartEffectDefinition> CharmBattleStartEffects;
		{
			FFinalRelicBattleStartEffectDefinition Effect;
			Effect.EffectType = EFinalRelicBattleStartEffectType::GainAP;
			Effect.Value = 1;
			CharmBattleStartEffects.Add(Effect);
		}

		TArray<FFinalRelicBattleStartEffectDefinition> RepairKitBattleStartEffects;
		{
			FFinalRelicBattleStartEffectDefinition Effect;
			Effect.EffectType = EFinalRelicBattleStartEffectType::GainShield;
			Effect.Value = 4;
			RepairKitBattleStartEffects.Add(Effect);
		}

		TArray<FFinalRelicPlayerTurnStartEffectDefinition> CharmTurnStartEffects;
		{
			FFinalRelicPlayerTurnStartEffectDefinition Effect;
			Effect.EffectType = EFinalRelicPlayerTurnStartEffectType::GainAP;
			Effect.Value = 1;
			CharmTurnStartEffects.Add(Effect);
		}

		TArray<FFinalRelicPlayerTurnStartEffectDefinition> RepairKitTurnStartEffects;
		{
			FFinalRelicPlayerTurnStartEffectDefinition Effect;
			Effect.EffectType = EFinalRelicPlayerTurnStartEffectType::GainShield;
			Effect.Value = 2;
			RepairKitTurnStartEffects.Add(Effect);
		}

		if (InOutDefinitions.RewardCharmRelic == nullptr)
		{
			RegisterTransientTestRelicDefinition(
				DataRegistry,
				Outer,
				RuntimeAssets,
				FFinalRelicId(RewardCharmRelicId),
				TEXT("Relic.Test.Charm"),
				FText::FromString(TEXT("试作护符")),
				CharmBattleStartEffects,
				CharmTurnStartEffects);
			InOutDefinitions.RewardCharmRelic = DataRegistry->FindRelicDefinition(FFinalRelicId(RewardCharmRelicId));
		}

		if (InOutDefinitions.ShopRepairKitRelic == nullptr)
		{
			RegisterTransientTestRelicDefinition(
				DataRegistry,
				Outer,
				RuntimeAssets,
				FFinalRelicId(ShopRepairKitRelicId),
				TEXT("Relic.Test.RepairKit"),
				FText::FromString(TEXT("试作修理包")),
				RepairKitBattleStartEffects,
				RepairKitTurnStartEffects);
			InOutDefinitions.ShopRepairKitRelic = DataRegistry->FindRelicDefinition(FFinalRelicId(ShopRepairKitRelicId));
		}
#endif

		if (InOutDefinitions.PrototypeRunRoute == nullptr)
		{
			FPrototypeRunRouteBuildArgs PrototypeRouteArgs;
			PrototypeRouteArgs.RouteId = PrototypeRouteId;
			PrototypeRouteArgs.RouteDisplayName = FText::FromString(TEXT("测试战斗外环"));
			PrototypeRouteArgs.OpeningBattleNodeId = OpeningBattleNodeId;
			PrototypeRouteArgs.RewardNodeId = RewardNodeId;
			PrototypeRouteArgs.EventNodeId = EventNodeId;
			PrototypeRouteArgs.ShopNodeId = ShopNodeId;
			PrototypeRouteArgs.FollowupBattleNodeId = FollowupBattleNodeId;
			PrototypeRouteArgs.EncounterId = InOutDefinitions.EncounterDefinition != nullptr ? InOutDefinitions.EncounterDefinition->EncounterId : FFinalEncounterId(EncounterId);
			PrototypeRouteArgs.RuleConfigId = InOutDefinitions.RuleConfig != nullptr ? InOutDefinitions.RuleConfig->RuleConfigId : FFinalRuleConfigId(RuleConfigId);
			PrototypeRouteArgs.RewardRemovedCardId = InOutDefinitions.GuardianStrikeCard != nullptr ? InOutDefinitions.GuardianStrikeCard->CardId : FFinalCardId(GuardianStrikeCardId);
			PrototypeRouteArgs.EventUpgradeFromCardId = InOutDefinitions.SupportShotCard != nullptr ? InOutDefinitions.SupportShotCard->CardId : FFinalCardId(SupportShotCardId);
			PrototypeRouteArgs.EventUpgradeToCardId = InOutDefinitions.SupportFocusCard != nullptr ? InOutDefinitions.SupportFocusCard->CardId : FFinalCardId(SupportFocusCardId);
			PrototypeRouteArgs.ShopGrantedCardId = InOutDefinitions.SupportFocusCard != nullptr ? InOutDefinitions.SupportFocusCard->CardId : FFinalCardId(SupportFocusCardId);
			PrototypeRouteArgs.RewardGrantedRelicId = FFinalRelicId(RewardCharmRelicId);
			PrototypeRouteArgs.ShopGrantedRelicId = FFinalRelicId(ShopRepairKitRelicId);
			PrototypeRouteArgs.GrowthTargetCharacterId = InOutDefinitions.SupportDefinition != nullptr ? InOutDefinitions.SupportDefinition->CharacterId : FFinalCharacterId(SupportCharacterId);
			PrototypeRouteArgs.GrowthEffectType = EFinalRunGrowthEffectType::ReduceStress;
			PrototypeRouteArgs.GrowthValue = 1;

			InOutDefinitions.PrototypeRunRoute = CreateTransientPrototypeRunRouteDefinition(Outer, PrototypeRouteArgs);
			if (InOutDefinitions.PrototypeRunRoute != nullptr)
			{
				RuntimeAssets.Add(InOutDefinitions.PrototypeRunRoute);
				DataRegistry->RegisterRunRouteDefinition(InOutDefinitions.PrototypeRunRoute);
			}
		}
	}
}

void UFinalGameInstance::Init()
{
	Super::Init();
	EnsureTestBattleBootstrapData();
}

bool UFinalGameInstance::EnsureTestBattleBootstrapData()
{
	LastTestFailureReason = FText::GetEmpty();

	if (bTestBattleBootstrapRegistered)
	{
		return true;
	}

	UFinalDataRegistry* DataRegistry = GetSubsystem<UFinalDataRegistry>();
	if (DataRegistry == nullptr)
	{
		LastTestFailureReason = FText::FromString(TEXT("FinalDataRegistry is unavailable."));
		return false;
	}

	RuntimeTestAssets.Reset();

	FinalTestBootstrap::FResolvedPrototypeDefinitions ResolvedDefinitions;
	if (!FinalTestBootstrap::ResolvePrototypeDefinitionsFromRegistry(DataRegistry, ResolvedDefinitions))
	{
		FinalTestBootstrap::EnsureTransientPrototypeFallbackDefinitions(
			DataRegistry,
			this,
			RuntimeTestAssets,
			ResolvedDefinitions);
	}

	if (!ResolvedDefinitions.HasRequiredDefinitions())
	{
		LastTestFailureReason = FText::FromString(TEXT("Failed to resolve prototype runtime definitions from FinalDataRegistry or transient fallback."));
		return false;
	}

	TestRuleConfig = ResolvedDefinitions.RuleConfig;
	TestEncounterDefinition = ResolvedDefinitions.EncounterDefinition;
	TestGuardianDefinition = ResolvedDefinitions.GuardianDefinition;
	TestSupportDefinition = ResolvedDefinitions.SupportDefinition;
	TestGuardianStrikeCard = ResolvedDefinitions.GuardianStrikeCard;
	TestGuardianGuardCard = ResolvedDefinitions.GuardianGuardCard;
	TestSupportShotCard = ResolvedDefinitions.SupportShotCard;
	TestSupportFocusCard = ResolvedDefinitions.SupportFocusCard;
	TestPrototypeRunRoute = ResolvedDefinitions.PrototypeRunRoute;

	bTestBattleBootstrapRegistered = true;

	UE_LOG(
		LogFinalGameInstance,
		Log,
		TEXT("Resolved test battle bootstrap data. TransientFallbackCount=%d"),
		RuntimeTestAssets.Num());
	return true;
}

bool UFinalGameInstance::PrepareTestBattleRun()
{
	LastTestFailureReason = FText::GetEmpty();

	if (!EnsureTestBattleBootstrapData())
	{
		return false;
	}

	UFinalGameFlowSubsystem* GameFlowSubsystem = GetSubsystem<UFinalGameFlowSubsystem>();
	if (GameFlowSubsystem == nullptr)
	{
		LastTestFailureReason = FText::FromString(TEXT("FinalGameFlowSubsystem is unavailable."));
		return false;
	}

	UFinalRunSession* RunSession = GameFlowSubsystem->BootstrapNewRun();
	if (RunSession == nullptr)
	{
		LastTestFailureReason = FText::FromString(TEXT("Failed to bootstrap a RunSession."));
		return false;
	}

	TArray<FFinalRunPersistentCharacterState> PartyStates;

	FFinalRunPersistentCharacterState GuardianState;
	GuardianState.CharacterId = TestGuardianDefinition->CharacterId;
	GuardianState.CurrentStress = 0;
	GuardianState.bCollapsed = false;
	PartyStates.Add(GuardianState);

	FFinalRunPersistentCharacterState SupportState;
	SupportState.CharacterId = TestSupportDefinition->CharacterId;
	SupportState.CurrentStress = 1;
	SupportState.bCollapsed = false;
	PartyStates.Add(SupportState);

	TArray<FFinalCardId> DeckCardIds;
	DeckCardIds.Append({
		TestGuardianStrikeCard->CardId,
		TestGuardianGuardCard->CardId,
		TestSupportFocusCard->CardId,
		TestSupportShotCard->CardId,
		TestGuardianStrikeCard->CardId,
		TestSupportShotCard->CardId,
		TestGuardianGuardCard->CardId
	});

	const int32 TeamCurrentHP = TestGuardianDefinition->BaseVitalShare + TestSupportDefinition->BaseVitalShare;

	RunSession->ConfigureBattleStartState(
		TestEncounterDefinition->EncounterId,
		TestRuleConfig->RuleConfigId,
		PartyStates,
		DeckCardIds,
		TeamCurrentHP);
	if (!RunSession->ConfigureRunRouteById(FinalTestBootstrap::PrototypeRouteId))
	{
		LastTestFailureReason = FText::Format(
			NSLOCTEXT("FinalGameInstance", "ConfigurePrototypeRunRouteFailed", "Failed to configure prototype run route {0}."),
			FText::FromName(FinalTestBootstrap::PrototypeRouteId));
		return false;
	}

	if (UFinalRunFlowSubsystem* RunFlowSubsystem = GetSubsystem<UFinalRunFlowSubsystem>())
	{
		RunFlowSubsystem->RefreshRunFlow(true);
	}

	return true;
}

bool UFinalGameInstance::StartTestBattle()
{
	LastTestFailureReason = FText::GetEmpty();

	if (!PrepareTestBattleRun())
	{
		return false;
	}

	UFinalGameFlowSubsystem* GameFlowSubsystem = GetSubsystem<UFinalGameFlowSubsystem>();
	if (GameFlowSubsystem == nullptr)
	{
		LastTestFailureReason = FText::FromString(TEXT("FinalGameFlowSubsystem is unavailable."));
		return false;
	}

	if (GameFlowSubsystem->GetActiveBattleSession() == nullptr)
	{
		GameFlowSubsystem->TryAutoStartPreparedBattleFromRun();
	}

	if (GameFlowSubsystem->GetActiveBattleSession() == nullptr)
	{
		LastTestFailureReason = GameFlowSubsystem->GetLastBattleFailureReason();
		if (LastTestFailureReason.IsEmpty())
		{
			LastTestFailureReason = FText::FromString(TEXT("Failed to auto-start the prepared test battle run."));
		}
		return false;
	}

	return true;
}

FText UFinalGameInstance::GetLastTestFailureReason() const
{
	return LastTestFailureReason;
}
