#include "App/FinalGameInstance.h"

#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalEnemyDefinition.h"
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
	RuntimeTestAssets.Add(TestGuardianStrikeCard);

	TestGuardianGuardCard = NewObject<UFinalCardDefinition>(this, TEXT("DA_TestGuardianGuardCard"));
	TestGuardianGuardCard->CardId = FFinalCardId(FinalTestBootstrap::GuardianGuardCardId);
	TestGuardianGuardCard->OwnerUnitId = TestGuardianDefinition->CharacterId.Value;
	TestGuardianGuardCard->DisplayName = FText::FromString(TEXT("试作格挡"));
	TestGuardianGuardCard->CardType = EFinalCardType::Skill;
	TestGuardianGuardCard->Rarity = EFinalRarity::Common;
	TestGuardianGuardCard->BaseCostAP = 1;
	TestGuardianGuardCard->RulesText = FText::FromString(TEXT("测试用防御牌。"));
	RuntimeTestAssets.Add(TestGuardianGuardCard);

	TestSupportShotCard = NewObject<UFinalCardDefinition>(this, TEXT("DA_TestSupportShotCard"));
	TestSupportShotCard->CardId = FFinalCardId(FinalTestBootstrap::SupportShotCardId);
	TestSupportShotCard->OwnerUnitId = TestSupportDefinition->CharacterId.Value;
	TestSupportShotCard->DisplayName = FText::FromString(TEXT("试作速射"));
	TestSupportShotCard->CardType = EFinalCardType::Attack;
	TestSupportShotCard->Rarity = EFinalRarity::Common;
	TestSupportShotCard->BaseCostAP = 1;
	TestSupportShotCard->RulesText = FText::FromString(TEXT("测试用远程攻击牌。"));
	RuntimeTestAssets.Add(TestSupportShotCard);

	TestSupportFocusCard = NewObject<UFinalCardDefinition>(this, TEXT("DA_TestSupportFocusCard"));
	TestSupportFocusCard->CardId = FFinalCardId(FinalTestBootstrap::SupportFocusCardId);
	TestSupportFocusCard->OwnerUnitId = TestSupportDefinition->CharacterId.Value;
	TestSupportFocusCard->DisplayName = FText::FromString(TEXT("试作整备"));
	TestSupportFocusCard->CardType = EFinalCardType::Skill;
	TestSupportFocusCard->Rarity = EFinalRarity::Common;
	TestSupportFocusCard->BaseCostAP = 1;
	TestSupportFocusCard->RulesText = FText::FromString(TEXT("测试用辅助牌。"));
	RuntimeTestAssets.Add(TestSupportFocusCard);

	UFinalEnemyDefinition* TestEnemyDefinition = NewObject<UFinalEnemyDefinition>(this, TEXT("DA_TestEnemyDefinition"));
	TestEnemyDefinition->EnemyId = FFinalEnemyId(FinalTestBootstrap::EnemyId);
	TestEnemyDefinition->DisplayName = FText::FromString(TEXT("测试劫匪"));
	TestEnemyDefinition->MaxHP = 36;
	TestEnemyDefinition->MaxBreakValue = 12;
	TestEnemyDefinition->BaseDamagePower = 6;
	TestEnemyDefinition->InitialInitiativeValue = 2;
	TestEnemyDefinition->InitiativeResponse = 1;
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
		TestGuardianStrikeCard->CardId,
		TestGuardianGuardCard->CardId,
		TestSupportShotCard->CardId,
		TestSupportShotCard->CardId,
		TestSupportFocusCard->CardId
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
