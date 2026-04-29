#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalEnemyDefinition.h"
#include "Battle/Definitions/FinalEnemyIntentDefinition.h"
#include "Battle/Definitions/FinalPassiveDefinition.h"
#include "Battle/Definitions/FinalStatusDefinition.h"
#include "Battle/Effects/FinalBattleEffectApplyPassive.h"
#include "Battle/Effects/FinalBattleEffectApplyStatus.h"
#include "Battle/Effects/FinalBattleEffectDamage.h"
#include "Commands/FinalBattleCommand.h"
#include "Facade/FinalBattleSession.h"
#include "Facade/FinalBattleSessionTypes.h"
#include "Queries/FinalBattleSnapshot.h"
#include "UObject/StrongObjectPtr.h"

namespace FinalBattlePassiveTests
{
	TStrongObjectPtr<UFinalBattleRuleConfig> MakeRuleConfig()
	{
		TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig(NewObject<UFinalBattleRuleConfig>(GetTransientPackage()));
		RuleConfig->RuleConfigId = FFinalRuleConfigId(FName(TEXT("rule.test.passive")));
		RuleConfig->InitialAP = 3;
		RuleConfig->InitialHandSize = 1;
		RuleConfig->TurnStartDrawCount = 1;
		return RuleConfig;
	}

	TStrongObjectPtr<UFinalEnemyIntentDefinition> MakeEnemyAttackIntent()
	{
		TStrongObjectPtr<UFinalEnemyIntentDefinition> Intent(NewObject<UFinalEnemyIntentDefinition>(GetTransientPackage()));
		Intent->IntentId = TEXT("intent.test.passive.attack");
		Intent->DisplayName = FText::FromString(TEXT("Test Attack"));
		Intent->PreviewText = FText::FromString(TEXT("Deal 3 team damage"));
		Intent->IntentType = EFinalIntentType::Attack;
		Intent->Weight = 1;

		UFinalBattleEffectDamage* DamageEffect = NewObject<UFinalBattleEffectDamage>(Intent.Get());
		DamageEffect->EffectId = TEXT("effect.test.passive.enemy_attack");
		DamageEffect->UnitTargetRule = EFinalBattleUnitTargetRule::TeamPlayer;
		DamageEffect->Scalar.ScaleMode = EFinalBattleScalarMode::Flat;
		DamageEffect->Scalar.BaseValue = 3.0f;
		Intent->Effects.Add(DamageEffect);
		return Intent;
	}

	TStrongObjectPtr<UFinalEnemyDefinition> MakeEnemyDefinition(UFinalEnemyIntentDefinition* IntentDefinition)
	{
		TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition(NewObject<UFinalEnemyDefinition>(GetTransientPackage()));
		EnemyDefinition->EnemyId = FFinalEnemyId(FName(TEXT("enemy.test.passive")));
		EnemyDefinition->DisplayName = FText::FromString(TEXT("Passive Dummy"));
		EnemyDefinition->MaxHP = 20;
		EnemyDefinition->MaxBreakValue = 10;
		EnemyDefinition->BaseDamagePower = 0;
		EnemyDefinition->InitialInitiativeValue = 1;
		EnemyDefinition->IntentSelectRule = EFinalIntentSelectRule::Cycle;
		EnemyDefinition->IntentPool.Add(TSoftObjectPtr<UFinalEnemyIntentDefinition>(IntentDefinition));
		return EnemyDefinition;
	}

	TStrongObjectPtr<UFinalBattleEncounterDefinition> MakeEncounter(UFinalBattleRuleConfig* RuleConfig, UFinalEnemyDefinition* EnemyDefinition)
	{
		TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition(NewObject<UFinalBattleEncounterDefinition>(GetTransientPackage()));
		EncounterDefinition->EncounterId = FFinalEncounterId(FName(TEXT("encounter.test.passive")));
		EncounterDefinition->DisplayName = FText::FromString(TEXT("Passive Encounter"));
		EncounterDefinition->RuleConfig = RuleConfig;

		FFinalEnemyRosterEntry& Entry = EncounterDefinition->EnemyRoster.AddDefaulted_GetRef();
		Entry.EnemyDefinition = EnemyDefinition;
		Entry.PositionIndex = 0;
		Entry.SpawnWave = 1;
		return EncounterDefinition;
	}

	TStrongObjectPtr<UFinalCharacterDefinition> MakeCharacterDefinition(const FFinalCharacterId& CharacterId)
	{
		TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition(NewObject<UFinalCharacterDefinition>(GetTransientPackage()));
		CharacterDefinition->CharacterId = CharacterId;
		CharacterDefinition->DisplayName = FText::FromString(TEXT("Passive Hero"));
		CharacterDefinition->BaseVitalShare = 20;
		CharacterDefinition->BaseStressCap = 12;
		CharacterDefinition->BaseAttack = 4;
		CharacterDefinition->BaseDefense = 1;
		CharacterDefinition->BaseBreakRate = 1.0f;
		CharacterDefinition->BaseCritChance = 0.0f;
		CharacterDefinition->BaseCritDamage = 1.5f;
		return CharacterDefinition;
	}

	TStrongObjectPtr<UFinalStatusDefinition> MakeDaoShiStatus()
	{
		TStrongObjectPtr<UFinalStatusDefinition> StatusDefinition(NewObject<UFinalStatusDefinition>(GetTransientPackage()));
		StatusDefinition->StatusId = FFinalStatusId(FName(TEXT("status.test.passive.daoshi")));
		StatusDefinition->DisplayName = FText::FromString(TEXT("刀势"));
		StatusDefinition->StatusCategory = EFinalStatusCategory::Signature;
		StatusDefinition->MaxStacks = 9;
		return StatusDefinition;
	}

	TStrongObjectPtr<UFinalPassiveDefinition> MakeTookDamagePassive(UFinalStatusDefinition* StatusDefinition)
	{
		TStrongObjectPtr<UFinalPassiveDefinition> PassiveDefinition(NewObject<UFinalPassiveDefinition>(GetTransientPackage()));
		PassiveDefinition->PassiveId = FFinalPassiveId(FName(TEXT("passive.test.took_damage_gain_daoshi")));
		PassiveDefinition->DisplayId = TEXT("Passive.Test.TookDamageGainDaoShi");
		PassiveDefinition->DisplayName = FText::FromString(TEXT("受压得刀势"));
		PassiveDefinition->SummaryText = FText::FromString(TEXT("承受共享生命伤害后，获得 1 层刀势。"));
		PassiveDefinition->StackPolicy = EFinalPassiveStackPolicy::RefreshExisting;
		PassiveDefinition->DurationType = EFinalPassiveDurationType::Battle;
		PassiveDefinition->MaxStacks = 1;

		FFinalRuntimeTriggerDefinition& Trigger = PassiveDefinition->RuntimeTriggers.AddDefaulted_GetRef();
		Trigger.Domain = EFinalRuntimeTriggerDomain::Battle;
		Trigger.Window = EFinalRuntimeTriggerWindow::OwnerTookHealthDamage;
		Trigger.Limit = EFinalRuntimeTriggerLimit::None;

		UFinalBattleEffectApplyStatus* ApplyStatusEffect = NewObject<UFinalBattleEffectApplyStatus>(PassiveDefinition.Get());
		ApplyStatusEffect->EffectId = TEXT("effect.test.passive.apply_daoshi");
		ApplyStatusEffect->UnitTargetRule = EFinalBattleUnitTargetRule::Self;
		ApplyStatusEffect->StatusDefinition = StatusDefinition;
		ApplyStatusEffect->StatusId = StatusDefinition->StatusId;
		ApplyStatusEffect->Stacks = 1;
		Trigger.Effects.Add(ApplyStatusEffect);
		return PassiveDefinition;
	}

	TStrongObjectPtr<UFinalCardDefinition> MakeApplyPassiveCard(
		const FFinalCardId& CardId,
		const FFinalCharacterId& CharacterId,
		UFinalPassiveDefinition* PassiveDefinition)
	{
		TStrongObjectPtr<UFinalCardDefinition> CardDefinition(NewObject<UFinalCardDefinition>(GetTransientPackage()));
		CardDefinition->CardId = CardId;
		CardDefinition->OwnerUnitId = CharacterId.Value;
		CardDefinition->DisplayName = FText::FromString(TEXT("受压蓄势"));
		CardDefinition->BaseCostAP = 1;
		CardDefinition->CardType = EFinalCardType::Ability;

		UFinalBattleEffectApplyPassive* ApplyPassiveEffect = NewObject<UFinalBattleEffectApplyPassive>(CardDefinition.Get());
		ApplyPassiveEffect->EffectId = TEXT("effect.test.card.apply_passive");
		ApplyPassiveEffect->UnitTargetRule = EFinalBattleUnitTargetRule::Self;
		ApplyPassiveEffect->PassiveDefinition = PassiveDefinition;
		ApplyPassiveEffect->PassiveId = PassiveDefinition->PassiveId;
		ApplyPassiveEffect->Stacks = 1;
		CardDefinition->Effects.Add(ApplyPassiveEffect);
		return CardDefinition;
	}

	TStrongObjectPtr<UFinalBattleSession> CreateSession(
		UFinalBattleEncounterDefinition* EncounterDefinition,
		UFinalBattleRuleConfig* RuleConfig,
		UFinalCharacterDefinition* CharacterDefinition,
		UFinalCardDefinition* CardDefinition)
	{
		FFinalBattleInitContext InitContext;
		InitContext.TeamCurrentHP = 20;

		FFinalBattleCharacterInitData& CharacterInit = InitContext.PartyMembers.AddDefaulted_GetRef();
		CharacterInit.CharacterDefinition = CharacterDefinition;
		CharacterInit.CurrentStress = 0;
		CharacterInit.bCollapsed = false;
		CharacterInit.CurrentAwakenCount = 0;
		CharacterInit.CollapseCount = 0;
		CharacterInit.VitalShare = CharacterDefinition->BaseVitalShare;
		CharacterInit.StressCap = CharacterDefinition->BaseStressCap;
		CharacterInit.RuntimeAttack = CharacterDefinition->BaseAttack;
		CharacterInit.RuntimeDefense = CharacterDefinition->BaseDefense;
		CharacterInit.RuntimeBreakRate = CharacterDefinition->BaseBreakRate;
		CharacterInit.RuntimeCritChance = CharacterDefinition->BaseCritChance;
		CharacterInit.RuntimeCritDamage = CharacterDefinition->BaseCritDamage;

		FFinalBattleCardInitData& CardInit = InitContext.DeckCards.AddDefaulted_GetRef();
		CardInit.CardDefinition = CardDefinition;
		CardInit.CardId = CardDefinition->CardId;
		CardInit.OwnerCharacterId = CharacterDefinition->CharacterId;
		CardInit.SourceRunCardInstanceId = TEXT("run.card.passive.test");
		InitContext.DeckDefinitions.Add(CardDefinition);

		TStrongObjectPtr<UFinalBattleSession> Session(NewObject<UFinalBattleSession>(GetTransientPackage()));
		Session->InitializeSession(EncounterDefinition, RuleConfig, InitContext);
		return Session;
	}

	const FFinalBattleCardViewData* FindHandCardById(const FFinalBattleSnapshot& Snapshot, const FFinalCardId& CardId)
	{
		return Snapshot.HandCards.FindByPredicate([&CardId](const FFinalBattleCardViewData& CardView)
		{
			return CardView.CardId == CardId;
		});
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattlePassiveApplyCreatesRuntimeInstanceTest,
	"Final.Battle.Passive.ApplyPassiveCreatesRuntimeInstance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattlePassiveApplyCreatesRuntimeInstanceTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattlePassiveTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.passive.apply")));
	const FFinalCardId AbilityCardId(FName(TEXT("card.test.passive.apply")));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig();
	TStrongObjectPtr<UFinalEnemyIntentDefinition> EnemyIntent = MakeEnemyAttackIntent();
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition(EnemyIntent.Get());
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition(CharacterId);
	TStrongObjectPtr<UFinalStatusDefinition> DaoShiStatus = MakeDaoShiStatus();
	TStrongObjectPtr<UFinalPassiveDefinition> PassiveDefinition = MakeTookDamagePassive(DaoShiStatus.Get());
	TStrongObjectPtr<UFinalCardDefinition> AbilityCard = MakeApplyPassiveCard(AbilityCardId, CharacterId, PassiveDefinition.Get());
	TStrongObjectPtr<UFinalBattleSession> Session = CreateSession(EncounterDefinition.Get(), RuleConfig.Get(), CharacterDefinition.Get(), AbilityCard.Get());

	const FFinalBattleSnapshot InitialSnapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* HandCard = FindHandCardById(InitialSnapshot, AbilityCardId);
	if (!TestNotNull(TEXT("Ability card should begin in hand."), HandCard))
	{
		return false;
	}

	FFinalBattleCommand Command;
	Command.CommandType = EFinalBattleCommandType::PlayCard;
	Command.CardInstanceId = HandCard->CardInstanceId;
	const FFinalBattleEvent Event = Session->SubmitCommand(Command);
	TestNotEqual(TEXT("ApplyPassive card should resolve successfully."), Event.EventType, EFinalBattleEventType::CommandRejected);

	const FFinalBattleSnapshot SnapshotAfterPlay = Session->GetSnapshot();
	TestEqual(TEXT("Playing the ability card should create one passive instance."), SnapshotAfterPlay.Passives.Num(), 1);
	if (SnapshotAfterPlay.Passives.Num() != 1)
	{
		return false;
	}

	const FFinalBattlePassiveViewData& PassiveView = SnapshotAfterPlay.Passives[0];
	TestEqual(TEXT("PassiveId should match the applied passive definition."), PassiveView.PassiveId, PassiveDefinition->PassiveId);
	TestEqual(TEXT("Passive owner should be the acting character."), PassiveView.OwnerUnitId, InitialSnapshot.Characters[0].RuntimeUnitId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattlePassiveTookDamageAppliesDaoShiTest,
	"Final.Battle.Passive.OwnerTookHealthDamageAppliesDaoShi",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattlePassiveTookDamageAppliesDaoShiTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattlePassiveTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.passive.took_damage")));
	const FFinalCardId AbilityCardId(FName(TEXT("card.test.passive.took_damage")));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig();
	TStrongObjectPtr<UFinalEnemyIntentDefinition> EnemyIntent = MakeEnemyAttackIntent();
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition(EnemyIntent.Get());
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition(CharacterId);
	TStrongObjectPtr<UFinalStatusDefinition> DaoShiStatus = MakeDaoShiStatus();
	TStrongObjectPtr<UFinalPassiveDefinition> PassiveDefinition = MakeTookDamagePassive(DaoShiStatus.Get());
	TStrongObjectPtr<UFinalCardDefinition> AbilityCard = MakeApplyPassiveCard(AbilityCardId, CharacterId, PassiveDefinition.Get());
	TStrongObjectPtr<UFinalBattleSession> Session = CreateSession(EncounterDefinition.Get(), RuleConfig.Get(), CharacterDefinition.Get(), AbilityCard.Get());

	FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* HandCard = FindHandCardById(Snapshot, AbilityCardId);
	if (!TestNotNull(TEXT("Ability card should begin in hand."), HandCard))
	{
		return false;
	}

	FFinalBattleCommand PlayCommand;
	PlayCommand.CommandType = EFinalBattleCommandType::PlayCard;
	PlayCommand.CardInstanceId = HandCard->CardInstanceId;
	TestNotEqual(TEXT("ApplyPassive card should resolve successfully."), Session->SubmitCommand(PlayCommand).EventType, EFinalBattleEventType::CommandRejected);

	FFinalBattleCommand EndTurnCommand;
	EndTurnCommand.CommandType = EFinalBattleCommandType::EndTurn;
	TestNotEqual(TEXT("Ending the turn should resolve successfully."), Session->SubmitCommand(EndTurnCommand).EventType, EFinalBattleEventType::CommandRejected);

	const FFinalBattleSnapshot SnapshotAfterDamage = Session->GetSnapshot();
	const FFinalBattleStatusViewData* DaoShiView = SnapshotAfterDamage.Statuses.FindByPredicate([&](const FFinalBattleStatusViewData& StatusView)
	{
		return StatusView.StatusId == DaoShiStatus->StatusId
			&& StatusView.OwnerUnitId == SnapshotAfterDamage.Characters[0].RuntimeUnitId;
	});
	if (!TestNotNull(TEXT("Passive should grant DaoShi after owner took health damage."), DaoShiView))
	{
		return false;
	}

	TestEqual(TEXT("Passive-granted DaoShi should add one stack."), DaoShiView->CurrentStacks, 1);
	return true;
}

#endif
