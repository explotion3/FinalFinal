#include "Bootstrap/FinalPrototypeContentBootstrapCommandlet.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalEnemyDefinition.h"
#include "Battle/Definitions/FinalEnemyIntentDefinition.h"
#include "Battle/Definitions/FinalStatusDefinition.h"
#include "Battle/Definitions/FinalUltimateDefinition.h"
#include "Battle/Effects/FinalBattleEffectDamage.h"
#include "Battle/Effects/FinalBattleEffectDrawCards.h"
#include "Battle/Effects/FinalBattleEffectGainShield.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Run/Definitions/FinalPrototypeBootstrapDefinition.h"
#include "Run/Definitions/FinalRelicDefinition.h"
#include "Run/Definitions/FinalRunNodeDefinition.h"
#include "Run/Definitions/FinalRunRouteDefinition.h"
#include "Run/Rewards/FinalRunRewardTypes.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY_STATIC(LogFinalPrototypeContentBootstrap, Log, All);

namespace FinalPrototypeContentBootstrap
{
	const FString RootPath(TEXT("/Game/Prototype/Definitions"));
	const FString RulesPath = RootPath / TEXT("Rules/DA_Rule_TestBootstrap");
	const FString GuardianCharacterPath = RootPath / TEXT("Characters/DA_Character_TestGuardian");
	const FString SupportCharacterPath = RootPath / TEXT("Characters/DA_Character_TestSupport");
	const FString GuardianStrikeCardPath = RootPath / TEXT("Cards/DA_Card_TestGuardianStrike");
	const FString GuardianGuardCardPath = RootPath / TEXT("Cards/DA_Card_TestGuardianGuard");
	const FString SupportShotCardPath = RootPath / TEXT("Cards/DA_Card_TestSupportShot");
	const FString SupportFocusCardPath = RootPath / TEXT("Cards/DA_Card_TestSupportFocus");
	const FString GuardianStatusPath = RootPath / TEXT("Statuses/DA_Status_TestGuardianSignature");
	const FString SupportStatusPath = RootPath / TEXT("Statuses/DA_Status_TestSupportSignature");
	const FString GuardianUltimatePath = RootPath / TEXT("Ultimates/DA_Ultimate_TestGuardian");
	const FString SupportUltimatePath = RootPath / TEXT("Ultimates/DA_Ultimate_TestSupport");
	const FString EnemyAttackIntentPath = RootPath / TEXT("EnemyIntents/DA_EnemyIntent_TestAttack");
	const FString EnemyGuardIntentPath = RootPath / TEXT("EnemyIntents/DA_EnemyIntent_TestGuard");
	const FString EnemyEnrageIntentPath = RootPath / TEXT("EnemyIntents/DA_EnemyIntent_TestEnrage");
	const FString EnemyPath = RootPath / TEXT("Enemies/DA_Enemy_TestRaider");
	const FString EncounterPath = RootPath / TEXT("Encounters/DA_Encounter_TestBootstrap");
	const FString PrototypeBootstrapPath = RootPath / TEXT("Bootstrap/DA_PrototypeBootstrap_Test");
	const FString CharmRelicPath = RootPath / TEXT("Relics/DA_Relic_TestCharm");
	const FString RepairKitRelicPath = RootPath / TEXT("Relics/DA_Relic_TestRepairKit");
	const FString PrototypeRunRoutePath = RootPath / TEXT("Run/DA_RunRoute_TestPrototype");
	const FString StarterRootPath = RootPath / TEXT("Starter");
	const FString StarterRulesPath = StarterRootPath / TEXT("Rules/DA_Rule_StarterChapter1");
	const FString StarterHuoCharacterPath = StarterRootPath / TEXT("Characters/DA_Character_Starter_HuoDuanyue");
	const FString StarterYeCharacterPath = StarterRootPath / TEXT("Characters/DA_Character_Starter_YeBanxia");
	const FString StarterShenCharacterPath = StarterRootPath / TEXT("Characters/DA_Character_Starter_ShenQingxian");
	const FString StarterHuoLieFengCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_HuoLieFeng");
	const FString StarterHuoWenJiaCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_HuoWenJia");
	const FString StarterHuoDuanYueZhanCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_HuoDuanYueZhan");
	const FString StarterHuoTieBiHuiFengCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_HuoTieBiHuiFeng");
	const FString StarterYeXingZhenCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_YeXingZhen");
	const FString StarterYeTiaoXiCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_YeTiaoXi");
	const FString StarterYeHuaYinCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_YeHuaYin");
	const FString StarterYeHuiChunSanCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_YeHuiChunSan");
	const FString StarterShenBuFengCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_ShenBuFeng");
	const FString StarterShenShouZhenCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_ShenShouZhen");
	const FString StarterShenYinZhenCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_ShenYinZhen");
	const FString StarterShenGuoPaiJianZhenCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_ShenGuoPaiJianZhen");
	const FString StarterHuoStatusPath = StarterRootPath / TEXT("Statuses/DA_Status_Starter_HuoDaoShi");
	const FString StarterYeStatusPath = StarterRootPath / TEXT("Statuses/DA_Status_Starter_YeYaoYin");
	const FString StarterShenStatusPath = StarterRootPath / TEXT("Statuses/DA_Status_Starter_ShenJianZhen");
	const FString StarterHuoUltimatePath = StarterRootPath / TEXT("Ultimates/DA_Ultimate_Starter_HuoDuanyue");
	const FString StarterYeUltimatePath = StarterRootPath / TEXT("Ultimates/DA_Ultimate_Starter_YeBanxia");
	const FString StarterShenUltimatePath = StarterRootPath / TEXT("Ultimates/DA_Ultimate_Starter_ShenQingxian");
	const FString StarterBladeQuickSlashIntentPath = StarterRootPath / TEXT("EnemyIntents/DA_EnemyIntent_Starter_BladeQuickSlash");
	const FString StarterBladeDoubleSlashIntentPath = StarterRootPath / TEXT("EnemyIntents/DA_EnemyIntent_Starter_BladeDoubleSlash");
	const FString StarterBladeBraceIntentPath = StarterRootPath / TEXT("EnemyIntents/DA_EnemyIntent_Starter_BladeBrace");
	const FString StarterCrossbowPiercingBoltIntentPath = StarterRootPath / TEXT("EnemyIntents/DA_EnemyIntent_Starter_CrossbowPiercingBolt");
	const FString StarterCrossbowVolleyIntentPath = StarterRootPath / TEXT("EnemyIntents/DA_EnemyIntent_Starter_CrossbowVolley");
	const FString StarterCrossbowRepositionIntentPath = StarterRootPath / TEXT("EnemyIntents/DA_EnemyIntent_Starter_CrossbowReposition");
	const FString StarterInstructorCommandSlashIntentPath = StarterRootPath / TEXT("EnemyIntents/DA_EnemyIntent_Starter_InstructorCommandSlash");
	const FString StarterInstructorHoldLineIntentPath = StarterRootPath / TEXT("EnemyIntents/DA_EnemyIntent_Starter_InstructorHoldLine");
	const FString StarterInstructorHeavyCleaveIntentPath = StarterRootPath / TEXT("EnemyIntents/DA_EnemyIntent_Starter_InstructorHeavyCleave");
	const FString StarterBladeEnemyPath = StarterRootPath / TEXT("Enemies/DA_Enemy_Starter_BanditBlade");
	const FString StarterCrossbowEnemyPath = StarterRootPath / TEXT("Enemies/DA_Enemy_Starter_BanditCrossbow");
	const FString StarterInstructorEnemyPath = StarterRootPath / TEXT("Enemies/DA_Enemy_Starter_BlackwindInstructor");
	const FString StarterNormalEncounterPath = StarterRootPath / TEXT("Encounters/DA_Encounter_Starter_Roadblock");
	const FString StarterEliteEncounterPath = StarterRootPath / TEXT("Encounters/DA_Encounter_Starter_Instructor");
	const FString StarterBootstrapPath = StarterRootPath / TEXT("Bootstrap/DA_PrototypeBootstrap_StarterChapter1");
	const FString StarterRunRoutePath = StarterRootPath / TEXT("Run/DA_RunRoute_StarterChapter1");

	const FName PrototypeBootstrapId(TEXT("prototype.bootstrap.test"));
	const FName RuleConfigId(TEXT("rule.test.bootstrap"));
	const FName EncounterId(TEXT("encounter.test.bootstrap"));
	const FName GuardianCharacterId(TEXT("character.test.guardian"));
	const FName SupportCharacterId(TEXT("character.test.support"));
	const FName GuardianStatusId(TEXT("status.test.guardian.signature"));
	const FName SupportStatusId(TEXT("status.test.support.signature"));
	const FName GuardianUltimateId(TEXT("ultimate.test.guardian"));
	const FName SupportUltimateId(TEXT("ultimate.test.support"));
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
	const FName StarterBootstrapId(TEXT("prototype.bootstrap.starter.chapter1"));
	const FName StarterRuleConfigId(TEXT("rule.starter.chapter1"));
	const FName StarterNormalEncounterId(TEXT("encounter.starter.chapter1.roadblock"));
	const FName StarterEliteEncounterId(TEXT("encounter.starter.chapter1.instructor"));
	const FName StarterHuoCharacterId(TEXT("character.starter.huo.duanyue"));
	const FName StarterYeCharacterId(TEXT("character.starter.ye.banxia"));
	const FName StarterShenCharacterId(TEXT("character.starter.shen.qingxian"));
	const FName StarterHuoStatusId(TEXT("status.starter.huo.daoshi"));
	const FName StarterYeStatusId(TEXT("status.starter.ye.yaoyin"));
	const FName StarterShenStatusId(TEXT("status.starter.shen.jianzhen"));
	const FName StarterHuoUltimateId(TEXT("ultimate.starter.huo.duanyuejueshi"));
	const FName StarterYeUltimateId(TEXT("ultimate.starter.ye.huitianxumai"));
	const FName StarterShenUltimateId(TEXT("ultimate.starter.shen.wanxiangguizhen"));
	const FName StarterHuoLieFengCardId(TEXT("card.starter.huo.liefeng"));
	const FName StarterHuoWenJiaCardId(TEXT("card.starter.huo.wenjia"));
	const FName StarterHuoDuanYueZhanCardId(TEXT("card.starter.huo.duanyuezhan"));
	const FName StarterHuoTieBiHuiFengCardId(TEXT("card.starter.huo.tiebihuifeng"));
	const FName StarterYeXingZhenCardId(TEXT("card.starter.ye.xingzhen"));
	const FName StarterYeTiaoXiCardId(TEXT("card.starter.ye.tiaoxi"));
	const FName StarterYeHuaYinCardId(TEXT("card.starter.ye.huayin"));
	const FName StarterYeHuiChunSanCardId(TEXT("card.starter.ye.huichunsan"));
	const FName StarterShenBuFengCardId(TEXT("card.starter.shen.bufeng"));
	const FName StarterShenShouZhenCardId(TEXT("card.starter.shen.shouzhen"));
	const FName StarterShenYinZhenCardId(TEXT("card.starter.shen.yinzhen"));
	const FName StarterShenGuoPaiJianZhenCardId(TEXT("card.starter.shen.guopaijianzhen"));
	const FName StarterBladeEnemyId(TEXT("enemy.starter.bandit.blade"));
	const FName StarterCrossbowEnemyId(TEXT("enemy.starter.bandit.crossbow"));
	const FName StarterInstructorEnemyId(TEXT("enemy.starter.blackwind.instructor"));
	const FName StarterRouteId(TEXT("run.route.starter.chapter1"));
	const FName StarterOpeningBattleNodeId(TEXT("run.starter.node.battle.roadblock"));
	const FName StarterRewardNodeId(TEXT("run.starter.node.reward.spoils"));
	const FName StarterEventNodeId(TEXT("run.starter.node.event.cliff_notice"));
	const FName StarterShopNodeId(TEXT("run.starter.node.shop.camp_trader"));
	const FName StarterEliteBattleNodeId(TEXT("run.starter.node.battle.instructor"));

	template<typename TAsset>
	TAsset* LoadOrCreateAsset(const FString& PackagePath, bool& bOutCreated)
	{
		const FString AssetName = FPackageName::GetLongPackageAssetName(PackagePath);
		const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackagePath, *AssetName);
		if (TAsset* LoadedAsset = LoadObject<TAsset>(nullptr, *ObjectPath))
		{
			bOutCreated = false;
			return LoadedAsset;
		}

		UPackage* Package = CreatePackage(*PackagePath);
		TAsset* Asset = NewObject<TAsset>(Package, TAsset::StaticClass(), *AssetName, RF_Public | RF_Standalone);
		FAssetRegistryModule::AssetCreated(Asset);
		bOutCreated = true;
		return Asset;
	}

	void TrackPackage(UObject* Asset, TSet<UPackage*>& OutPackages)
	{
		if (Asset == nullptr)
		{
			return;
		}

		if (UPackage* Package = Asset->GetOutermost())
		{
			Package->MarkPackageDirty();
			OutPackages.Add(Package);
		}
	}

	void SavePackages(const TSet<UPackage*>& PackagesToSave)
	{
		for (UPackage* Package : PackagesToSave)
		{
			if (Package == nullptr)
			{
				continue;
			}

			const FString PackageName = Package->GetName();
			const FString PackageFilename = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
			const FString FullPackageFilename = FPaths::ConvertRelativePathToFull(PackageFilename);
			IFileManager::Get().MakeDirectory(*FPaths::GetPath(FullPackageFilename), true);

			FSavePackageArgs SaveArgs;
			SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
			SaveArgs.SaveFlags = SAVE_NoError;

			UPackage::SavePackage(Package, nullptr, *FullPackageFilename, SaveArgs);
		}
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

	UFinalBattleEffectDamage* AddDamageEffect(
		UObject* Owner,
		TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects,
		const FName EffectId,
		const EFinalBattleUnitTargetRule TargetRule,
		const float BaseValue,
		const EFinalBattleScalarMode ScaleMode,
		const EFinalBattleSourceStat SourceStat,
		const int32 HitCount = 1,
		const FText& Notes = FText::GetEmpty())
	{
		UFinalBattleEffectDamage* DamageEffect = NewObject<UFinalBattleEffectDamage>(Owner);
		DamageEffect->EffectId = EffectId;
		DamageEffect->UnitTargetRule = TargetRule;
		DamageEffect->Scalar.BaseValue = BaseValue;
		DamageEffect->Scalar.ScaleMode = ScaleMode;
		DamageEffect->Scalar.SourceStat = SourceStat;
		DamageEffect->HitCount = HitCount;
		DamageEffect->Notes = Notes;
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
		const EFinalBattleSourceStat SourceStat = EFinalBattleSourceStat::None,
		const FText& Notes = FText::GetEmpty())
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
		const FText& Notes = FText::GetEmpty())
	{
		UFinalBattleEffectDrawCards* DrawEffect = NewObject<UFinalBattleEffectDrawCards>(Owner);
		DrawEffect->EffectId = EffectId;
		DrawEffect->DrawCount = DrawCount;
		DrawEffect->Notes = Notes;
		Effects.Add(DrawEffect);
		return DrawEffect;
	}
}

UFinalPrototypeContentBootstrapCommandlet::UFinalPrototypeContentBootstrapCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
}

int32 UFinalPrototypeContentBootstrapCommandlet::Main(const FString& Params)
{
	using namespace FinalPrototypeContentBootstrap;

	TSet<UPackage*> PackagesToSave;

	bool bCreatedAsset = false;
	UFinalBattleRuleConfig* RuleConfig = LoadOrCreateAsset<UFinalBattleRuleConfig>(RulesPath, bCreatedAsset);
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
	TrackPackage(RuleConfig, PackagesToSave);

	UFinalStatusDefinition* GuardianStatus = LoadOrCreateAsset<UFinalStatusDefinition>(GuardianStatusPath, bCreatedAsset);
	GuardianStatus->StatusId = FFinalStatusId(GuardianStatusId);
	GuardianStatus->DisplayName = FText::FromString(TEXT("先锋印记"));
	GuardianStatus->SummaryText = FText::FromString(TEXT("测试先锋的签名状态占位。"));
	GuardianStatus->MaxStacks = 1;
	GuardianStatus->DefaultDuration = 0;
	GuardianStatus->OnTickEffects.Reset();
	TrackPackage(GuardianStatus, PackagesToSave);

	UFinalStatusDefinition* SupportStatus = LoadOrCreateAsset<UFinalStatusDefinition>(SupportStatusPath, bCreatedAsset);
	SupportStatus->StatusId = FFinalStatusId(SupportStatusId);
	SupportStatus->DisplayName = FText::FromString(TEXT("策应印记"));
	SupportStatus->SummaryText = FText::FromString(TEXT("测试策应的签名状态占位。"));
	SupportStatus->MaxStacks = 1;
	SupportStatus->DefaultDuration = 0;
	SupportStatus->OnTickEffects.Reset();
	TrackPackage(SupportStatus, PackagesToSave);

	UFinalUltimateDefinition* GuardianUltimate = LoadOrCreateAsset<UFinalUltimateDefinition>(GuardianUltimatePath, bCreatedAsset);
	GuardianUltimate->UltimateId = FFinalUltimateId(GuardianUltimateId);
	GuardianUltimate->OwnerUnitId = GuardianCharacterId;
	GuardianUltimate->DisplayName = FText::FromString(TEXT("试作突袭"));
	GuardianUltimate->BaseCostEP = 45;
	GuardianUltimate->RulesText = FText::FromString(TEXT("对单体造成高额伤害。"));
	GuardianUltimate->Effects.Reset();
	{
		UFinalBattleEffectDamage* DamageEffect = NewObject<UFinalBattleEffectDamage>(GuardianUltimate);
		DamageEffect->EffectId = TEXT("effect.test.guardian.ultimate.damage");
		DamageEffect->Scalar.BaseValue = 2.0f;
		DamageEffect->Scalar.ScaleMode = EFinalBattleScalarMode::SourceStatMultiplier;
		DamageEffect->Scalar.SourceStat = EFinalBattleSourceStat::Attack;
		GuardianUltimate->Effects.Add(DamageEffect);
	}
	TrackPackage(GuardianUltimate, PackagesToSave);

	UFinalUltimateDefinition* SupportUltimate = LoadOrCreateAsset<UFinalUltimateDefinition>(SupportUltimatePath, bCreatedAsset);
	SupportUltimate->UltimateId = FFinalUltimateId(SupportUltimateId);
	SupportUltimate->OwnerUnitId = SupportCharacterId;
	SupportUltimate->DisplayName = FText::FromString(TEXT("试作重整"));
	SupportUltimate->BaseCostEP = 45;
	SupportUltimate->RulesText = FText::FromString(TEXT("抽两张牌，维持节奏。"));
	SupportUltimate->Effects.Reset();
	{
		UFinalBattleEffectDrawCards* DrawEffect = NewObject<UFinalBattleEffectDrawCards>(SupportUltimate);
		DrawEffect->EffectId = TEXT("effect.test.support.ultimate.draw");
		DrawEffect->DrawCount = 2;
		SupportUltimate->Effects.Add(DrawEffect);
	}
	TrackPackage(SupportUltimate, PackagesToSave);

	UFinalCharacterDefinition* GuardianCharacter = LoadOrCreateAsset<UFinalCharacterDefinition>(GuardianCharacterPath, bCreatedAsset);
	GuardianCharacter->CharacterId = FFinalCharacterId(GuardianCharacterId);
	GuardianCharacter->DisplayName = FText::FromString(TEXT("测试先锋"));
	GuardianCharacter->BaseVitalShare = 24;
	GuardianCharacter->BaseStressCap = 12;
	GuardianCharacter->BaseAttack = 7;
	GuardianCharacter->BaseDefense = 3;
	GuardianCharacter->BaseBreakRate = 1.2f;
	GuardianCharacter->BaseCritChance = 0.05f;
	GuardianCharacter->BaseCritDamage = 1.5f;
	GuardianCharacter->EpGainPerAP = 1;
	GuardianCharacter->UltimateId = GuardianUltimate->UltimateId;
	GuardianCharacter->SignatureStatusId = GuardianStatus->StatusId;
	TrackPackage(GuardianCharacter, PackagesToSave);

	UFinalCharacterDefinition* SupportCharacter = LoadOrCreateAsset<UFinalCharacterDefinition>(SupportCharacterPath, bCreatedAsset);
	SupportCharacter->CharacterId = FFinalCharacterId(SupportCharacterId);
	SupportCharacter->DisplayName = FText::FromString(TEXT("测试策应"));
	SupportCharacter->BaseVitalShare = 18;
	SupportCharacter->BaseStressCap = 14;
	SupportCharacter->BaseAttack = 5;
	SupportCharacter->BaseDefense = 2;
	SupportCharacter->BaseBreakRate = 1.0f;
	SupportCharacter->BaseCritChance = 0.08f;
	SupportCharacter->BaseCritDamage = 1.5f;
	SupportCharacter->EpGainPerAP = 1;
	SupportCharacter->UltimateId = SupportUltimate->UltimateId;
	SupportCharacter->SignatureStatusId = SupportStatus->StatusId;
	TrackPackage(SupportCharacter, PackagesToSave);

	UFinalCardDefinition* GuardianStrikeCard = LoadOrCreateAsset<UFinalCardDefinition>(GuardianStrikeCardPath, bCreatedAsset);
	GuardianStrikeCard->CardId = FFinalCardId(GuardianStrikeCardId);
	GuardianStrikeCard->OwnerUnitId = GuardianCharacterId;
	GuardianStrikeCard->DisplayName = FText::FromString(TEXT("试作斩击"));
	GuardianStrikeCard->CardType = EFinalCardType::Attack;
	GuardianStrikeCard->Rarity = EFinalRarity::Common;
	GuardianStrikeCard->BaseCostAP = 1;
	GuardianStrikeCard->RulesText = FText::FromString(TEXT("测试用普通攻击牌。"));
	GuardianStrikeCard->Effects.Reset();
	{
		UFinalBattleEffectDamage* DamageEffect = NewObject<UFinalBattleEffectDamage>(GuardianStrikeCard);
		DamageEffect->EffectId = TEXT("effect.test.guardian.strike.damage");
		DamageEffect->Scalar.BaseValue = 1.0f;
		DamageEffect->Scalar.ScaleMode = EFinalBattleScalarMode::SourceStatMultiplier;
		DamageEffect->Scalar.SourceStat = EFinalBattleSourceStat::Attack;
		GuardianStrikeCard->Effects.Add(DamageEffect);
	}
	TrackPackage(GuardianStrikeCard, PackagesToSave);

	UFinalCardDefinition* GuardianGuardCard = LoadOrCreateAsset<UFinalCardDefinition>(GuardianGuardCardPath, bCreatedAsset);
	GuardianGuardCard->CardId = FFinalCardId(GuardianGuardCardId);
	GuardianGuardCard->OwnerUnitId = GuardianCharacterId;
	GuardianGuardCard->DisplayName = FText::FromString(TEXT("试作格挡"));
	GuardianGuardCard->CardType = EFinalCardType::Skill;
	GuardianGuardCard->Rarity = EFinalRarity::Common;
	GuardianGuardCard->BaseCostAP = 1;
	GuardianGuardCard->RulesText = FText::FromString(TEXT("测试用防御牌。"));
	GuardianGuardCard->Effects.Reset();
	{
		UFinalBattleEffectGainShield* ShieldEffect = NewObject<UFinalBattleEffectGainShield>(GuardianGuardCard);
		ShieldEffect->EffectId = TEXT("effect.test.guardian.guard.shield");
		ShieldEffect->Scalar.BaseValue = 1.0f;
		ShieldEffect->Scalar.ScaleMode = EFinalBattleScalarMode::SourceStatMultiplier;
		ShieldEffect->Scalar.SourceStat = EFinalBattleSourceStat::Defense;
		GuardianGuardCard->Effects.Add(ShieldEffect);
	}
	TrackPackage(GuardianGuardCard, PackagesToSave);

	UFinalCardDefinition* SupportShotCard = LoadOrCreateAsset<UFinalCardDefinition>(SupportShotCardPath, bCreatedAsset);
	SupportShotCard->CardId = FFinalCardId(SupportShotCardId);
	SupportShotCard->OwnerUnitId = SupportCharacterId;
	SupportShotCard->DisplayName = FText::FromString(TEXT("试作速射"));
	SupportShotCard->CardType = EFinalCardType::Attack;
	SupportShotCard->Rarity = EFinalRarity::Common;
	SupportShotCard->BaseCostAP = 1;
	SupportShotCard->RulesText = FText::FromString(TEXT("测试用远程攻击牌。"));
	SupportShotCard->Effects.Reset();
	{
		UFinalBattleEffectDamage* DamageEffect = NewObject<UFinalBattleEffectDamage>(SupportShotCard);
		DamageEffect->EffectId = TEXT("effect.test.support.shot.damage");
		DamageEffect->Scalar.BaseValue = 1.0f;
		DamageEffect->Scalar.ScaleMode = EFinalBattleScalarMode::SourceStatMultiplier;
		DamageEffect->Scalar.SourceStat = EFinalBattleSourceStat::Attack;
		SupportShotCard->Effects.Add(DamageEffect);
	}
	TrackPackage(SupportShotCard, PackagesToSave);

	UFinalCardDefinition* SupportFocusCard = LoadOrCreateAsset<UFinalCardDefinition>(SupportFocusCardPath, bCreatedAsset);
	SupportFocusCard->CardId = FFinalCardId(SupportFocusCardId);
	SupportFocusCard->OwnerUnitId = SupportCharacterId;
	SupportFocusCard->DisplayName = FText::FromString(TEXT("试作整备"));
	SupportFocusCard->CardType = EFinalCardType::Skill;
	SupportFocusCard->Rarity = EFinalRarity::Common;
	SupportFocusCard->BaseCostAP = 1;
	SupportFocusCard->RulesText = FText::FromString(TEXT("测试用辅助牌。"));
	SupportFocusCard->Effects.Reset();
	{
		UFinalBattleEffectDrawCards* DrawEffect = NewObject<UFinalBattleEffectDrawCards>(SupportFocusCard);
		DrawEffect->EffectId = TEXT("effect.test.support.focus.draw");
		DrawEffect->DrawCount = 2;
		SupportFocusCard->Effects.Add(DrawEffect);
	}
	TrackPackage(SupportFocusCard, PackagesToSave);

	GuardianCharacter->InitialLoadoutCards = {
		{ GuardianStrikeCard->CardId, 2, EFinalLoadoutRole::BaseAttack },
		{ GuardianGuardCard->CardId, 2, EFinalLoadoutRole::BaseDefense }
	};
	GuardianCharacter->CharacterCardPoolIds = {
		GuardianStrikeCard->CardId,
		GuardianGuardCard->CardId
	};
	TrackPackage(GuardianCharacter, PackagesToSave);

	SupportCharacter->InitialLoadoutCards = {
		{ SupportShotCard->CardId, 2, EFinalLoadoutRole::BaseAttack },
		{ SupportFocusCard->CardId, 2, EFinalLoadoutRole::BaseTactic }
	};
	SupportCharacter->CharacterCardPoolIds = {
		SupportShotCard->CardId,
		SupportFocusCard->CardId
	};
	TrackPackage(SupportCharacter, PackagesToSave);

	UFinalEnemyIntentDefinition* AttackIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(EnemyAttackIntentPath, bCreatedAsset);
	AttackIntent->IntentId = TEXT("intent.test.enemy.attack");
	AttackIntent->DisplayName = FText::FromString(TEXT("试作劈砍"));
	AttackIntent->IntentType = EFinalIntentType::Attack;
	AttackIntent->PreviewText = FText::FromString(TEXT("劈砍 6"));
	AttackIntent->PhaseTags = { PhaseOneTag };
	AttackIntent->CooldownTurns = 0;
	AttackIntent->UseLimitPerBattle = 0;
	AttackIntent->Effects.Reset();
	{
		UFinalBattleEffectDamage* AttackEffect = NewObject<UFinalBattleEffectDamage>(AttackIntent);
		AttackEffect->EffectId = TEXT("effect.test.enemy.attack.damage");
		AttackEffect->UnitTargetRule = EFinalBattleUnitTargetRule::TeamPlayer;
		AttackEffect->Scalar.BaseValue = 1.0f;
		AttackEffect->Scalar.ScaleMode = EFinalBattleScalarMode::SourceStatMultiplier;
		AttackEffect->Scalar.SourceStat = EFinalBattleSourceStat::BaseDamagePower;
		AttackIntent->Effects.Add(AttackEffect);
	}
	TrackPackage(AttackIntent, PackagesToSave);

	UFinalEnemyIntentDefinition* GuardIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(EnemyGuardIntentPath, bCreatedAsset);
	GuardIntent->IntentId = TEXT("intent.test.enemy.guard");
	GuardIntent->DisplayName = FText::FromString(TEXT("试作整备"));
	GuardIntent->IntentType = EFinalIntentType::Defense;
	GuardIntent->PreviewText = FText::FromString(TEXT("获得 4 护盾"));
	GuardIntent->PhaseTags = { PhaseOneTag };
	GuardIntent->CooldownTurns = 1;
	GuardIntent->UseLimitPerBattle = 2;
	GuardIntent->Effects.Reset();
	{
		UFinalBattleEffectGainShield* GuardEffect = NewObject<UFinalBattleEffectGainShield>(GuardIntent);
		GuardEffect->EffectId = TEXT("effect.test.enemy.guard.shield");
		GuardEffect->UnitTargetRule = EFinalBattleUnitTargetRule::Self;
		GuardEffect->Scalar.BaseValue = 4.0f;
		GuardEffect->Scalar.ScaleMode = EFinalBattleScalarMode::Flat;
		GuardIntent->Effects.Add(GuardEffect);
	}
	TrackPackage(GuardIntent, PackagesToSave);

	UFinalEnemyIntentDefinition* EnrageIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(EnemyEnrageIntentPath, bCreatedAsset);
	EnrageIntent->IntentId = TEXT("intent.test.enemy.enrage");
	EnrageIntent->DisplayName = FText::FromString(TEXT("试作狂斩"));
	EnrageIntent->IntentType = EFinalIntentType::Attack;
	EnrageIntent->PreviewText = FText::FromString(TEXT("狂斩 10"));
	EnrageIntent->PhaseTags = { PhaseTwoTag };
	EnrageIntent->CooldownTurns = 0;
	EnrageIntent->UseLimitPerBattle = 0;
	EnrageIntent->Effects.Reset();
	{
		UFinalBattleEffectDamage* EnrageEffect = NewObject<UFinalBattleEffectDamage>(EnrageIntent);
		EnrageEffect->EffectId = TEXT("effect.test.enemy.enrage.damage");
		EnrageEffect->UnitTargetRule = EFinalBattleUnitTargetRule::TeamPlayer;
		EnrageEffect->Scalar.BaseValue = 10.0f;
		EnrageEffect->Scalar.ScaleMode = EFinalBattleScalarMode::Flat;
		EnrageIntent->Effects.Add(EnrageEffect);
	}
	TrackPackage(EnrageIntent, PackagesToSave);

	UFinalEnemyDefinition* EnemyDefinition = LoadOrCreateAsset<UFinalEnemyDefinition>(EnemyPath, bCreatedAsset);
	EnemyDefinition->EnemyId = FFinalEnemyId(EnemyId);
	EnemyDefinition->DisplayName = FText::FromString(TEXT("测试劫匪"));
	EnemyDefinition->MaxHP = 36;
	EnemyDefinition->MaxBreakValue = 12;
	EnemyDefinition->BaseDamagePower = 6;
	EnemyDefinition->InitialInitiativeValue = 2;
	EnemyDefinition->InitiativeResponse = 1;
	EnemyDefinition->IntentSelectRule = EFinalIntentSelectRule::PhaseSequence;
	EnemyDefinition->PhaseSequence.Reset();
	{
		FFinalEnemyPhaseDefinition& PhaseOneDefinition = EnemyDefinition->PhaseSequence.AddDefaulted_GetRef();
		PhaseOneDefinition.PhaseTag = PhaseOneTag;
		PhaseOneDefinition.MaxHpPercent = 1.0f;

		FFinalEnemyPhaseDefinition& PhaseTwoDefinition = EnemyDefinition->PhaseSequence.AddDefaulted_GetRef();
		PhaseTwoDefinition.PhaseTag = PhaseTwoTag;
		PhaseTwoDefinition.MaxHpPercent = 0.5f;
	}
	EnemyDefinition->IntentPool = {
		AttackIntent,
		GuardIntent,
		EnrageIntent
	};
	TrackPackage(EnemyDefinition, PackagesToSave);

	UFinalBattleEncounterDefinition* EncounterDefinition = LoadOrCreateAsset<UFinalBattleEncounterDefinition>(EncounterPath, bCreatedAsset);
	EncounterDefinition->EncounterId = FFinalEncounterId(EncounterId);
	EncounterDefinition->DisplayName = FText::FromString(TEXT("测试遭遇"));
	EncounterDefinition->RuleConfig = RuleConfig;
	EncounterDefinition->EnemyRoster.Reset();
	{
		FFinalEnemyRosterEntry EnemyRosterEntry;
		EnemyRosterEntry.EnemyDefinition = EnemyDefinition;
		EnemyRosterEntry.PositionIndex = 0;
		EnemyRosterEntry.SpawnWave = 1;
		EncounterDefinition->EnemyRoster.Add(EnemyRosterEntry);
	}
	TrackPackage(EncounterDefinition, PackagesToSave);

	UFinalRelicDefinition* CharmRelic = LoadOrCreateAsset<UFinalRelicDefinition>(CharmRelicPath, bCreatedAsset);
	CharmRelic->RelicId = FFinalRelicId(RewardCharmRelicId);
	CharmRelic->DisplayId = TEXT("Relic.Test.Charm");
	CharmRelic->DisplayName = FText::FromString(TEXT("试作护符"));
	CharmRelic->Description = FText::FromString(TEXT("战斗开始与玩家回合开始额外获得 1 AP。"));
	CharmRelic->BattleStartEffects = {
		{ EFinalRelicBattleStartEffectType::GainAP, 1 }
	};
	CharmRelic->PlayerTurnStartEffects = {
		{ EFinalRelicPlayerTurnStartEffectType::GainAP, 1 }
	};
	TrackPackage(CharmRelic, PackagesToSave);

	UFinalRelicDefinition* RepairKitRelic = LoadOrCreateAsset<UFinalRelicDefinition>(RepairKitRelicPath, bCreatedAsset);
	RepairKitRelic->RelicId = FFinalRelicId(ShopRepairKitRelicId);
	RepairKitRelic->DisplayId = TEXT("Relic.Test.RepairKit");
	RepairKitRelic->DisplayName = FText::FromString(TEXT("试作修理包"));
	RepairKitRelic->Description = FText::FromString(TEXT("战斗开始获得 4 护盾，玩家回合开始获得 2 护盾。"));
	RepairKitRelic->BattleStartEffects = {
		{ EFinalRelicBattleStartEffectType::GainShield, 4 }
	};
	RepairKitRelic->PlayerTurnStartEffects = {
		{ EFinalRelicPlayerTurnStartEffectType::GainShield, 2 }
	};
	TrackPackage(RepairKitRelic, PackagesToSave);

	UFinalRunRouteDefinition* PrototypeRunRoute = LoadOrCreateAsset<UFinalRunRouteDefinition>(PrototypeRunRoutePath, bCreatedAsset);
	PrototypeRunRoute->RouteId = PrototypeRouteId;
	PrototypeRunRoute->DisplayName = FText::FromString(TEXT("测试战斗外环"));
	PrototypeRunRoute->EntryNodeId = OpeningBattleNodeId;
	PrototypeRunRoute->NodeDefinitions.Reset();
	{
		FFinalRunNodeDefinition OpeningBattleNode;
		OpeningBattleNode.NodeId = OpeningBattleNodeId;
		OpeningBattleNode.NodeType = EFinalRunNodeType::Battle;
		OpeningBattleNode.DisplayName = FText::FromString(TEXT("外环巡逻"));
		OpeningBattleNode.DisplayLabel = TEXT("RunNode.Test.OpeningBattle");
		OpeningBattleNode.ChapterIndex = 1;
		OpeningBattleNode.FloorIndex = 1;
		OpeningBattleNode.EncounterId = EncounterDefinition->EncounterId;
		OpeningBattleNode.RuleConfigId = RuleConfig->RuleConfigId;
		OpeningBattleNode.NextNodeIds.Add(RewardNodeId);

		FFinalRunNodeDefinition RewardNode;
		RewardNode.NodeId = RewardNodeId;
		RewardNode.NodeType = EFinalRunNodeType::Reward;
		RewardNode.DisplayName = FText::FromString(TEXT("战利品分拣"));
		RewardNode.DisplayLabel = TEXT("RunNode.Test.Reward");
		RewardNode.ChapterIndex = 1;
		RewardNode.FloorIndex = 2;
		RewardNode.NextNodeIds.Add(EventNodeId);
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
			CharmRelic->RelicId,
			FText::FromString(TEXT("试作护符"))));
		RewardNode.RewardContent.RewardEntries.Add(MakeRemoveCardRewardEntry(
			TEXT("reward.node.cache.remove_guardian_strike"),
			GuardianStrikeCard->CardId,
			FText::FromString(TEXT("移除一张试作斩击"))));

		FFinalRunNodeDefinition EventNode;
		EventNode.NodeId = EventNodeId;
		EventNode.NodeType = EFinalRunNodeType::Event;
		EventNode.DisplayName = FText::FromString(TEXT("岔路告示"));
		EventNode.DisplayLabel = TEXT("RunNode.Test.Event");
		EventNode.ChapterIndex = 1;
		EventNode.FloorIndex = 3;
		EventNode.NextNodeIds.Add(ShopNodeId);
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
			SupportShotCard->CardId,
			SupportFocusCard->CardId,
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
			SupportCharacter->CharacterId,
			EFinalRunGrowthEffectType::ReduceStress,
			1,
			TEXT("Growth.Test.Support.ReduceStress"),
			FText::FromString(TEXT("测试策应·减压"))));
		EventNode.EventContent.Options.Add(TakeRestOption);

		FFinalRunNodeDefinition ShopNode;
		ShopNode.NodeId = ShopNodeId;
		ShopNode.NodeType = EFinalRunNodeType::Shop;
		ShopNode.DisplayName = FText::FromString(TEXT("流动补给摊"));
		ShopNode.DisplayLabel = TEXT("RunNode.Test.Shop");
		ShopNode.ChapterIndex = 1;
		ShopNode.FloorIndex = 4;
		ShopNode.NextNodeIds.Add(FollowupBattleNodeId);
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
			RepairKitRelic->RelicId,
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
			SupportFocusCard->CardId,
			FText::FromString(TEXT("试作整备"))));
		ShopNode.ShopContent.Offers.Add(PremiumBundleOffer);

		FFinalRunNodeDefinition FollowupBattleNode;
		FollowupBattleNode.NodeId = FollowupBattleNodeId;
		FollowupBattleNode.NodeType = EFinalRunNodeType::EliteBattle;
		FollowupBattleNode.DisplayName = FText::FromString(TEXT("巷战回响"));
		FollowupBattleNode.DisplayLabel = TEXT("RunNode.Test.FollowupBattle");
		FollowupBattleNode.ChapterIndex = 1;
		FollowupBattleNode.FloorIndex = 5;
		FollowupBattleNode.EncounterId = EncounterDefinition->EncounterId;
		FollowupBattleNode.RuleConfigId = RuleConfig->RuleConfigId;

		PrototypeRunRoute->NodeDefinitions = {
			OpeningBattleNode,
			RewardNode,
			EventNode,
			ShopNode,
			FollowupBattleNode
		};
	}
	TrackPackage(PrototypeRunRoute, PackagesToSave);

	UFinalPrototypeBootstrapDefinition* PrototypeBootstrap = LoadOrCreateAsset<UFinalPrototypeBootstrapDefinition>(PrototypeBootstrapPath, bCreatedAsset);
	PrototypeBootstrap->BootstrapId = PrototypeBootstrapId;
	PrototypeBootstrap->DisplayName = FText::FromString(TEXT("测试原型启动配置"));
	PrototypeBootstrap->RuleConfigId = RuleConfig->RuleConfigId;
	PrototypeBootstrap->EncounterId = EncounterDefinition->EncounterId;
	PrototypeBootstrap->RunRouteId = PrototypeRunRoute->RouteId;
	PrototypeBootstrap->PartyCharacterIds = {
		GuardianCharacter->CharacterId,
		SupportCharacter->CharacterId
	};
	PrototypeBootstrap->InitialCharacterStates.Reset();
	{
		FFinalPrototypeBootstrapCharacterState GuardianState;
		GuardianState.CharacterId = GuardianCharacter->CharacterId;
		GuardianState.CurrentStress = 0;
		GuardianState.bCollapsed = false;
		GuardianState.CurrentAwakenCount = 0;
		GuardianState.CollapseCount = 0;
		PrototypeBootstrap->InitialCharacterStates.Add(GuardianState);

		FFinalPrototypeBootstrapCharacterState SupportState;
		SupportState.CharacterId = SupportCharacter->CharacterId;
		SupportState.CurrentStress = 1;
		SupportState.bCollapsed = false;
		SupportState.CurrentAwakenCount = 0;
		SupportState.CollapseCount = 0;
		PrototypeBootstrap->InitialCharacterStates.Add(SupportState);
	}
	PrototypeBootstrap->StarterDeckCardIds = {
		GuardianStrikeCard->CardId,
		GuardianGuardCard->CardId,
		SupportFocusCard->CardId,
		SupportShotCard->CardId,
		GuardianStrikeCard->CardId,
		SupportShotCard->CardId,
		GuardianGuardCard->CardId
	};
	PrototypeBootstrap->InitialTeamCurrentHP = 42;
	TrackPackage(PrototypeBootstrap, PackagesToSave);

	UFinalBattleRuleConfig* StarterRuleConfig = LoadOrCreateAsset<UFinalBattleRuleConfig>(StarterRulesPath, bCreatedAsset);
	StarterRuleConfig->RuleConfigId = FFinalRuleConfigId(StarterRuleConfigId);
	StarterRuleConfig->InitialAP = 3;
	StarterRuleConfig->InitialHandSize = 5;
	StarterRuleConfig->HandLimit = 10;
	StarterRuleConfig->MaxEP = 70;
	StarterRuleConfig->EndTurnEpGain = 3;
	StarterRuleConfig->OnHitEpGain = 4;
	StarterRuleConfig->BaseCardEpGain = 1;
	StarterRuleConfig->BreakRewardAP = 1;
	StarterRuleConfig->NormalCardInitiativeEventCount = 1;
	StarterRuleConfig->CollapsedCardInitiativeEventCount = 1;
	StarterRuleConfig->StressHpLossPerPoint = 5;
	StarterRuleConfig->StressHealPerPoint = 8;
	StarterRuleConfig->MinStressChangePerEvent = 1;
	StarterRuleConfig->MaxStressGainPerHit = 3;
	StarterRuleConfig->StressRandomProtectionCount = 2;
	StarterRuleConfig->DamageToBreakCap = 6;
	TrackPackage(StarterRuleConfig, PackagesToSave);

	UFinalStatusDefinition* StarterHuoStatus = LoadOrCreateAsset<UFinalStatusDefinition>(StarterHuoStatusPath, bCreatedAsset);
	StarterHuoStatus->StatusId = FFinalStatusId(StarterHuoStatusId);
	StarterHuoStatus->DisplayName = FText::FromString(TEXT("刀势"));
	StarterHuoStatus->StatusCategory = EFinalStatusCategory::Signature;
	StarterHuoStatus->SummaryText = FText::FromString(TEXT("霍断岳的专属机制占位。首版 starter content 先只落稳定 ID 与说明文本，不额外补刀势运行时协议。"));
	StarterHuoStatus->MaxStacks = 6;
	StarterHuoStatus->DefaultDuration = 0;
	StarterHuoStatus->OnTickEffects.Reset();
	TrackPackage(StarterHuoStatus, PackagesToSave);

	UFinalStatusDefinition* StarterYeStatus = LoadOrCreateAsset<UFinalStatusDefinition>(StarterYeStatusPath, bCreatedAsset);
	StarterYeStatus->StatusId = FFinalStatusId(StarterYeStatusId);
	StarterYeStatus->DisplayName = FText::FromString(TEXT("药引"));
	StarterYeStatus->StatusCategory = EFinalStatusCategory::Signature;
	StarterYeStatus->SummaryText = FText::FromString(TEXT("叶半夏的药引机制占位。首版 starter content 先保留文本真相，实际资源转化仍由后续协议承接。"));
	StarterYeStatus->MaxStacks = 9;
	StarterYeStatus->DefaultDuration = 0;
	StarterYeStatus->OnTickEffects.Reset();
	TrackPackage(StarterYeStatus, PackagesToSave);

	UFinalStatusDefinition* StarterShenStatus = LoadOrCreateAsset<UFinalStatusDefinition>(StarterShenStatusPath, bCreatedAsset);
	StarterShenStatus->StatusId = FFinalStatusId(StarterShenStatusId);
	StarterShenStatus->DisplayName = FText::FromString(TEXT("剑阵"));
	StarterShenStatus->StatusCategory = EFinalStatusCategory::Signature;
	StarterShenStatus->SummaryText = FText::FromString(TEXT("沈清弦的剑阵机制占位。首版 starter content 先以过牌/护盾近似剑阵供能，不新增生成牌协议。"));
	StarterShenStatus->MaxStacks = 9;
	StarterShenStatus->DefaultDuration = 0;
	StarterShenStatus->OnTickEffects.Reset();
	TrackPackage(StarterShenStatus, PackagesToSave);

	UFinalUltimateDefinition* StarterHuoUltimate = LoadOrCreateAsset<UFinalUltimateDefinition>(StarterHuoUltimatePath, bCreatedAsset);
	StarterHuoUltimate->UltimateId = FFinalUltimateId(StarterHuoUltimateId);
	StarterHuoUltimate->OwnerUnitId = StarterHuoCharacterId;
	StarterHuoUltimate->DisplayName = FText::FromString(TEXT("断岳绝式"));
	StarterHuoUltimate->BaseCostEP = 45;
	StarterHuoUltimate->RulesText = FText::FromString(TEXT("对单体目标造成重击，并强压韧性。当前首版仅落地高倍率伤害，额外压韧与刀势仍为文本占位。"));
	StarterHuoUltimate->Effects.Reset();
	AddDamageEffect(
		StarterHuoUltimate,
		StarterHuoUltimate->Effects,
		TEXT("effect.starter.huo.ultimate.duanyuejueshi.damage"),
		EFinalBattleUnitTargetRule::SelectedEnemy,
		2.2f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::Attack,
		1,
		FText::FromString(TEXT("首版占位：未额外落地强压韧性与刀势获得。")));
	TrackPackage(StarterHuoUltimate, PackagesToSave);

	UFinalUltimateDefinition* StarterYeUltimate = LoadOrCreateAsset<UFinalUltimateDefinition>(StarterYeUltimatePath, bCreatedAsset);
	StarterYeUltimate->UltimateId = FFinalUltimateId(StarterYeUltimateId);
	StarterYeUltimate->OwnerUnitId = StarterYeCharacterId;
	StarterYeUltimate->DisplayName = FText::FromString(TEXT("回天续脉"));
	StarterYeUltimate->BaseCostEP = 45;
	StarterYeUltimate->RulesText = FText::FromString(TEXT("大幅回复队伍生命、施加免疫并回复 1 AP。当前首版以全队护盾 + 抽牌近似急救与节奏修正。"));
	StarterYeUltimate->Effects.Reset();
	AddShieldEffect(
		StarterYeUltimate,
		StarterYeUltimate->Effects,
		TEXT("effect.starter.ye.ultimate.huitianxumai.shield"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		12.0f,
		EFinalBattleScalarMode::Flat,
		EFinalBattleSourceStat::None,
		FText::FromString(TEXT("首版占位：治疗、免疫与回 AP 暂以全队护盾近似。")));
	AddDrawEffect(
		StarterYeUltimate,
		StarterYeUltimate->Effects,
		TEXT("effect.starter.ye.ultimate.huitianxumai.draw"),
		2,
		FText::FromString(TEXT("首版占位：用抽牌近似药引转节奏。")));
	TrackPackage(StarterYeUltimate, PackagesToSave);

	UFinalUltimateDefinition* StarterShenUltimate = LoadOrCreateAsset<UFinalUltimateDefinition>(StarterShenUltimatePath, bCreatedAsset);
	StarterShenUltimate->UltimateId = FFinalUltimateId(StarterShenUltimateId);
	StarterShenUltimate->OwnerUnitId = StarterShenCharacterId;
	StarterShenUltimate->DisplayName = FText::FromString(TEXT("万象归阵"));
	StarterShenUltimate->BaseCostEP = 45;
	StarterShenUltimate->RulesText = FText::FromString(TEXT("团队支援、过牌强化、阵牌扩散。当前首版以抽牌与全队护盾近似团队增益。"));
	StarterShenUltimate->Effects.Reset();
	AddDrawEffect(
		StarterShenUltimate,
		StarterShenUltimate->Effects,
		TEXT("effect.starter.shen.ultimate.wanxiangguizhen.draw"),
		2,
		FText::FromString(TEXT("首版占位：未实际生成剑阵牌。")));
	AddShieldEffect(
		StarterShenUltimate,
		StarterShenUltimate->Effects,
		TEXT("effect.starter.shen.ultimate.wanxiangguizhen.shield"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		5.0f,
		EFinalBattleScalarMode::Flat,
		EFinalBattleSourceStat::None,
		FText::FromString(TEXT("首版占位：士气与阵势扩散暂以全队护盾近似。")));
	TrackPackage(StarterShenUltimate, PackagesToSave);

	UFinalCharacterDefinition* StarterHuoCharacter = LoadOrCreateAsset<UFinalCharacterDefinition>(StarterHuoCharacterPath, bCreatedAsset);
	StarterHuoCharacter->CharacterId = FFinalCharacterId(StarterHuoCharacterId);
	StarterHuoCharacter->DisplayName = FText::FromString(TEXT("霍断岳"));
	StarterHuoCharacter->BaseVitalShare = 24;
	StarterHuoCharacter->BaseStressCap = 12;
	StarterHuoCharacter->BaseAttack = 7;
	StarterHuoCharacter->BaseDefense = 3;
	StarterHuoCharacter->BaseBreakRate = 1.2f;
	StarterHuoCharacter->BaseCritChance = 0.05f;
	StarterHuoCharacter->BaseCritDamage = 1.5f;
	StarterHuoCharacter->EpGainPerAP = 1;
	StarterHuoCharacter->UltimateId = StarterHuoUltimate->UltimateId;
	StarterHuoCharacter->SignatureStatusId = StarterHuoStatus->StatusId;
	TrackPackage(StarterHuoCharacter, PackagesToSave);

	UFinalCharacterDefinition* StarterYeCharacter = LoadOrCreateAsset<UFinalCharacterDefinition>(StarterYeCharacterPath, bCreatedAsset);
	StarterYeCharacter->CharacterId = FFinalCharacterId(StarterYeCharacterId);
	StarterYeCharacter->DisplayName = FText::FromString(TEXT("叶半夏"));
	StarterYeCharacter->BaseVitalShare = 18;
	StarterYeCharacter->BaseStressCap = 14;
	StarterYeCharacter->BaseAttack = 4;
	StarterYeCharacter->BaseDefense = 2;
	StarterYeCharacter->BaseBreakRate = 0.9f;
	StarterYeCharacter->BaseCritChance = 0.05f;
	StarterYeCharacter->BaseCritDamage = 1.5f;
	StarterYeCharacter->EpGainPerAP = 1;
	StarterYeCharacter->UltimateId = StarterYeUltimate->UltimateId;
	StarterYeCharacter->SignatureStatusId = StarterYeStatus->StatusId;
	TrackPackage(StarterYeCharacter, PackagesToSave);

	UFinalCharacterDefinition* StarterShenCharacter = LoadOrCreateAsset<UFinalCharacterDefinition>(StarterShenCharacterPath, bCreatedAsset);
	StarterShenCharacter->CharacterId = FFinalCharacterId(StarterShenCharacterId);
	StarterShenCharacter->DisplayName = FText::FromString(TEXT("沈清弦"));
	StarterShenCharacter->BaseVitalShare = 20;
	StarterShenCharacter->BaseStressCap = 13;
	StarterShenCharacter->BaseAttack = 5;
	StarterShenCharacter->BaseDefense = 3;
	StarterShenCharacter->BaseBreakRate = 1.0f;
	StarterShenCharacter->BaseCritChance = 0.08f;
	StarterShenCharacter->BaseCritDamage = 1.5f;
	StarterShenCharacter->EpGainPerAP = 1;
	StarterShenCharacter->UltimateId = StarterShenUltimate->UltimateId;
	StarterShenCharacter->SignatureStatusId = StarterShenStatus->StatusId;
	TrackPackage(StarterShenCharacter, PackagesToSave);

	UFinalCardDefinition* StarterHuoLieFengCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterHuoLieFengCardPath, bCreatedAsset);
	StarterHuoLieFengCard->CardId = FFinalCardId(StarterHuoLieFengCardId);
	StarterHuoLieFengCard->OwnerUnitId = StarterHuoCharacterId;
	StarterHuoLieFengCard->DisplayName = FText::FromString(TEXT("裂锋"));
	StarterHuoLieFengCard->CardType = EFinalCardType::Attack;
	StarterHuoLieFengCard->Rarity = EFinalRarity::Common;
	StarterHuoLieFengCard->BaseCostAP = 1;
	StarterHuoLieFengCard->RulesText = FText::FromString(TEXT("对目标造成相当于攻击力 110% 的伤害。额外削韧与刀势仍为首版文本占位。"));
	StarterHuoLieFengCard->Effects.Reset();
	AddDamageEffect(
		StarterHuoLieFengCard,
		StarterHuoLieFengCard->Effects,
		TEXT("effect.starter.huo.liefeng.damage"),
		EFinalBattleUnitTargetRule::SelectedEnemy,
		1.1f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::Attack,
		1,
		FText::FromString(TEXT("首版占位：未额外落地削韧与刀势。")));
	TrackPackage(StarterHuoLieFengCard, PackagesToSave);

	UFinalCardDefinition* StarterHuoWenJiaCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterHuoWenJiaCardPath, bCreatedAsset);
	StarterHuoWenJiaCard->CardId = FFinalCardId(StarterHuoWenJiaCardId);
	StarterHuoWenJiaCard->OwnerUnitId = StarterHuoCharacterId;
	StarterHuoWenJiaCard->DisplayName = FText::FromString(TEXT("稳架"));
	StarterHuoWenJiaCard->CardType = EFinalCardType::Skill;
	StarterHuoWenJiaCard->Rarity = EFinalRarity::Common;
	StarterHuoWenJiaCard->BaseCostAP = 1;
	StarterHuoWenJiaCard->RulesText = FText::FromString(TEXT("获得相当于防御力 100% 的护盾。受压得刀势仍为首版文本占位。"));
	StarterHuoWenJiaCard->Effects.Reset();
	AddShieldEffect(
		StarterHuoWenJiaCard,
		StarterHuoWenJiaCard->Effects,
		TEXT("effect.starter.huo.wenjia.shield"),
		EFinalBattleUnitTargetRule::Self,
		1.0f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::Defense,
		FText::FromString(TEXT("首版占位：未额外落地受压得刀势。")));
	TrackPackage(StarterHuoWenJiaCard, PackagesToSave);

	UFinalCardDefinition* StarterHuoDuanYueZhanCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterHuoDuanYueZhanCardPath, bCreatedAsset);
	StarterHuoDuanYueZhanCard->CardId = FFinalCardId(StarterHuoDuanYueZhanCardId);
	StarterHuoDuanYueZhanCard->OwnerUnitId = StarterHuoCharacterId;
	StarterHuoDuanYueZhanCard->DisplayName = FText::FromString(TEXT("断岳斩"));
	StarterHuoDuanYueZhanCard->CardType = EFinalCardType::Attack;
	StarterHuoDuanYueZhanCard->Rarity = EFinalRarity::Common;
	StarterHuoDuanYueZhanCard->BaseCostAP = 1;
	StarterHuoDuanYueZhanCard->RulesText = FText::FromString(TEXT("对目标造成相当于攻击力 130% 的伤害。Break 追伤与额外削韧仍为首版文本占位。"));
	StarterHuoDuanYueZhanCard->Effects.Reset();
	AddDamageEffect(
		StarterHuoDuanYueZhanCard,
		StarterHuoDuanYueZhanCard->Effects,
		TEXT("effect.starter.huo.duanyuezhan.damage"),
		EFinalBattleUnitTargetRule::SelectedEnemy,
		1.3f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::Attack,
		1,
		FText::FromString(TEXT("首版占位：未额外落地 Break 条件加成与额外削韧。")));
	TrackPackage(StarterHuoDuanYueZhanCard, PackagesToSave);

	UFinalCardDefinition* StarterHuoTieBiHuiFengCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterHuoTieBiHuiFengCardPath, bCreatedAsset);
	StarterHuoTieBiHuiFengCard->CardId = FFinalCardId(StarterHuoTieBiHuiFengCardId);
	StarterHuoTieBiHuiFengCard->OwnerUnitId = StarterHuoCharacterId;
	StarterHuoTieBiHuiFengCard->DisplayName = FText::FromString(TEXT("铁壁回锋"));
	StarterHuoTieBiHuiFengCard->CardType = EFinalCardType::Skill;
	StarterHuoTieBiHuiFengCard->Rarity = EFinalRarity::Common;
	StarterHuoTieBiHuiFengCard->BaseCostAP = 1;
	StarterHuoTieBiHuiFengCard->RulesText = FText::FromString(TEXT("获得相当于防御力 120% 的护盾。刀势与下张攻击额外削韧仍为首版文本占位。"));
	StarterHuoTieBiHuiFengCard->Effects.Reset();
	AddShieldEffect(
		StarterHuoTieBiHuiFengCard,
		StarterHuoTieBiHuiFengCard->Effects,
		TEXT("effect.starter.huo.tiebihuifeng.shield"),
		EFinalBattleUnitTargetRule::Self,
		1.2f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::Defense,
		FText::FromString(TEXT("首版占位：未额外落地刀势与下一张攻击加削韧。")));
	TrackPackage(StarterHuoTieBiHuiFengCard, PackagesToSave);

	UFinalCardDefinition* StarterYeXingZhenCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterYeXingZhenCardPath, bCreatedAsset);
	StarterYeXingZhenCard->CardId = FFinalCardId(StarterYeXingZhenCardId);
	StarterYeXingZhenCard->OwnerUnitId = StarterYeCharacterId;
	StarterYeXingZhenCard->DisplayName = FText::FromString(TEXT("行针"));
	StarterYeXingZhenCard->CardType = EFinalCardType::Attack;
	StarterYeXingZhenCard->Rarity = EFinalRarity::Common;
	StarterYeXingZhenCard->BaseCostAP = 1;
	StarterYeXingZhenCard->RulesText = FText::FromString(TEXT("对目标造成相当于攻击力 90% 的伤害。药引获得仍为首版文本占位。"));
	StarterYeXingZhenCard->Effects.Reset();
	AddDamageEffect(
		StarterYeXingZhenCard,
		StarterYeXingZhenCard->Effects,
		TEXT("effect.starter.ye.xingzhen.damage"),
		EFinalBattleUnitTargetRule::SelectedEnemy,
		0.9f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::Attack,
		1,
		FText::FromString(TEXT("首版占位：未实际获得药引层数。")));
	TrackPackage(StarterYeXingZhenCard, PackagesToSave);

	UFinalCardDefinition* StarterYeTiaoXiCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterYeTiaoXiCardPath, bCreatedAsset);
	StarterYeTiaoXiCard->CardId = FFinalCardId(StarterYeTiaoXiCardId);
	StarterYeTiaoXiCard->OwnerUnitId = StarterYeCharacterId;
	StarterYeTiaoXiCard->DisplayName = FText::FromString(TEXT("调息"));
	StarterYeTiaoXiCard->CardType = EFinalCardType::Skill;
	StarterYeTiaoXiCard->Rarity = EFinalRarity::Common;
	StarterYeTiaoXiCard->BaseCostAP = 0;
	StarterYeTiaoXiCard->RulesText = FText::FromString(TEXT("回复 8% 生命份额血量并获得 1 层药引。当前首版以全队少量护盾近似小治疗。"));
	StarterYeTiaoXiCard->Effects.Reset();
	AddShieldEffect(
		StarterYeTiaoXiCard,
		StarterYeTiaoXiCard->Effects,
		TEXT("effect.starter.ye.tiaoxi.shield"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		3.0f,
		EFinalBattleScalarMode::Flat,
		EFinalBattleSourceStat::None,
		FText::FromString(TEXT("首版占位：治疗与药引改为全队护盾近似。")));
	TrackPackage(StarterYeTiaoXiCard, PackagesToSave);

	UFinalCardDefinition* StarterYeHuaYinCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterYeHuaYinCardPath, bCreatedAsset);
	StarterYeHuaYinCard->CardId = FFinalCardId(StarterYeHuaYinCardId);
	StarterYeHuaYinCard->OwnerUnitId = StarterYeCharacterId;
	StarterYeHuaYinCard->DisplayName = FText::FromString(TEXT("化引"));
	StarterYeHuaYinCard->CardType = EFinalCardType::Skill;
	StarterYeHuaYinCard->Rarity = EFinalRarity::Common;
	StarterYeHuaYinCard->BaseCostAP = 1;
	StarterYeHuaYinCard->RulesText = FText::FromString(TEXT("回复 8% 生命份额血量。消耗 1 层药引：抽 1 张牌，并回复 1 AP。当前首版以护盾 + 抽牌近似。"));
	StarterYeHuaYinCard->Effects.Reset();
	AddShieldEffect(
		StarterYeHuaYinCard,
		StarterYeHuaYinCard->Effects,
		TEXT("effect.starter.ye.huayin.shield"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		4.0f,
		EFinalBattleScalarMode::Flat,
		EFinalBattleSourceStat::None,
		FText::FromString(TEXT("首版占位：治疗、药引消耗与回 AP 暂以护盾近似。")));
	AddDrawEffect(
		StarterYeHuaYinCard,
		StarterYeHuaYinCard->Effects,
		TEXT("effect.starter.ye.huayin.draw"),
		1,
		FText::FromString(TEXT("首版保留抽牌节奏收益。")));
	TrackPackage(StarterYeHuaYinCard, PackagesToSave);

	UFinalCardDefinition* StarterYeHuiChunSanCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterYeHuiChunSanCardPath, bCreatedAsset);
	StarterYeHuiChunSanCard->CardId = FFinalCardId(StarterYeHuiChunSanCardId);
	StarterYeHuiChunSanCard->OwnerUnitId = StarterYeCharacterId;
	StarterYeHuiChunSanCard->DisplayName = FText::FromString(TEXT("回春散"));
	StarterYeHuiChunSanCard->CardType = EFinalCardType::Skill;
	StarterYeHuiChunSanCard->Rarity = EFinalRarity::Common;
	StarterYeHuiChunSanCard->BaseCostAP = 1;
	StarterYeHuiChunSanCard->RulesText = FText::FromString(TEXT("回复 20% 生命份额血量并提供免疫。当前首版以更高全队护盾近似急救稳场。"));
	StarterYeHuiChunSanCard->Effects.Reset();
	AddShieldEffect(
		StarterYeHuiChunSanCard,
		StarterYeHuiChunSanCard->Effects,
		TEXT("effect.starter.ye.huichunsan.shield"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		8.0f,
		EFinalBattleScalarMode::Flat,
		EFinalBattleSourceStat::None,
		FText::FromString(TEXT("首版占位：治疗、免疫与药引消耗暂以全队护盾近似。")));
	TrackPackage(StarterYeHuiChunSanCard, PackagesToSave);

	UFinalCardDefinition* StarterShenBuFengCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterShenBuFengCardPath, bCreatedAsset);
	StarterShenBuFengCard->CardId = FFinalCardId(StarterShenBuFengCardId);
	StarterShenBuFengCard->OwnerUnitId = StarterShenCharacterId;
	StarterShenBuFengCard->DisplayName = FText::FromString(TEXT("布锋"));
	StarterShenBuFengCard->CardType = EFinalCardType::Attack;
	StarterShenBuFengCard->Rarity = EFinalRarity::Common;
	StarterShenBuFengCard->BaseCostAP = 1;
	StarterShenBuFengCard->RulesText = FText::FromString(TEXT("对目标造成相当于攻击力 100% 的伤害。生成剑阵牌仍为首版文本占位。"));
	StarterShenBuFengCard->Effects.Reset();
	AddDamageEffect(
		StarterShenBuFengCard,
		StarterShenBuFengCard->Effects,
		TEXT("effect.starter.shen.bufeng.damage"),
		EFinalBattleUnitTargetRule::SelectedEnemy,
		1.0f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::Attack,
		1,
		FText::FromString(TEXT("首版占位：未实际生成剑阵牌。")));
	TrackPackage(StarterShenBuFengCard, PackagesToSave);

	UFinalCardDefinition* StarterShenShouZhenCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterShenShouZhenCardPath, bCreatedAsset);
	StarterShenShouZhenCard->CardId = FFinalCardId(StarterShenShouZhenCardId);
	StarterShenShouZhenCard->OwnerUnitId = StarterShenCharacterId;
	StarterShenShouZhenCard->DisplayName = FText::FromString(TEXT("守阵"));
	StarterShenShouZhenCard->CardType = EFinalCardType::Skill;
	StarterShenShouZhenCard->Rarity = EFinalRarity::Common;
	StarterShenShouZhenCard->BaseCostAP = 1;
	StarterShenShouZhenCard->RulesText = FText::FromString(TEXT("获得相当于防御力 80% 的护盾。当前首版直接补 1 张牌，近似维持手牌节奏。"));
	StarterShenShouZhenCard->Effects.Reset();
	AddShieldEffect(
		StarterShenShouZhenCard,
		StarterShenShouZhenCard->Effects,
		TEXT("effect.starter.shen.shouzhen.shield"),
		EFinalBattleUnitTargetRule::Self,
		0.8f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::Defense);
	AddDrawEffect(
		StarterShenShouZhenCard,
		StarterShenShouZhenCard->Effects,
		TEXT("effect.starter.shen.shouzhen.draw"),
		1,
		FText::FromString(TEXT("首版占位：未校验手中是否已有剑阵牌。")));
	TrackPackage(StarterShenShouZhenCard, PackagesToSave);

	UFinalCardDefinition* StarterShenYinZhenCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterShenYinZhenCardPath, bCreatedAsset);
	StarterShenYinZhenCard->CardId = FFinalCardId(StarterShenYinZhenCardId);
	StarterShenYinZhenCard->OwnerUnitId = StarterShenCharacterId;
	StarterShenYinZhenCard->DisplayName = FText::FromString(TEXT("引阵"));
	StarterShenYinZhenCard->CardType = EFinalCardType::Skill;
	StarterShenYinZhenCard->Rarity = EFinalRarity::Common;
	StarterShenYinZhenCard->BaseCostAP = 1;
	StarterShenYinZhenCard->RulesText = FText::FromString(TEXT("从剑阵牌列表中选择 1 张并抽 1 张牌。当前首版以直接抽 2 张牌近似。"));
	StarterShenYinZhenCard->Effects.Reset();
	AddDrawEffect(
		StarterShenYinZhenCard,
		StarterShenYinZhenCard->Effects,
		TEXT("effect.starter.shen.yinzhen.draw"),
		2,
		FText::FromString(TEXT("首版占位：未实际生成指定剑阵牌。")));
	TrackPackage(StarterShenYinZhenCard, PackagesToSave);

	UFinalCardDefinition* StarterShenGuoPaiJianZhenCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterShenGuoPaiJianZhenCardPath, bCreatedAsset);
	StarterShenGuoPaiJianZhenCard->CardId = FFinalCardId(StarterShenGuoPaiJianZhenCardId);
	StarterShenGuoPaiJianZhenCard->OwnerUnitId = StarterShenCharacterId;
	StarterShenGuoPaiJianZhenCard->DisplayName = FText::FromString(TEXT("过牌剑阵"));
	StarterShenGuoPaiJianZhenCard->CardType = EFinalCardType::Skill;
	StarterShenGuoPaiJianZhenCard->Rarity = EFinalRarity::Common;
	StarterShenGuoPaiJianZhenCard->BaseCostAP = 0;
	StarterShenGuoPaiJianZhenCard->RulesText = FText::FromString(TEXT("抽 1 张牌。"));
	StarterShenGuoPaiJianZhenCard->Effects.Reset();
	AddDrawEffect(
		StarterShenGuoPaiJianZhenCard,
		StarterShenGuoPaiJianZhenCard->Effects,
		TEXT("effect.starter.shen.guopaijianzhen.draw"),
		1);
	TrackPackage(StarterShenGuoPaiJianZhenCard, PackagesToSave);

	StarterHuoCharacter->InitialLoadoutCards = {
		MakeLoadoutEntry(StarterHuoLieFengCard->CardId, 2, EFinalLoadoutRole::BaseAttack),
		MakeLoadoutEntry(StarterHuoWenJiaCard->CardId, 1, EFinalLoadoutRole::BaseDefense),
		MakeLoadoutEntry(StarterHuoDuanYueZhanCard->CardId, 1, EFinalLoadoutRole::BaseAttack),
		MakeLoadoutEntry(StarterHuoTieBiHuiFengCard->CardId, 1, EFinalLoadoutRole::BaseDefense)
	};
	StarterHuoCharacter->CharacterCardPoolIds = {
		StarterHuoLieFengCard->CardId,
		StarterHuoWenJiaCard->CardId,
		StarterHuoDuanYueZhanCard->CardId,
		StarterHuoTieBiHuiFengCard->CardId
	};
	TrackPackage(StarterHuoCharacter, PackagesToSave);

	StarterYeCharacter->InitialLoadoutCards = {
		MakeLoadoutEntry(StarterYeXingZhenCard->CardId, 1, EFinalLoadoutRole::BaseAttack),
		MakeLoadoutEntry(StarterYeTiaoXiCard->CardId, 1, EFinalLoadoutRole::BaseTactic),
		MakeLoadoutEntry(StarterYeHuaYinCard->CardId, 1, EFinalLoadoutRole::BaseTactic),
		MakeLoadoutEntry(StarterYeHuiChunSanCard->CardId, 1, EFinalLoadoutRole::BaseDefense)
	};
	StarterYeCharacter->CharacterCardPoolIds = {
		StarterYeXingZhenCard->CardId,
		StarterYeTiaoXiCard->CardId,
		StarterYeHuaYinCard->CardId,
		StarterYeHuiChunSanCard->CardId
	};
	TrackPackage(StarterYeCharacter, PackagesToSave);

	StarterShenCharacter->InitialLoadoutCards = {
		MakeLoadoutEntry(StarterShenBuFengCard->CardId, 1, EFinalLoadoutRole::BaseAttack),
		MakeLoadoutEntry(StarterShenShouZhenCard->CardId, 1, EFinalLoadoutRole::BaseDefense),
		MakeLoadoutEntry(StarterShenYinZhenCard->CardId, 1, EFinalLoadoutRole::BaseTactic),
		MakeLoadoutEntry(StarterShenGuoPaiJianZhenCard->CardId, 1, EFinalLoadoutRole::InitialSignature)
	};
	StarterShenCharacter->CharacterCardPoolIds = {
		StarterShenBuFengCard->CardId,
		StarterShenShouZhenCard->CardId,
		StarterShenYinZhenCard->CardId,
		StarterShenGuoPaiJianZhenCard->CardId
	};
	TrackPackage(StarterShenCharacter, PackagesToSave);

	UFinalEnemyIntentDefinition* StarterBladeQuickSlashIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterBladeQuickSlashIntentPath, bCreatedAsset);
	StarterBladeQuickSlashIntent->IntentId = TEXT("intent.starter.bandit.blade.quick_slash");
	StarterBladeQuickSlashIntent->DisplayName = FText::FromString(TEXT("快斩"));
	StarterBladeQuickSlashIntent->IntentType = EFinalIntentType::Attack;
	StarterBladeQuickSlashIntent->PreviewText = FText::FromString(TEXT("快斩 0.9 × BaseDamagePower"));
	StarterBladeQuickSlashIntent->CooldownTurns = 0;
	StarterBladeQuickSlashIntent->UseLimitPerBattle = 0;
	StarterBladeQuickSlashIntent->Effects.Reset();
	AddDamageEffect(
		StarterBladeQuickSlashIntent,
		StarterBladeQuickSlashIntent->Effects,
		TEXT("effect.starter.enemy.blade.quick_slash.damage"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		0.9f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::BaseDamagePower);
	TrackPackage(StarterBladeQuickSlashIntent, PackagesToSave);

	UFinalEnemyIntentDefinition* StarterBladeDoubleSlashIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterBladeDoubleSlashIntentPath, bCreatedAsset);
	StarterBladeDoubleSlashIntent->IntentId = TEXT("intent.starter.bandit.blade.double_slash");
	StarterBladeDoubleSlashIntent->DisplayName = FText::FromString(TEXT("二连斩"));
	StarterBladeDoubleSlashIntent->IntentType = EFinalIntentType::Attack;
	StarterBladeDoubleSlashIntent->PreviewText = FText::FromString(TEXT("二连斩 2 段，每段 0.45 × BaseDamagePower"));
	StarterBladeDoubleSlashIntent->CooldownTurns = 0;
	StarterBladeDoubleSlashIntent->UseLimitPerBattle = 0;
	StarterBladeDoubleSlashIntent->Effects.Reset();
	AddDamageEffect(
		StarterBladeDoubleSlashIntent,
		StarterBladeDoubleSlashIntent->Effects,
		TEXT("effect.starter.enemy.blade.double_slash.damage"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		0.45f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::BaseDamagePower,
		2);
	TrackPackage(StarterBladeDoubleSlashIntent, PackagesToSave);

	UFinalEnemyIntentDefinition* StarterBladeBraceIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterBladeBraceIntentPath, bCreatedAsset);
	StarterBladeBraceIntent->IntentId = TEXT("intent.starter.bandit.blade.brace");
	StarterBladeBraceIntent->DisplayName = FText::FromString(TEXT("整备"));
	StarterBladeBraceIntent->IntentType = EFinalIntentType::Defense;
	StarterBladeBraceIntent->PreviewText = FText::FromString(TEXT("获得 10 护盾"));
	StarterBladeBraceIntent->CooldownTurns = 1;
	StarterBladeBraceIntent->UseLimitPerBattle = 0;
	StarterBladeBraceIntent->Effects.Reset();
	AddShieldEffect(
		StarterBladeBraceIntent,
		StarterBladeBraceIntent->Effects,
		TEXT("effect.starter.enemy.blade.brace.shield"),
		EFinalBattleUnitTargetRule::Self,
		10.0f,
		EFinalBattleScalarMode::Flat);
	TrackPackage(StarterBladeBraceIntent, PackagesToSave);

	UFinalEnemyIntentDefinition* StarterCrossbowPiercingBoltIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterCrossbowPiercingBoltIntentPath, bCreatedAsset);
	StarterCrossbowPiercingBoltIntent->IntentId = TEXT("intent.starter.bandit.crossbow.piercing_bolt");
	StarterCrossbowPiercingBoltIntent->DisplayName = FText::FromString(TEXT("透甲箭"));
	StarterCrossbowPiercingBoltIntent->IntentType = EFinalIntentType::Attack;
	StarterCrossbowPiercingBoltIntent->PreviewText = FText::FromString(TEXT("透甲箭 1.0 × BaseDamagePower。易伤仍为首版文本占位。"));
	StarterCrossbowPiercingBoltIntent->CooldownTurns = 0;
	StarterCrossbowPiercingBoltIntent->UseLimitPerBattle = 0;
	StarterCrossbowPiercingBoltIntent->Effects.Reset();
	AddDamageEffect(
		StarterCrossbowPiercingBoltIntent,
		StarterCrossbowPiercingBoltIntent->Effects,
		TEXT("effect.starter.enemy.crossbow.piercing_bolt.damage"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		1.0f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::BaseDamagePower,
		1,
		FText::FromString(TEXT("首版占位：未实际施加易伤。")));
	TrackPackage(StarterCrossbowPiercingBoltIntent, PackagesToSave);

	UFinalEnemyIntentDefinition* StarterCrossbowVolleyIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterCrossbowVolleyIntentPath, bCreatedAsset);
	StarterCrossbowVolleyIntent->IntentId = TEXT("intent.starter.bandit.crossbow.volley");
	StarterCrossbowVolleyIntent->DisplayName = FText::FromString(TEXT("连珠箭"));
	StarterCrossbowVolleyIntent->IntentType = EFinalIntentType::Attack;
	StarterCrossbowVolleyIntent->PreviewText = FText::FromString(TEXT("连珠箭 2 段，每段 0.4 × BaseDamagePower"));
	StarterCrossbowVolleyIntent->CooldownTurns = 0;
	StarterCrossbowVolleyIntent->UseLimitPerBattle = 0;
	StarterCrossbowVolleyIntent->Effects.Reset();
	AddDamageEffect(
		StarterCrossbowVolleyIntent,
		StarterCrossbowVolleyIntent->Effects,
		TEXT("effect.starter.enemy.crossbow.volley.damage"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		0.4f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::BaseDamagePower,
		2);
	TrackPackage(StarterCrossbowVolleyIntent, PackagesToSave);

	UFinalEnemyIntentDefinition* StarterCrossbowRepositionIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterCrossbowRepositionIntentPath, bCreatedAsset);
	StarterCrossbowRepositionIntent->IntentId = TEXT("intent.starter.bandit.crossbow.reposition");
	StarterCrossbowRepositionIntent->DisplayName = FText::FromString(TEXT("后撤整弦"));
	StarterCrossbowRepositionIntent->IntentType = EFinalIntentType::Buff;
	StarterCrossbowRepositionIntent->PreviewText = FText::FromString(TEXT("后撤整弦：下次攻击伤害提高 30%。当前首版以自护盾占位。"));
	StarterCrossbowRepositionIntent->CooldownTurns = 1;
	StarterCrossbowRepositionIntent->UseLimitPerBattle = 0;
	StarterCrossbowRepositionIntent->Effects.Reset();
	AddShieldEffect(
		StarterCrossbowRepositionIntent,
		StarterCrossbowRepositionIntent->Effects,
		TEXT("effect.starter.enemy.crossbow.reposition.shield"),
		EFinalBattleUnitTargetRule::Self,
		6.0f,
		EFinalBattleScalarMode::Flat,
		EFinalBattleSourceStat::None,
		FText::FromString(TEXT("首版占位：未实际实现下次攻击增伤。")));
	TrackPackage(StarterCrossbowRepositionIntent, PackagesToSave);

	UFinalEnemyIntentDefinition* StarterInstructorCommandSlashIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterInstructorCommandSlashIntentPath, bCreatedAsset);
	StarterInstructorCommandSlashIntent->IntentId = TEXT("intent.starter.blackwind.instructor.command_slash");
	StarterInstructorCommandSlashIntent->DisplayName = FText::FromString(TEXT("号令斩"));
	StarterInstructorCommandSlashIntent->IntentType = EFinalIntentType::Attack;
	StarterInstructorCommandSlashIntent->PreviewText = FText::FromString(TEXT("号令斩 1.2 × BaseDamagePower。同伙增伤仍为首版文本占位。"));
	StarterInstructorCommandSlashIntent->CooldownTurns = 0;
	StarterInstructorCommandSlashIntent->UseLimitPerBattle = 0;
	StarterInstructorCommandSlashIntent->Effects.Reset();
	AddDamageEffect(
		StarterInstructorCommandSlashIntent,
		StarterInstructorCommandSlashIntent->Effects,
		TEXT("effect.starter.enemy.instructor.command_slash.damage"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		1.2f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::BaseDamagePower,
		1,
		FText::FromString(TEXT("首版占位：未实际实现同伙下一次攻击增伤。")));
	TrackPackage(StarterInstructorCommandSlashIntent, PackagesToSave);

	UFinalEnemyIntentDefinition* StarterInstructorHoldLineIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterInstructorHoldLineIntentPath, bCreatedAsset);
	StarterInstructorHoldLineIntent->IntentId = TEXT("intent.starter.blackwind.instructor.hold_line");
	StarterInstructorHoldLineIntent->DisplayName = FText::FromString(TEXT("压阵"));
	StarterInstructorHoldLineIntent->IntentType = EFinalIntentType::Defense;
	StarterInstructorHoldLineIntent->PreviewText = FText::FromString(TEXT("压阵：自己获得 14 护盾。护援同伙仍为首版文本占位。"));
	StarterInstructorHoldLineIntent->CooldownTurns = 1;
	StarterInstructorHoldLineIntent->UseLimitPerBattle = 0;
	StarterInstructorHoldLineIntent->Effects.Reset();
	AddShieldEffect(
		StarterInstructorHoldLineIntent,
		StarterInstructorHoldLineIntent->Effects,
		TEXT("effect.starter.enemy.instructor.hold_line.shield"),
		EFinalBattleUnitTargetRule::Self,
		14.0f,
		EFinalBattleScalarMode::Flat,
		EFinalBattleSourceStat::None,
		FText::FromString(TEXT("首版占位：未额外给同伙护盾。")));
	TrackPackage(StarterInstructorHoldLineIntent, PackagesToSave);

	UFinalEnemyIntentDefinition* StarterInstructorHeavyCleaveIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterInstructorHeavyCleaveIntentPath, bCreatedAsset);
	StarterInstructorHeavyCleaveIntent->IntentId = TEXT("intent.starter.blackwind.instructor.heavy_cleave");
	StarterInstructorHeavyCleaveIntent->DisplayName = FText::FromString(TEXT("破阵重劈"));
	StarterInstructorHeavyCleaveIntent->IntentType = EFinalIntentType::Attack;
	StarterInstructorHeavyCleaveIntent->PreviewText = FText::FromString(TEXT("破阵重劈 1.6 × BaseDamagePower。易伤追加伤害仍为首版文本占位。"));
	StarterInstructorHeavyCleaveIntent->CooldownTurns = 1;
	StarterInstructorHeavyCleaveIntent->UseLimitPerBattle = 0;
	StarterInstructorHeavyCleaveIntent->Effects.Reset();
	AddDamageEffect(
		StarterInstructorHeavyCleaveIntent,
		StarterInstructorHeavyCleaveIntent->Effects,
		TEXT("effect.starter.enemy.instructor.heavy_cleave.damage"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		1.6f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::BaseDamagePower,
		1,
		FText::FromString(TEXT("首版占位：未实际实现对易伤目标的额外增伤。")));
	TrackPackage(StarterInstructorHeavyCleaveIntent, PackagesToSave);

	UFinalEnemyDefinition* StarterBladeEnemy = LoadOrCreateAsset<UFinalEnemyDefinition>(StarterBladeEnemyPath, bCreatedAsset);
	StarterBladeEnemy->EnemyId = FFinalEnemyId(StarterBladeEnemyId);
	StarterBladeEnemy->DisplayName = FText::FromString(TEXT("山匪刀手"));
	StarterBladeEnemy->MaxHP = 34;
	StarterBladeEnemy->MaxBreakValue = 10;
	StarterBladeEnemy->BaseDamagePower = 9;
	StarterBladeEnemy->InitialInitiativeValue = 4;
	StarterBladeEnemy->InitiativeResponse = 1;
	StarterBladeEnemy->IntentSelectRule = EFinalIntentSelectRule::Cycle;
	StarterBladeEnemy->PhaseSequence.Reset();
	StarterBladeEnemy->IntentPool = {
		StarterBladeQuickSlashIntent,
		StarterBladeDoubleSlashIntent,
		StarterBladeBraceIntent
	};
	TrackPackage(StarterBladeEnemy, PackagesToSave);

	UFinalEnemyDefinition* StarterCrossbowEnemy = LoadOrCreateAsset<UFinalEnemyDefinition>(StarterCrossbowEnemyPath, bCreatedAsset);
	StarterCrossbowEnemy->EnemyId = FFinalEnemyId(StarterCrossbowEnemyId);
	StarterCrossbowEnemy->DisplayName = FText::FromString(TEXT("山匪弩手"));
	StarterCrossbowEnemy->MaxHP = 30;
	StarterCrossbowEnemy->MaxBreakValue = 10;
	StarterCrossbowEnemy->BaseDamagePower = 9;
	StarterCrossbowEnemy->InitialInitiativeValue = 5;
	StarterCrossbowEnemy->InitiativeResponse = 1;
	StarterCrossbowEnemy->IntentSelectRule = EFinalIntentSelectRule::Cycle;
	StarterCrossbowEnemy->PhaseSequence.Reset();
	StarterCrossbowEnemy->IntentPool = {
		StarterCrossbowPiercingBoltIntent,
		StarterCrossbowVolleyIntent,
		StarterCrossbowRepositionIntent
	};
	TrackPackage(StarterCrossbowEnemy, PackagesToSave);

	UFinalEnemyDefinition* StarterInstructorEnemy = LoadOrCreateAsset<UFinalEnemyDefinition>(StarterInstructorEnemyPath, bCreatedAsset);
	StarterInstructorEnemy->EnemyId = FFinalEnemyId(StarterInstructorEnemyId);
	StarterInstructorEnemy->DisplayName = FText::FromString(TEXT("黑风教头"));
	StarterInstructorEnemy->MaxHP = 110;
	StarterInstructorEnemy->MaxBreakValue = 28;
	StarterInstructorEnemy->BaseDamagePower = 12;
	StarterInstructorEnemy->InitialInitiativeValue = 7;
	StarterInstructorEnemy->InitiativeResponse = 1;
	StarterInstructorEnemy->IntentSelectRule = EFinalIntentSelectRule::Cycle;
	StarterInstructorEnemy->PhaseSequence.Reset();
	StarterInstructorEnemy->IntentPool = {
		StarterInstructorCommandSlashIntent,
		StarterInstructorHoldLineIntent,
		StarterInstructorHeavyCleaveIntent
	};
	TrackPackage(StarterInstructorEnemy, PackagesToSave);

	UFinalBattleEncounterDefinition* StarterNormalEncounter = LoadOrCreateAsset<UFinalBattleEncounterDefinition>(StarterNormalEncounterPath, bCreatedAsset);
	StarterNormalEncounter->EncounterId = FFinalEncounterId(StarterNormalEncounterId);
	StarterNormalEncounter->DisplayName = FText::FromString(TEXT("山道拦截"));
	StarterNormalEncounter->RuleConfig = StarterRuleConfig;
	StarterNormalEncounter->EnemyRoster.Reset();
	{
		FFinalEnemyRosterEntry BladeEntry;
		BladeEntry.EnemyDefinition = StarterBladeEnemy;
		BladeEntry.PositionIndex = 0;
		BladeEntry.SpawnWave = 1;
		StarterNormalEncounter->EnemyRoster.Add(BladeEntry);

		FFinalEnemyRosterEntry CrossbowEntry;
		CrossbowEntry.EnemyDefinition = StarterCrossbowEnemy;
		CrossbowEntry.PositionIndex = 1;
		CrossbowEntry.SpawnWave = 1;
		StarterNormalEncounter->EnemyRoster.Add(CrossbowEntry);
	}
	TrackPackage(StarterNormalEncounter, PackagesToSave);

	UFinalBattleEncounterDefinition* StarterEliteEncounter = LoadOrCreateAsset<UFinalBattleEncounterDefinition>(StarterEliteEncounterPath, bCreatedAsset);
	StarterEliteEncounter->EncounterId = FFinalEncounterId(StarterEliteEncounterId);
	StarterEliteEncounter->DisplayName = FText::FromString(TEXT("教头压阵"));
	StarterEliteEncounter->RuleConfig = StarterRuleConfig;
	StarterEliteEncounter->EnemyRoster.Reset();
	{
		FFinalEnemyRosterEntry InstructorEntry;
		InstructorEntry.EnemyDefinition = StarterInstructorEnemy;
		InstructorEntry.PositionIndex = 0;
		InstructorEntry.SpawnWave = 1;
		StarterEliteEncounter->EnemyRoster.Add(InstructorEntry);

		FFinalEnemyRosterEntry BladeEntry;
		BladeEntry.EnemyDefinition = StarterBladeEnemy;
		BladeEntry.PositionIndex = 1;
		BladeEntry.SpawnWave = 1;
		StarterEliteEncounter->EnemyRoster.Add(BladeEntry);

		FFinalEnemyRosterEntry CrossbowEntry;
		CrossbowEntry.EnemyDefinition = StarterCrossbowEnemy;
		CrossbowEntry.PositionIndex = 2;
		CrossbowEntry.SpawnWave = 1;
		StarterEliteEncounter->EnemyRoster.Add(CrossbowEntry);
	}
	TrackPackage(StarterEliteEncounter, PackagesToSave);

	UFinalRunRouteDefinition* StarterRunRoute = LoadOrCreateAsset<UFinalRunRouteDefinition>(StarterRunRoutePath, bCreatedAsset);
	StarterRunRoute->RouteId = StarterRouteId;
	StarterRunRoute->DisplayName = FText::FromString(TEXT("首章 starter 竖切路线"));
	StarterRunRoute->EntryNodeId = StarterOpeningBattleNodeId;
	StarterRunRoute->NodeDefinitions.Reset();
	{
		FFinalRunNodeDefinition OpeningBattleNode;
		OpeningBattleNode.NodeId = StarterOpeningBattleNodeId;
		OpeningBattleNode.NodeType = EFinalRunNodeType::Battle;
		OpeningBattleNode.DisplayName = FText::FromString(TEXT("山道拦截"));
		OpeningBattleNode.DisplayLabel = TEXT("RunNode.Starter.OpeningBattle");
		OpeningBattleNode.ChapterIndex = 1;
		OpeningBattleNode.FloorIndex = 1;
		OpeningBattleNode.EncounterId = StarterNormalEncounter->EncounterId;
		OpeningBattleNode.RuleConfigId = StarterRuleConfig->RuleConfigId;
		OpeningBattleNode.NextNodeIds.Add(StarterRewardNodeId);

		FFinalRunNodeDefinition RewardNode;
		RewardNode.NodeId = StarterRewardNodeId;
		RewardNode.NodeType = EFinalRunNodeType::Reward;
		RewardNode.DisplayName = FText::FromString(TEXT("缴获清点"));
		RewardNode.DisplayLabel = TEXT("RunNode.Starter.Reward");
		RewardNode.ChapterIndex = 1;
		RewardNode.FloorIndex = 2;
		RewardNode.NextNodeIds.Add(StarterEventNodeId);
		RewardNode.RewardContent.Title = FText::FromString(TEXT("缴获清点"));
		RewardNode.RewardContent.Summary = FText::FromString(TEXT("首版 starter 奖励节点，占位验证金币、卡牌与成长奖励。"));
		RewardNode.RewardContent.RewardEntries.Add(MakeBaseRewardEntry(
			TEXT("reward.starter.spoils.gold"),
			EFinalRunRewardType::Gold,
			18,
			TEXT("Currency.Gold"),
			FText::FromString(TEXT("剿匪赏金"))));
		RewardNode.RewardContent.RewardEntries.Add(MakeCardRewardEntry(
			TEXT("reward.starter.spoils.card.guopaijianzhen"),
			StarterShenGuoPaiJianZhenCard->CardId,
			FText::FromString(TEXT("过牌剑阵"))));
		RewardNode.RewardContent.RewardEntries.Add(MakeGrowthRewardEntry(
			TEXT("reward.starter.spoils.growth.ye"),
			StarterYeCharacter->CharacterId,
			EFinalRunGrowthEffectType::ReduceStress,
			1,
			TEXT("Growth.Starter.Ye.ReduceStress"),
			FText::FromString(TEXT("叶半夏·稳神"))));

		FFinalRunNodeDefinition EventNode;
		EventNode.NodeId = StarterEventNodeId;
		EventNode.NodeType = EFinalRunNodeType::Event;
		EventNode.DisplayName = FText::FromString(TEXT("崖边告示"));
		EventNode.DisplayLabel = TEXT("RunNode.Starter.Event");
		EventNode.ChapterIndex = 1;
		EventNode.FloorIndex = 3;
		EventNode.NextNodeIds.Add(StarterShopNodeId);
		EventNode.EventContent.Title = FText::FromString(TEXT("崖边告示"));
		EventNode.EventContent.Summary = FText::FromString(TEXT("首版 starter 事件节点，占位验证多选项与最小结果回写。"));

		FFinalRunEventOptionDefinition HuoOption;
		HuoOption.OptionId = TEXT("event.starter.option.huo_regroup");
		HuoOption.DisplayText = FText::FromString(TEXT("整顿刀势"));
		HuoOption.OutcomeSummary = FText::FromString(TEXT("霍断岳压住心火，压力 -1。"));
		HuoOption.RewardEntries.Add(MakeGrowthRewardEntry(
			TEXT("reward.starter.event.huo_regroup"),
			StarterHuoCharacter->CharacterId,
			EFinalRunGrowthEffectType::ReduceStress,
			1,
			TEXT("Growth.Starter.Huo.ReduceStress"),
			FText::FromString(TEXT("霍断岳·稳阵"))));
		EventNode.EventContent.Options.Add(HuoOption);

		FFinalRunEventOptionDefinition ShenOption;
		ShenOption.OptionId = TEXT("event.starter.option.shen_supply");
		ShenOption.DisplayText = FText::FromString(TEXT("翻检箭囊"));
		ShenOption.OutcomeSummary = FText::FromString(TEXT("找到一张布锋。"));
		ShenOption.RewardEntries.Add(MakeCardRewardEntry(
			TEXT("reward.starter.event.shen_supply"),
			StarterShenBuFengCard->CardId,
			FText::FromString(TEXT("布锋"))));
		EventNode.EventContent.Options.Add(ShenOption);

		FFinalRunEventOptionDefinition LockedOption;
		LockedOption.OptionId = TEXT("event.starter.option.chase_smoke");
		LockedOption.DisplayText = FText::FromString(TEXT("追查烟迹"));
		LockedOption.OutcomeSummary = FText::FromString(TEXT("毒烟支线尚未进入 starter 首版内容。"));
		LockedOption.bStartsDisabled = true;
		LockedOption.DisabledReason = FText::FromString(TEXT("首版 starter content 先不开放毒烟分支。"));
		EventNode.EventContent.Options.Add(LockedOption);

		FFinalRunNodeDefinition ShopNode;
		ShopNode.NodeId = StarterShopNodeId;
		ShopNode.NodeType = EFinalRunNodeType::Shop;
		ShopNode.DisplayName = FText::FromString(TEXT("营地行商"));
		ShopNode.DisplayLabel = TEXT("RunNode.Starter.Shop");
		ShopNode.ChapterIndex = 1;
		ShopNode.FloorIndex = 4;
		ShopNode.NextNodeIds.Add(StarterEliteBattleNodeId);
		ShopNode.ShopContent.Title = FText::FromString(TEXT("营地行商"));
		ShopNode.ShopContent.Summary = FText::FromString(TEXT("首版 starter 商店节点，用于验证 starter 卡牌可进入商店 reward payload。"));

		FFinalRunShopOfferDefinition TieBiOffer;
		TieBiOffer.OfferId = TEXT("shop.starter.offer.tiebihuifeng");
		TieBiOffer.DisplayId = TEXT("Shop.Starter.Huo.TieBiHuiFeng");
		TieBiOffer.DisplayName = FText::FromString(TEXT("铁壁回锋"));
		TieBiOffer.Description = FText::FromString(TEXT("补一张前排防守转反打牌。"));
		TieBiOffer.Price = 14;
		TieBiOffer.RewardEntries.Add(MakeCardRewardEntry(
			TEXT("reward.starter.shop.tiebihuifeng"),
			StarterHuoTieBiHuiFengCard->CardId,
			FText::FromString(TEXT("铁壁回锋"))));
		ShopNode.ShopContent.Offers.Add(TieBiOffer);

		FFinalRunShopOfferDefinition HuiChunOffer;
		HuiChunOffer.OfferId = TEXT("shop.starter.offer.huichunsan");
		HuiChunOffer.DisplayId = TEXT("Shop.Starter.Ye.HuiChunSan");
		HuiChunOffer.DisplayName = FText::FromString(TEXT("回春散"));
		HuiChunOffer.Description = FText::FromString(TEXT("补一张团队急救占位牌。"));
		HuiChunOffer.Price = 12;
		HuiChunOffer.RewardEntries.Add(MakeCardRewardEntry(
			TEXT("reward.starter.shop.huichunsan"),
			StarterYeHuiChunSanCard->CardId,
			FText::FromString(TEXT("回春散"))));
		ShopNode.ShopContent.Offers.Add(HuiChunOffer);

		FFinalRunNodeDefinition EliteBattleNode;
		EliteBattleNode.NodeId = StarterEliteBattleNodeId;
		EliteBattleNode.NodeType = EFinalRunNodeType::EliteBattle;
		EliteBattleNode.DisplayName = FText::FromString(TEXT("教头压阵"));
		EliteBattleNode.DisplayLabel = TEXT("RunNode.Starter.EliteBattle");
		EliteBattleNode.ChapterIndex = 1;
		EliteBattleNode.FloorIndex = 5;
		EliteBattleNode.EncounterId = StarterEliteEncounter->EncounterId;
		EliteBattleNode.RuleConfigId = StarterRuleConfig->RuleConfigId;

		StarterRunRoute->NodeDefinitions = {
			OpeningBattleNode,
			RewardNode,
			EventNode,
			ShopNode,
			EliteBattleNode
		};
	}
	TrackPackage(StarterRunRoute, PackagesToSave);

	UFinalPrototypeBootstrapDefinition* StarterBootstrap = LoadOrCreateAsset<UFinalPrototypeBootstrapDefinition>(StarterBootstrapPath, bCreatedAsset);
	StarterBootstrap->BootstrapId = StarterBootstrapId;
	StarterBootstrap->DisplayName = FText::FromString(TEXT("首章 starter 启动配置"));
	StarterBootstrap->RuleConfigId = StarterRuleConfig->RuleConfigId;
	StarterBootstrap->EncounterId = StarterNormalEncounter->EncounterId;
	StarterBootstrap->RunRouteId = StarterRunRoute->RouteId;
	StarterBootstrap->PartyCharacterIds = {
		StarterHuoCharacter->CharacterId,
		StarterYeCharacter->CharacterId,
		StarterShenCharacter->CharacterId
	};
	StarterBootstrap->InitialCharacterStates = {
		MakeBootstrapCharacterState(StarterHuoCharacter->CharacterId, 0),
		MakeBootstrapCharacterState(StarterYeCharacter->CharacterId, 0),
		MakeBootstrapCharacterState(StarterShenCharacter->CharacterId, 0)
	};
	StarterBootstrap->StarterDeckCardIds = {
		StarterHuoLieFengCard->CardId,
		StarterHuoLieFengCard->CardId,
		StarterHuoWenJiaCard->CardId,
		StarterHuoDuanYueZhanCard->CardId,
		StarterHuoTieBiHuiFengCard->CardId,
		StarterYeXingZhenCard->CardId,
		StarterYeTiaoXiCard->CardId,
		StarterYeHuaYinCard->CardId,
		StarterYeHuiChunSanCard->CardId,
		StarterShenBuFengCard->CardId,
		StarterShenShouZhenCard->CardId,
		StarterShenYinZhenCard->CardId,
		StarterShenGuoPaiJianZhenCard->CardId
	};
	StarterBootstrap->InitialTeamCurrentHP = 62;
	TrackPackage(StarterBootstrap, PackagesToSave);

	SavePackages(PackagesToSave);

	UE_LOG(LogFinalPrototypeContentBootstrap, Display, TEXT("Prototype/starter content bootstrap completed. Saved %d packages."), PackagesToSave.Num());
	return 0;
}
