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
#include "Battle/Conditions/FinalBattleConditionResolvedCard.h"
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
	TStrongObjectPtr<UFinalBattleRuleConfig> MakeRuleConfig(const int32 InitialHandSize = 1)
	{
		TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig(NewObject<UFinalBattleRuleConfig>(GetTransientPackage()));
		RuleConfig->RuleConfigId = FFinalRuleConfigId(FName(TEXT("rule.test.passive")));
		RuleConfig->InitialAP = 3;
		RuleConfig->InitialHandSize = InitialHandSize;
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

	TStrongObjectPtr<UFinalPassiveDefinition> MakeFirstAttackPassive(UFinalStatusDefinition* StatusDefinition)
	{
		TStrongObjectPtr<UFinalPassiveDefinition> PassiveDefinition(NewObject<UFinalPassiveDefinition>(GetTransientPackage()));
		PassiveDefinition->PassiveId = FFinalPassiveId(FName(TEXT("passive.test.first_attack_gain_daoshi")));
		PassiveDefinition->DisplayId = TEXT("Passive.Test.FirstAttackGainDaoShi");
		PassiveDefinition->DisplayName = FText::FromString(TEXT("压势追刀"));
		PassiveDefinition->SummaryText = FText::FromString(TEXT("每回合第一次打出攻击牌后，获得 1 层刀势。"));
		PassiveDefinition->StackPolicy = EFinalPassiveStackPolicy::RefreshExisting;
		PassiveDefinition->DurationType = EFinalPassiveDurationType::Battle;
		PassiveDefinition->MaxStacks = 1;

		FFinalRuntimeTriggerDefinition& Trigger = PassiveDefinition->RuntimeTriggers.AddDefaulted_GetRef();
		Trigger.Domain = EFinalRuntimeTriggerDomain::Battle;
		Trigger.Window = EFinalRuntimeTriggerWindow::PlayerCardResolved;
		Trigger.Limit = EFinalRuntimeTriggerLimit::OncePerPlayerTurn;

		FFinalBattleResolvedCardRequirement Requirement;
		Requirement.bRequireCardType = true;
		Requirement.RequiredCardType = EFinalCardType::Attack;
		UFinalBattleConditionResolvedCard* ResolvedCardCondition = NewObject<UFinalBattleConditionResolvedCard>(PassiveDefinition.Get());
		ResolvedCardCondition->ConditionId = TEXT("condition.test.passive.first_attack");
		ResolvedCardCondition->Requirement = Requirement;
		Trigger.Conditions.Add(ResolvedCardCondition);

		UFinalBattleEffectApplyStatus* ApplyStatusEffect = NewObject<UFinalBattleEffectApplyStatus>(PassiveDefinition.Get());
		ApplyStatusEffect->EffectId = TEXT("effect.test.passive.first_attack_daoshi");
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

	TStrongObjectPtr<UFinalCardDefinition> MakeAttackCard(
		const FFinalCardId& CardId,
		const FFinalCharacterId& CharacterId)
	{
		TStrongObjectPtr<UFinalCardDefinition> CardDefinition(NewObject<UFinalCardDefinition>(GetTransientPackage()));
		CardDefinition->CardId = CardId;
		CardDefinition->OwnerUnitId = CharacterId.Value;
		CardDefinition->DisplayName = FText::FromString(TEXT("测试斩击"));
		CardDefinition->BaseCostAP = 1;
		CardDefinition->CardType = EFinalCardType::Attack;

		UFinalBattleEffectDamage* DamageEffect = NewObject<UFinalBattleEffectDamage>(CardDefinition.Get());
		DamageEffect->EffectId = TEXT("effect.test.card.attack");
		DamageEffect->UnitTargetRule = EFinalBattleUnitTargetRule::SelectedEnemy;
		DamageEffect->Scalar.ScaleMode = EFinalBattleScalarMode::Flat;
		DamageEffect->Scalar.BaseValue = 3.0f;
		CardDefinition->Effects.Add(DamageEffect);
		return CardDefinition;
	}

	TStrongObjectPtr<UFinalBattleSession> CreateSession(
		UFinalBattleEncounterDefinition* EncounterDefinition,
		UFinalBattleRuleConfig* RuleConfig,
		UFinalCharacterDefinition* CharacterDefinition,
		const TArray<UFinalCardDefinition*>& CardDefinitions,
		const TArray<UFinalPassiveDefinition*>& InitialPassives = {})
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
		for (UFinalPassiveDefinition* PassiveDefinition : InitialPassives)
		{
			if (PassiveDefinition == nullptr)
			{
				continue;
			}

			FFinalBattleInitialPassiveGrantData& PassiveGrant = CharacterInit.InitialPassiveGrants.AddDefaulted_GetRef();
			PassiveGrant.PassiveId = PassiveDefinition->PassiveId;
			PassiveGrant.PassiveDefinition = PassiveDefinition;
			PassiveGrant.InitialStacks = 1;
			PassiveGrant.DurationOverride = 0;
		}

		for (int32 CardIndex = 0; CardIndex < CardDefinitions.Num(); ++CardIndex)
		{
			UFinalCardDefinition* CardDefinition = CardDefinitions[CardIndex];
			if (CardDefinition == nullptr)
			{
				continue;
			}

			FFinalBattleCardInitData& CardInit = InitContext.DeckCards.AddDefaulted_GetRef();
			CardInit.CardDefinition = CardDefinition;
			CardInit.CardId = CardDefinition->CardId;
			CardInit.OwnerCharacterId = CharacterDefinition->CharacterId;
			CardInit.SourceRunCardInstanceId = FName(*FString::Printf(TEXT("run.card.passive.test.%d"), CardIndex));
			InitContext.DeckDefinitions.Add(CardDefinition);
		}

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
	TStrongObjectPtr<UFinalBattleSession> Session = CreateSession(EncounterDefinition.Get(), RuleConfig.Get(), CharacterDefinition.Get(), { AbilityCard.Get() });

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
	TStrongObjectPtr<UFinalBattleSession> Session = CreateSession(EncounterDefinition.Get(), RuleConfig.Get(), CharacterDefinition.Get(), { AbilityCard.Get() });

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattlePassiveInitialGrantCreatesRuntimeInstanceTest,
	"Final.Battle.Passive.InitialPassiveGrantCreatesRuntimeInstance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattlePassiveInitialGrantCreatesRuntimeInstanceTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattlePassiveTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.passive.initial_grant")));
	const FFinalCardId AttackCardId(FName(TEXT("card.test.passive.initial_grant.attack")));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig();
	TStrongObjectPtr<UFinalEnemyIntentDefinition> EnemyIntent = MakeEnemyAttackIntent();
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition(EnemyIntent.Get());
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition(CharacterId);
	TStrongObjectPtr<UFinalStatusDefinition> DaoShiStatus = MakeDaoShiStatus();
	TStrongObjectPtr<UFinalPassiveDefinition> PassiveDefinition = MakeTookDamagePassive(DaoShiStatus.Get());
	TStrongObjectPtr<UFinalCardDefinition> AttackCard = MakeAttackCard(AttackCardId, CharacterId);
	TStrongObjectPtr<UFinalBattleSession> Session = CreateSession(
		EncounterDefinition.Get(),
		RuleConfig.Get(),
		CharacterDefinition.Get(),
		{ AttackCard.Get() },
		{ PassiveDefinition.Get() });

	const FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
	TestEqual(TEXT("Initial passive grants should create one passive instance at battle start."), Snapshot.Passives.Num(), 1);
	if (Snapshot.Passives.Num() != 1)
	{
		return false;
	}

	TestEqual(TEXT("Innate passive id should match granted passive."), Snapshot.Passives[0].PassiveId, PassiveDefinition->PassiveId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattlePassiveFirstAttackGainDaoShiOncePerTurnTest,
	"Final.Battle.Passive.PlayerCardResolvedAttackAppliesDaoShiOncePerTurn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattlePassiveFirstAttackGainDaoShiOncePerTurnTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattlePassiveTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.passive.first_attack")));
	const FFinalCardId AbilityCardId(FName(TEXT("card.test.passive.first_attack.apply")));
	const FFinalCardId AttackCardOneId(FName(TEXT("card.test.passive.first_attack.attack_one")));
	const FFinalCardId AttackCardTwoId(FName(TEXT("card.test.passive.first_attack.attack_two")));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig(3);
	TStrongObjectPtr<UFinalEnemyIntentDefinition> EnemyIntent = MakeEnemyAttackIntent();
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition(EnemyIntent.Get());
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition(CharacterId);
	TStrongObjectPtr<UFinalStatusDefinition> DaoShiStatus = MakeDaoShiStatus();
	TStrongObjectPtr<UFinalPassiveDefinition> PassiveDefinition = MakeFirstAttackPassive(DaoShiStatus.Get());
	TStrongObjectPtr<UFinalCardDefinition> AbilityCard = MakeApplyPassiveCard(AbilityCardId, CharacterId, PassiveDefinition.Get());
	TStrongObjectPtr<UFinalCardDefinition> AttackCardOne = MakeAttackCard(AttackCardOneId, CharacterId);
	TStrongObjectPtr<UFinalCardDefinition> AttackCardTwo = MakeAttackCard(AttackCardTwoId, CharacterId);
	TStrongObjectPtr<UFinalBattleSession> Session = CreateSession(
		EncounterDefinition.Get(),
		RuleConfig.Get(),
		CharacterDefinition.Get(),
		{ AbilityCard.Get(), AttackCardOne.Get(), AttackCardTwo.Get() });

	FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* HandAbilityCard = FindHandCardById(Snapshot, AbilityCardId);
	if (!TestNotNull(TEXT("Ability card should begin in hand."), HandAbilityCard))
	{
		return false;
	}

	FFinalBattleCommand PlayAbilityCommand;
	PlayAbilityCommand.CommandType = EFinalBattleCommandType::PlayCard;
	PlayAbilityCommand.CardInstanceId = HandAbilityCard->CardInstanceId;
	TestNotEqual(TEXT("ApplyPassive ability should resolve successfully."), Session->SubmitCommand(PlayAbilityCommand).EventType, EFinalBattleEventType::CommandRejected);

	Snapshot = Session->GetSnapshot();
	const FFinalBattlePassiveViewData* AppliedPassiveView = Snapshot.Passives.FindByPredicate([&PassiveDefinition](const FFinalBattlePassiveViewData& PassiveView)
	{
		return PassiveView.PassiveId == PassiveDefinition->PassiveId;
	});
	if (!TestNotNull(TEXT("Ability card should create the first-attack passive."), AppliedPassiveView))
	{
		return false;
	}

	const FFinalBattleCardViewData* FirstAttackCard = FindHandCardById(Snapshot, AttackCardOneId);
	if (!TestNotNull(TEXT("First attack card should still be in hand after ability resolves."), FirstAttackCard))
	{
		return false;
	}

	FFinalBattleCommand FirstAttackCommand;
	FirstAttackCommand.CommandType = EFinalBattleCommandType::PlayCard;
	FirstAttackCommand.CardInstanceId = FirstAttackCard->CardInstanceId;
	TestNotEqual(TEXT("First attack card should resolve successfully."), Session->SubmitCommand(FirstAttackCommand).EventType, EFinalBattleEventType::CommandRejected);

	Snapshot = Session->GetSnapshot();
	const FFinalBattleStatusViewData* DaoShiAfterFirstAttack = Snapshot.Statuses.FindByPredicate([&](const FFinalBattleStatusViewData& StatusView)
	{
		return StatusView.StatusId == DaoShiStatus->StatusId
			&& StatusView.OwnerUnitId == Snapshot.Characters[0].RuntimeUnitId;
	});
	if (!TestNotNull(TEXT("First attack passive should grant DaoShi after first attack."), DaoShiAfterFirstAttack))
	{
		return false;
	}
	TestEqual(TEXT("First attack passive should add one DaoShi stack."), DaoShiAfterFirstAttack->CurrentStacks, 1);

	const FFinalBattleCardViewData* SecondAttackCard = FindHandCardById(Snapshot, AttackCardTwoId);
	if (!TestNotNull(TEXT("Second attack card should still be in hand after first attack resolves."), SecondAttackCard))
	{
		return false;
	}

	FFinalBattleCommand SecondAttackCommand;
	SecondAttackCommand.CommandType = EFinalBattleCommandType::PlayCard;
	SecondAttackCommand.CardInstanceId = SecondAttackCard->CardInstanceId;
	TestNotEqual(TEXT("Second attack card should resolve successfully."), Session->SubmitCommand(SecondAttackCommand).EventType, EFinalBattleEventType::CommandRejected);

	Snapshot = Session->GetSnapshot();
	const FFinalBattleStatusViewData* DaoShiAfterSecondAttack = Snapshot.Statuses.FindByPredicate([&](const FFinalBattleStatusViewData& StatusView)
	{
		return StatusView.StatusId == DaoShiStatus->StatusId
			&& StatusView.OwnerUnitId == Snapshot.Characters[0].RuntimeUnitId;
	});
	if (!TestNotNull(TEXT("DaoShi status should still exist after second attack."), DaoShiAfterSecondAttack))
	{
		return false;
	}
	TestEqual(TEXT("First-attack passive should not trigger on the second attack in the same turn."), DaoShiAfterSecondAttack->CurrentStacks, 1);
	return true;
}

#endif
