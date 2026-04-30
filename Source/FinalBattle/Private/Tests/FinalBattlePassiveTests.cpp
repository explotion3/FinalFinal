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
#include "Battle/Effects/FinalBattleEffectDrawCards.h"
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

	TStrongObjectPtr<UFinalPassiveDefinition> MakeFirstAttackPassive()
	{
		TStrongObjectPtr<UFinalPassiveDefinition> PassiveDefinition(NewObject<UFinalPassiveDefinition>(GetTransientPackage()));
		PassiveDefinition->PassiveId = FFinalPassiveId(FName(TEXT("passive.test.first_attack_gain_daoshi")));
		PassiveDefinition->DisplayId = TEXT("Passive.Test.FirstAttackGainDaoShi");
		PassiveDefinition->DisplayName = FText::FromString(TEXT("压势追刀"));
		PassiveDefinition->SummaryText = FText::FromString(TEXT("每回合第一次打出攻击牌后，当前手牌中的攻击牌本回合费用 -1 AP，且伤害提高 20%。"));
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

		FFinalTriggeredCardModifierDefinition& TriggeredModifier = Trigger.TriggeredCardModifiers.AddDefaulted_GetRef();
		TriggeredModifier.TargetSource = EFinalTriggeredCardModifierTargetSource::CurrentOwnedHandCards;
		TriggeredModifier.bRequireCardType = true;
		TriggeredModifier.RequiredCardType = EFinalCardType::Attack;
		TriggeredModifier.CostDeltaAP = -1;
		TriggeredModifier.OutgoingDamagePercentDelta = 20;
		TriggeredModifier.DurationPolicy = EFinalTriggeredCardModifierDurationPolicy::UntilPlayed;
		TriggeredModifier.bExpireAtPlayerTurnEnd = true;
		return PassiveDefinition;
	}

	TStrongObjectPtr<UFinalPassiveDefinition> MakePlayerTurnDurationPassive()
	{
		TStrongObjectPtr<UFinalPassiveDefinition> PassiveDefinition(NewObject<UFinalPassiveDefinition>(GetTransientPackage()));
		PassiveDefinition->PassiveId = FFinalPassiveId(FName(TEXT("passive.test.player_turn_duration")));
		PassiveDefinition->DisplayId = TEXT("Passive.Test.PlayerTurnDuration");
		PassiveDefinition->DisplayName = FText::FromString(TEXT("瞬息蓄势"));
		PassiveDefinition->SummaryText = FText::FromString(TEXT("持续到玩家回合结束。"));
		PassiveDefinition->StackPolicy = EFinalPassiveStackPolicy::RefreshExisting;
		PassiveDefinition->DurationType = EFinalPassiveDurationType::PlayerTurns;
		PassiveDefinition->MaxStacks = 1;
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

	TStrongObjectPtr<UFinalCardDefinition> MakeDrawCardsSkill(
		const FFinalCardId& CardId,
		const FFinalCharacterId& CharacterId,
		const FString& DisplayName,
		const int32 BaseCostAP,
		const int32 DrawCount)
	{
		TStrongObjectPtr<UFinalCardDefinition> CardDefinition(NewObject<UFinalCardDefinition>(GetTransientPackage()));
		CardDefinition->CardId = CardId;
		CardDefinition->OwnerUnitId = CharacterId.Value;
		CardDefinition->DisplayName = FText::FromString(DisplayName);
		CardDefinition->BaseCostAP = BaseCostAP;
		CardDefinition->CardType = EFinalCardType::Skill;

		UFinalBattleEffectDrawCards* DrawCardsEffect = NewObject<UFinalBattleEffectDrawCards>(CardDefinition.Get());
		DrawCardsEffect->EffectId = FName(*FString::Printf(TEXT("effect.%s.draw"), *CardId.Value.ToString()));
		DrawCardsEffect->DrawCount = DrawCount;
		CardDefinition->Effects.Add(DrawCardsEffect);
		return CardDefinition;
	}

	TStrongObjectPtr<UFinalCardDefinition> MakeApplyStatusSkill(
		const FFinalCardId& CardId,
		const FFinalCharacterId& CharacterId,
		const FString& DisplayName,
		UFinalStatusDefinition* StatusDefinition,
		const int32 Stacks)
	{
		TStrongObjectPtr<UFinalCardDefinition> CardDefinition(NewObject<UFinalCardDefinition>(GetTransientPackage()));
		CardDefinition->CardId = CardId;
		CardDefinition->OwnerUnitId = CharacterId.Value;
		CardDefinition->DisplayName = FText::FromString(DisplayName);
		CardDefinition->BaseCostAP = 1;
		CardDefinition->CardType = EFinalCardType::Skill;

		UFinalBattleEffectApplyStatus* ApplyStatusEffect = NewObject<UFinalBattleEffectApplyStatus>(CardDefinition.Get());
		ApplyStatusEffect->EffectId = FName(*FString::Printf(TEXT("effect.%s.apply_status"), *CardId.Value.ToString()));
		ApplyStatusEffect->UnitTargetRule = EFinalBattleUnitTargetRule::Self;
		ApplyStatusEffect->StatusDefinition = StatusDefinition;
		ApplyStatusEffect->StatusId = StatusDefinition != nullptr ? StatusDefinition->StatusId : FFinalStatusId();
		ApplyStatusEffect->Stacks = Stacks;
		CardDefinition->Effects.Add(ApplyStatusEffect);
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

	const FFinalBattleEvent* FindFirstBattleEventByType(
		const TArray<FFinalBattleEvent>& Events,
		const EFinalBattleEventType EventType)
	{
		return Events.FindByPredicate([EventType](const FFinalBattleEvent& Event)
		{
			return Event.EventType == EventType;
		});
	}

	const FFinalBattleEvent* FindFirstBattleEventByTypeAndReason(
		const TArray<FFinalBattleEvent>& Events,
		const EFinalBattleEventType EventType,
		const FName ReasonTag)
	{
		return Events.FindByPredicate([EventType, ReasonTag](const FFinalBattleEvent& Event)
		{
			return Event.EventType == EventType && Event.ReasonTag == ReasonTag;
		});
	}

	const FFinalBattleEvent* FindFirstBattleEventByTypeAndRelatedTag(
		const TArray<FFinalBattleEvent>& Events,
		const EFinalBattleEventType EventType,
		const FName RelatedTag)
	{
		return Events.FindByPredicate([EventType, RelatedTag](const FFinalBattleEvent& Event)
		{
			return Event.EventType == EventType && Event.RelatedTag == RelatedTag;
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
	const TArray<FFinalBattleEvent> BattleLogEntries = Session->GetBattleLogEntries();
	const FFinalBattleEvent* PassiveAppliedEvent = FindFirstBattleEventByTypeAndReason(
		BattleLogEntries,
		EFinalBattleEventType::PassiveApplied,
		FName(TEXT("passive.applied.effect")));
	if (!TestNotNull(TEXT("ApplyPassive effect should emit PassiveApplied(effect)."), PassiveAppliedEvent))
	{
		return false;
	}

	TestEqual(TEXT("ApplyPassive event should carry the applied passive id."), PassiveAppliedEvent->PassiveId, PassiveDefinition->PassiveId);
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
	const TArray<FFinalBattleEvent> BattleLogEntries = Session->GetBattleLogEntries();
	const FFinalBattleEvent* PassiveTriggeredEvent = FindFirstBattleEventByTypeAndRelatedTag(
		BattleLogEntries,
		EFinalBattleEventType::PassiveTriggered,
		FName(TEXT("battle.trigger.owner_took_health_damage")));
	if (!TestNotNull(TEXT("Owner-took-damage passive should emit PassiveTriggered."), PassiveTriggeredEvent))
	{
		return false;
	}

	TestEqual(TEXT("Triggered passive event should carry the correct passive id."), PassiveTriggeredEvent->PassiveId, PassiveDefinition->PassiveId);
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
	const TArray<FFinalBattleEvent> BattleLogEntries = Session->GetBattleLogEntries();
	const FFinalBattleEvent* PassiveAppliedEvent = FindFirstBattleEventByTypeAndReason(
		BattleLogEntries,
		EFinalBattleEventType::PassiveApplied,
		FName(TEXT("passive.applied.initial_grant")));
	if (!TestNotNull(TEXT("Initial passive grants should emit PassiveApplied(initial_grant)."), PassiveAppliedEvent))
	{
		return false;
	}

	TestEqual(TEXT("Initial passive grant event should carry the granted passive id."), PassiveAppliedEvent->PassiveId, PassiveDefinition->PassiveId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattlePassiveFirstAttackProjectsHandAttackModifiersTest,
	"Final.Battle.Passive.PlayerCardResolvedAttackProjectsHandAttackModifiers",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattlePassiveFirstAttackProjectsHandAttackModifiersTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattlePassiveTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.passive.project_attack")));
	const FFinalCardId AbilityCardId(FName(TEXT("card.test.passive.project_attack.apply")));
	const FFinalCardId TriggerAttackCardId(FName(TEXT("card.test.passive.project_attack.trigger")));
	const FFinalCardId BuffedAttackCardId(FName(TEXT("card.test.passive.project_attack.buff_target")));
	const FFinalCardId SkillCardId(FName(TEXT("card.test.passive.project_attack.skill_target")));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig(4);
	TStrongObjectPtr<UFinalEnemyIntentDefinition> EnemyIntent = MakeEnemyAttackIntent();
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition(EnemyIntent.Get());
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition(CharacterId);
	TStrongObjectPtr<UFinalStatusDefinition> DaoShiStatus = MakeDaoShiStatus();
	TStrongObjectPtr<UFinalPassiveDefinition> PassiveDefinition = MakeFirstAttackPassive();
	TStrongObjectPtr<UFinalCardDefinition> AbilityCard = MakeApplyPassiveCard(AbilityCardId, CharacterId, PassiveDefinition.Get());
	TStrongObjectPtr<UFinalCardDefinition> TriggerAttackCard = MakeAttackCard(TriggerAttackCardId, CharacterId);
	TStrongObjectPtr<UFinalCardDefinition> BuffedAttackCard = MakeAttackCard(BuffedAttackCardId, CharacterId);
	TStrongObjectPtr<UFinalCardDefinition> SkillCard = MakeApplyStatusSkill(SkillCardId, CharacterId, TEXT("测试调息"), DaoShiStatus.Get(), 1);
	AbilityCard->Keywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Opening")));
	TriggerAttackCard->Keywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Opening")));
	BuffedAttackCard->Keywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Opening")));
	SkillCard->Keywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Opening")));
	TStrongObjectPtr<UFinalBattleSession> Session = CreateSession(
		EncounterDefinition.Get(),
		RuleConfig.Get(),
		CharacterDefinition.Get(),
		{ AbilityCard.Get(), TriggerAttackCard.Get(), BuffedAttackCard.Get(), SkillCard.Get() });

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

	const FFinalBattleCardViewData* FirstAttackCard = FindHandCardById(Snapshot, TriggerAttackCardId);
	if (!TestNotNull(TEXT("Trigger attack card should still be in hand after ability resolves."), FirstAttackCard))
	{
		return false;
	}

	FFinalBattleCommand FirstAttackCommand;
	FirstAttackCommand.CommandType = EFinalBattleCommandType::PlayCard;
	FirstAttackCommand.CardInstanceId = FirstAttackCard->CardInstanceId;
	TestNotEqual(TEXT("First attack card should resolve successfully."), Session->SubmitCommand(FirstAttackCommand).EventType, EFinalBattleEventType::CommandRejected);

	Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* BuffedAttackHandCard = FindHandCardById(Snapshot, BuffedAttackCardId);
	if (!TestNotNull(TEXT("Remaining hand attack should still be in hand after trigger attack resolves."), BuffedAttackHandCard))
	{
		return false;
	}

	const FFinalBattleCardProjectionView BuffedAttackProjection = Session->GetCardProjectionView(BuffedAttackHandCard->CardInstanceId);
	TestEqual(TEXT("Triggered passive should reduce remaining hand attack AP cost by 1."), BuffedAttackProjection.EffectiveCostAP, BuffedAttackCard->BaseCostAP - 1);
	TestEqual(TEXT("Triggered passive should add +20% outgoing damage to remaining hand attack."), BuffedAttackProjection.EffectiveOutgoingDamagePercent, 20);
	const TArray<FFinalBattleEvent> TriggerEvents = Session->GetBattleLogEntries();
	const FFinalBattleEvent* PassiveTriggeredEvent = FindFirstBattleEventByTypeAndRelatedTag(
		TriggerEvents,
		EFinalBattleEventType::PassiveTriggered,
		FName(TEXT("battle.trigger.player_card_resolved")));
	if (!TestNotNull(TEXT("First-attack passive should emit PassiveTriggered when it projects hand modifiers."), PassiveTriggeredEvent))
	{
		return false;
	}

	TestEqual(TEXT("First-attack passive event should carry the correct passive id."), PassiveTriggeredEvent->PassiveId, PassiveDefinition->PassiveId);

	const FFinalBattleCardViewData* RemainingSkillCard = FindHandCardById(Snapshot, SkillCardId);
	if (!TestNotNull(TEXT("Non-attack skill should remain in hand after trigger attack resolves."), RemainingSkillCard))
	{
		return false;
	}

	const FFinalBattleCardProjectionView RemainingSkillProjection = Session->GetCardProjectionView(RemainingSkillCard->CardInstanceId);
	TestEqual(TEXT("Non-attack skills should keep their base AP cost."), RemainingSkillProjection.EffectiveCostAP, SkillCard->BaseCostAP);
	TestEqual(TEXT("Non-attack skills should not gain outgoing damage from the passive."), RemainingSkillProjection.EffectiveOutgoingDamagePercent, 0);

	FFinalBattleCommand BuffedAttackCommand;
	BuffedAttackCommand.CommandType = EFinalBattleCommandType::PlayCard;
	BuffedAttackCommand.CardInstanceId = BuffedAttackHandCard->CardInstanceId;
	BuffedAttackCommand.TargetUnitId = Snapshot.Enemies[0].RuntimeUnitId;
	TestNotEqual(TEXT("Buffed hand attack should resolve successfully."), Session->SubmitCommand(BuffedAttackCommand).EventType, EFinalBattleEventType::CommandRejected);

	const FFinalBattleCardProjectionView ProjectionAfterPlay = Session->GetCardProjectionView(BuffedAttackHandCard->CardInstanceId);
	TestEqual(TEXT("Triggered passive modifier should clear after the buffed attack is played."), ProjectionAfterPlay.EffectiveOutgoingDamagePercent, 0);
	TestEqual(TEXT("Triggered passive cost reduction should clear after the buffed attack is played."), ProjectionAfterPlay.EffectiveCostAP, BuffedAttackCard->BaseCostAP);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattlePassiveFirstAttackDoesNotReapplySecondAttackSameTurnTest,
	"Final.Battle.Passive.PlayerCardResolvedAttackDoesNotReapplySecondAttackSameTurn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattlePassiveFirstAttackDoesNotReapplySecondAttackSameTurnTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattlePassiveTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.passive.second_attack_no_reapply")));
	const FFinalCardId AbilityCardId(FName(TEXT("card.test.passive.second_attack.apply")));
	const FFinalCardId FirstAttackCardId(FName(TEXT("card.test.passive.second_attack.trigger")));
	const FFinalCardId DrawSkillCardId(FName(TEXT("card.test.passive.second_attack.draw_skill")));
	const FFinalCardId DrawnAttackOneId(FName(TEXT("card.test.passive.second_attack.drawn_one")));
	const FFinalCardId DrawnAttackTwoId(FName(TEXT("card.test.passive.second_attack.drawn_two")));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig(3);
	TStrongObjectPtr<UFinalEnemyIntentDefinition> EnemyIntent = MakeEnemyAttackIntent();
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition(EnemyIntent.Get());
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition(CharacterId);
	TStrongObjectPtr<UFinalStatusDefinition> DaoShiStatus = MakeDaoShiStatus();
	TStrongObjectPtr<UFinalPassiveDefinition> PassiveDefinition = MakeFirstAttackPassive();
	TStrongObjectPtr<UFinalCardDefinition> AbilityCard = MakeApplyPassiveCard(AbilityCardId, CharacterId, PassiveDefinition.Get());
	TStrongObjectPtr<UFinalCardDefinition> FirstAttackCard = MakeAttackCard(FirstAttackCardId, CharacterId);
	TStrongObjectPtr<UFinalCardDefinition> DrawSkillCard = MakeDrawCardsSkill(DrawSkillCardId, CharacterId, TEXT("测试过牌"), 0, 2);
	TStrongObjectPtr<UFinalCardDefinition> DrawnAttackOne = MakeAttackCard(DrawnAttackOneId, CharacterId);
	TStrongObjectPtr<UFinalCardDefinition> DrawnAttackTwo = MakeAttackCard(DrawnAttackTwoId, CharacterId);
	AbilityCard->Keywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Opening")));
	FirstAttackCard->Keywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Opening")));
	DrawSkillCard->Keywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Opening")));
	TStrongObjectPtr<UFinalBattleSession> Session = CreateSession(
		EncounterDefinition.Get(),
		RuleConfig.Get(),
		CharacterDefinition.Get(),
		{ AbilityCard.Get(), FirstAttackCard.Get(), DrawSkillCard.Get(), DrawnAttackOne.Get(), DrawnAttackTwo.Get() });

	FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* AbilityHandCard = FindHandCardById(Snapshot, AbilityCardId);
	const FFinalBattleCardViewData* TriggerAttackHandCard = FindHandCardById(Snapshot, FirstAttackCardId);
	const FFinalBattleCardViewData* DrawSkillHandCard = FindHandCardById(Snapshot, DrawSkillCardId);
	if (!TestNotNull(TEXT("Ability card should begin in hand."), AbilityHandCard)
		|| !TestNotNull(TEXT("First attack card should begin in hand."), TriggerAttackHandCard)
		|| !TestNotNull(TEXT("Draw skill should begin in hand."), DrawSkillHandCard))
	{
		return false;
	}

	FFinalBattleCommand Command;
	Command.CommandType = EFinalBattleCommandType::PlayCard;
	Command.CardInstanceId = AbilityHandCard->CardInstanceId;
	TestNotEqual(TEXT("ApplyPassive ability should resolve successfully."), Session->SubmitCommand(Command).EventType, EFinalBattleEventType::CommandRejected);

	Snapshot = Session->GetSnapshot();
	TriggerAttackHandCard = FindHandCardById(Snapshot, FirstAttackCardId);
	DrawSkillHandCard = FindHandCardById(Snapshot, DrawSkillCardId);
	if (!TestNotNull(TEXT("First attack card should still be in hand after ability resolves."), TriggerAttackHandCard)
		|| !TestNotNull(TEXT("Draw skill should still be in hand after ability resolves."), DrawSkillHandCard))
	{
		return false;
	}

	Command.CardInstanceId = TriggerAttackHandCard->CardInstanceId;
	Command.TargetUnitId = Snapshot.Enemies[0].RuntimeUnitId;
	TestNotEqual(TEXT("First attack card should resolve successfully."), Session->SubmitCommand(Command).EventType, EFinalBattleEventType::CommandRejected);

	Snapshot = Session->GetSnapshot();
	DrawSkillHandCard = FindHandCardById(Snapshot, DrawSkillCardId);
	if (!TestNotNull(TEXT("Draw skill should remain in hand after the first attack resolves."), DrawSkillHandCard))
	{
		return false;
	}
	Command.CardInstanceId = DrawSkillHandCard->CardInstanceId;
	Command.TargetUnitId = NAME_None;
	TestNotEqual(TEXT("Draw skill should resolve successfully."), Session->SubmitCommand(Command).EventType, EFinalBattleEventType::CommandRejected);

	Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* DrawnAttackOneHandCard = FindHandCardById(Snapshot, DrawnAttackOneId);
	const FFinalBattleCardViewData* DrawnAttackTwoHandCard = FindHandCardById(Snapshot, DrawnAttackTwoId);
	if (!TestNotNull(TEXT("First drawn attack should now be in hand."), DrawnAttackOneHandCard)
		|| !TestNotNull(TEXT("Second drawn attack should now be in hand."), DrawnAttackTwoHandCard))
	{
		return false;
	}

	TestEqual(TEXT("Attacks drawn after the passive already triggered should not inherit the previous buff."), Session->GetCardProjectionView(DrawnAttackTwoHandCard->CardInstanceId).EffectiveOutgoingDamagePercent, 0);

	Command.CardInstanceId = DrawnAttackOneHandCard->CardInstanceId;
	Command.TargetUnitId = Snapshot.Enemies[0].RuntimeUnitId;
	TestNotEqual(TEXT("Second attack of the turn should resolve successfully."), Session->SubmitCommand(Command).EventType, EFinalBattleEventType::CommandRejected);

	const FFinalBattleCardProjectionView RemainingAttackProjection = Session->GetCardProjectionView(DrawnAttackTwoHandCard->CardInstanceId);
	TestEqual(TEXT("Second attack in the same turn should not apply a new projected outgoing damage bonus."), RemainingAttackProjection.EffectiveOutgoingDamagePercent, 0);
	TestEqual(TEXT("Second attack in the same turn should not reduce the remaining attack AP cost."), RemainingAttackProjection.EffectiveCostAP, DrawnAttackTwo->BaseCostAP);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattlePassiveFirstAttackModifierClearsAtTurnEndTest,
	"Final.Battle.Passive.PlayerCardResolvedAttackModifierClearsAtTurnEnd",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattlePassiveFirstAttackModifierClearsAtTurnEndTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattlePassiveTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.passive.clear_turn_end")));
	const FFinalCardId AbilityCardId(FName(TEXT("card.test.passive.clear_turn_end.apply")));
	const FFinalCardId TriggerAttackCardId(FName(TEXT("card.test.passive.clear_turn_end.trigger")));
	const FFinalCardId RetainedAttackCardId(FName(TEXT("card.test.passive.clear_turn_end.retained")));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig(3);
	TStrongObjectPtr<UFinalEnemyIntentDefinition> EnemyIntent = MakeEnemyAttackIntent();
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition(EnemyIntent.Get());
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition(CharacterId);
	TStrongObjectPtr<UFinalStatusDefinition> DaoShiStatus = MakeDaoShiStatus();
	TStrongObjectPtr<UFinalPassiveDefinition> PassiveDefinition = MakeFirstAttackPassive();
	TStrongObjectPtr<UFinalCardDefinition> AbilityCard = MakeApplyPassiveCard(AbilityCardId, CharacterId, PassiveDefinition.Get());
	TStrongObjectPtr<UFinalCardDefinition> TriggerAttackCard = MakeAttackCard(TriggerAttackCardId, CharacterId);
	TStrongObjectPtr<UFinalCardDefinition> RetainedAttackCard = MakeAttackCard(RetainedAttackCardId, CharacterId);
	AbilityCard->Keywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Opening")));
	TriggerAttackCard->Keywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Opening")));
	RetainedAttackCard->Keywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Opening")));
	RetainedAttackCard->Keywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Retain")));
	TStrongObjectPtr<UFinalBattleSession> Session = CreateSession(
		EncounterDefinition.Get(),
		RuleConfig.Get(),
		CharacterDefinition.Get(),
		{ AbilityCard.Get(), TriggerAttackCard.Get(), RetainedAttackCard.Get() });

	FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* AbilityHandCard = FindHandCardById(Snapshot, AbilityCardId);
	const FFinalBattleCardViewData* TriggerAttackHandCard = FindHandCardById(Snapshot, TriggerAttackCardId);
	if (!TestNotNull(TEXT("Ability card should begin in hand."), AbilityHandCard)
		|| !TestNotNull(TEXT("Trigger attack card should begin in hand."), TriggerAttackHandCard))
	{
		return false;
	}

	FFinalBattleCommand Command;
	Command.CommandType = EFinalBattleCommandType::PlayCard;
	Command.CardInstanceId = AbilityHandCard->CardInstanceId;
	TestNotEqual(TEXT("ApplyPassive ability should resolve successfully."), Session->SubmitCommand(Command).EventType, EFinalBattleEventType::CommandRejected);

	Snapshot = Session->GetSnapshot();
	TriggerAttackHandCard = FindHandCardById(Snapshot, TriggerAttackCardId);
	if (!TestNotNull(TEXT("Trigger attack card should remain in hand after applying passive."), TriggerAttackHandCard))
	{
		return false;
	}
	Command.CardInstanceId = TriggerAttackHandCard->CardInstanceId;
	Command.TargetUnitId = Snapshot.Enemies[0].RuntimeUnitId;
	TestNotEqual(TEXT("Trigger attack should resolve successfully."), Session->SubmitCommand(Command).EventType, EFinalBattleEventType::CommandRejected);

	Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* RetainedAttackHandCard = FindHandCardById(Snapshot, RetainedAttackCardId);
	if (!TestNotNull(TEXT("Retained attack should remain in hand after trigger attack resolves."), RetainedAttackHandCard))
	{
		return false;
	}

	const FFinalBattleCardProjectionView ProjectionBeforeEndTurn = Session->GetCardProjectionView(RetainedAttackHandCard->CardInstanceId);
	TestEqual(TEXT("Retained attack should be buffed before end-turn cleanup."), ProjectionBeforeEndTurn.EffectiveOutgoingDamagePercent, 20);

	Command.CommandType = EFinalBattleCommandType::EndTurn;
	Command.CardInstanceId = FGuid();
	Command.TargetUnitId = NAME_None;
	TestNotEqual(TEXT("Ending the turn should resolve successfully."), Session->SubmitCommand(Command).EventType, EFinalBattleEventType::CommandRejected);

	Snapshot = Session->GetSnapshot();
	RetainedAttackHandCard = FindHandCardById(Snapshot, RetainedAttackCardId);
	if (!TestNotNull(TEXT("Retained attack should still be in hand after end turn."), RetainedAttackHandCard))
	{
		return false;
	}

	const FFinalBattleCardProjectionView ProjectionAfterEndTurn = Session->GetCardProjectionView(RetainedAttackHandCard->CardInstanceId);
	TestEqual(TEXT("Turn end should clear the passive-projected outgoing damage bonus."), ProjectionAfterEndTurn.EffectiveOutgoingDamagePercent, 0);
	TestEqual(TEXT("Turn end should clear the passive-projected AP reduction."), ProjectionAfterEndTurn.EffectiveCostAP, RetainedAttackCard->BaseCostAP);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattlePassivePlayerTurnDurationExpiresWithEventTest,
	"Final.Battle.Passive.PlayerTurnsPassiveExpiresWithEvent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattlePassivePlayerTurnDurationExpiresWithEventTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattlePassiveTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.passive.player_turn_expire")));
	const FFinalCardId AttackCardId(FName(TEXT("card.test.passive.player_turn_expire.attack")));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig();
	TStrongObjectPtr<UFinalEnemyIntentDefinition> EnemyIntent = MakeEnemyAttackIntent();
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition(EnemyIntent.Get());
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition(CharacterId);
	TStrongObjectPtr<UFinalPassiveDefinition> PassiveDefinition = MakePlayerTurnDurationPassive();
	TStrongObjectPtr<UFinalCardDefinition> AttackCard = MakeAttackCard(AttackCardId, CharacterId);
	TStrongObjectPtr<UFinalBattleSession> Session = CreateSession(
		EncounterDefinition.Get(),
		RuleConfig.Get(),
		CharacterDefinition.Get(),
		{ AttackCard.Get() },
		{ PassiveDefinition.Get() });

	FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
	TestEqual(TEXT("Initial player-turn passive grant should create one passive instance."), Snapshot.Passives.Num(), 1);

	FFinalBattleCommand EndTurnCommand;
	EndTurnCommand.CommandType = EFinalBattleCommandType::EndTurn;
	const FFinalBattleEvent EndTurnEvent = Session->SubmitCommand(EndTurnCommand);
	TestNotEqual(TEXT("EndTurn should resolve successfully."), EndTurnEvent.EventType, EFinalBattleEventType::CommandRejected);

	Snapshot = Session->GetSnapshot();
	TestEqual(TEXT("Player-turn passive should be removed after end turn."), Snapshot.Passives.Num(), 0);

	const TArray<FFinalBattleEvent> BattleLogEntries = Session->GetBattleLogEntries();
	const FFinalBattleEvent* PassiveRemovedEvent = FindFirstBattleEventByTypeAndReason(
		BattleLogEntries,
		EFinalBattleEventType::PassiveRemoved,
		FName(TEXT("passive.removed.expired")));
	if (!TestNotNull(TEXT("Expiring player-turn passive should emit PassiveRemoved(expired)."), PassiveRemovedEvent))
	{
		return false;
	}

	TestEqual(TEXT("Expired passive removal event should carry the correct passive id."), PassiveRemovedEvent->PassiveId, PassiveDefinition->PassiveId);
	return true;
}

#endif
