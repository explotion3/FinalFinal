#include "Bootstrap/FinalStarterContentBundleBuilder.h"

#include "Bootstrap/FinalPrototypeContentBootstrapAssetUtils.h"
#include "Bootstrap/FinalPrototypeContentBootstrapEffectUtils.h"

#include "Battle/Conditions/FinalBattleConditionStatusChanged.h"
#include "Battle/Conditions/FinalBattleConditionHandCard.h"
#include "Battle/Conditions/FinalBattleConditionMovedCards.h"
#include "Battle/Conditions/FinalBattleConditionResolvedCard.h"
#include "Battle/Conditions/FinalBattleConditionTargetState.h"
#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalRuntimeTriggerDefinition.h"
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
#include "Run/Definitions/FinalCardEvolutionDefinition.h"
#include "Run/Definitions/FinalCharacterGrowthConfig.h"
#include "Run/Definitions/FinalPrototypeBootstrapDefinition.h"
#include "Run/Definitions/FinalRelicDefinition.h"
#include "Run/Definitions/FinalRunNodeDefinition.h"
#include "Run/Definitions/FinalRunRouteDefinition.h"
#include "Run/Rewards/FinalRunRewardTypes.h"

namespace FinalPrototypeContentBootstrap
{
	const FString RootPath(TEXT("/Game/Prototype/Definitions"));
	const FString StarterRootPath = RootPath / TEXT("Starter");
	const FString StarterRulesPath = StarterRootPath / TEXT("Rules/DA_Rule_StarterChapter1");
	const FString StarterHuoCharacterPath = StarterRootPath / TEXT("Characters/DA_Character_Starter_HuoDuanyue");
	const FString StarterYeCharacterPath = StarterRootPath / TEXT("Characters/DA_Character_Starter_YeBanxia");
	const FString StarterShenCharacterPath = StarterRootPath / TEXT("Characters/DA_Character_Starter_ShenQingxian");
	const FString StarterHuoLieFengCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_HuoLieFeng");
	const FString StarterHuoWenJiaCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_HuoWenJia");
	const FString StarterHuoDuanYueZhanCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_HuoDuanYueZhan");
	const FString StarterHuoDuanYueZhanPoZhenCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_HuoDuanYueZhanPoZhen");
	const FString StarterHuoTieBiHuiFengCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_HuoTieBiHuiFeng");
	const FString StarterYeXingZhenCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_YeXingZhen");
	const FString StarterYeTiaoXiCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_YeTiaoXi");
	const FString StarterYeHuaYinCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_YeHuaYin");
	const FString StarterYeHuiChunSanCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_YeHuiChunSan");
	const FString StarterShenBuFengCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_ShenBuFeng");
	const FString StarterShenShouZhenCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_ShenShouZhen");
	const FString StarterShenYinZhenCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_ShenYinZhen");
	const FString StarterShenGuoPaiJianZhenCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_ShenGuoPaiJianZhen");
	const FString StarterShenPoZhenJianZhenCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_ShenPoZhenJianZhen");
	const FString StarterShenFengRuiJianZhenCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_ShenFengRuiJianZhen");
	const FString StarterShenYinBaoJianZhenCardPath = StarterRootPath / TEXT("Cards/DA_Card_Starter_ShenYinBaoJianZhen");
	const FString StarterHuoStatusPath = StarterRootPath / TEXT("Statuses/DA_Status_Starter_HuoDaoShi");
	const FString StarterYeStatusPath = StarterRootPath / TEXT("Statuses/DA_Status_Starter_YeYaoYin");
	const FString StarterYeImmunityStatusPath = StarterRootPath / TEXT("Statuses/DA_Status_Starter_YeMianYi");
	const FString StarterShenStatusPath = StarterRootPath / TEXT("Statuses/DA_Status_Starter_ShenJianZhen");
	const FString StarterShenShiQiStatusPath = StarterRootPath / TEXT("Statuses/DA_Status_Starter_ShenShiQi");
	const FString StarterShenFengRuiStatusPath = StarterRootPath / TEXT("Statuses/DA_Status_Starter_ShenFengRui");
	const FString StarterVulnerableStatusPath = StarterRootPath / TEXT("Statuses/DA_Status_Starter_Vulnerable");
	const FString StarterCorrosionStatusPath = StarterRootPath / TEXT("Statuses/DA_Status_Starter_Corrosion");
	const FString StarterWeakStatusPath = StarterRootPath / TEXT("Statuses/DA_Status_Starter_Weak");
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
	const FString StarterShieldGuardShieldBashIntentPath = StarterRootPath / TEXT("EnemyIntents/DA_EnemyIntent_Starter_ShieldGuardShieldBash");
	const FString StarterShieldGuardProtectIntentPath = StarterRootPath / TEXT("EnemyIntents/DA_EnemyIntent_Starter_ShieldGuardProtect");
	const FString StarterShieldGuardHeavySlamIntentPath = StarterRootPath / TEXT("EnemyIntents/DA_EnemyIntent_Starter_ShieldGuardHeavySlam");
	const FString StarterSmokeCorrosionBombIntentPath = StarterRootPath / TEXT("EnemyIntents/DA_EnemyIntent_Starter_SmokeCorrosionBomb");
	const FString StarterSmokeDullingSmokeIntentPath = StarterRootPath / TEXT("EnemyIntents/DA_EnemyIntent_Starter_SmokeDullingSmoke");
	const FString StarterSmokeFireJarIntentPath = StarterRootPath / TEXT("EnemyIntents/DA_EnemyIntent_Starter_SmokeFireJar");
	const FString StarterToxicMistSpreadIntentPath = StarterRootPath / TEXT("EnemyIntents/DA_EnemyIntent_Starter_ToxicMistSpread");
	const FString StarterToxicIgniteIntentPath = StarterRootPath / TEXT("EnemyIntents/DA_EnemyIntent_Starter_ToxicIgnite");
	const FString StarterToxicCallBanditIntentPath = StarterRootPath / TEXT("EnemyIntents/DA_EnemyIntent_Starter_ToxicCallBandit");
	const FString StarterBossMountainSlashIntentPath = StarterRootPath / TEXT("EnemyIntents/DA_EnemyIntent_Starter_BossMountainSlash");
	const FString StarterBossTripleStrikeIntentPath = StarterRootPath / TEXT("EnemyIntents/DA_EnemyIntent_Starter_BossTripleStrike");
	const FString StarterBossRallyIntentPath = StarterRootPath / TEXT("EnemyIntents/DA_EnemyIntent_Starter_BossRally");
	const FString StarterBossChargeIntentPath = StarterRootPath / TEXT("EnemyIntents/DA_EnemyIntent_Starter_BossCharge");
	const FString StarterBladeEnemyPath = StarterRootPath / TEXT("Enemies/DA_Enemy_Starter_BanditBlade");
	const FString StarterCrossbowEnemyPath = StarterRootPath / TEXT("Enemies/DA_Enemy_Starter_BanditCrossbow");
	const FString StarterInstructorEnemyPath = StarterRootPath / TEXT("Enemies/DA_Enemy_Starter_BlackwindInstructor");
	const FString StarterShieldGuardEnemyPath = StarterRootPath / TEXT("Enemies/DA_Enemy_Starter_BanditShieldGuard");
	const FString StarterSmokeEnemyPath = StarterRootPath / TEXT("Enemies/DA_Enemy_Starter_MedicineSmokeBandit");
	const FString StarterToxicEnemyPath = StarterRootPath / TEXT("Enemies/DA_Enemy_Starter_ToxicSmokeAdept");
	const FString StarterBossEnemyPath = StarterRootPath / TEXT("Enemies/DA_Enemy_Starter_BlackwindChiefLikui");
	const FString StarterNormalEncounterPath = StarterRootPath / TEXT("Encounters/DA_Encounter_Starter_Roadblock");
	const FString StarterEliteEncounterPath = StarterRootPath / TEXT("Encounters/DA_Encounter_Starter_Instructor");
	const FString StarterNormalDoubleBladeEncounterPath = StarterRootPath / TEXT("Encounters/DA_Encounter_Starter_DoubleBlade");
	const FString StarterNormalSmokeBladeEncounterPath = StarterRootPath / TEXT("Encounters/DA_Encounter_Starter_SmokeBlade");
	const FString StarterNormalShieldBladeEncounterPath = StarterRootPath / TEXT("Encounters/DA_Encounter_Starter_ShieldBlade");
	const FString StarterNormalSmokeShieldEncounterPath = StarterRootPath / TEXT("Encounters/DA_Encounter_Starter_SmokeShield");
	const FString StarterNormalToxicArrowsEncounterPath = StarterRootPath / TEXT("Encounters/DA_Encounter_Starter_ToxicArrows");
	const FString StarterEliteIronWallEncounterPath = StarterRootPath / TEXT("Encounters/DA_Encounter_Starter_IronWallInstructor");
	const FString StarterEliteToxicRitualEncounterPath = StarterRootPath / TEXT("Encounters/DA_Encounter_Starter_ToxicRitual");
	const FString StarterBossEncounterPath = StarterRootPath / TEXT("Encounters/DA_Encounter_Starter_BlackwindChief");
	const FString StarterBootstrapPath = StarterRootPath / TEXT("Bootstrap/DA_PrototypeBootstrap_StarterChapter1");
	const FString StarterRunRoutePath = StarterRootPath / TEXT("Run/DA_RunRoute_StarterChapter1");
	const FString StarterHuoGrowthConfigPath = StarterRootPath / TEXT("Growth/DA_CharacterGrowthConfig_Starter_HuoDuanyue");
	const FString StarterYeGrowthConfigPath = StarterRootPath / TEXT("Growth/DA_CharacterGrowthConfig_Starter_YeBanxia");
	const FString StarterShenGrowthConfigPath = StarterRootPath / TEXT("Growth/DA_CharacterGrowthConfig_Starter_ShenQingxian");
	const FString StarterHuoDuanYueZhanPoZhenEvolutionPath = StarterRootPath / TEXT("Growth/DA_CardEvolution_Starter_HuoDuanYueZhanPoZhen");
	const FString StarterBronzeMirrorGuardRelicPath = StarterRootPath / TEXT("Relics/DA_Relic_Starter_BronzeMirrorGuard");
	const FString StarterTokenZeroDrawRelicPath = StarterRootPath / TEXT("Relics/DA_Relic_Starter_TokenZeroDraw");
	const FName StarterBootstrapId(TEXT("prototype.bootstrap.starter.chapter1"));
	const FName StarterRuleConfigId(TEXT("rule.starter.chapter1"));
	const FName StarterNormalEncounterId(TEXT("encounter.starter.chapter1.roadblock"));
	const FName StarterEliteEncounterId(TEXT("encounter.starter.chapter1.instructor"));
	const FName StarterHuoCharacterId(TEXT("character.starter.huo.duanyue"));
	const FName StarterYeCharacterId(TEXT("character.starter.ye.banxia"));
	const FName StarterShenCharacterId(TEXT("character.starter.shen.qingxian"));
	const FName StarterHuoStatusId(TEXT("status.starter.huo.daoshi"));
	const FName StarterYeStatusId(TEXT("status.starter.ye.yaoyin"));
	const FName StarterYeImmunityStatusId(TEXT("status.starter.ye.mianyi"));
	const FName StarterShenStatusId(TEXT("status.starter.shen.jianzhen"));
	const FName StarterShenShiQiStatusId(TEXT("status.starter.shen.shiqi"));
	const FName StarterShenFengRuiStatusId(TEXT("status.starter.shen.fengrui"));
	const FName StarterVulnerableStatusId(TEXT("status.starter.enemy.vulnerable"));
	const FName StarterCorrosionStatusId(TEXT("status.starter.enemy.corrosion"));
	const FName StarterWeakStatusId(TEXT("status.starter.enemy.weak"));
	const FName StarterHuoUltimateId(TEXT("ultimate.starter.huo.duanyuejueshi"));
	const FName StarterYeUltimateId(TEXT("ultimate.starter.ye.huitianxumai"));
	const FName StarterShenUltimateId(TEXT("ultimate.starter.shen.wanxiangguizhen"));
	const FName StarterHuoGrowthConfigId(TEXT("growth.config.starter.huo.duanyue"));
	const FName StarterYeGrowthConfigId(TEXT("growth.config.starter.ye.banxia"));
	const FName StarterShenGrowthConfigId(TEXT("growth.config.starter.shen.qingxian"));
	const FName StarterHuoLieFengCardId(TEXT("card.starter.huo.liefeng"));
	const FName StarterHuoWenJiaCardId(TEXT("card.starter.huo.wenjia"));
	const FName StarterHuoDuanYueZhanCardId(TEXT("card.starter.huo.duanyuezhan"));
	const FName StarterHuoDuanYueZhanPoZhenCardId(TEXT("card.starter.huo.duanyuezhan.pozhen"));
	const FName StarterHuoTieBiHuiFengCardId(TEXT("card.starter.huo.tiebihuifeng"));
	const FName StarterYeXingZhenCardId(TEXT("card.starter.ye.xingzhen"));
	const FName StarterYeTiaoXiCardId(TEXT("card.starter.ye.tiaoxi"));
	const FName StarterYeHuaYinCardId(TEXT("card.starter.ye.huayin"));
	const FName StarterYeHuiChunSanCardId(TEXT("card.starter.ye.huichunsan"));
	const FName StarterShenBuFengCardId(TEXT("card.starter.shen.bufeng"));
	const FName StarterShenShouZhenCardId(TEXT("card.starter.shen.shouzhen"));
	const FName StarterShenYinZhenCardId(TEXT("card.starter.shen.yinzhen"));
	const FName StarterShenGuoPaiJianZhenCardId(TEXT("card.starter.shen.guopaijianzhen"));
	const FName StarterShenPoZhenJianZhenCardId(TEXT("card.starter.shen.pozhenjianzhen"));
	const FName StarterShenFengRuiJianZhenCardId(TEXT("card.starter.shen.fengruijianzhen"));
	const FName StarterShenYinBaoJianZhenCardId(TEXT("card.starter.shen.yinbaojianzhen"));
	const FName StarterBladeEnemyId(TEXT("enemy.starter.bandit.blade"));
	const FName StarterCrossbowEnemyId(TEXT("enemy.starter.bandit.crossbow"));
	const FName StarterInstructorEnemyId(TEXT("enemy.starter.blackwind.instructor"));
	const FName StarterShieldGuardEnemyId(TEXT("enemy.starter.bandit.shield_guard"));
	const FName StarterSmokeEnemyId(TEXT("enemy.starter.bandit.medicine_smoke"));
	const FName StarterToxicEnemyId(TEXT("enemy.starter.toxic_smoke_adept"));
	const FName StarterBossEnemyId(TEXT("enemy.starter.blackwind.chief.likui"));
	const FName StarterNormalDoubleBladeEncounterId(TEXT("encounter.starter.chapter1.double_blade"));
	const FName StarterNormalSmokeBladeEncounterId(TEXT("encounter.starter.chapter1.smoke_blade"));
	const FName StarterNormalShieldBladeEncounterId(TEXT("encounter.starter.chapter1.shield_blade"));
	const FName StarterNormalSmokeShieldEncounterId(TEXT("encounter.starter.chapter1.smoke_shield"));
	const FName StarterNormalToxicArrowsEncounterId(TEXT("encounter.starter.chapter1.toxic_arrows"));
	const FName StarterEliteIronWallEncounterId(TEXT("encounter.starter.chapter1.iron_wall_instructor"));
	const FName StarterEliteToxicRitualEncounterId(TEXT("encounter.starter.chapter1.toxic_ritual"));
	const FName StarterBossEncounterId(TEXT("encounter.starter.chapter1.blackwind_chief"));
	const FName StarterRouteId(TEXT("run.route.starter.chapter1"));
	const FName StarterHuoDuanYueZhanPoZhenEvolutionId(TEXT("evo.starter.huo.duanyuezhan.pozhen"));
	const FName StarterBronzeMirrorGuardRelicId(TEXT("relic_bronze_mirror_guard"));
	const FName StarterTokenZeroDrawRelicId(TEXT("relic_token_zero_draw"));
	const FName StarterOpeningBattleNodeId(TEXT("run.starter.node.battle.roadblock"));
	const FName StarterRewardNodeId(TEXT("run.starter.node.reward.spoils"));
	const FName StarterEventNodeId(TEXT("run.starter.node.event.cliff_notice"));
	const FName StarterShopNodeId(TEXT("run.starter.node.shop.camp_trader"));
	const FName StarterEliteBattleNodeId(TEXT("run.starter.node.battle.instructor"));
	const FName StarterBossBattleNodeId(TEXT("run.starter.node.battle.blackwind_chief"));

	FFinalEnemyRosterEntry MakeEnemyRosterEntry(UFinalEnemyDefinition* EnemyDefinition, const int32 PositionIndex)
	{
		FFinalEnemyRosterEntry Entry;
		Entry.EnemyDefinition = EnemyDefinition;
		Entry.PositionIndex = PositionIndex;
		Entry.SpawnWave = 1;
		return Entry;
	}

	void ResetIntentSelectionDefaults(UFinalEnemyIntentDefinition* IntentDefinition)
	{
		if (IntentDefinition == nullptr)
		{
			return;
		}

		IntentDefinition->Weight = 1;
		IntentDefinition->MinPreviewRound = 1;
		IntentDefinition->MaxPreviewRound = 0;
		IntentDefinition->MinEnemyHpPercent = 0.0f;
		IntentDefinition->MaxEnemyHpPercent = 1.0f;
		IntentDefinition->bDisallowRepeatLastIntent = false;
		IntentDefinition->PhaseTags.Reset();
	}

	void ConfigureStarterEncounter(
		UFinalBattleEncounterDefinition* Encounter,
		const FName EncounterId,
		const FText& DisplayName,
		UFinalBattleRuleConfig* RuleConfig,
		const TArray<UFinalEnemyDefinition*>& Enemies)
	{
		if (Encounter == nullptr)
		{
			return;
		}

		Encounter->EncounterId = FFinalEncounterId(EncounterId);
		Encounter->DisplayName = DisplayName;
		Encounter->RuleConfig = RuleConfig;
		Encounter->EnemyRoster.Reset();
		for (int32 Index = 0; Index < Enemies.Num(); ++Index)
		{
			Encounter->EnemyRoster.Add(MakeEnemyRosterEntry(Enemies[Index], Index));
		}
	}
}

void FFinalStarterContentBundleBuilder::Build(TSet<UPackage*>& PackagesToSave)
{
	using namespace FinalPrototypeContentBootstrap;

	bool bCreatedAsset = false;
	UFinalBattleRuleConfig* StarterRuleConfig = LoadOrCreateAsset<UFinalBattleRuleConfig>(StarterRulesPath, bCreatedAsset);
	StarterRuleConfig->RuleConfigId = FFinalRuleConfigId(StarterRuleConfigId);
	StarterRuleConfig->InitialAP = 3;
	StarterRuleConfig->InitialHandSize = 5;
	StarterRuleConfig->TurnStartDrawCount = 5;
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
	StarterHuoStatus->SummaryText = FText::FromString(TEXT("霍断岳的签名资源。Runtime 已支持通过牌效果和 OwnerTookHealthDamage 触发获得层数，并由断岳斩显式消耗来追加削韧。"));
	StarterHuoStatus->MaxStacks = 6;
	StarterHuoStatus->DefaultDuration = 0;
	StarterHuoStatus->OnTickEffects.Reset();
	TrackPackage(StarterHuoStatus, PackagesToSave);

	UFinalStatusDefinition* StarterYeStatus = LoadOrCreateAsset<UFinalStatusDefinition>(StarterYeStatusPath, bCreatedAsset);
	StarterYeStatus->StatusId = FFinalStatusId(StarterYeStatusId);
	StarterYeStatus->DisplayName = FText::FromString(TEXT("药引"));
	StarterYeStatus->StatusCategory = EFinalStatusCategory::Signature;
	StarterYeStatus->SummaryText = FText::FromString(TEXT("叶半夏的签名资源。首波 Runtime 已支持通过行针与调息获得层数，并由化引与回春散显式消耗换取 AP 与过牌收益。"));
	StarterYeStatus->MaxStacks = 9;
	StarterYeStatus->DefaultDuration = 0;
	StarterYeStatus->OnTickEffects.Reset();
	TrackPackage(StarterYeStatus, PackagesToSave);

	UFinalStatusDefinition* StarterYeImmunityStatus = LoadOrCreateAsset<UFinalStatusDefinition>(StarterYeImmunityStatusPath, bCreatedAsset);
	StarterYeImmunityStatus->StatusId = FFinalStatusId(StarterYeImmunityStatusId);
	StarterYeImmunityStatus->DisplayName = FText::FromString(TEXT("生命免疫"));
	StarterYeImmunityStatus->StatusCategory = EFinalStatusCategory::Buff;
	StarterYeImmunityStatus->SummaryText = FText::FromString(TEXT("抵消下一次穿透护盾的共享生命伤害；这是免疫体系下的首版生命保护子类。触发后消耗，若到玩家回合结束仍未触发则失效。"));
	StarterYeImmunityStatus->MaxStacks = 1;
	StarterYeImmunityStatus->DefaultDuration = 1;
	StarterYeImmunityStatus->OutgoingDamagePercentPerStack = 0;
	StarterYeImmunityStatus->bExpireAtPlayerTurnEnd = true;
	StarterYeImmunityStatus->bConsumeOnSuccessfulOwnerDamage = false;
	StarterYeImmunityStatus->bOnlyAffectAttackCards = false;
	StarterYeImmunityStatus->IncomingTeamHealthDamageReductionPercentPerStack = 100;
	StarterYeImmunityStatus->bConsumeOnPreventedTeamHealthDamage = true;
	StarterYeImmunityStatus->OnTickEffects.Reset();
	TrackPackage(StarterYeImmunityStatus, PackagesToSave);

	UFinalStatusDefinition* StarterShenStatus = LoadOrCreateAsset<UFinalStatusDefinition>(StarterShenStatusPath, bCreatedAsset);
	StarterShenStatus->StatusId = FFinalStatusId(StarterShenStatusId);
	StarterShenStatus->DisplayName = FText::FromString(TEXT("阵诀"));
	StarterShenStatus->StatusCategory = EFinalStatusCategory::Signature;
	StarterShenStatus->SummaryText = FText::FromString(TEXT("沈清弦的剑阵本体已改为 Battle 内衍生牌协议；此签名状态资产仅保留角色签名展示占位。"));
	StarterShenStatus->MaxStacks = 9;
	StarterShenStatus->DefaultDuration = 0;
	StarterShenStatus->OutgoingDamagePercentPerStack = 0;
	StarterShenStatus->bExpireAtPlayerTurnEnd = false;
	StarterShenStatus->bConsumeOnSuccessfulOwnerDamage = false;
	StarterShenStatus->bOnlyAffectAttackCards = false;
	StarterShenStatus->OnTickEffects.Reset();
	TrackPackage(StarterShenStatus, PackagesToSave);

	UFinalStatusDefinition* StarterShenShiQiStatus = LoadOrCreateAsset<UFinalStatusDefinition>(StarterShenShiQiStatusPath, bCreatedAsset);
	StarterShenShiQiStatus->StatusId = FFinalStatusId(StarterShenShiQiStatusId);
	StarterShenShiQiStatus->DisplayName = FText::FromString(TEXT("士气"));
	StarterShenShiQiStatus->StatusCategory = EFinalStatusCategory::Buff;
	StarterShenShiQiStatus->SummaryText = FText::FromString(TEXT("本回合内伤害提高 20%。"));
	StarterShenShiQiStatus->MaxStacks = 9;
	StarterShenShiQiStatus->DefaultDuration = 0;
	StarterShenShiQiStatus->OutgoingDamagePercentPerStack = 20;
	StarterShenShiQiStatus->bExpireAtPlayerTurnEnd = true;
	StarterShenShiQiStatus->bConsumeOnSuccessfulOwnerDamage = false;
	StarterShenShiQiStatus->bOnlyAffectAttackCards = false;
	StarterShenShiQiStatus->OnTickEffects.Reset();
	TrackPackage(StarterShenShiQiStatus, PackagesToSave);

	UFinalStatusDefinition* StarterShenFengRuiStatus = LoadOrCreateAsset<UFinalStatusDefinition>(StarterShenFengRuiStatusPath, bCreatedAsset);
	StarterShenFengRuiStatus->StatusId = FFinalStatusId(StarterShenFengRuiStatusId);
	StarterShenFengRuiStatus->DisplayName = FText::FromString(TEXT("锋锐"));
	StarterShenFengRuiStatus->StatusCategory = EFinalStatusCategory::Buff;
	StarterShenFengRuiStatus->SummaryText = FText::FromString(TEXT("下一张攻击牌伤害提高 20%，若本回合未触发则在回合结束时失效。"));
	StarterShenFengRuiStatus->MaxStacks = 9;
	StarterShenFengRuiStatus->DefaultDuration = 0;
	StarterShenFengRuiStatus->OutgoingDamagePercentPerStack = 0;
	StarterShenFengRuiStatus->bExpireAtPlayerTurnEnd = true;
	StarterShenFengRuiStatus->bConsumeOnSuccessfulOwnerDamage = true;
	StarterShenFengRuiStatus->bOnlyAffectAttackCards = true;
	StarterShenFengRuiStatus->bProjectToOwnedHandCards = true;
	StarterShenFengRuiStatus->ProjectedCardTypeFilter = EFinalCardType::Attack;
	StarterShenFengRuiStatus->ProjectedOutgoingDamagePercentPerStack = 20;
	StarterShenFengRuiStatus->OnTickEffects.Reset();
	TrackPackage(StarterShenFengRuiStatus, PackagesToSave);

	UFinalStatusDefinition* StarterVulnerableStatus = LoadOrCreateAsset<UFinalStatusDefinition>(StarterVulnerableStatusPath, bCreatedAsset);
	StarterVulnerableStatus->StatusId = FFinalStatusId(StarterVulnerableStatusId);
	StarterVulnerableStatus->DisplayName = FText::FromString(TEXT("易伤"));
	StarterVulnerableStatus->StatusCategory = EFinalStatusCategory::Debuff;
	StarterVulnerableStatus->SummaryText = FText::FromString(TEXT("首版敌人意图可施加的团队减益；当前主要用于预告和状态验证，后续补充受击增伤规则。"));
	StarterVulnerableStatus->MaxStacks = 3;
	StarterVulnerableStatus->DefaultDuration = 1;
	StarterVulnerableStatus->OutgoingDamagePercentPerStack = 0;
	StarterVulnerableStatus->bExpireAtPlayerTurnEnd = true;
	StarterVulnerableStatus->bConsumeOnSuccessfulOwnerDamage = false;
	StarterVulnerableStatus->bOnlyAffectAttackCards = false;
	StarterVulnerableStatus->IncomingTeamHealthDamageReductionPercentPerStack = 0;
	StarterVulnerableStatus->bConsumeOnPreventedTeamHealthDamage = false;
	StarterVulnerableStatus->OnTickEffects.Reset();
	TrackPackage(StarterVulnerableStatus, PackagesToSave);

	UFinalStatusDefinition* StarterCorrosionStatus = LoadOrCreateAsset<UFinalStatusDefinition>(StarterCorrosionStatusPath, bCreatedAsset);
	StarterCorrosionStatus->StatusId = FFinalStatusId(StarterCorrosionStatusId);
	StarterCorrosionStatus->DisplayName = FText::FromString(TEXT("腐蚀"));
	StarterCorrosionStatus->StatusCategory = EFinalStatusCategory::Debuff;
	StarterCorrosionStatus->SummaryText = FText::FromString(TEXT("首版持续伤害减益；当前可被敌人意图施加，OnTick 伤害规则后续补全。"));
	StarterCorrosionStatus->MaxStacks = 9;
	StarterCorrosionStatus->DefaultDuration = 3;
	StarterCorrosionStatus->OutgoingDamagePercentPerStack = 0;
	StarterCorrosionStatus->bExpireAtPlayerTurnEnd = false;
	StarterCorrosionStatus->bConsumeOnSuccessfulOwnerDamage = false;
	StarterCorrosionStatus->bOnlyAffectAttackCards = false;
	StarterCorrosionStatus->IncomingTeamHealthDamageReductionPercentPerStack = 0;
	StarterCorrosionStatus->bConsumeOnPreventedTeamHealthDamage = false;
	StarterCorrosionStatus->OnTickEffects.Reset();
	TrackPackage(StarterCorrosionStatus, PackagesToSave);

	UFinalStatusDefinition* StarterWeakStatus = LoadOrCreateAsset<UFinalStatusDefinition>(StarterWeakStatusPath, bCreatedAsset);
	StarterWeakStatus->StatusId = FFinalStatusId(StarterWeakStatusId);
	StarterWeakStatus->DisplayName = FText::FromString(TEXT("虚弱"));
	StarterWeakStatus->StatusCategory = EFinalStatusCategory::Debuff;
	StarterWeakStatus->SummaryText = FText::FromString(TEXT("首版敌人软控制减益；当前可被施加和展示，实际输出削减规则后续补全。"));
	StarterWeakStatus->MaxStacks = 3;
	StarterWeakStatus->DefaultDuration = 1;
	StarterWeakStatus->OutgoingDamagePercentPerStack = 0;
	StarterWeakStatus->bExpireAtPlayerTurnEnd = true;
	StarterWeakStatus->bConsumeOnSuccessfulOwnerDamage = false;
	StarterWeakStatus->bOnlyAffectAttackCards = false;
	StarterWeakStatus->IncomingTeamHealthDamageReductionPercentPerStack = 0;
	StarterWeakStatus->bConsumeOnPreventedTeamHealthDamage = false;
	StarterWeakStatus->OnTickEffects.Reset();
	TrackPackage(StarterWeakStatus, PackagesToSave);

	UFinalRelicDefinition* StarterBronzeMirrorGuardRelic = LoadOrCreateAsset<UFinalRelicDefinition>(StarterBronzeMirrorGuardRelicPath, bCreatedAsset);
	StarterBronzeMirrorGuardRelic->RelicId = FFinalRelicId(StarterBronzeMirrorGuardRelicId);
	StarterBronzeMirrorGuardRelic->DisplayId = TEXT("Relic.Starter.BronzeMirrorGuard");
	StarterBronzeMirrorGuardRelic->DisplayName = FText::FromString(TEXT("护心铜镜"));
	StarterBronzeMirrorGuardRelic->Rarity = EFinalRarity::Common;
	StarterBronzeMirrorGuardRelic->Description = FText::FromString(TEXT("每回合第一次承受实际生命损失后，获得 8 护盾。"));
	StarterBronzeMirrorGuardRelic->RuntimeTriggers.Reset();
	{
		FFinalRuntimeTriggerDefinition& TriggerDefinition = StarterBronzeMirrorGuardRelic->RuntimeTriggers.AddDefaulted_GetRef();
		TriggerDefinition.Domain = EFinalRuntimeTriggerDomain::Battle;
		TriggerDefinition.Window = EFinalRuntimeTriggerWindow::PlayerTeamTookHealthDamage;
		TriggerDefinition.Limit = EFinalRuntimeTriggerLimit::OncePerPlayerTurn;
		AddShieldEffect(
			StarterBronzeMirrorGuardRelic,
			TriggerDefinition.Effects,
			TEXT("effect.starter.relic.bronze_mirror_guard.shield"),
			EFinalBattleUnitTargetRule::TeamPlayer,
			8.0f,
			EFinalBattleScalarMode::Flat,
			EFinalBattleSourceStat::None,
			FText::FromString(TEXT("每回合第一次承受实际生命损失后，获得 8 护盾。")));
	}
	TrackPackage(StarterBronzeMirrorGuardRelic, PackagesToSave);

	UFinalRelicDefinition* StarterTokenZeroDrawRelic = LoadOrCreateAsset<UFinalRelicDefinition>(StarterTokenZeroDrawRelicPath, bCreatedAsset);
	StarterTokenZeroDrawRelic->RelicId = FFinalRelicId(StarterTokenZeroDrawRelicId);
	StarterTokenZeroDrawRelic->DisplayId = TEXT("Relic.Starter.TokenZeroDraw");
	StarterTokenZeroDrawRelic->DisplayName = FText::FromString(TEXT("阵门木签"));
	StarterTokenZeroDrawRelic->Rarity = EFinalRarity::Common;
	StarterTokenZeroDrawRelic->Description = FText::FromString(TEXT("每回合第一次打出 0 AP 牌时，抽 1 张牌。若抽到攻击牌，则该牌及其同源实例本回合费用 -1 AP，且伤害提高 20%。"));
	StarterTokenZeroDrawRelic->RuntimeTriggers.Reset();
	{
		FFinalRuntimeTriggerDefinition& TriggerDefinition = StarterTokenZeroDrawRelic->RuntimeTriggers.AddDefaulted_GetRef();
		TriggerDefinition.Domain = EFinalRuntimeTriggerDomain::Battle;
		TriggerDefinition.Window = EFinalRuntimeTriggerWindow::PlayerCardResolved;
		TriggerDefinition.Limit = EFinalRuntimeTriggerLimit::OncePerPlayerTurn;

		FFinalBattleResolvedCardRequirement ResolvedCardRequirement;
		ResolvedCardRequirement.bRequireCardCostAP = true;
		ResolvedCardRequirement.RequiredCardCostAP = 0;
		AddResolvedCardCondition(StarterTokenZeroDrawRelic, TriggerDefinition.Conditions, ResolvedCardRequirement);

		AddDrawEffect(
			StarterTokenZeroDrawRelic,
			TriggerDefinition.Effects,
			TEXT("effect.starter.relic.token_zero_draw.draw"),
			1,
			FText::FromString(TEXT("每回合第一次打出 0 AP 牌时，抽 1 张牌。")));

		FFinalTriggeredCardModifierDefinition& TriggeredModifier = TriggerDefinition.TriggeredCardModifiers.AddDefaulted_GetRef();
		TriggeredModifier.TargetSource = EFinalTriggeredCardModifierTargetSource::DrawnCardsFromExecutedEffects;
		TriggeredModifier.bRequireCardType = true;
		TriggeredModifier.RequiredCardType = EFinalCardType::Attack;
		TriggeredModifier.CostDeltaAP = -1;
		TriggeredModifier.OutgoingDamagePercentDelta = 20;
		TriggeredModifier.DurationPolicy = EFinalTriggeredCardModifierDurationPolicy::UntilPlayed;
		TriggeredModifier.bExpireAtPlayerTurnEnd = true;
		TriggeredModifier.bApplyToAllSameSourceRunCardInstances = true;
	}
	TrackPackage(StarterTokenZeroDrawRelic, PackagesToSave);

	UFinalUltimateDefinition* StarterHuoUltimate = LoadOrCreateAsset<UFinalUltimateDefinition>(StarterHuoUltimatePath, bCreatedAsset);
	StarterHuoUltimate->UltimateId = FFinalUltimateId(StarterHuoUltimateId);
	StarterHuoUltimate->OwnerUnitId = StarterHuoCharacterId;
	StarterHuoUltimate->DisplayName = FText::FromString(TEXT("断岳绝式"));
	StarterHuoUltimate->BaseCostEP = 45;
	StarterHuoUltimate->RulesText = FText::FromString(TEXT("对单体目标造成相当于攻击力 220% 的伤害，并额外造成 6 点削韧。若目标处于 Break，额外造成相当于攻击力 88% 的伤害。获得 2 层刀势。"));
	StarterHuoUltimate->Effects.Reset();
	AddBonusBreakEffect(
		StarterHuoUltimate,
		StarterHuoUltimate->Effects,
		TEXT("effect.starter.huo.ultimate.duanyuejueshi.break"),
		EFinalBattleUnitTargetRule::SelectedEnemy,
		6.0f,
		EFinalBattleScalarMode::Flat);
	AddDamageEffect(
		StarterHuoUltimate,
		StarterHuoUltimate->Effects,
		TEXT("effect.starter.huo.ultimate.duanyuejueshi.damage"),
		EFinalBattleUnitTargetRule::SelectedEnemy,
		2.2f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::Attack,
		1);
	{
		FFinalBattleTargetStateRequirement BrokenTargetRequirement;
		BrokenTargetRequirement.bRequireEnemyTarget = true;
		BrokenTargetRequirement.bRequireTargetBroken = true;
		BrokenTargetRequirement.bRequireTargetAlive = true;
		AddDamageEffect(
			StarterHuoUltimate,
			StarterHuoUltimate->Effects,
			TEXT("effect.starter.huo.ultimate.duanyuejueshi.break_damage"),
			EFinalBattleUnitTargetRule::SelectedEnemy,
			0.88f,
			EFinalBattleScalarMode::SourceStatMultiplier,
			EFinalBattleSourceStat::Attack,
			1,
			FText::FromString(TEXT("目标处于 Break 时执行的额外伤害。")),
			&BrokenTargetRequirement);
	}
	AddApplyStatusEffect(
		StarterHuoUltimate,
		StarterHuoUltimate->Effects,
		TEXT("effect.starter.huo.ultimate.duanyuejueshi.daoshi"),
		EFinalBattleUnitTargetRule::Self,
		StarterHuoStatus,
		2);
	TrackPackage(StarterHuoUltimate, PackagesToSave);

	UFinalUltimateDefinition* StarterYeUltimate = LoadOrCreateAsset<UFinalUltimateDefinition>(StarterYeUltimatePath, bCreatedAsset);
	StarterYeUltimate->UltimateId = FFinalUltimateId(StarterYeUltimateId);
	StarterYeUltimate->OwnerUnitId = StarterYeCharacterId;
	StarterYeUltimate->DisplayName = FText::FromString(TEXT("回天续脉"));
	StarterYeUltimate->BaseCostEP = 45;
	StarterYeUltimate->RulesText = FText::FromString(TEXT("回复 18 点共享生命，获得 1 层生命免疫，并回复 1 AP。"));
	StarterYeUltimate->Effects.Reset();
	AddHealEffect(
		StarterYeUltimate,
		StarterYeUltimate->Effects,
		TEXT("effect.starter.ye.ultimate.huitianxumai.heal"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		18.0f,
		EFinalBattleScalarMode::Flat,
		EFinalBattleSourceStat::None);
	AddApplyStatusEffect(
		StarterYeUltimate,
		StarterYeUltimate->Effects,
		TEXT("effect.starter.ye.ultimate.huitianxumai.immunity"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		StarterYeImmunityStatus,
		1,
		FText::FromString(TEXT("对 team_player 施加一次生命免疫。")));
	AddGainApEffect(
		StarterYeUltimate,
		StarterYeUltimate->Effects,
		TEXT("effect.starter.ye.ultimate.huitianxumai.gain_ap"),
		1,
		FText::FromString(TEXT("回天续脉的少量 AP 修正。")));
	TrackPackage(StarterYeUltimate, PackagesToSave);

	UFinalUltimateDefinition* StarterShenUltimate = LoadOrCreateAsset<UFinalUltimateDefinition>(StarterShenUltimatePath, bCreatedAsset);
	StarterShenUltimate->UltimateId = FFinalUltimateId(StarterShenUltimateId);
	StarterShenUltimate->OwnerUnitId = StarterShenCharacterId;
	StarterShenUltimate->DisplayName = FText::FromString(TEXT("万象归阵"));
	StarterShenUltimate->BaseCostEP = 45;
	StarterShenUltimate->RulesText = FText::FromString(TEXT("抽 2 张牌。生成 1 张剑阵牌到手牌。每名角色获得 1 层士气。"));
	StarterShenUltimate->Effects.Reset();
	TrackPackage(StarterShenUltimate, PackagesToSave);

	UFinalCharacterDefinition* StarterHuoCharacter = LoadOrCreateAsset<UFinalCharacterDefinition>(StarterHuoCharacterPath, bCreatedAsset);
	UFinalCharacterGrowthConfig* StarterHuoGrowthConfig = LoadOrCreateAsset<UFinalCharacterGrowthConfig>(StarterHuoGrowthConfigPath, bCreatedAsset);
	StarterHuoGrowthConfig->GrowthConfigId = FFinalCharacterGrowthConfigId(StarterHuoGrowthConfigId);
	StarterHuoGrowthConfig->BaseBreakthroughRequiredValue = 100;
	StarterHuoGrowthConfig->PreferredBreakthroughFactTypes = {
		EFinalBattleGrowthFactType::OwnedCardResolved,
		EFinalBattleGrowthFactType::BreakDamageDealt,
		EFinalBattleGrowthFactType::EnemyKilled,
		EFinalBattleGrowthFactType::BattleVictoryBaseReward
	};
	StarterHuoGrowthConfig->BreakthroughGainScalarByFactType = {
		{ EFinalBattleGrowthFactType::OwnedCardResolved, 8.0f },
		{ EFinalBattleGrowthFactType::BreakDamageDealt, 2.0f },
		{ EFinalBattleGrowthFactType::EnemyKilled, 6.0f },
		{ EFinalBattleGrowthFactType::BattleVictoryBaseReward, 1.0f }
	};
	TrackPackage(StarterHuoGrowthConfig, PackagesToSave);

	UFinalCharacterGrowthConfig* StarterYeGrowthConfig = LoadOrCreateAsset<UFinalCharacterGrowthConfig>(StarterYeGrowthConfigPath, bCreatedAsset);
	StarterYeGrowthConfig->GrowthConfigId = FFinalCharacterGrowthConfigId(StarterYeGrowthConfigId);
	StarterYeGrowthConfig->BaseBreakthroughRequiredValue = 100;
	StarterYeGrowthConfig->PreferredBreakthroughFactTypes = {
		EFinalBattleGrowthFactType::OwnedCardResolved,
		EFinalBattleGrowthFactType::EffectiveHealingDone,
		EFinalBattleGrowthFactType::BattleVictoryBaseReward
	};
	StarterYeGrowthConfig->BreakthroughGainScalarByFactType = {
		{ EFinalBattleGrowthFactType::OwnedCardResolved, 4.0f },
		{ EFinalBattleGrowthFactType::EffectiveHealingDone, 1.5f },
		{ EFinalBattleGrowthFactType::BattleVictoryBaseReward, 1.0f }
	};
	TrackPackage(StarterYeGrowthConfig, PackagesToSave);

	UFinalCharacterGrowthConfig* StarterShenGrowthConfig = LoadOrCreateAsset<UFinalCharacterGrowthConfig>(StarterShenGrowthConfigPath, bCreatedAsset);
	StarterShenGrowthConfig->GrowthConfigId = FFinalCharacterGrowthConfigId(StarterShenGrowthConfigId);
	StarterShenGrowthConfig->BaseBreakthroughRequiredValue = 100;
	StarterShenGrowthConfig->PreferredBreakthroughFactTypes = {
		EFinalBattleGrowthFactType::OwnedCardResolved,
		EFinalBattleGrowthFactType::BattleVictoryBaseReward
	};
	StarterShenGrowthConfig->BreakthroughGainScalarByFactType = {
		{ EFinalBattleGrowthFactType::OwnedCardResolved, 5.0f },
		{ EFinalBattleGrowthFactType::BattleVictoryBaseReward, 1.0f }
	};
	TrackPackage(StarterShenGrowthConfig, PackagesToSave);

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
	StarterHuoCharacter->GrowthConfigId = StarterHuoGrowthConfig->GrowthConfigId;
	StarterHuoCharacter->UltimateId = StarterHuoUltimate->UltimateId;
	StarterHuoCharacter->SignatureStatusId = StarterHuoStatus->StatusId;
	StarterHuoCharacter->BattleTriggers.Reset();
	{
		FFinalRuntimeTriggerDefinition& TookDamageTrigger = StarterHuoCharacter->BattleTriggers.AddDefaulted_GetRef();
		TookDamageTrigger.Domain = EFinalRuntimeTriggerDomain::Battle;
		TookDamageTrigger.Window = EFinalRuntimeTriggerWindow::OwnerTookHealthDamage;
		TookDamageTrigger.Limit = EFinalRuntimeTriggerLimit::None;
		AddApplyStatusEffect(
			StarterHuoCharacter,
			TookDamageTrigger.Effects,
			TEXT("effect.starter.huo.trigger.took_damage.daoshi"),
			EFinalBattleUnitTargetRule::Self,
			StarterHuoStatus,
			1,
			FText::FromString(TEXT("受压得刀势：霍断岳所属队伍生命受损时，霍断岳获得 1 层刀势。")));
	}
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
	StarterYeCharacter->GrowthConfigId = StarterYeGrowthConfig->GrowthConfigId;
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
	StarterShenCharacter->GrowthConfigId = StarterShenGrowthConfig->GrowthConfigId;
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
	StarterHuoLieFengCard->RulesText = FText::FromString(TEXT("对目标造成相当于攻击力 110% 的伤害。额外造成 2 点削韧，并获得 1 层刀势。"));
	StarterHuoLieFengCard->Effects.Reset();
	AddBonusBreakEffect(
		StarterHuoLieFengCard,
		StarterHuoLieFengCard->Effects,
		TEXT("effect.starter.huo.liefeng.break"),
		EFinalBattleUnitTargetRule::SelectedEnemy,
		2.0f,
		EFinalBattleScalarMode::Flat);
	AddDamageEffect(
		StarterHuoLieFengCard,
		StarterHuoLieFengCard->Effects,
		TEXT("effect.starter.huo.liefeng.damage"),
		EFinalBattleUnitTargetRule::SelectedEnemy,
		1.1f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::Attack,
		1);
	AddApplyStatusEffect(
		StarterHuoLieFengCard,
		StarterHuoLieFengCard->Effects,
		TEXT("effect.starter.huo.liefeng.daoshi"),
		EFinalBattleUnitTargetRule::Self,
		StarterHuoStatus,
		1);
	TrackPackage(StarterHuoLieFengCard, PackagesToSave);

	UFinalCardDefinition* StarterHuoWenJiaCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterHuoWenJiaCardPath, bCreatedAsset);
	StarterHuoWenJiaCard->CardId = FFinalCardId(StarterHuoWenJiaCardId);
	StarterHuoWenJiaCard->OwnerUnitId = StarterHuoCharacterId;
	StarterHuoWenJiaCard->DisplayName = FText::FromString(TEXT("稳架"));
	StarterHuoWenJiaCard->CardType = EFinalCardType::Skill;
	StarterHuoWenJiaCard->Rarity = EFinalRarity::Common;
	StarterHuoWenJiaCard->BaseCostAP = 1;
	StarterHuoWenJiaCard->RulesText = FText::FromString(TEXT("获得相当于防御力 100% 的护盾。"));
	StarterHuoWenJiaCard->Effects.Reset();
	AddShieldEffect(
		StarterHuoWenJiaCard,
		StarterHuoWenJiaCard->Effects,
		TEXT("effect.starter.huo.wenjia.shield"),
		EFinalBattleUnitTargetRule::Self,
		1.0f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::Defense);
	TrackPackage(StarterHuoWenJiaCard, PackagesToSave);

	UFinalCardDefinition* StarterHuoDuanYueZhanCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterHuoDuanYueZhanCardPath, bCreatedAsset);
	StarterHuoDuanYueZhanCard->CardId = FFinalCardId(StarterHuoDuanYueZhanCardId);
	StarterHuoDuanYueZhanCard->OwnerUnitId = StarterHuoCharacterId;
	StarterHuoDuanYueZhanCard->DisplayName = FText::FromString(TEXT("断岳斩"));
	StarterHuoDuanYueZhanCard->CardType = EFinalCardType::Attack;
	StarterHuoDuanYueZhanCard->Rarity = EFinalRarity::Common;
	StarterHuoDuanYueZhanCard->BaseCostAP = 1;
	StarterHuoDuanYueZhanCard->RulesText = FText::FromString(TEXT("对目标造成相当于攻击力 130% 的伤害。额外造成 3 点削韧。消耗 1 层刀势：再额外造成 2 点削韧。"));
	StarterHuoDuanYueZhanCard->Effects.Reset();
	AddBonusBreakEffect(
		StarterHuoDuanYueZhanCard,
		StarterHuoDuanYueZhanCard->Effects,
		TEXT("effect.starter.huo.duanyuezhan.break"),
		EFinalBattleUnitTargetRule::SelectedEnemy,
		3.0f,
		EFinalBattleScalarMode::Flat);
	AddRemoveStatusEffect(
		StarterHuoDuanYueZhanCard,
		StarterHuoDuanYueZhanCard->Effects,
		TEXT("effect.starter.huo.duanyuezhan.consume_daoshi"),
		EFinalBattleUnitTargetRule::Self,
		StarterHuoStatus,
		1);
	UFinalBattleEffectBonusBreak* StarterHuoDuanYueZhanConsumeBreak = AddBonusBreakEffect(
		StarterHuoDuanYueZhanCard,
		StarterHuoDuanYueZhanCard->Effects,
		TEXT("effect.starter.huo.duanyuezhan.consume_break"),
		EFinalBattleUnitTargetRule::SelectedEnemy,
		2.0f,
		EFinalBattleScalarMode::Flat);
	AddStatusChangedCondition(StarterHuoDuanYueZhanConsumeBreak, StarterHuoStatus->StatusId, 1);
	AddDamageEffect(
		StarterHuoDuanYueZhanCard,
		StarterHuoDuanYueZhanCard->Effects,
		TEXT("effect.starter.huo.duanyuezhan.damage"),
		EFinalBattleUnitTargetRule::SelectedEnemy,
		1.3f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::Attack,
		1);
	TrackPackage(StarterHuoDuanYueZhanCard, PackagesToSave);

	UFinalCardDefinition* StarterHuoDuanYueZhanPoZhenCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterHuoDuanYueZhanPoZhenCardPath, bCreatedAsset);
	StarterHuoDuanYueZhanPoZhenCard->CardId = FFinalCardId(StarterHuoDuanYueZhanPoZhenCardId);
	StarterHuoDuanYueZhanPoZhenCard->OwnerUnitId = StarterHuoCharacterId;
	StarterHuoDuanYueZhanPoZhenCard->DisplayName = FText::FromString(TEXT("断岳斩·破阵"));
	StarterHuoDuanYueZhanPoZhenCard->CardType = EFinalCardType::Attack;
	StarterHuoDuanYueZhanPoZhenCard->Rarity = EFinalRarity::Rare;
	StarterHuoDuanYueZhanPoZhenCard->BaseCostAP = 1;
	StarterHuoDuanYueZhanPoZhenCard->RulesText = FText::FromString(TEXT("对目标造成相当于攻击力 150% 的伤害。额外造成 4 点削韧。消耗 1 层刀势：再额外造成 3 点削韧。"));
	StarterHuoDuanYueZhanPoZhenCard->Effects.Reset();
	AddBonusBreakEffect(
		StarterHuoDuanYueZhanPoZhenCard,
		StarterHuoDuanYueZhanPoZhenCard->Effects,
		TEXT("effect.starter.huo.duanyuezhan_pozhen.break"),
		EFinalBattleUnitTargetRule::SelectedEnemy,
		4.0f,
		EFinalBattleScalarMode::Flat);
	AddRemoveStatusEffect(
		StarterHuoDuanYueZhanPoZhenCard,
		StarterHuoDuanYueZhanPoZhenCard->Effects,
		TEXT("effect.starter.huo.duanyuezhan_pozhen.consume_daoshi"),
		EFinalBattleUnitTargetRule::Self,
		StarterHuoStatus,
		1);
	UFinalBattleEffectBonusBreak* StarterHuoDuanYueZhanPoZhenConsumeBreak = AddBonusBreakEffect(
		StarterHuoDuanYueZhanPoZhenCard,
		StarterHuoDuanYueZhanPoZhenCard->Effects,
		TEXT("effect.starter.huo.duanyuezhan_pozhen.consume_break"),
		EFinalBattleUnitTargetRule::SelectedEnemy,
		3.0f,
		EFinalBattleScalarMode::Flat);
	AddStatusChangedCondition(StarterHuoDuanYueZhanPoZhenConsumeBreak, StarterHuoStatus->StatusId, 1);
	AddDamageEffect(
		StarterHuoDuanYueZhanPoZhenCard,
		StarterHuoDuanYueZhanPoZhenCard->Effects,
		TEXT("effect.starter.huo.duanyuezhan_pozhen.damage"),
		EFinalBattleUnitTargetRule::SelectedEnemy,
		1.5f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::Attack,
		1);
	TrackPackage(StarterHuoDuanYueZhanPoZhenCard, PackagesToSave);

	UFinalCardEvolutionDefinition* StarterHuoDuanYueZhanPoZhenEvolution = LoadOrCreateAsset<UFinalCardEvolutionDefinition>(StarterHuoDuanYueZhanPoZhenEvolutionPath, bCreatedAsset);
	StarterHuoDuanYueZhanPoZhenEvolution->EvolutionId = FFinalCardEvolutionId(StarterHuoDuanYueZhanPoZhenEvolutionId);
	StarterHuoDuanYueZhanPoZhenEvolution->FromCardId = StarterHuoDuanYueZhanCard->CardId;
	StarterHuoDuanYueZhanPoZhenEvolution->ToCardId = StarterHuoDuanYueZhanPoZhenCard->CardId;
	StarterHuoDuanYueZhanPoZhenEvolution->FromStage = EFinalCardEvolutionStage::Base;
	StarterHuoDuanYueZhanPoZhenEvolution->ToStage = EFinalCardEvolutionStage::Evolved;
	StarterHuoDuanYueZhanPoZhenEvolution->EvolutionType = EFinalCardEvolutionType::ImmediatePower;
	StarterHuoDuanYueZhanPoZhenEvolution->RequiredOwnerCharacterId = StarterHuoCharacter->CharacterId;
	StarterHuoDuanYueZhanPoZhenEvolution->RequiredCardTags.Reset();
	StarterHuoDuanYueZhanPoZhenEvolution->bAllowAsLevelUpCandidate = true;
	StarterHuoDuanYueZhanPoZhenEvolution->DisplayName = FText::FromString(TEXT("断岳斩·破阵"));
	StarterHuoDuanYueZhanPoZhenEvolution->Description = FText::FromString(TEXT("让断岳斩更适合破开敌阵：基础伤害与削韧同步提高，消耗刀势后的额外削韧也更强。"));
	TrackPackage(StarterHuoDuanYueZhanPoZhenEvolution, PackagesToSave);

	UFinalCardDefinition* StarterHuoTieBiHuiFengCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterHuoTieBiHuiFengCardPath, bCreatedAsset);
	StarterHuoTieBiHuiFengCard->CardId = FFinalCardId(StarterHuoTieBiHuiFengCardId);
	StarterHuoTieBiHuiFengCard->OwnerUnitId = StarterHuoCharacterId;
	StarterHuoTieBiHuiFengCard->DisplayName = FText::FromString(TEXT("铁壁回锋"));
	StarterHuoTieBiHuiFengCard->CardType = EFinalCardType::Skill;
	StarterHuoTieBiHuiFengCard->Rarity = EFinalRarity::Common;
	StarterHuoTieBiHuiFengCard->BaseCostAP = 1;
	StarterHuoTieBiHuiFengCard->RulesText = FText::FromString(TEXT("获得相当于防御力 120% 的护盾，并获得 1 层刀势。"));
	StarterHuoTieBiHuiFengCard->Effects.Reset();
	AddShieldEffect(
		StarterHuoTieBiHuiFengCard,
		StarterHuoTieBiHuiFengCard->Effects,
		TEXT("effect.starter.huo.tiebihuifeng.shield"),
		EFinalBattleUnitTargetRule::Self,
		1.2f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::Defense);
	AddApplyStatusEffect(
		StarterHuoTieBiHuiFengCard,
		StarterHuoTieBiHuiFengCard->Effects,
		TEXT("effect.starter.huo.tiebihuifeng.daoshi"),
		EFinalBattleUnitTargetRule::Self,
		StarterHuoStatus,
		1);
	TrackPackage(StarterHuoTieBiHuiFengCard, PackagesToSave);

	UFinalCardDefinition* StarterYeXingZhenCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterYeXingZhenCardPath, bCreatedAsset);
	StarterYeXingZhenCard->CardId = FFinalCardId(StarterYeXingZhenCardId);
	StarterYeXingZhenCard->OwnerUnitId = StarterYeCharacterId;
	StarterYeXingZhenCard->DisplayName = FText::FromString(TEXT("行针"));
	StarterYeXingZhenCard->CardType = EFinalCardType::Attack;
	StarterYeXingZhenCard->Rarity = EFinalRarity::Common;
	StarterYeXingZhenCard->BaseCostAP = 1;
	StarterYeXingZhenCard->RulesText = FText::FromString(TEXT("对目标造成相当于攻击力 90% 的伤害。获得 2 层药引。"));
	StarterYeXingZhenCard->Effects.Reset();
	AddDamageEffect(
		StarterYeXingZhenCard,
		StarterYeXingZhenCard->Effects,
		TEXT("effect.starter.ye.xingzhen.damage"),
		EFinalBattleUnitTargetRule::SelectedEnemy,
		0.9f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::Attack,
		1);
	AddApplyStatusEffect(
		StarterYeXingZhenCard,
		StarterYeXingZhenCard->Effects,
		TEXT("effect.starter.ye.xingzhen.yaoyin"),
		EFinalBattleUnitTargetRule::Self,
		StarterYeStatus,
		2);
	TrackPackage(StarterYeXingZhenCard, PackagesToSave);

	UFinalCardDefinition* StarterYeTiaoXiCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterYeTiaoXiCardPath, bCreatedAsset);
	StarterYeTiaoXiCard->CardId = FFinalCardId(StarterYeTiaoXiCardId);
	StarterYeTiaoXiCard->OwnerUnitId = StarterYeCharacterId;
	StarterYeTiaoXiCard->DisplayName = FText::FromString(TEXT("调息"));
	StarterYeTiaoXiCard->CardType = EFinalCardType::Skill;
	StarterYeTiaoXiCard->Rarity = EFinalRarity::Common;
	StarterYeTiaoXiCard->BaseCostAP = 0;
	StarterYeTiaoXiCard->RulesText = FText::FromString(TEXT("回复 5 点共享生命，并获得 1 层药引。"));
	StarterYeTiaoXiCard->Effects.Reset();
	AddHealEffect(
		StarterYeTiaoXiCard,
		StarterYeTiaoXiCard->Effects,
		TEXT("effect.starter.ye.tiaoxi.heal"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		5.0f,
		EFinalBattleScalarMode::Flat,
		EFinalBattleSourceStat::None);
	AddApplyStatusEffect(
		StarterYeTiaoXiCard,
		StarterYeTiaoXiCard->Effects,
		TEXT("effect.starter.ye.tiaoxi.yaoyin"),
		EFinalBattleUnitTargetRule::Self,
		StarterYeStatus,
		1);
	TrackPackage(StarterYeTiaoXiCard, PackagesToSave);

	UFinalCardDefinition* StarterYeHuaYinCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterYeHuaYinCardPath, bCreatedAsset);
	StarterYeHuaYinCard->CardId = FFinalCardId(StarterYeHuaYinCardId);
	StarterYeHuaYinCard->OwnerUnitId = StarterYeCharacterId;
	StarterYeHuaYinCard->DisplayName = FText::FromString(TEXT("化引"));
	StarterYeHuaYinCard->CardType = EFinalCardType::Skill;
	StarterYeHuaYinCard->Rarity = EFinalRarity::Common;
	StarterYeHuaYinCard->BaseCostAP = 1;
	StarterYeHuaYinCard->RulesText = FText::FromString(TEXT("回复 5 点共享生命。消耗 1 层药引：抽 1 张牌，并回复 1 AP。"));
	StarterYeHuaYinCard->Effects.Reset();
	AddHealEffect(
		StarterYeHuaYinCard,
		StarterYeHuaYinCard->Effects,
		TEXT("effect.starter.ye.huayin.heal"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		5.0f,
		EFinalBattleScalarMode::Flat,
		EFinalBattleSourceStat::None);
	AddRemoveStatusEffect(
		StarterYeHuaYinCard,
		StarterYeHuaYinCard->Effects,
		TEXT("effect.starter.ye.huayin.consume_yaoyin"),
		EFinalBattleUnitTargetRule::Self,
		StarterYeStatus,
		1);
	UFinalBattleEffectDrawCards* StarterYeHuaYinDrawEffect = AddDrawEffect(
		StarterYeHuaYinCard,
		StarterYeHuaYinCard->Effects,
		TEXT("effect.starter.ye.huayin.draw"),
		1);
	AddStatusChangedCondition(StarterYeHuaYinDrawEffect, StarterYeStatus->StatusId, 1);
	UFinalBattleEffectGainAP* StarterYeHuaYinGainApEffect = AddGainApEffect(
		StarterYeHuaYinCard,
		StarterYeHuaYinCard->Effects,
		TEXT("effect.starter.ye.huayin.gain_ap"),
		1);
	AddStatusChangedCondition(StarterYeHuaYinGainApEffect, StarterYeStatus->StatusId, 1);
	TrackPackage(StarterYeHuaYinCard, PackagesToSave);

	UFinalCardDefinition* StarterYeHuiChunSanCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterYeHuiChunSanCardPath, bCreatedAsset);
	StarterYeHuiChunSanCard->CardId = FFinalCardId(StarterYeHuiChunSanCardId);
	StarterYeHuiChunSanCard->OwnerUnitId = StarterYeCharacterId;
	StarterYeHuiChunSanCard->DisplayName = FText::FromString(TEXT("回春散"));
	StarterYeHuiChunSanCard->CardType = EFinalCardType::Skill;
	StarterYeHuiChunSanCard->Rarity = EFinalRarity::Common;
	StarterYeHuiChunSanCard->BaseCostAP = 1;
	StarterYeHuiChunSanCard->RulesText = FText::FromString(TEXT("回复 12 点共享生命。消耗 1 层药引：回复 1 AP。"));
	StarterYeHuiChunSanCard->Effects.Reset();
	AddHealEffect(
		StarterYeHuiChunSanCard,
		StarterYeHuiChunSanCard->Effects,
		TEXT("effect.starter.ye.huichunsan.heal"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		12.0f,
		EFinalBattleScalarMode::Flat,
		EFinalBattleSourceStat::None,
		FText::FromString(TEXT("当前生命免疫入口集中在回天续脉；回春散只保留治疗与药引回 AP。")));
	AddRemoveStatusEffect(
		StarterYeHuiChunSanCard,
		StarterYeHuiChunSanCard->Effects,
		TEXT("effect.starter.ye.huichunsan.consume_yaoyin"),
		EFinalBattleUnitTargetRule::Self,
		StarterYeStatus,
		1);
	UFinalBattleEffectGainAP* StarterYeHuiChunSanGainApEffect = AddGainApEffect(
		StarterYeHuiChunSanCard,
		StarterYeHuiChunSanCard->Effects,
		TEXT("effect.starter.ye.huichunsan.gain_ap"),
		1,
		FText::FromString(TEXT("首波 Runtime 仅落地药引消耗后的回 AP。")));
	AddStatusChangedCondition(StarterYeHuiChunSanGainApEffect, StarterYeStatus->StatusId, 1);
	TrackPackage(StarterYeHuiChunSanCard, PackagesToSave);

	UFinalCardDefinition* StarterShenBuFengCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterShenBuFengCardPath, bCreatedAsset);
	StarterShenBuFengCard->CardId = FFinalCardId(StarterShenBuFengCardId);
	StarterShenBuFengCard->OwnerUnitId = StarterShenCharacterId;
	StarterShenBuFengCard->DisplayName = FText::FromString(TEXT("布锋"));
	StarterShenBuFengCard->CardType = EFinalCardType::Attack;
	StarterShenBuFengCard->Rarity = EFinalRarity::Common;
	StarterShenBuFengCard->BaseCostAP = 1;
	StarterShenBuFengCard->Keywords.Reset();
	StarterShenBuFengCard->RulesText = FText::FromString(TEXT("对目标造成相当于攻击力 100% 的伤害。随机生成 1 张剑阵牌到手牌。"));
	StarterShenBuFengCard->Effects.Reset();
	AddDamageEffect(
		StarterShenBuFengCard,
		StarterShenBuFengCard->Effects,
		TEXT("effect.starter.shen.bufeng.damage"),
		EFinalBattleUnitTargetRule::SelectedEnemy,
		1.0f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::Attack,
		1);
	UFinalCardDefinition* StarterShenPoZhenJianZhenCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterShenPoZhenJianZhenCardPath, bCreatedAsset);
	StarterShenPoZhenJianZhenCard->CardId = FFinalCardId(StarterShenPoZhenJianZhenCardId);
	StarterShenPoZhenJianZhenCard->OwnerUnitId = StarterShenCharacterId;
	StarterShenPoZhenJianZhenCard->DisplayName = FText::FromString(TEXT("破阵剑阵"));
	StarterShenPoZhenJianZhenCard->CardType = EFinalCardType::Skill;
	StarterShenPoZhenJianZhenCard->Rarity = EFinalRarity::Common;
	StarterShenPoZhenJianZhenCard->BaseCostAP = 0;
	StarterShenPoZhenJianZhenCard->Keywords.Reset();
	StarterShenPoZhenJianZhenCard->Keywords.AddTag(GetRetainKeyword());
	StarterShenPoZhenJianZhenCard->Keywords.AddTag(GetExpendKeyword());
	StarterShenPoZhenJianZhenCard->Keywords.AddTag(GetSwordArrayKeyword());
	StarterShenPoZhenJianZhenCard->RulesText = FText::FromString(TEXT("衍生牌。保留，消耗。额外造成 2 点削韧。"));
	StarterShenPoZhenJianZhenCard->Effects.Reset();
	AddBonusBreakEffect(
		StarterShenPoZhenJianZhenCard,
		StarterShenPoZhenJianZhenCard->Effects,
		TEXT("effect.starter.shen.pozhenjianzhen.break"),
		EFinalBattleUnitTargetRule::SelectedEnemy,
		2.0f,
		EFinalBattleScalarMode::Flat);
	TrackPackage(StarterShenPoZhenJianZhenCard, PackagesToSave);
	UFinalCardDefinition* StarterShenGuoPaiJianZhenCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterShenGuoPaiJianZhenCardPath, bCreatedAsset);
	StarterShenGuoPaiJianZhenCard->CardId = FFinalCardId(StarterShenGuoPaiJianZhenCardId);
	StarterShenGuoPaiJianZhenCard->OwnerUnitId = StarterShenCharacterId;
	StarterShenGuoPaiJianZhenCard->DisplayName = FText::FromString(TEXT("过牌剑阵"));
	StarterShenGuoPaiJianZhenCard->CardType = EFinalCardType::Skill;
	StarterShenGuoPaiJianZhenCard->Rarity = EFinalRarity::Common;
	StarterShenGuoPaiJianZhenCard->BaseCostAP = 0;
	StarterShenGuoPaiJianZhenCard->Keywords.Reset();
	StarterShenGuoPaiJianZhenCard->Keywords.AddTag(GetRetainKeyword());
	StarterShenGuoPaiJianZhenCard->Keywords.AddTag(GetExpendKeyword());
	StarterShenGuoPaiJianZhenCard->Keywords.AddTag(GetSwordArrayKeyword());
	StarterShenGuoPaiJianZhenCard->RulesText = FText::FromString(TEXT("衍生牌。保留，消耗。抽 1 张牌。"));
	StarterShenGuoPaiJianZhenCard->Effects.Reset();
	AddDrawEffect(
		StarterShenGuoPaiJianZhenCard,
		StarterShenGuoPaiJianZhenCard->Effects,
		TEXT("effect.starter.shen.guopaijianzhen.draw"),
		1);
	TrackPackage(StarterShenGuoPaiJianZhenCard, PackagesToSave);
	UFinalCardDefinition* StarterShenFengRuiJianZhenCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterShenFengRuiJianZhenCardPath, bCreatedAsset);
	StarterShenFengRuiJianZhenCard->CardId = FFinalCardId(StarterShenFengRuiJianZhenCardId);
	StarterShenFengRuiJianZhenCard->OwnerUnitId = StarterShenCharacterId;
	StarterShenFengRuiJianZhenCard->DisplayName = FText::FromString(TEXT("锋锐剑阵"));
	StarterShenFengRuiJianZhenCard->CardType = EFinalCardType::Skill;
	StarterShenFengRuiJianZhenCard->Rarity = EFinalRarity::Common;
	StarterShenFengRuiJianZhenCard->BaseCostAP = 0;
	StarterShenFengRuiJianZhenCard->Keywords.Reset();
	StarterShenFengRuiJianZhenCard->Keywords.AddTag(GetRetainKeyword());
	StarterShenFengRuiJianZhenCard->Keywords.AddTag(GetExpendKeyword());
	StarterShenFengRuiJianZhenCard->Keywords.AddTag(GetSwordArrayKeyword());
	StarterShenFengRuiJianZhenCard->RulesText = FText::FromString(TEXT("衍生牌。保留，消耗。获得 1 层锋锐，使下一张攻击牌伤害提高 20%。"));
	StarterShenFengRuiJianZhenCard->Effects.Reset();
	AddApplyStatusEffect(
		StarterShenFengRuiJianZhenCard,
		StarterShenFengRuiJianZhenCard->Effects,
		TEXT("effect.starter.shen.fengruijianzhen.apply_fengrui"),
		EFinalBattleUnitTargetRule::Self,
		StarterShenFengRuiStatus,
		1);
	TrackPackage(StarterShenFengRuiJianZhenCard, PackagesToSave);
	AddGenerateCardEffect(
		StarterShenBuFengCard,
		StarterShenBuFengCard->Effects,
		TEXT("effect.starter.shen.bufeng.generate_jianzhen"),
		nullptr,
		TArray<UFinalCardDefinition*>{ StarterShenGuoPaiJianZhenCard, StarterShenPoZhenJianZhenCard, StarterShenFengRuiJianZhenCard },
		1,
		true);
	TrackPackage(StarterShenBuFengCard, PackagesToSave);

	UFinalCardDefinition* StarterShenShouZhenCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterShenShouZhenCardPath, bCreatedAsset);
	StarterShenShouZhenCard->CardId = FFinalCardId(StarterShenShouZhenCardId);
	StarterShenShouZhenCard->OwnerUnitId = StarterShenCharacterId;
	StarterShenShouZhenCard->DisplayName = FText::FromString(TEXT("守阵"));
	StarterShenShouZhenCard->CardType = EFinalCardType::Skill;
	StarterShenShouZhenCard->Rarity = EFinalRarity::Common;
	StarterShenShouZhenCard->BaseCostAP = 1;
	StarterShenShouZhenCard->Keywords.Reset();
	StarterShenShouZhenCard->RulesText = FText::FromString(TEXT("获得相当于防御力 80% 的护盾。若手中有剑阵牌，抽 1 张牌。"));
	StarterShenShouZhenCard->Effects.Reset();
	AddShieldEffect(
		StarterShenShouZhenCard,
		StarterShenShouZhenCard->Effects,
		TEXT("effect.starter.shen.shouzhen.shield"),
		EFinalBattleUnitTargetRule::Self,
		0.8f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::Defense);
	UFinalBattleEffectDrawCards* StarterShenShouZhenDraw = AddDrawEffect(
		StarterShenShouZhenCard,
		StarterShenShouZhenCard->Effects,
		TEXT("effect.starter.shen.shouzhen.draw"),
		1,
		FText::FromString(TEXT("若手中有剑阵牌，则补 1 张牌。")));
	AddHandCardCondition(StarterShenShouZhenDraw, GetSwordArrayKeyword(), 1, true);
	TrackPackage(StarterShenShouZhenCard, PackagesToSave);

	UFinalCardDefinition* StarterShenYinZhenCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterShenYinZhenCardPath, bCreatedAsset);
	StarterShenYinZhenCard->CardId = FFinalCardId(StarterShenYinZhenCardId);
	StarterShenYinZhenCard->OwnerUnitId = StarterShenCharacterId;
	StarterShenYinZhenCard->DisplayName = FText::FromString(TEXT("引阵"));
	StarterShenYinZhenCard->CardType = EFinalCardType::Skill;
	StarterShenYinZhenCard->Rarity = EFinalRarity::Common;
	StarterShenYinZhenCard->BaseCostAP = 1;
	StarterShenYinZhenCard->Keywords.Reset();
	StarterShenYinZhenCard->RulesText = FText::FromString(TEXT("首版固定生成 1 张过牌剑阵到手牌，并抽 1 张牌。"));
	StarterShenYinZhenCard->Effects.Reset();
	AddGenerateCardEffect(
		StarterShenYinZhenCard,
		StarterShenYinZhenCard->Effects,
		TEXT("effect.starter.shen.yinzhen.generate_guopai"),
		StarterShenGuoPaiJianZhenCard,
		TArray<UFinalCardDefinition*>{},
		1,
		false);
	AddDrawEffect(
		StarterShenYinZhenCard,
		StarterShenYinZhenCard->Effects,
		TEXT("effect.starter.shen.yinzhen.draw"),
		1);
	TrackPackage(StarterShenYinZhenCard, PackagesToSave);

	UFinalCardDefinition* StarterShenYinBaoJianZhenCard = LoadOrCreateAsset<UFinalCardDefinition>(StarterShenYinBaoJianZhenCardPath, bCreatedAsset);
	StarterShenYinBaoJianZhenCard->CardId = FFinalCardId(StarterShenYinBaoJianZhenCardId);
	StarterShenYinBaoJianZhenCard->OwnerUnitId = StarterShenCharacterId;
	StarterShenYinBaoJianZhenCard->DisplayName = FText::FromString(TEXT("引爆剑阵"));
	StarterShenYinBaoJianZhenCard->CardType = EFinalCardType::Skill;
	StarterShenYinBaoJianZhenCard->Rarity = EFinalRarity::Common;
	StarterShenYinBaoJianZhenCard->BaseCostAP = 1;
	StarterShenYinBaoJianZhenCard->Keywords.Reset();
	StarterShenYinBaoJianZhenCard->Keywords.AddTag(GetRetainKeyword());
	StarterShenYinBaoJianZhenCard->RulesText = FText::FromString(TEXT("消耗 1 张手中的剑阵牌。若成功， 对目标造成相当于攻击力 130% 的伤害，并抽 1 张牌。"));
	StarterShenYinBaoJianZhenCard->Effects.Reset();
	AddMoveCardsEffect(
		StarterShenYinBaoJianZhenCard,
		StarterShenYinBaoJianZhenCard->Effects,
		TEXT("effect.starter.shen.yinbaojianzhen.consume_jianzhen"),
		EFinalBattleCardZoneRule::Hand,
		EFinalBattleCardZoneRule::ConsumePile,
		FFinalCardId(),
		GetSwordArrayKeyword(),
		1,
		true,
		true,
		FText::FromString(TEXT("需要先消耗 1 张衍生剑阵牌。")));
	UFinalBattleEffectDamage* StarterShenYinBaoDamage = AddDamageEffect(
		StarterShenYinBaoJianZhenCard,
		StarterShenYinBaoJianZhenCard->Effects,
		TEXT("effect.starter.shen.yinbaojianzhen.damage"),
		EFinalBattleUnitTargetRule::SelectedEnemy,
		1.3f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::Attack);
	AddMovedCardsCondition(
		StarterShenYinBaoDamage,
		GetSwordArrayKeyword(),
		1,
		true,
		true,
		EFinalBattleCardZoneRule::Hand,
		true,
		EFinalBattleCardZoneRule::ConsumePile);
	UFinalBattleEffectDrawCards* StarterShenYinBaoDraw = AddDrawEffect(
		StarterShenYinBaoJianZhenCard,
		StarterShenYinBaoJianZhenCard->Effects,
		TEXT("effect.starter.shen.yinbaojianzhen.draw"),
		1);
	AddMovedCardsCondition(
		StarterShenYinBaoDraw,
		GetSwordArrayKeyword(),
		1,
		true,
		true,
		EFinalBattleCardZoneRule::Hand,
		true,
		EFinalBattleCardZoneRule::ConsumePile);
	TrackPackage(StarterShenYinBaoJianZhenCard, PackagesToSave);

	StarterShenUltimate->RulesText = FText::FromString(TEXT("抽 2 张牌。生成 1 张剑阵牌到手牌。每名角色获得 1 层士气。"));
	StarterShenUltimate->Effects.Reset();
	AddDrawEffect(
		StarterShenUltimate,
		StarterShenUltimate->Effects,
		TEXT("effect.starter.shen.ultimate.wanxiangguizhen.draw"),
		2);
	AddGenerateCardEffect(
		StarterShenUltimate,
		StarterShenUltimate->Effects,
		TEXT("effect.starter.shen.ultimate.wanxiangguizhen.generate_jianzhen"),
		nullptr,
		TArray<UFinalCardDefinition*>{ StarterShenGuoPaiJianZhenCard, StarterShenPoZhenJianZhenCard, StarterShenFengRuiJianZhenCard },
		1,
		true,
		FText::FromString(TEXT("首版随机生成 1 张剑阵牌到手牌。")));
	AddApplyStatusEffect(
		StarterShenUltimate,
		StarterShenUltimate->Effects,
		TEXT("effect.starter.shen.ultimate.wanxiangguizhen.apply_shiqi"),
		EFinalBattleUnitTargetRule::AllPlayerCharacters,
		StarterShenShiQiStatus,
		1);
	TrackPackage(StarterShenUltimate, PackagesToSave);

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
		MakeLoadoutEntry(StarterShenYinBaoJianZhenCard->CardId, 1, EFinalLoadoutRole::InitialSignature)
	};
	StarterShenCharacter->CharacterCardPoolIds = {
		StarterShenBuFengCard->CardId,
		StarterShenShouZhenCard->CardId,
		StarterShenYinZhenCard->CardId,
		StarterShenYinBaoJianZhenCard->CardId,
		StarterShenFengRuiJianZhenCard->CardId
	};
	TrackPackage(StarterShenCharacter, PackagesToSave);

	UFinalEnemyIntentDefinition* StarterBladeQuickSlashIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterBladeQuickSlashIntentPath, bCreatedAsset);
	StarterBladeQuickSlashIntent->IntentId = TEXT("intent.starter.bandit.blade.quick_slash");
	StarterBladeQuickSlashIntent->DisplayName = FText::FromString(TEXT("快斩"));
	StarterBladeQuickSlashIntent->IntentType = EFinalIntentType::Attack;
	StarterBladeQuickSlashIntent->PreviewText = FText::FromString(TEXT("快斩 0.9 × BaseDamagePower"));
	ResetIntentSelectionDefaults(StarterBladeQuickSlashIntent);
	StarterBladeQuickSlashIntent->bDisallowRepeatLastIntent = true;
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
	ResetIntentSelectionDefaults(StarterBladeDoubleSlashIntent);
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
	ResetIntentSelectionDefaults(StarterBladeBraceIntent);
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
	StarterCrossbowPiercingBoltIntent->PreviewText = FText::FromString(TEXT("透甲箭 1.0 × BaseDamagePower，并施加 1 层易伤"));
	ResetIntentSelectionDefaults(StarterCrossbowPiercingBoltIntent);
	StarterCrossbowPiercingBoltIntent->Weight = 5;
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
		1);
	AddApplyStatusEffect(
		StarterCrossbowPiercingBoltIntent,
		StarterCrossbowPiercingBoltIntent->Effects,
		TEXT("effect.starter.enemy.crossbow.piercing_bolt.vulnerable"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		StarterVulnerableStatus,
		1);
	TrackPackage(StarterCrossbowPiercingBoltIntent, PackagesToSave);

	UFinalEnemyIntentDefinition* StarterCrossbowVolleyIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterCrossbowVolleyIntentPath, bCreatedAsset);
	StarterCrossbowVolleyIntent->IntentId = TEXT("intent.starter.bandit.crossbow.volley");
	StarterCrossbowVolleyIntent->DisplayName = FText::FromString(TEXT("连珠箭"));
	StarterCrossbowVolleyIntent->IntentType = EFinalIntentType::Attack;
	StarterCrossbowVolleyIntent->PreviewText = FText::FromString(TEXT("连珠箭 2 段，每段 0.4 × BaseDamagePower"));
	ResetIntentSelectionDefaults(StarterCrossbowVolleyIntent);
	StarterCrossbowVolleyIntent->Weight = 3;
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
	ResetIntentSelectionDefaults(StarterCrossbowRepositionIntent);
	StarterCrossbowRepositionIntent->Weight = 1;
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
	ResetIntentSelectionDefaults(StarterInstructorCommandSlashIntent);
	StarterInstructorCommandSlashIntent->PhaseTags = { TEXT("phase.high") };
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
	ResetIntentSelectionDefaults(StarterInstructorHoldLineIntent);
	StarterInstructorHoldLineIntent->PhaseTags = { TEXT("phase.high") };
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
	ResetIntentSelectionDefaults(StarterInstructorHeavyCleaveIntent);
	StarterInstructorHeavyCleaveIntent->PhaseTags = { TEXT("phase.low") };
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

	UFinalEnemyIntentDefinition* StarterShieldGuardShieldBashIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterShieldGuardShieldBashIntentPath, bCreatedAsset);
	StarterShieldGuardShieldBashIntent->IntentId = TEXT("intent.starter.bandit.shield_guard.shield_bash");
	StarterShieldGuardShieldBashIntent->DisplayName = FText::FromString(TEXT("盾击"));
	StarterShieldGuardShieldBashIntent->IntentType = EFinalIntentType::Attack;
	StarterShieldGuardShieldBashIntent->PreviewText = FText::FromString(TEXT("盾击 1.1 × BaseDamagePower，并获得 8 护盾"));
	ResetIntentSelectionDefaults(StarterShieldGuardShieldBashIntent);
	StarterShieldGuardShieldBashIntent->CooldownTurns = 0;
	StarterShieldGuardShieldBashIntent->UseLimitPerBattle = 0;
	StarterShieldGuardShieldBashIntent->Effects.Reset();
	AddDamageEffect(
		StarterShieldGuardShieldBashIntent,
		StarterShieldGuardShieldBashIntent->Effects,
		TEXT("effect.starter.enemy.shield_guard.shield_bash.damage"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		1.1f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::BaseDamagePower);
	AddShieldEffect(
		StarterShieldGuardShieldBashIntent,
		StarterShieldGuardShieldBashIntent->Effects,
		TEXT("effect.starter.enemy.shield_guard.shield_bash.shield"),
		EFinalBattleUnitTargetRule::Self,
		8.0f,
		EFinalBattleScalarMode::Flat);
	TrackPackage(StarterShieldGuardShieldBashIntent, PackagesToSave);

	UFinalEnemyIntentDefinition* StarterShieldGuardProtectIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterShieldGuardProtectIntentPath, bCreatedAsset);
	StarterShieldGuardProtectIntent->IntentId = TEXT("intent.starter.bandit.shield_guard.protect");
	StarterShieldGuardProtectIntent->DisplayName = FText::FromString(TEXT("护寨"));
	StarterShieldGuardProtectIntent->IntentType = EFinalIntentType::Defense;
	StarterShieldGuardProtectIntent->PreviewText = FText::FromString(TEXT("获得 12 护盾；护援同伙为首版占位"));
	ResetIntentSelectionDefaults(StarterShieldGuardProtectIntent);
	StarterShieldGuardProtectIntent->CooldownTurns = 1;
	StarterShieldGuardProtectIntent->UseLimitPerBattle = 0;
	StarterShieldGuardProtectIntent->Effects.Reset();
	AddShieldEffect(
		StarterShieldGuardProtectIntent,
		StarterShieldGuardProtectIntent->Effects,
		TEXT("effect.starter.enemy.shield_guard.protect.self_shield"),
		EFinalBattleUnitTargetRule::Self,
		12.0f,
		EFinalBattleScalarMode::Flat,
		EFinalBattleSourceStat::None,
		FText::FromString(TEXT("首版占位：未额外给同伙护盾。")));
	TrackPackage(StarterShieldGuardProtectIntent, PackagesToSave);

	UFinalEnemyIntentDefinition* StarterShieldGuardHeavySlamIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterShieldGuardHeavySlamIntentPath, bCreatedAsset);
	StarterShieldGuardHeavySlamIntent->IntentId = TEXT("intent.starter.bandit.shield_guard.heavy_slam");
	StarterShieldGuardHeavySlamIntent->DisplayName = FText::FromString(TEXT("压步重撞"));
	StarterShieldGuardHeavySlamIntent->IntentType = EFinalIntentType::Attack;
	StarterShieldGuardHeavySlamIntent->PreviewText = FText::FromString(TEXT("压步重撞 1.45 × BaseDamagePower"));
	ResetIntentSelectionDefaults(StarterShieldGuardHeavySlamIntent);
	StarterShieldGuardHeavySlamIntent->CooldownTurns = 1;
	StarterShieldGuardHeavySlamIntent->UseLimitPerBattle = 0;
	StarterShieldGuardHeavySlamIntent->Effects.Reset();
	AddDamageEffect(
		StarterShieldGuardHeavySlamIntent,
		StarterShieldGuardHeavySlamIntent->Effects,
		TEXT("effect.starter.enemy.shield_guard.heavy_slam.damage"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		1.45f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::BaseDamagePower);
	TrackPackage(StarterShieldGuardHeavySlamIntent, PackagesToSave);

	UFinalEnemyIntentDefinition* StarterSmokeCorrosionBombIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterSmokeCorrosionBombIntentPath, bCreatedAsset);
	StarterSmokeCorrosionBombIntent->IntentId = TEXT("intent.starter.bandit.smoke.corrosion_bomb");
	StarterSmokeCorrosionBombIntent->DisplayName = FText::FromString(TEXT("腐烟包"));
	StarterSmokeCorrosionBombIntent->IntentType = EFinalIntentType::Debuff;
	StarterSmokeCorrosionBombIntent->PreviewText = FText::FromString(TEXT("0.7 × BaseDamagePower，并施加 2 层腐蚀"));
	ResetIntentSelectionDefaults(StarterSmokeCorrosionBombIntent);
	StarterSmokeCorrosionBombIntent->CooldownTurns = 0;
	StarterSmokeCorrosionBombIntent->UseLimitPerBattle = 0;
	StarterSmokeCorrosionBombIntent->Effects.Reset();
	AddDamageEffect(
		StarterSmokeCorrosionBombIntent,
		StarterSmokeCorrosionBombIntent->Effects,
		TEXT("effect.starter.enemy.smoke.corrosion_bomb.damage"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		0.7f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::BaseDamagePower);
	AddApplyStatusEffect(
		StarterSmokeCorrosionBombIntent,
		StarterSmokeCorrosionBombIntent->Effects,
		TEXT("effect.starter.enemy.smoke.corrosion_bomb.corrosion"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		StarterCorrosionStatus,
		2);
	TrackPackage(StarterSmokeCorrosionBombIntent, PackagesToSave);

	UFinalEnemyIntentDefinition* StarterSmokeDullingSmokeIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterSmokeDullingSmokeIntentPath, bCreatedAsset);
	StarterSmokeDullingSmokeIntent->IntentId = TEXT("intent.starter.bandit.smoke.dulling_smoke");
	StarterSmokeDullingSmokeIntent->DisplayName = FText::FromString(TEXT("闷烟散"));
	StarterSmokeDullingSmokeIntent->IntentType = EFinalIntentType::Debuff;
	StarterSmokeDullingSmokeIntent->PreviewText = FText::FromString(TEXT("0.6 × BaseDamagePower，并施加 1 层虚弱"));
	ResetIntentSelectionDefaults(StarterSmokeDullingSmokeIntent);
	StarterSmokeDullingSmokeIntent->CooldownTurns = 1;
	StarterSmokeDullingSmokeIntent->UseLimitPerBattle = 0;
	StarterSmokeDullingSmokeIntent->Effects.Reset();
	AddDamageEffect(
		StarterSmokeDullingSmokeIntent,
		StarterSmokeDullingSmokeIntent->Effects,
		TEXT("effect.starter.enemy.smoke.dulling_smoke.damage"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		0.6f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::BaseDamagePower);
	AddApplyStatusEffect(
		StarterSmokeDullingSmokeIntent,
		StarterSmokeDullingSmokeIntent->Effects,
		TEXT("effect.starter.enemy.smoke.dulling_smoke.weak"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		StarterWeakStatus,
		1);
	TrackPackage(StarterSmokeDullingSmokeIntent, PackagesToSave);

	UFinalEnemyIntentDefinition* StarterSmokeFireJarIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterSmokeFireJarIntentPath, bCreatedAsset);
	StarterSmokeFireJarIntent->IntentId = TEXT("intent.starter.bandit.smoke.fire_jar");
	StarterSmokeFireJarIntent->DisplayName = FText::FromString(TEXT("藏火罐"));
	StarterSmokeFireJarIntent->IntentType = EFinalIntentType::Attack;
	StarterSmokeFireJarIntent->PreviewText = FText::FromString(TEXT("藏火罐 1.1 × BaseDamagePower"));
	ResetIntentSelectionDefaults(StarterSmokeFireJarIntent);
	StarterSmokeFireJarIntent->CooldownTurns = 0;
	StarterSmokeFireJarIntent->UseLimitPerBattle = 0;
	StarterSmokeFireJarIntent->Effects.Reset();
	AddDamageEffect(
		StarterSmokeFireJarIntent,
		StarterSmokeFireJarIntent->Effects,
		TEXT("effect.starter.enemy.smoke.fire_jar.damage"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		1.1f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::BaseDamagePower);
	TrackPackage(StarterSmokeFireJarIntent, PackagesToSave);

	UFinalEnemyIntentDefinition* StarterToxicMistSpreadIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterToxicMistSpreadIntentPath, bCreatedAsset);
	StarterToxicMistSpreadIntent->IntentId = TEXT("intent.starter.toxic.mist_spread");
	StarterToxicMistSpreadIntent->DisplayName = FText::FromString(TEXT("腐烟弥散"));
	StarterToxicMistSpreadIntent->IntentType = EFinalIntentType::Debuff;
	StarterToxicMistSpreadIntent->PreviewText = FText::FromString(TEXT("0.8 × BaseDamagePower，并施加 2 层腐蚀"));
	ResetIntentSelectionDefaults(StarterToxicMistSpreadIntent);
	StarterToxicMistSpreadIntent->CooldownTurns = 0;
	StarterToxicMistSpreadIntent->UseLimitPerBattle = 0;
	StarterToxicMistSpreadIntent->Effects.Reset();
	AddDamageEffect(
		StarterToxicMistSpreadIntent,
		StarterToxicMistSpreadIntent->Effects,
		TEXT("effect.starter.enemy.toxic.mist_spread.damage"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		0.8f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::BaseDamagePower);
	AddApplyStatusEffect(
		StarterToxicMistSpreadIntent,
		StarterToxicMistSpreadIntent->Effects,
		TEXT("effect.starter.enemy.toxic.mist_spread.corrosion"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		StarterCorrosionStatus,
		2);
	TrackPackage(StarterToxicMistSpreadIntent, PackagesToSave);

	UFinalEnemyIntentDefinition* StarterToxicIgniteIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterToxicIgniteIntentPath, bCreatedAsset);
	StarterToxicIgniteIntent->IntentId = TEXT("intent.starter.toxic.ignite");
	StarterToxicIgniteIntent->DisplayName = FText::FromString(TEXT("焚香引爆"));
	StarterToxicIgniteIntent->IntentType = EFinalIntentType::Attack;
	StarterToxicIgniteIntent->PreviewText = FText::FromString(TEXT("1.45 × BaseDamagePower；腐蚀追加伤害为首版占位"));
	ResetIntentSelectionDefaults(StarterToxicIgniteIntent);
	StarterToxicIgniteIntent->CooldownTurns = 1;
	StarterToxicIgniteIntent->UseLimitPerBattle = 0;
	StarterToxicIgniteIntent->Effects.Reset();
	AddDamageEffect(
		StarterToxicIgniteIntent,
		StarterToxicIgniteIntent->Effects,
		TEXT("effect.starter.enemy.toxic.ignite.damage"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		1.45f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::BaseDamagePower,
		1,
		FText::FromString(TEXT("首版占位：未实际检查目标腐蚀层数。")));
	TrackPackage(StarterToxicIgniteIntent, PackagesToSave);

	UFinalEnemyIntentDefinition* StarterToxicCallBanditIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterToxicCallBanditIntentPath, bCreatedAsset);
	StarterToxicCallBanditIntent->IntentId = TEXT("intent.starter.toxic.call_bandit");
	StarterToxicCallBanditIntent->DisplayName = FText::FromString(TEXT("唤匪"));
	StarterToxicCallBanditIntent->IntentType = EFinalIntentType::Summon;
	StarterToxicCallBanditIntent->PreviewText = FText::FromString(TEXT("召唤 1 名山匪刀手（首版占位），并获得 8 护盾"));
	ResetIntentSelectionDefaults(StarterToxicCallBanditIntent);
	StarterToxicCallBanditIntent->CooldownTurns = 0;
	StarterToxicCallBanditIntent->UseLimitPerBattle = 1;
	StarterToxicCallBanditIntent->Effects.Reset();
	AddShieldEffect(
		StarterToxicCallBanditIntent,
		StarterToxicCallBanditIntent->Effects,
		TEXT("effect.starter.enemy.toxic.call_bandit.shield"),
		EFinalBattleUnitTargetRule::Self,
		8.0f,
		EFinalBattleScalarMode::Flat,
		EFinalBattleSourceStat::None,
		FText::FromString(TEXT("首版占位：未实际召援。")));
	TrackPackage(StarterToxicCallBanditIntent, PackagesToSave);

	UFinalEnemyIntentDefinition* StarterBossMountainSlashIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterBossMountainSlashIntentPath, bCreatedAsset);
	StarterBossMountainSlashIntent->IntentId = TEXT("intent.starter.boss.mountain_slash");
	StarterBossMountainSlashIntent->DisplayName = FText::FromString(TEXT("裂山斩"));
	StarterBossMountainSlashIntent->IntentType = EFinalIntentType::Attack;
	StarterBossMountainSlashIntent->PreviewText = FText::FromString(TEXT("裂山斩 1.6 × BaseDamagePower"));
	ResetIntentSelectionDefaults(StarterBossMountainSlashIntent);
	StarterBossMountainSlashIntent->CooldownTurns = 0;
	StarterBossMountainSlashIntent->UseLimitPerBattle = 0;
	StarterBossMountainSlashIntent->Effects.Reset();
	AddDamageEffect(
		StarterBossMountainSlashIntent,
		StarterBossMountainSlashIntent->Effects,
		TEXT("effect.starter.enemy.boss.mountain_slash.damage"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		1.6f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::BaseDamagePower);
	TrackPackage(StarterBossMountainSlashIntent, PackagesToSave);

	UFinalEnemyIntentDefinition* StarterBossTripleStrikeIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterBossTripleStrikeIntentPath, bCreatedAsset);
	StarterBossTripleStrikeIntent->IntentId = TEXT("intent.starter.boss.triple_strike");
	StarterBossTripleStrikeIntent->DisplayName = FText::FromString(TEXT("震寨三连"));
	StarterBossTripleStrikeIntent->IntentType = EFinalIntentType::Attack;
	StarterBossTripleStrikeIntent->PreviewText = FText::FromString(TEXT("震寨三连 3 段，每段 0.55 × BaseDamagePower"));
	ResetIntentSelectionDefaults(StarterBossTripleStrikeIntent);
	StarterBossTripleStrikeIntent->CooldownTurns = 0;
	StarterBossTripleStrikeIntent->UseLimitPerBattle = 0;
	StarterBossTripleStrikeIntent->Effects.Reset();
	AddDamageEffect(
		StarterBossTripleStrikeIntent,
		StarterBossTripleStrikeIntent->Effects,
		TEXT("effect.starter.enemy.boss.triple_strike.damage"),
		EFinalBattleUnitTargetRule::TeamPlayer,
		0.55f,
		EFinalBattleScalarMode::SourceStatMultiplier,
		EFinalBattleSourceStat::BaseDamagePower,
		3);
	TrackPackage(StarterBossTripleStrikeIntent, PackagesToSave);

	UFinalEnemyIntentDefinition* StarterBossRallyIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterBossRallyIntentPath, bCreatedAsset);
	StarterBossRallyIntent->IntentId = TEXT("intent.starter.boss.rally");
	StarterBossRallyIntent->DisplayName = FText::FromString(TEXT("号山令"));
	StarterBossRallyIntent->IntentType = EFinalIntentType::Summon;
	StarterBossRallyIntent->PreviewText = FText::FromString(TEXT("召援占位，并获得 10 护盾"));
	ResetIntentSelectionDefaults(StarterBossRallyIntent);
	StarterBossRallyIntent->CooldownTurns = 0;
	StarterBossRallyIntent->UseLimitPerBattle = 1;
	StarterBossRallyIntent->Effects.Reset();
	AddShieldEffect(
		StarterBossRallyIntent,
		StarterBossRallyIntent->Effects,
		TEXT("effect.starter.enemy.boss.rally.shield"),
		EFinalBattleUnitTargetRule::Self,
		10.0f,
		EFinalBattleScalarMode::Flat,
		EFinalBattleSourceStat::None,
		FText::FromString(TEXT("首版占位：未实际召援。")));
	TrackPackage(StarterBossRallyIntent, PackagesToSave);

	UFinalEnemyIntentDefinition* StarterBossChargeIntent = LoadOrCreateAsset<UFinalEnemyIntentDefinition>(StarterBossChargeIntentPath, bCreatedAsset);
	StarterBossChargeIntent->IntentId = TEXT("intent.starter.boss.charge");
	StarterBossChargeIntent->DisplayName = FText::FromString(TEXT("聚力破门"));
	StarterBossChargeIntent->IntentType = EFinalIntentType::Charge;
	StarterBossChargeIntent->PreviewText = FText::FromString(TEXT("蓄势终结占位；本轮获得 16 护盾"));
	ResetIntentSelectionDefaults(StarterBossChargeIntent);
	StarterBossChargeIntent->CooldownTurns = 1;
	StarterBossChargeIntent->UseLimitPerBattle = 0;
	StarterBossChargeIntent->Effects.Reset();
	AddShieldEffect(
		StarterBossChargeIntent,
		StarterBossChargeIntent->Effects,
		TEXT("effect.starter.enemy.boss.charge.shield"),
		EFinalBattleUnitTargetRule::Self,
		16.0f,
		EFinalBattleScalarMode::Flat,
		EFinalBattleSourceStat::None,
		FText::FromString(TEXT("首版占位：未实际提高下一次裂山斩伤害。")));
	TrackPackage(StarterBossChargeIntent, PackagesToSave);

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
	StarterBladeEnemy->ScriptedIntentSequence.Reset();
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
	StarterCrossbowEnemy->IntentSelectRule = EFinalIntentSelectRule::WeightedRandom;
	StarterCrossbowEnemy->PhaseSequence.Reset();
	StarterCrossbowEnemy->ScriptedIntentSequence.Reset();
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
	StarterInstructorEnemy->IntentSelectRule = EFinalIntentSelectRule::PhaseSequence;
	StarterInstructorEnemy->PhaseSequence.Reset();
	{
		FFinalEnemyPhaseDefinition& HighPhase = StarterInstructorEnemy->PhaseSequence.AddDefaulted_GetRef();
		HighPhase.PhaseTag = TEXT("phase.high");
		HighPhase.MaxHpPercent = 1.0f;

		FFinalEnemyPhaseDefinition& LowPhase = StarterInstructorEnemy->PhaseSequence.AddDefaulted_GetRef();
		LowPhase.PhaseTag = TEXT("phase.low");
		LowPhase.MaxHpPercent = 0.5f;
	}
	StarterInstructorEnemy->ScriptedIntentSequence.Reset();
	StarterInstructorEnemy->IntentPool = {
		StarterInstructorCommandSlashIntent,
		StarterInstructorHoldLineIntent,
		StarterInstructorHeavyCleaveIntent
	};
	TrackPackage(StarterInstructorEnemy, PackagesToSave);

	UFinalEnemyDefinition* StarterShieldGuardEnemy = LoadOrCreateAsset<UFinalEnemyDefinition>(StarterShieldGuardEnemyPath, bCreatedAsset);
	StarterShieldGuardEnemy->EnemyId = FFinalEnemyId(StarterShieldGuardEnemyId);
	StarterShieldGuardEnemy->DisplayName = FText::FromString(TEXT("山匪盾卫"));
	StarterShieldGuardEnemy->MaxHP = 36;
	StarterShieldGuardEnemy->MaxBreakValue = 12;
	StarterShieldGuardEnemy->BaseDamagePower = 9;
	StarterShieldGuardEnemy->InitialInitiativeValue = 6;
	StarterShieldGuardEnemy->InitiativeResponse = 1;
	StarterShieldGuardEnemy->IntentSelectRule = EFinalIntentSelectRule::Cycle;
	StarterShieldGuardEnemy->PhaseSequence.Reset();
	StarterShieldGuardEnemy->ScriptedIntentSequence.Reset();
	StarterShieldGuardEnemy->IntentPool = {
		StarterShieldGuardShieldBashIntent,
		StarterShieldGuardProtectIntent,
		StarterShieldGuardHeavySlamIntent
	};
	TrackPackage(StarterShieldGuardEnemy, PackagesToSave);

	UFinalEnemyDefinition* StarterSmokeEnemy = LoadOrCreateAsset<UFinalEnemyDefinition>(StarterSmokeEnemyPath, bCreatedAsset);
	StarterSmokeEnemy->EnemyId = FFinalEnemyId(StarterSmokeEnemyId);
	StarterSmokeEnemy->DisplayName = FText::FromString(TEXT("药烟匪徒"));
	StarterSmokeEnemy->MaxHP = 32;
	StarterSmokeEnemy->MaxBreakValue = 11;
	StarterSmokeEnemy->BaseDamagePower = 8;
	StarterSmokeEnemy->InitialInitiativeValue = 5;
	StarterSmokeEnemy->InitiativeResponse = 1;
	StarterSmokeEnemy->IntentSelectRule = EFinalIntentSelectRule::WeightedRandom;
	StarterSmokeEnemy->PhaseSequence.Reset();
	StarterSmokeEnemy->ScriptedIntentSequence.Reset();
	StarterSmokeEnemy->IntentPool = {
		StarterSmokeCorrosionBombIntent,
		StarterSmokeDullingSmokeIntent,
		StarterSmokeFireJarIntent
	};
	TrackPackage(StarterSmokeEnemy, PackagesToSave);

	UFinalEnemyDefinition* StarterToxicEnemy = LoadOrCreateAsset<UFinalEnemyDefinition>(StarterToxicEnemyPath, bCreatedAsset);
	StarterToxicEnemy->EnemyId = FFinalEnemyId(StarterToxicEnemyId);
	StarterToxicEnemy->DisplayName = FText::FromString(TEXT("毒烟术师"));
	StarterToxicEnemy->MaxHP = 102;
	StarterToxicEnemy->MaxBreakValue = 24;
	StarterToxicEnemy->BaseDamagePower = 11;
	StarterToxicEnemy->InitialInitiativeValue = 7;
	StarterToxicEnemy->InitiativeResponse = 2;
	StarterToxicEnemy->IntentSelectRule = EFinalIntentSelectRule::Cycle;
	StarterToxicEnemy->PhaseSequence.Reset();
	StarterToxicEnemy->ScriptedIntentSequence.Reset();
	StarterToxicEnemy->IntentPool = {
		StarterToxicMistSpreadIntent,
		StarterToxicIgniteIntent,
		StarterToxicCallBanditIntent
	};
	TrackPackage(StarterToxicEnemy, PackagesToSave);

	UFinalEnemyDefinition* StarterBossEnemy = LoadOrCreateAsset<UFinalEnemyDefinition>(StarterBossEnemyPath, bCreatedAsset);
	StarterBossEnemy->EnemyId = FFinalEnemyId(StarterBossEnemyId);
	StarterBossEnemy->DisplayName = FText::FromString(TEXT("黑风寨主·厉魁"));
	StarterBossEnemy->MaxHP = 222;
	StarterBossEnemy->MaxBreakValue = 38;
	StarterBossEnemy->BaseDamagePower = 16;
	StarterBossEnemy->InitialInitiativeValue = 9;
	StarterBossEnemy->InitiativeResponse = 2;
	StarterBossEnemy->IntentSelectRule = EFinalIntentSelectRule::Scripted;
	StarterBossEnemy->PhaseSequence.Reset();
	StarterBossEnemy->ScriptedIntentSequence.Reset();
	StarterBossEnemy->ScriptedIntentSequence.Add({ StarterBossMountainSlashIntent->IntentId, NAME_None, false });
	StarterBossEnemy->ScriptedIntentSequence.Add({ StarterBossTripleStrikeIntent->IntentId, NAME_None, false });
	StarterBossEnemy->ScriptedIntentSequence.Add({ StarterBossRallyIntent->IntentId, NAME_None, false });
	StarterBossEnemy->ScriptedIntentSequence.Add({ StarterBossChargeIntent->IntentId, NAME_None, true });
	StarterBossEnemy->IntentPool = {
		StarterBossMountainSlashIntent,
		StarterBossTripleStrikeIntent,
		StarterBossRallyIntent,
		StarterBossChargeIntent
	};
	TrackPackage(StarterBossEnemy, PackagesToSave);

	UFinalBattleEncounterDefinition* StarterNormalEncounter = LoadOrCreateAsset<UFinalBattleEncounterDefinition>(StarterNormalEncounterPath, bCreatedAsset);
	ConfigureStarterEncounter(
		StarterNormalEncounter,
		StarterNormalEncounterId,
		FText::FromString(TEXT("山道拦截")),
		StarterRuleConfig,
		{ StarterBladeEnemy, StarterCrossbowEnemy });
	TrackPackage(StarterNormalEncounter, PackagesToSave);

	UFinalBattleEncounterDefinition* StarterEliteEncounter = LoadOrCreateAsset<UFinalBattleEncounterDefinition>(StarterEliteEncounterPath, bCreatedAsset);
	ConfigureStarterEncounter(
		StarterEliteEncounter,
		StarterEliteEncounterId,
		FText::FromString(TEXT("教头压阵")),
		StarterRuleConfig,
		{ StarterInstructorEnemy, StarterBladeEnemy, StarterCrossbowEnemy });
	TrackPackage(StarterEliteEncounter, PackagesToSave);

	UFinalBattleEncounterDefinition* StarterNormalDoubleBladeEncounter = LoadOrCreateAsset<UFinalBattleEncounterDefinition>(StarterNormalDoubleBladeEncounterPath, bCreatedAsset);
	ConfigureStarterEncounter(
		StarterNormalDoubleBladeEncounter,
		StarterNormalDoubleBladeEncounterId,
		FText::FromString(TEXT("刀弩夹击")),
		StarterRuleConfig,
		{ StarterBladeEnemy, StarterBladeEnemy });
	TrackPackage(StarterNormalDoubleBladeEncounter, PackagesToSave);

	UFinalBattleEncounterDefinition* StarterNormalSmokeBladeEncounter = LoadOrCreateAsset<UFinalBattleEncounterDefinition>(StarterNormalSmokeBladeEncounterPath, bCreatedAsset);
	ConfigureStarterEncounter(
		StarterNormalSmokeBladeEncounter,
		StarterNormalSmokeBladeEncounterId,
		FText::FromString(TEXT("烟障游斗")),
		StarterRuleConfig,
		{ StarterSmokeEnemy, StarterBladeEnemy });
	TrackPackage(StarterNormalSmokeBladeEncounter, PackagesToSave);

	UFinalBattleEncounterDefinition* StarterNormalShieldBladeEncounter = LoadOrCreateAsset<UFinalBattleEncounterDefinition>(StarterNormalShieldBladeEncounterPath, bCreatedAsset);
	ConfigureStarterEncounter(
		StarterNormalShieldBladeEncounter,
		StarterNormalShieldBladeEncounterId,
		FText::FromString(TEXT("隘口据守")),
		StarterRuleConfig,
		{ StarterShieldGuardEnemy, StarterBladeEnemy });
	TrackPackage(StarterNormalShieldBladeEncounter, PackagesToSave);

	UFinalBattleEncounterDefinition* StarterNormalSmokeShieldEncounter = LoadOrCreateAsset<UFinalBattleEncounterDefinition>(StarterNormalSmokeShieldEncounterPath, bCreatedAsset);
	ConfigureStarterEncounter(
		StarterNormalSmokeShieldEncounter,
		StarterNormalSmokeShieldEncounterId,
		FText::FromString(TEXT("烟障伏击")),
		StarterRuleConfig,
		{ StarterSmokeEnemy, StarterShieldGuardEnemy });
	TrackPackage(StarterNormalSmokeShieldEncounter, PackagesToSave);

	UFinalBattleEncounterDefinition* StarterNormalToxicArrowsEncounter = LoadOrCreateAsset<UFinalBattleEncounterDefinition>(StarterNormalToxicArrowsEncounterPath, bCreatedAsset);
	ConfigureStarterEncounter(
		StarterNormalToxicArrowsEncounter,
		StarterNormalToxicArrowsEncounterId,
		FText::FromString(TEXT("毒箭夹阵")),
		StarterRuleConfig,
		{ StarterCrossbowEnemy, StarterSmokeEnemy });
	TrackPackage(StarterNormalToxicArrowsEncounter, PackagesToSave);

	UFinalBattleEncounterDefinition* StarterEliteIronWallEncounter = LoadOrCreateAsset<UFinalBattleEncounterDefinition>(StarterEliteIronWallEncounterPath, bCreatedAsset);
	ConfigureStarterEncounter(
		StarterEliteIronWallEncounter,
		StarterEliteIronWallEncounterId,
		FText::FromString(TEXT("铁壁教头")),
		StarterRuleConfig,
		{ StarterInstructorEnemy, StarterShieldGuardEnemy });
	TrackPackage(StarterEliteIronWallEncounter, PackagesToSave);

	UFinalBattleEncounterDefinition* StarterEliteToxicRitualEncounter = LoadOrCreateAsset<UFinalBattleEncounterDefinition>(StarterEliteToxicRitualEncounterPath, bCreatedAsset);
	ConfigureStarterEncounter(
		StarterEliteToxicRitualEncounter,
		StarterEliteToxicRitualEncounterId,
		FText::FromString(TEXT("毒烟法阵")),
		StarterRuleConfig,
		{ StarterToxicEnemy, StarterSmokeEnemy });
	TrackPackage(StarterEliteToxicRitualEncounter, PackagesToSave);

	UFinalBattleEncounterDefinition* StarterBossEncounter = LoadOrCreateAsset<UFinalBattleEncounterDefinition>(StarterBossEncounterPath, bCreatedAsset);
	ConfigureStarterEncounter(
		StarterBossEncounter,
		StarterBossEncounterId,
		FText::FromString(TEXT("黑风寨主·厉魁")),
		StarterRuleConfig,
		{ StarterBossEnemy });
	TrackPackage(StarterBossEncounter, PackagesToSave);

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
		RewardNode.DisplayName = FText::FromString(TEXT("战后整备"));
		RewardNode.DisplayLabel = TEXT("RunNode.Starter.Reward");
		RewardNode.ChapterIndex = 1;
		RewardNode.FloorIndex = 2;
		RewardNode.NextNodeIds.Add(StarterEventNodeId);
		RewardNode.RewardContent.Title = FText::FromString(TEXT("战后整备"));
		RewardNode.RewardContent.Summary = FText::FromString(TEXT("清点缴获，整理队伍状态。"));
		RewardNode.RewardContent.RewardEntries.Add(MakeBaseRewardEntry(
			TEXT("reward.starter.spoils.gold"),
			EFinalRunRewardType::Gold,
			18,
			TEXT("Currency.Gold"),
			FText::FromString(TEXT("剿匪赏金"))));
		RewardNode.RewardContent.RewardEntries.Add(MakeCardRewardEntry(
			TEXT("reward.starter.spoils.card.yinbaojianzhen"),
			StarterShenYinBaoJianZhenCard->CardId,
			FText::FromString(TEXT("引爆剑阵"))));
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
		EventNode.DisplayName = FText::FromString(TEXT("山间岔路"));
		EventNode.DisplayLabel = TEXT("RunNode.Starter.Event");
		EventNode.ChapterIndex = 1;
		EventNode.FloorIndex = 3;
		EventNode.NextNodeIds.Add(StarterShopNodeId);
		EventNode.EventContent.Title = FText::FromString(TEXT("山间岔路"));
		EventNode.EventContent.Summary = FText::FromString(TEXT("山路分岔，队伍必须决定接下来的行进方式。"));

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
		ShopNode.DisplayName = FText::FromString(TEXT("行脚商"));
		ShopNode.DisplayLabel = TEXT("RunNode.Starter.Shop");
		ShopNode.ChapterIndex = 1;
		ShopNode.FloorIndex = 4;
		ShopNode.NextNodeIds.Add(StarterEliteBattleNodeId);
		ShopNode.ShopContent.Title = FText::FromString(TEXT("行脚商"));
		ShopNode.ShopContent.Summary = FText::FromString(TEXT("简陋摊位上摆着几张可补入牌库的招式。"));

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
		EliteBattleNode.DisplayName = FText::FromString(TEXT("黑风压阵"));
		EliteBattleNode.DisplayLabel = TEXT("RunNode.Starter.EliteBattle");
		EliteBattleNode.ChapterIndex = 1;
		EliteBattleNode.FloorIndex = 5;
		EliteBattleNode.EncounterId = StarterEliteEncounter->EncounterId;
		EliteBattleNode.RuleConfigId = StarterRuleConfig->RuleConfigId;
		EliteBattleNode.NextNodeIds.Add(StarterBossBattleNodeId);

		FFinalRunNodeDefinition BossBattleNode;
		BossBattleNode.NodeId = StarterBossBattleNodeId;
		BossBattleNode.NodeType = EFinalRunNodeType::BossBattle;
		BossBattleNode.DisplayName = FText::FromString(TEXT("黑风寨主"));
		BossBattleNode.DisplayLabel = TEXT("RunNode.Starter.BossBattle");
		BossBattleNode.ChapterIndex = 1;
		BossBattleNode.FloorIndex = 6;
		BossBattleNode.EncounterId = StarterBossEncounter->EncounterId;
		BossBattleNode.RuleConfigId = StarterRuleConfig->RuleConfigId;

		StarterRunRoute->NodeDefinitions = {
			OpeningBattleNode,
			RewardNode,
			EventNode,
			ShopNode,
			EliteBattleNode,
			BossBattleNode
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
		MakeBootstrapCharacterState(StarterHuoCharacter->CharacterId, 0, 1, 80, 100, 0, 0, 0),
		MakeBootstrapCharacterState(StarterYeCharacter->CharacterId, 0, 1, 0, 100, 0, 0, 0),
		MakeBootstrapCharacterState(StarterShenCharacter->CharacterId, 0, 1, 0, 100, 0, 0, 0)
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
		StarterShenYinBaoJianZhenCard->CardId
	};
	StarterBootstrap->InitialTeamCurrentHP = 62;
	TrackPackage(StarterBootstrap, PackagesToSave);
}
