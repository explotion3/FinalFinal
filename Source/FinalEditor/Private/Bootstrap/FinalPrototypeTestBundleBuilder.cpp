#include "Bootstrap/FinalPrototypeTestBundleBuilder.h"

#include "Bootstrap/FinalPrototypeContentBootstrapAssetUtils.h"
#include "Bootstrap/FinalPrototypeContentBootstrapEffectUtils.h"

#include "Battle/Conditions/FinalBattleConditionStatusChanged.h"
#include "Battle/Conditions/FinalBattleConditionHandCard.h"
#include "Battle/Conditions/FinalBattleConditionMovedCards.h"
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
}

void FFinalPrototypeTestBundleBuilder::Build(TSet<UPackage*>& PackagesToSave)
{
	using namespace FinalPrototypeContentBootstrap;

	bool bCreatedAsset = false;
	UFinalBattleRuleConfig* RuleConfig = LoadOrCreateAsset<UFinalBattleRuleConfig>(RulesPath, bCreatedAsset);
	RuleConfig->RuleConfigId = FFinalRuleConfigId(RuleConfigId);
	RuleConfig->InitialAP = 3;
	RuleConfig->InitialHandSize = 5;
	RuleConfig->TurnStartDrawCount = 5;
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
	CharmRelic->RuntimeTriggers.Reset();
	{
		FFinalRuntimeTriggerDefinition& BattleStartTrigger = CharmRelic->RuntimeTriggers.AddDefaulted_GetRef();
		BattleStartTrigger.Domain = EFinalRuntimeTriggerDomain::Battle;
		BattleStartTrigger.Window = EFinalRuntimeTriggerWindow::BattleStart;
		AddGainApEffect(
			CharmRelic,
			BattleStartTrigger.Effects,
			TEXT("effect.test.relic.charm.battle_start_ap"),
			1,
			FText::FromString(TEXT("战斗开始额外获得 1 AP。")));

		FFinalRuntimeTriggerDefinition& TurnStartTrigger = CharmRelic->RuntimeTriggers.AddDefaulted_GetRef();
		TurnStartTrigger.Domain = EFinalRuntimeTriggerDomain::Battle;
		TurnStartTrigger.Window = EFinalRuntimeTriggerWindow::PlayerTurnStart;
		AddGainApEffect(
			CharmRelic,
			TurnStartTrigger.Effects,
			TEXT("effect.test.relic.charm.player_turn_start_ap"),
			1,
			FText::FromString(TEXT("玩家回合开始额外获得 1 AP。")));
	}
	TrackPackage(CharmRelic, PackagesToSave);

	UFinalRelicDefinition* RepairKitRelic = LoadOrCreateAsset<UFinalRelicDefinition>(RepairKitRelicPath, bCreatedAsset);
	RepairKitRelic->RelicId = FFinalRelicId(ShopRepairKitRelicId);
	RepairKitRelic->DisplayId = TEXT("Relic.Test.RepairKit");
	RepairKitRelic->DisplayName = FText::FromString(TEXT("试作修理包"));
	RepairKitRelic->Description = FText::FromString(TEXT("战斗开始获得 4 护盾，玩家回合开始获得 2 护盾。"));
	RepairKitRelic->RuntimeTriggers.Reset();
	{
		FFinalRuntimeTriggerDefinition& BattleStartTrigger = RepairKitRelic->RuntimeTriggers.AddDefaulted_GetRef();
		BattleStartTrigger.Domain = EFinalRuntimeTriggerDomain::Battle;
		BattleStartTrigger.Window = EFinalRuntimeTriggerWindow::BattleStart;
		AddShieldEffect(
			RepairKitRelic,
			BattleStartTrigger.Effects,
			TEXT("effect.test.relic.repair_kit.battle_start_shield"),
			EFinalBattleUnitTargetRule::TeamPlayer,
			4.0f,
			EFinalBattleScalarMode::Flat,
			EFinalBattleSourceStat::None,
			FText::FromString(TEXT("战斗开始获得 4 护盾。")));

		FFinalRuntimeTriggerDefinition& TurnStartTrigger = RepairKitRelic->RuntimeTriggers.AddDefaulted_GetRef();
		TurnStartTrigger.Domain = EFinalRuntimeTriggerDomain::Battle;
		TurnStartTrigger.Window = EFinalRuntimeTriggerWindow::PlayerTurnStart;
		AddShieldEffect(
			RepairKitRelic,
			TurnStartTrigger.Effects,
			TEXT("effect.test.relic.repair_kit.player_turn_start_shield"),
			EFinalBattleUnitTargetRule::TeamPlayer,
			2.0f,
			EFinalBattleScalarMode::Flat,
			EFinalBattleSourceStat::None,
			FText::FromString(TEXT("玩家回合开始获得 2 护盾。")));
	}
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
}
