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
#include "Runtime/FinalRunPersistentCharacterState.h"
#include "Subsystems/FinalGameFlowSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogFinalGameInstance, Log, All);

namespace FinalTestBootstrap
{
	const FName RuleConfigId(TEXT("rule.test.bootstrap"));
	const FName EncounterId(TEXT("encounter.test.bootstrap"));
	const FName GuardianCharacterId(TEXT("character.test.guardian"));
	const FName SupportCharacterId(TEXT("character.test.support"));
	const FName EnemyId(TEXT("enemy.test.raider"));
	const FName GuardianStrikeCardId(TEXT("card.test.guardian.strike"));
	const FName GuardianGuardCardId(TEXT("card.test.guardian.guard"));
	const FName SupportShotCardId(TEXT("card.test.support.shot"));
	const FName SupportFocusCardId(TEXT("card.test.support.focus"));
	const FName PhaseOneTag(TEXT("phase.one"));
	const FName PhaseTwoTag(TEXT("phase.two"));
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

	TestRuleConfig = NewObject<UFinalBattleRuleConfig>(this, TEXT("DA_TestBattleRuleConfig"));
	TestRuleConfig->RuleConfigId = FFinalRuleConfigId(FinalTestBootstrap::RuleConfigId);
	TestRuleConfig->InitialAP = 3;
	TestRuleConfig->InitialHandSize = 5;
	TestRuleConfig->HandLimit = 10;
	TestRuleConfig->MaxEP = 70;
	TestRuleConfig->EndTurnEpGain = 3;
	TestRuleConfig->OnHitEpGain = 4;
	TestRuleConfig->BaseCardEpGain = 1;
	TestRuleConfig->BreakRewardAP = 1;
	TestRuleConfig->NormalCardInitiativeEventCount = 1;
	TestRuleConfig->CollapsedCardInitiativeEventCount = 1;
	TestRuleConfig->StressHpLossPerPoint = 5;
	TestRuleConfig->StressHealPerPoint = 8;
	TestRuleConfig->MinStressChangePerEvent = 1;
	TestRuleConfig->MaxStressGainPerHit = 3;
	TestRuleConfig->StressRandomProtectionCount = 2;
	TestRuleConfig->DamageToBreakCap = 6;
	RuntimeTestAssets.Add(TestRuleConfig);

	TestGuardianDefinition = NewObject<UFinalCharacterDefinition>(this, TEXT("DA_TestGuardianCharacter"));
	TestGuardianDefinition->CharacterId = FFinalCharacterId(FinalTestBootstrap::GuardianCharacterId);
	TestGuardianDefinition->DisplayName = FText::FromString(TEXT("测试先锋"));
	TestGuardianDefinition->BaseVitalShare = 24;
	TestGuardianDefinition->BaseStressCap = 12;
	TestGuardianDefinition->BaseAttack = 7;
	TestGuardianDefinition->BaseDefense = 3;
	TestGuardianDefinition->BaseBreakRate = 1.2f;
	TestGuardianDefinition->BaseCritChance = 0.05f;
	TestGuardianDefinition->BaseCritDamage = 1.5f;
	TestGuardianDefinition->EpGainPerAP = 1;
	RuntimeTestAssets.Add(TestGuardianDefinition);

	TestSupportDefinition = NewObject<UFinalCharacterDefinition>(this, TEXT("DA_TestSupportCharacter"));
	TestSupportDefinition->CharacterId = FFinalCharacterId(FinalTestBootstrap::SupportCharacterId);
	TestSupportDefinition->DisplayName = FText::FromString(TEXT("测试策应"));
	TestSupportDefinition->BaseVitalShare = 18;
	TestSupportDefinition->BaseStressCap = 14;
	TestSupportDefinition->BaseAttack = 5;
	TestSupportDefinition->BaseDefense = 2;
	TestSupportDefinition->BaseBreakRate = 1.0f;
	TestSupportDefinition->BaseCritChance = 0.08f;
	TestSupportDefinition->BaseCritDamage = 1.5f;
	TestSupportDefinition->EpGainPerAP = 1;
	RuntimeTestAssets.Add(TestSupportDefinition);

	TestGuardianStrikeCard = NewObject<UFinalCardDefinition>(this, TEXT("DA_TestGuardianStrikeCard"));
	TestGuardianStrikeCard->CardId = FFinalCardId(FinalTestBootstrap::GuardianStrikeCardId);
	TestGuardianStrikeCard->OwnerUnitId = TestGuardianDefinition->CharacterId.Value;
	TestGuardianStrikeCard->DisplayName = FText::FromString(TEXT("试作斩击"));
	TestGuardianStrikeCard->CardType = EFinalCardType::Attack;
	TestGuardianStrikeCard->Rarity = EFinalRarity::Common;
	TestGuardianStrikeCard->BaseCostAP = 1;
	TestGuardianStrikeCard->RulesText = FText::FromString(TEXT("测试用普通攻击牌。"));
	UFinalBattleEffectDamage* GuardianStrikeDamage = NewObject<UFinalBattleEffectDamage>(TestGuardianStrikeCard);
	GuardianStrikeDamage->EffectId = TEXT("effect.test.guardian.strike.damage");
	GuardianStrikeDamage->Scalar.BaseValue = 1.0f;
	GuardianStrikeDamage->Scalar.ScaleMode = EFinalBattleScalarMode::SourceStatMultiplier;
	GuardianStrikeDamage->Scalar.SourceStat = EFinalBattleSourceStat::Attack;
	TestGuardianStrikeCard->Effects.Add(GuardianStrikeDamage);
	RuntimeTestAssets.Add(TestGuardianStrikeCard);

	TestGuardianGuardCard = NewObject<UFinalCardDefinition>(this, TEXT("DA_TestGuardianGuardCard"));
	TestGuardianGuardCard->CardId = FFinalCardId(FinalTestBootstrap::GuardianGuardCardId);
	TestGuardianGuardCard->OwnerUnitId = TestGuardianDefinition->CharacterId.Value;
	TestGuardianGuardCard->DisplayName = FText::FromString(TEXT("试作格挡"));
	TestGuardianGuardCard->CardType = EFinalCardType::Skill;
	TestGuardianGuardCard->Rarity = EFinalRarity::Common;
	TestGuardianGuardCard->BaseCostAP = 1;
	TestGuardianGuardCard->RulesText = FText::FromString(TEXT("测试用防御牌。"));
	UFinalBattleEffectGainShield* GuardianGuardShield = NewObject<UFinalBattleEffectGainShield>(TestGuardianGuardCard);
	GuardianGuardShield->EffectId = TEXT("effect.test.guardian.guard.shield");
	GuardianGuardShield->Scalar.BaseValue = 1.0f;
	GuardianGuardShield->Scalar.ScaleMode = EFinalBattleScalarMode::SourceStatMultiplier;
	GuardianGuardShield->Scalar.SourceStat = EFinalBattleSourceStat::Defense;
	TestGuardianGuardCard->Effects.Add(GuardianGuardShield);
	RuntimeTestAssets.Add(TestGuardianGuardCard);

	TestSupportShotCard = NewObject<UFinalCardDefinition>(this, TEXT("DA_TestSupportShotCard"));
	TestSupportShotCard->CardId = FFinalCardId(FinalTestBootstrap::SupportShotCardId);
	TestSupportShotCard->OwnerUnitId = TestSupportDefinition->CharacterId.Value;
	TestSupportShotCard->DisplayName = FText::FromString(TEXT("试作速射"));
	TestSupportShotCard->CardType = EFinalCardType::Attack;
	TestSupportShotCard->Rarity = EFinalRarity::Common;
	TestSupportShotCard->BaseCostAP = 1;
	TestSupportShotCard->RulesText = FText::FromString(TEXT("测试用远程攻击牌。"));
	UFinalBattleEffectDamage* SupportShotDamage = NewObject<UFinalBattleEffectDamage>(TestSupportShotCard);
	SupportShotDamage->EffectId = TEXT("effect.test.support.shot.damage");
	SupportShotDamage->Scalar.BaseValue = 1.0f;
	SupportShotDamage->Scalar.ScaleMode = EFinalBattleScalarMode::SourceStatMultiplier;
	SupportShotDamage->Scalar.SourceStat = EFinalBattleSourceStat::Attack;
	TestSupportShotCard->Effects.Add(SupportShotDamage);
	RuntimeTestAssets.Add(TestSupportShotCard);

	TestSupportFocusCard = NewObject<UFinalCardDefinition>(this, TEXT("DA_TestSupportFocusCard"));
	TestSupportFocusCard->CardId = FFinalCardId(FinalTestBootstrap::SupportFocusCardId);
	TestSupportFocusCard->OwnerUnitId = TestSupportDefinition->CharacterId.Value;
	TestSupportFocusCard->DisplayName = FText::FromString(TEXT("试作整备"));
	TestSupportFocusCard->CardType = EFinalCardType::Skill;
	TestSupportFocusCard->Rarity = EFinalRarity::Common;
	TestSupportFocusCard->BaseCostAP = 1;
	TestSupportFocusCard->RulesText = FText::FromString(TEXT("测试用辅助牌。"));
	UFinalBattleEffectDrawCards* SupportFocusDraw = NewObject<UFinalBattleEffectDrawCards>(TestSupportFocusCard);
	SupportFocusDraw->EffectId = TEXT("effect.test.support.focus.draw");
	SupportFocusDraw->DrawCount = 2;
	TestSupportFocusCard->Effects.Add(SupportFocusDraw);
	RuntimeTestAssets.Add(TestSupportFocusCard);

	UFinalEnemyDefinition* TestEnemyDefinition = NewObject<UFinalEnemyDefinition>(this, TEXT("DA_TestEnemyDefinition"));
	TestEnemyDefinition->EnemyId = FFinalEnemyId(FinalTestBootstrap::EnemyId);
	TestEnemyDefinition->DisplayName = FText::FromString(TEXT("测试劫匪"));
	TestEnemyDefinition->MaxHP = 36;
	TestEnemyDefinition->MaxBreakValue = 12;
	TestEnemyDefinition->BaseDamagePower = 6;
	TestEnemyDefinition->InitialInitiativeValue = 2;
	TestEnemyDefinition->InitiativeResponse = 1;
	TestEnemyDefinition->IntentSelectRule = EFinalIntentSelectRule::PhaseSequence;

	FFinalEnemyPhaseDefinition& PhaseOneDefinition = TestEnemyDefinition->PhaseSequence.AddDefaulted_GetRef();
	PhaseOneDefinition.PhaseTag = FinalTestBootstrap::PhaseOneTag;
	PhaseOneDefinition.MaxHpPercent = 1.0f;

	FFinalEnemyPhaseDefinition& PhaseTwoDefinition = TestEnemyDefinition->PhaseSequence.AddDefaulted_GetRef();
	PhaseTwoDefinition.PhaseTag = FinalTestBootstrap::PhaseTwoTag;
	PhaseTwoDefinition.MaxHpPercent = 0.5f;

	UFinalEnemyIntentDefinition* TestEnemyAttackIntent = NewObject<UFinalEnemyIntentDefinition>(this, TEXT("DA_TestEnemyAttackIntent"));
	TestEnemyAttackIntent->IntentId = TEXT("intent.test.enemy.attack");
	TestEnemyAttackIntent->DisplayName = FText::FromString(TEXT("试作劈砍"));
	TestEnemyAttackIntent->IntentType = EFinalIntentType::Attack;
	TestEnemyAttackIntent->PreviewText = FText::FromString(TEXT("劈砍 6"));
	TestEnemyAttackIntent->PhaseTags.Add(FinalTestBootstrap::PhaseOneTag);
	UFinalBattleEffectDamage* TestEnemyAttackEffect = NewObject<UFinalBattleEffectDamage>(TestEnemyAttackIntent);
	TestEnemyAttackEffect->EffectId = TEXT("effect.test.enemy.attack.damage");
	TestEnemyAttackEffect->UnitTargetRule = EFinalBattleUnitTargetRule::TeamPlayer;
	TestEnemyAttackEffect->Scalar.BaseValue = 1.0f;
	TestEnemyAttackEffect->Scalar.ScaleMode = EFinalBattleScalarMode::SourceStatMultiplier;
	TestEnemyAttackEffect->Scalar.SourceStat = EFinalBattleSourceStat::BaseDamagePower;
	TestEnemyAttackIntent->Effects.Add(TestEnemyAttackEffect);
	RuntimeTestAssets.Add(TestEnemyAttackIntent);

	UFinalEnemyIntentDefinition* TestEnemyGuardIntent = NewObject<UFinalEnemyIntentDefinition>(this, TEXT("DA_TestEnemyGuardIntent"));
	TestEnemyGuardIntent->IntentId = TEXT("intent.test.enemy.guard");
	TestEnemyGuardIntent->DisplayName = FText::FromString(TEXT("试作整备"));
	TestEnemyGuardIntent->IntentType = EFinalIntentType::Defense;
	TestEnemyGuardIntent->PreviewText = FText::FromString(TEXT("获得 4 护盾"));
	TestEnemyGuardIntent->PhaseTags.Add(FinalTestBootstrap::PhaseOneTag);
	TestEnemyGuardIntent->CooldownTurns = 1;
	TestEnemyGuardIntent->UseLimitPerBattle = 2;
	UFinalBattleEffectGainShield* TestEnemyGuardEffect = NewObject<UFinalBattleEffectGainShield>(TestEnemyGuardIntent);
	TestEnemyGuardEffect->EffectId = TEXT("effect.test.enemy.guard.shield");
	TestEnemyGuardEffect->UnitTargetRule = EFinalBattleUnitTargetRule::Self;
	TestEnemyGuardEffect->Scalar.BaseValue = 4.0f;
	TestEnemyGuardEffect->Scalar.ScaleMode = EFinalBattleScalarMode::Flat;
	TestEnemyGuardIntent->Effects.Add(TestEnemyGuardEffect);
	RuntimeTestAssets.Add(TestEnemyGuardIntent);

	UFinalEnemyIntentDefinition* TestEnemyEnrageIntent = NewObject<UFinalEnemyIntentDefinition>(this, TEXT("DA_TestEnemyEnrageIntent"));
	TestEnemyEnrageIntent->IntentId = TEXT("intent.test.enemy.enrage");
	TestEnemyEnrageIntent->DisplayName = FText::FromString(TEXT("试作狂斩"));
	TestEnemyEnrageIntent->IntentType = EFinalIntentType::Attack;
	TestEnemyEnrageIntent->PreviewText = FText::FromString(TEXT("狂斩 10"));
	TestEnemyEnrageIntent->PhaseTags.Add(FinalTestBootstrap::PhaseTwoTag);
	UFinalBattleEffectDamage* TestEnemyEnrageEffect = NewObject<UFinalBattleEffectDamage>(TestEnemyEnrageIntent);
	TestEnemyEnrageEffect->EffectId = TEXT("effect.test.enemy.enrage.damage");
	TestEnemyEnrageEffect->UnitTargetRule = EFinalBattleUnitTargetRule::TeamPlayer;
	TestEnemyEnrageEffect->Scalar.BaseValue = 10.0f;
	TestEnemyEnrageEffect->Scalar.ScaleMode = EFinalBattleScalarMode::Flat;
	TestEnemyEnrageIntent->Effects.Add(TestEnemyEnrageEffect);
	RuntimeTestAssets.Add(TestEnemyEnrageIntent);

	TestEnemyDefinition->IntentPool.Add(TestEnemyAttackIntent);
	TestEnemyDefinition->IntentPool.Add(TestEnemyGuardIntent);
	TestEnemyDefinition->IntentPool.Add(TestEnemyEnrageIntent);
	RuntimeTestAssets.Add(TestEnemyDefinition);

	TestEncounterDefinition = NewObject<UFinalBattleEncounterDefinition>(this, TEXT("DA_TestEncounterDefinition"));
	TestEncounterDefinition->EncounterId = FFinalEncounterId(FinalTestBootstrap::EncounterId);
	TestEncounterDefinition->DisplayName = FText::FromString(TEXT("测试遭遇"));
	TestEncounterDefinition->RuleConfig = TestRuleConfig;

	FFinalEnemyRosterEntry EnemyRosterEntry;
	EnemyRosterEntry.EnemyDefinition = TestEnemyDefinition;
	EnemyRosterEntry.PositionIndex = 0;
	EnemyRosterEntry.SpawnWave = 1;
	TestEncounterDefinition->EnemyRoster.Add(EnemyRosterEntry);
	RuntimeTestAssets.Add(TestEncounterDefinition);

	DataRegistry->RegisterRuleConfig(TestRuleConfig);
	DataRegistry->RegisterCharacterDefinition(TestGuardianDefinition);
	DataRegistry->RegisterCharacterDefinition(TestSupportDefinition);
	DataRegistry->RegisterCardDefinition(TestGuardianStrikeCard);
	DataRegistry->RegisterCardDefinition(TestGuardianGuardCard);
	DataRegistry->RegisterCardDefinition(TestSupportShotCard);
	DataRegistry->RegisterCardDefinition(TestSupportFocusCard);
	DataRegistry->RegisterEncounterDefinition(TestEncounterDefinition);

	bTestBattleBootstrapRegistered = true;

	UE_LOG(LogFinalGameInstance, Log, TEXT("Registered transient bootstrap data for test battle."));
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

	UFinalBattleSession* BattleSession = GameFlowSubsystem->StartBattleFromRunSession();
	if (BattleSession == nullptr)
	{
		LastTestFailureReason = GameFlowSubsystem->GetLastBattleFailureReason();
		if (LastTestFailureReason.IsEmpty())
		{
			LastTestFailureReason = FText::FromString(TEXT("Failed to start test battle."));
		}
		return false;
	}

	return true;
}

FText UFinalGameInstance::GetLastTestFailureReason() const
{
	return LastTestFailureReason;
}
