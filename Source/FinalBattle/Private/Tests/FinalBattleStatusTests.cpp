#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalEnemyDefinition.h"
#include "Battle/Definitions/FinalEnemyIntentDefinition.h"
#include "Battle/Definitions/FinalStatusDefinition.h"
#include "Battle/Conditions/FinalBattleConditionResourceConsumed.h"
#include "Battle/Effects/FinalBattleEffectApplyStatus.h"
#include "Battle/Effects/FinalBattleEffectConsumeStatusResource.h"
#include "Battle/Effects/FinalBattleEffectDamage.h"
#include "Battle/Effects/FinalBattleEffectDrawCards.h"
#include "Battle/Effects/FinalBattleEffectGainAP.h"
#include "Battle/Effects/FinalBattleEffectBonusBreak.h"
#include "Commands/FinalBattleCommand.h"
#include "Facade/FinalBattleSession.h"
#include "Facade/FinalBattleSessionTypes.h"
#include "Queries/FinalBattleSnapshot.h"
#include "UObject/StrongObjectPtr.h"

namespace FinalBattleStatusTests
{
	FGameplayTag GetOpeningKeyword()
	{
		return FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Opening"));
	}

	TStrongObjectPtr<UFinalBattleRuleConfig> MakeRuleConfig(const int32 InitialHandSize = 1)
	{
		TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig(NewObject<UFinalBattleRuleConfig>(GetTransientPackage()));
		RuleConfig->RuleConfigId = FFinalRuleConfigId(FName(TEXT("rule.test.status_runtime_modifiers")));
		RuleConfig->InitialAP = 3;
		RuleConfig->InitialHandSize = InitialHandSize;
		RuleConfig->TurnStartDrawCount = 1;
		return RuleConfig;
	}

	TStrongObjectPtr<UFinalEnemyIntentDefinition> MakeEnemyAttackIntent()
	{
		TStrongObjectPtr<UFinalEnemyIntentDefinition> Intent(NewObject<UFinalEnemyIntentDefinition>(GetTransientPackage()));
		Intent->IntentId = TEXT("intent.test.status.attack");
		Intent->DisplayName = FText::FromString(TEXT("Test Attack"));
		Intent->PreviewText = FText::FromString(TEXT("Deal 3 team damage"));
		Intent->IntentType = EFinalIntentType::Attack;
		Intent->Weight = 1;

		UFinalBattleEffectDamage* DamageEffect = NewObject<UFinalBattleEffectDamage>(Intent.Get());
		DamageEffect->EffectId = TEXT("effect.test.status.enemy_attack");
		DamageEffect->UnitTargetRule = EFinalBattleUnitTargetRule::TeamPlayer;
		DamageEffect->Scalar.ScaleMode = EFinalBattleScalarMode::Flat;
		DamageEffect->Scalar.BaseValue = 3.0f;
		Intent->Effects.Add(DamageEffect);
		return Intent;
	}

	TStrongObjectPtr<UFinalEnemyDefinition> MakeEnemyDefinition(UFinalEnemyIntentDefinition* IntentDefinition = nullptr, const int32 MaxHP = 20)
	{
		TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition(NewObject<UFinalEnemyDefinition>(GetTransientPackage()));
		EnemyDefinition->EnemyId = FFinalEnemyId(FName(TEXT("enemy.test.status")));
		EnemyDefinition->DisplayName = FText::FromString(TEXT("Status Dummy"));
		EnemyDefinition->MaxHP = MaxHP;
		EnemyDefinition->MaxBreakValue = 10;
		EnemyDefinition->BaseDamagePower = 0;
		EnemyDefinition->InitialInitiativeValue = IntentDefinition != nullptr ? 1 : 0;
		EnemyDefinition->IntentSelectRule = EFinalIntentSelectRule::Cycle;
		if (IntentDefinition != nullptr)
		{
			EnemyDefinition->IntentPool.Add(TSoftObjectPtr<UFinalEnemyIntentDefinition>(IntentDefinition));
		}
		return EnemyDefinition;
	}

	TStrongObjectPtr<UFinalBattleEncounterDefinition> MakeEncounter(UFinalBattleRuleConfig* RuleConfig, UFinalEnemyDefinition* EnemyDefinition)
	{
		TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition(NewObject<UFinalBattleEncounterDefinition>(GetTransientPackage()));
		EncounterDefinition->EncounterId = FFinalEncounterId(FName(TEXT("encounter.test.status")));
		EncounterDefinition->DisplayName = FText::FromString(TEXT("Status Encounter"));
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
		CharacterDefinition->DisplayName = FText::FromString(TEXT("Status Hero"));
		CharacterDefinition->BaseVitalShare = 20;
		CharacterDefinition->BaseStressCap = 12;
		CharacterDefinition->BaseAttack = 4;
		CharacterDefinition->BaseDefense = 1;
		CharacterDefinition->BaseBreakRate = 1.0f;
		CharacterDefinition->BaseCritChance = 0.0f;
		CharacterDefinition->BaseCritDamage = 1.5f;
		return CharacterDefinition;
	}

	TStrongObjectPtr<UFinalStatusDefinition> MakeRuntimeModifierStatusDefinition(
		const FName StatusName,
		const FString& DisplayName,
		const int32 OutgoingDamagePercentPerStack,
		const int32 IncomingTeamHealthDamageReductionPercentPerStack,
		const bool bConsumeOnSuccessfulOwnerDamage,
		const bool bConsumeOnPreventedTeamHealthDamage,
		const bool bOnlyAffectAttackCards,
		const bool bExpireAtPlayerTurnEnd,
		const int32 MaxStacks = 9,
		const int32 DefaultDuration = 0)
	{
		TStrongObjectPtr<UFinalStatusDefinition> StatusDefinition(NewObject<UFinalStatusDefinition>(GetTransientPackage()));
		StatusDefinition->StatusId = FFinalStatusId(StatusName);
		StatusDefinition->DisplayName = FText::FromString(DisplayName);
		StatusDefinition->StatusCategory = EFinalStatusCategory::Buff;
		StatusDefinition->MaxStacks = MaxStacks;
		StatusDefinition->DefaultDuration = DefaultDuration;
		StatusDefinition->DurationType = EFinalStatusDurationType::PlayerTurns;
		StatusDefinition->ExpireWindow = bExpireAtPlayerTurnEnd ? EFinalStatusExpireWindow::PlayerTurnEnd : EFinalStatusExpireWindow::None;
		FFinalStatusRuntimeModifierDefinition& RuntimeModifier = StatusDefinition->RuntimeModifiers.AddDefaulted_GetRef();
		RuntimeModifier.OutgoingDamagePercentPerStack = OutgoingDamagePercentPerStack;
		RuntimeModifier.IncomingTeamHealthDamageReductionPercentPerStack = IncomingTeamHealthDamageReductionPercentPerStack;
		RuntimeModifier.bConsumeOnSuccessfulOwnerDamage = bConsumeOnSuccessfulOwnerDamage;
		RuntimeModifier.bConsumeOnPreventedTeamHealthDamage = bConsumeOnPreventedTeamHealthDamage;
		RuntimeModifier.bOnlyAffectAttackCards = bOnlyAffectAttackCards;

		StatusDefinition->OutgoingDamagePercentPerStack = 0;
		StatusDefinition->bExpireAtPlayerTurnEnd = false;
		StatusDefinition->bConsumeOnSuccessfulOwnerDamage = false;
		StatusDefinition->bOnlyAffectAttackCards = false;
		StatusDefinition->IncomingTeamHealthDamageReductionPercentPerStack = 0;
		StatusDefinition->bConsumeOnPreventedTeamHealthDamage = false;
		StatusDefinition->bProjectToOwnedHandCards = false;
		StatusDefinition->ProjectedCardTypeFilter = EFinalCardType::Attack;
		StatusDefinition->ProjectedOutgoingDamagePercentPerStack = 0;
		return StatusDefinition;
	}

	TStrongObjectPtr<UFinalStatusDefinition> MakeResourceStatusDefinition(
		const FName StatusName,
		const FString& DisplayName,
		const int32 MaxStacks = 9)
	{
		TStrongObjectPtr<UFinalStatusDefinition> StatusDefinition(NewObject<UFinalStatusDefinition>(GetTransientPackage()));
		StatusDefinition->StatusId = FFinalStatusId(StatusName);
		StatusDefinition->DisplayName = FText::FromString(DisplayName);
		StatusDefinition->StatusCategory = EFinalStatusCategory::Signature;
		StatusDefinition->MaxStacks = MaxStacks;
		StatusDefinition->DefaultDuration = 0;
		StatusDefinition->DurationType = EFinalStatusDurationType::Battle;
		StatusDefinition->ExpireWindow = EFinalStatusExpireWindow::None;
		StatusDefinition->bIsResourceStatus = true;
		StatusDefinition->ResourceBehavior = EFinalStatusResourceBehavior::AccumulateAndConsume;
		StatusDefinition->bAutoAffectBattleRules = false;
		StatusDefinition->bAutoProjectToCards = false;
		StatusDefinition->RuntimeModifiers.Reset();
		StatusDefinition->ProjectedCardModifiers.Reset();
		StatusDefinition->RuntimeTriggers.Reset();
		StatusDefinition->OutgoingDamagePercentPerStack = 0;
		StatusDefinition->bExpireAtPlayerTurnEnd = false;
		StatusDefinition->bConsumeOnSuccessfulOwnerDamage = false;
		StatusDefinition->bOnlyAffectAttackCards = false;
		StatusDefinition->IncomingTeamHealthDamageReductionPercentPerStack = 0;
		StatusDefinition->bConsumeOnPreventedTeamHealthDamage = false;
		StatusDefinition->bProjectToOwnedHandCards = false;
		StatusDefinition->ProjectedCardTypeFilter = EFinalCardType::Attack;
		StatusDefinition->ProjectedOutgoingDamagePercentPerStack = 0;
		return StatusDefinition;
	}

	TStrongObjectPtr<UFinalCardDefinition> MakeDamageCard(
		const FFinalCardId& CardId,
		const FFinalCharacterId& CharacterId,
		const FString& DisplayName,
		const int32 BaseCostAP,
		const float Damage)
	{
		TStrongObjectPtr<UFinalCardDefinition> CardDefinition(NewObject<UFinalCardDefinition>(GetTransientPackage()));
		CardDefinition->CardId = CardId;
		CardDefinition->OwnerUnitId = CharacterId.Value;
		CardDefinition->DisplayName = FText::FromString(DisplayName);
		CardDefinition->BaseCostAP = BaseCostAP;
		CardDefinition->CardType = EFinalCardType::Attack;

		UFinalBattleEffectDamage* DamageEffect = NewObject<UFinalBattleEffectDamage>(CardDefinition.Get());
		DamageEffect->EffectId = FName(*FString::Printf(TEXT("effect.%s.damage"), *CardId.Value.ToString()));
		DamageEffect->UnitTargetRule = EFinalBattleUnitTargetRule::SelectedEnemy;
		DamageEffect->Scalar.ScaleMode = EFinalBattleScalarMode::Flat;
		DamageEffect->Scalar.BaseValue = Damage;
		CardDefinition->Effects.Add(DamageEffect);
		return CardDefinition;
	}

	TStrongObjectPtr<UFinalCardDefinition> MakeApplyStatusCard(
		const FFinalCardId& CardId,
		const FFinalCharacterId& CharacterId,
		const FString& DisplayName,
		UFinalStatusDefinition* StatusDefinition,
		const int32 Stacks,
		const EFinalBattleUnitTargetRule TargetRule)
	{
		TStrongObjectPtr<UFinalCardDefinition> CardDefinition(NewObject<UFinalCardDefinition>(GetTransientPackage()));
		CardDefinition->CardId = CardId;
		CardDefinition->OwnerUnitId = CharacterId.Value;
		CardDefinition->DisplayName = FText::FromString(DisplayName);
		CardDefinition->BaseCostAP = 0;
		CardDefinition->CardType = EFinalCardType::Skill;

		UFinalBattleEffectApplyStatus* ApplyStatusEffect = NewObject<UFinalBattleEffectApplyStatus>(CardDefinition.Get());
		ApplyStatusEffect->EffectId = FName(*FString::Printf(TEXT("effect.%s.apply_status"), *CardId.Value.ToString()));
		ApplyStatusEffect->UnitTargetRule = TargetRule;
		ApplyStatusEffect->StatusDefinition = StatusDefinition;
		ApplyStatusEffect->StatusId = StatusDefinition != nullptr ? StatusDefinition->StatusId : FFinalStatusId();
		ApplyStatusEffect->Stacks = Stacks;
		CardDefinition->Effects.Add(ApplyStatusEffect);
		return CardDefinition;
	}

	void AddOpeningKeyword(UFinalCardDefinition* CardDefinition)
	{
		if (CardDefinition == nullptr)
		{
			return;
		}

		CardDefinition->Keywords.AddTag(GetOpeningKeyword());
	}

	TStrongObjectPtr<UFinalCardDefinition> MakeTeamDamageCard(
		const FFinalCardId& CardId,
		const FFinalCharacterId& CharacterId,
		const FString& DisplayName,
		const int32 BaseCostAP,
		const float Damage)
	{
		TStrongObjectPtr<UFinalCardDefinition> CardDefinition(NewObject<UFinalCardDefinition>(GetTransientPackage()));
		CardDefinition->CardId = CardId;
		CardDefinition->OwnerUnitId = CharacterId.Value;
		CardDefinition->DisplayName = FText::FromString(DisplayName);
		CardDefinition->BaseCostAP = BaseCostAP;
		CardDefinition->CardType = EFinalCardType::Skill;

		UFinalBattleEffectDamage* DamageEffect = NewObject<UFinalBattleEffectDamage>(CardDefinition.Get());
		DamageEffect->EffectId = FName(*FString::Printf(TEXT("effect.%s.team_damage"), *CardId.Value.ToString()));
		DamageEffect->UnitTargetRule = EFinalBattleUnitTargetRule::TeamPlayer;
		DamageEffect->Scalar.ScaleMode = EFinalBattleScalarMode::Flat;
		DamageEffect->Scalar.BaseValue = Damage;
		CardDefinition->Effects.Add(DamageEffect);
		return CardDefinition;
	}

	TStrongObjectPtr<UFinalCardDefinition> MakeDaoShiConsumerCard(
		const FFinalCardId& CardId,
		const FFinalCharacterId& CharacterId,
		UFinalStatusDefinition* StatusDefinition)
	{
		TStrongObjectPtr<UFinalCardDefinition> CardDefinition(NewObject<UFinalCardDefinition>(GetTransientPackage()));
		CardDefinition->CardId = CardId;
		CardDefinition->OwnerUnitId = CharacterId.Value;
		CardDefinition->DisplayName = FText::FromString(TEXT("断岳斩"));
		CardDefinition->BaseCostAP = 1;
		CardDefinition->CardType = EFinalCardType::Attack;

		UFinalBattleEffectBonusBreak* BaseBreak = NewObject<UFinalBattleEffectBonusBreak>(CardDefinition.Get());
		BaseBreak->EffectId = FName(*FString::Printf(TEXT("effect.%s.base_break"), *CardId.Value.ToString()));
		BaseBreak->UnitTargetRule = EFinalBattleUnitTargetRule::SelectedEnemy;
		BaseBreak->Scalar.ScaleMode = EFinalBattleScalarMode::Flat;
		BaseBreak->Scalar.BaseValue = 3.0f;
		CardDefinition->Effects.Add(BaseBreak);

		UFinalBattleEffectConsumeStatusResource* ConsumeEffect = NewObject<UFinalBattleEffectConsumeStatusResource>(CardDefinition.Get());
		ConsumeEffect->EffectId = FName(*FString::Printf(TEXT("effect.%s.consume"), *CardId.Value.ToString()));
		ConsumeEffect->UnitTargetRule = EFinalBattleUnitTargetRule::Self;
		ConsumeEffect->StatusDefinition = StatusDefinition;
		ConsumeEffect->StatusId = StatusDefinition != nullptr ? StatusDefinition->StatusId : FFinalStatusId();
		ConsumeEffect->StacksToConsume = 1;
		CardDefinition->Effects.Add(ConsumeEffect);

		UFinalBattleEffectBonusBreak* BonusBreak = NewObject<UFinalBattleEffectBonusBreak>(CardDefinition.Get());
		BonusBreak->EffectId = FName(*FString::Printf(TEXT("effect.%s.bonus_break"), *CardId.Value.ToString()));
		BonusBreak->UnitTargetRule = EFinalBattleUnitTargetRule::SelectedEnemy;
		BonusBreak->Scalar.ScaleMode = EFinalBattleScalarMode::Flat;
		BonusBreak->Scalar.BaseValue = 2.0f;
		UFinalBattleConditionResourceConsumed* ResourceConsumedCondition = NewObject<UFinalBattleConditionResourceConsumed>(BonusBreak);
		ResourceConsumedCondition->ConditionId = TEXT("condition.resource_consumed");
		ResourceConsumedCondition->Requirement.RequiredStatusId = StatusDefinition != nullptr ? StatusDefinition->StatusId : FFinalStatusId();
		ResourceConsumedCondition->Requirement.MinimumStacks = 1;
		BonusBreak->Conditions.Add(ResourceConsumedCondition);
		CardDefinition->Effects.Add(BonusBreak);
		return CardDefinition;
	}

	TStrongObjectPtr<UFinalCardDefinition> MakeYaoYinConsumerCard(
		const FFinalCardId& CardId,
		const FFinalCharacterId& CharacterId,
		UFinalStatusDefinition* StatusDefinition)
	{
		TStrongObjectPtr<UFinalCardDefinition> CardDefinition(NewObject<UFinalCardDefinition>(GetTransientPackage()));
		CardDefinition->CardId = CardId;
		CardDefinition->OwnerUnitId = CharacterId.Value;
		CardDefinition->DisplayName = FText::FromString(TEXT("化引"));
		CardDefinition->BaseCostAP = 1;
		CardDefinition->CardType = EFinalCardType::Skill;

		UFinalBattleEffectConsumeStatusResource* ConsumeEffect = NewObject<UFinalBattleEffectConsumeStatusResource>(CardDefinition.Get());
		ConsumeEffect->EffectId = FName(*FString::Printf(TEXT("effect.%s.consume"), *CardId.Value.ToString()));
		ConsumeEffect->UnitTargetRule = EFinalBattleUnitTargetRule::Self;
		ConsumeEffect->StatusDefinition = StatusDefinition;
		ConsumeEffect->StatusId = StatusDefinition != nullptr ? StatusDefinition->StatusId : FFinalStatusId();
		ConsumeEffect->StacksToConsume = 1;
		CardDefinition->Effects.Add(ConsumeEffect);

		UFinalBattleEffectDrawCards* DrawEffect = NewObject<UFinalBattleEffectDrawCards>(CardDefinition.Get());
		DrawEffect->EffectId = FName(*FString::Printf(TEXT("effect.%s.draw"), *CardId.Value.ToString()));
		DrawEffect->DrawCount = 1;
		UFinalBattleConditionResourceConsumed* DrawCondition = NewObject<UFinalBattleConditionResourceConsumed>(DrawEffect);
		DrawCondition->ConditionId = TEXT("condition.resource_consumed");
		DrawCondition->Requirement.RequiredStatusId = StatusDefinition != nullptr ? StatusDefinition->StatusId : FFinalStatusId();
		DrawCondition->Requirement.MinimumStacks = 1;
		DrawEffect->Conditions.Add(DrawCondition);
		CardDefinition->Effects.Add(DrawEffect);

		UFinalBattleEffectGainAP* GainApEffect = NewObject<UFinalBattleEffectGainAP>(CardDefinition.Get());
		GainApEffect->EffectId = FName(*FString::Printf(TEXT("effect.%s.gain_ap"), *CardId.Value.ToString()));
		GainApEffect->GainValue = 1;
		UFinalBattleConditionResourceConsumed* ApCondition = NewObject<UFinalBattleConditionResourceConsumed>(GainApEffect);
		ApCondition->ConditionId = TEXT("condition.resource_consumed");
		ApCondition->Requirement.RequiredStatusId = StatusDefinition != nullptr ? StatusDefinition->StatusId : FFinalStatusId();
		ApCondition->Requirement.MinimumStacks = 1;
		GainApEffect->Conditions.Add(ApCondition);
		CardDefinition->Effects.Add(GainApEffect);
		return CardDefinition;
	}

	TStrongObjectPtr<UFinalBattleSession> CreateSession(
		UFinalBattleEncounterDefinition* EncounterDefinition,
		UFinalBattleRuleConfig* RuleConfig,
		UFinalCharacterDefinition* CharacterDefinition,
		const TArray<UFinalCardDefinition*>& CardDefinitions)
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

		for (UFinalCardDefinition* CardDefinition : CardDefinitions)
		{
			if (CardDefinition == nullptr)
			{
				continue;
			}

			FFinalBattleCardInitData& CardInit = InitContext.DeckCards.AddDefaulted_GetRef();
			CardInit.CardDefinition = CardDefinition;
			CardInit.CardId = CardDefinition->CardId;
			CardInit.OwnerCharacterId = CharacterDefinition->CharacterId;
			InitContext.DeckDefinitions.Add(CardDefinition);
		}

		TStrongObjectPtr<UFinalBattleSession> Session(NewObject<UFinalBattleSession>(GetTransientPackage()));
		Session->InitializeSession(EncounterDefinition, RuleConfig, InitContext);
		return Session;
	}

	const FFinalBattleCardViewData* FindHandCardById(const FFinalBattleSnapshot& Snapshot, const FFinalCardId& CardId)
	{
		return Snapshot.HandCards.FindByPredicate(
			[&CardId](const FFinalBattleCardViewData& Candidate)
			{
				return Candidate.CardId == CardId;
			});
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleStatusRuntimeModifiersShiQiAffectsDamageTest,
	"Final.Battle.Status.RuntimeModifiers.ShiQiAffectsDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleStatusRuntimeModifiersShiQiAffectsDamageTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattleStatusTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.status.shiqi_damage")));
	const FFinalCardId ShiQiCardId(FName(TEXT("card.test.status.shiqi")));
	const FFinalCardId AttackCardId(FName(TEXT("card.test.status.attack")));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig(2);
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition(nullptr, 20);
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition(CharacterId);
	TStrongObjectPtr<UFinalStatusDefinition> ShiQiStatus = MakeRuntimeModifierStatusDefinition(
		TEXT("status.test.runtime_modifier.shiqi"),
		TEXT("士气"),
		20,
		0,
		false,
		false,
		false,
		true);
	TStrongObjectPtr<UFinalCardDefinition> ApplyShiQiCard = MakeApplyStatusCard(ShiQiCardId, CharacterId, TEXT("Apply ShiQi"), ShiQiStatus.Get(), 1, EFinalBattleUnitTargetRule::Self);
	TStrongObjectPtr<UFinalCardDefinition> AttackCard = MakeDamageCard(AttackCardId, CharacterId, TEXT("Heavy Slash"), 1, 10.0f);
	TStrongObjectPtr<UFinalBattleSession> Session = CreateSession(EncounterDefinition.Get(), RuleConfig.Get(), CharacterDefinition.Get(), { ApplyShiQiCard.Get(), AttackCard.Get() });

	FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* ApplyCardView = FindHandCardById(Snapshot, ShiQiCardId);
	const FFinalBattleCardViewData* AttackCardView = FindHandCardById(Snapshot, AttackCardId);
	if (!TestNotNull(TEXT("ShiQi apply card should begin in hand."), ApplyCardView)
		|| !TestNotNull(TEXT("Attack card should begin in hand."), AttackCardView))
	{
		return false;
	}

	FFinalBattleCommand Command;
	Command.CommandType = EFinalBattleCommandType::PlayCard;
	Command.CardInstanceId = ApplyCardView->CardInstanceId;
	TestNotEqual(TEXT("Applying ShiQi should resolve successfully."), Session->SubmitCommand(Command).EventType, EFinalBattleEventType::CommandRejected);

	Snapshot = Session->GetSnapshot();
	AttackCardView = FindHandCardById(Snapshot, AttackCardId);
	if (!TestNotNull(TEXT("Attack card should remain in hand after applying ShiQi."), AttackCardView))
	{
		return false;
	}

	Command.CommandType = EFinalBattleCommandType::PlayCard;
	Command.CardInstanceId = AttackCardView->CardInstanceId;
	Command.TargetUnitId = Snapshot.Enemies[0].RuntimeUnitId;
	TestNotEqual(TEXT("Attacking under ShiQi should resolve successfully."), Session->SubmitCommand(Command).EventType, EFinalBattleEventType::CommandRejected);

	const FFinalBattleSnapshot SnapshotAfterAttack = Session->GetSnapshot();
	TestEqual(TEXT("Base 10 damage with +20% ShiQi should resolve to 12 total damage."), SnapshotAfterAttack.Enemies[0].CurrentHP, 8);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleStatusRuntimeModifiersShiQiExpiresAtTurnEndTest,
	"Final.Battle.Status.RuntimeModifiers.ShiQiExpiresAtTurnEnd",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleStatusRuntimeModifiersShiQiExpiresAtTurnEndTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattleStatusTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.status.shiqi_expire")));
	const FFinalCardId ShiQiCardId(FName(TEXT("card.test.status.shiqi_expire")));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig(2);
	TStrongObjectPtr<UFinalEnemyIntentDefinition> EnemyIntent = MakeEnemyAttackIntent();
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition(EnemyIntent.Get(), 20);
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition(CharacterId);
	TStrongObjectPtr<UFinalStatusDefinition> ShiQiStatus = MakeRuntimeModifierStatusDefinition(
		TEXT("status.test.runtime_modifier.shiqi_expire"),
		TEXT("士气"),
		20,
		0,
		false,
		false,
		false,
		true);
	TStrongObjectPtr<UFinalCardDefinition> ApplyShiQiCard = MakeApplyStatusCard(ShiQiCardId, CharacterId, TEXT("Apply ShiQi"), ShiQiStatus.Get(), 1, EFinalBattleUnitTargetRule::Self);
	TStrongObjectPtr<UFinalBattleSession> Session = CreateSession(EncounterDefinition.Get(), RuleConfig.Get(), CharacterDefinition.Get(), { ApplyShiQiCard.Get() });

	FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* ApplyCardView = FindHandCardById(Snapshot, ShiQiCardId);
	if (!TestNotNull(TEXT("ShiQi apply card should begin in hand."), ApplyCardView))
	{
		return false;
	}

	FFinalBattleCommand Command;
	Command.CommandType = EFinalBattleCommandType::PlayCard;
	Command.CardInstanceId = ApplyCardView->CardInstanceId;
	TestNotEqual(TEXT("Applying ShiQi should resolve successfully."), Session->SubmitCommand(Command).EventType, EFinalBattleEventType::CommandRejected);

	FFinalBattleCommand EndTurnCommand;
	EndTurnCommand.CommandType = EFinalBattleCommandType::EndTurn;
	TestNotEqual(TEXT("Ending the turn should resolve successfully."), Session->SubmitCommand(EndTurnCommand).EventType, EFinalBattleEventType::CommandRejected);

	const FFinalBattleSnapshot SnapshotAfterTurnEnd = Session->GetSnapshot();
	const FFinalBattleStatusViewData* ShiQiView = SnapshotAfterTurnEnd.Statuses.FindByPredicate(
		[&](const FFinalBattleStatusViewData& StatusView)
		{
			return StatusView.StatusId == ShiQiStatus->StatusId
				&& StatusView.OwnerUnitId == SnapshotAfterTurnEnd.Characters[0].RuntimeUnitId;
		});
	TestNull(TEXT("ShiQi should expire at player turn end."), ShiQiView);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleStatusRuntimeModifiersImmunityProtectsAndConsumesTest,
	"Final.Battle.Status.RuntimeModifiers.ImmunityProtectsAndConsumes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleStatusRuntimeModifiersImmunityProtectsAndConsumesTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattleStatusTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.status.immunity")));
	const FFinalCardId ImmunityCardId(FName(TEXT("card.test.status.immunity")));
	const FFinalCardId TeamDamageCardId(FName(TEXT("card.test.status.team_damage")));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig(2);
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition(nullptr, 20);
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition(CharacterId);
	TStrongObjectPtr<UFinalStatusDefinition> ImmunityStatus = MakeRuntimeModifierStatusDefinition(
		TEXT("status.test.runtime_modifier.immunity"),
		TEXT("生命免疫"),
		0,
		100,
		false,
		true,
		false,
		true,
		1,
		1);
	TStrongObjectPtr<UFinalCardDefinition> ApplyImmunityCard = MakeApplyStatusCard(ImmunityCardId, CharacterId, TEXT("Apply Immunity"), ImmunityStatus.Get(), 1, EFinalBattleUnitTargetRule::TeamPlayer);
	TStrongObjectPtr<UFinalCardDefinition> TeamDamageCard = MakeTeamDamageCard(TeamDamageCardId, CharacterId, TEXT("Self Damage"), 0, 3.0f);
	TStrongObjectPtr<UFinalBattleSession> Session = CreateSession(EncounterDefinition.Get(), RuleConfig.Get(), CharacterDefinition.Get(), { ApplyImmunityCard.Get(), TeamDamageCard.Get() });

	FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* ApplyCardView = FindHandCardById(Snapshot, ImmunityCardId);
	const FFinalBattleCardViewData* TeamDamageCardView = FindHandCardById(Snapshot, TeamDamageCardId);
	if (!TestNotNull(TEXT("Immunity apply card should begin in hand."), ApplyCardView)
		|| !TestNotNull(TEXT("Team damage card should begin in hand."), TeamDamageCardView))
	{
		return false;
	}

	FFinalBattleCommand Command;
	Command.CommandType = EFinalBattleCommandType::PlayCard;
	Command.CardInstanceId = ApplyCardView->CardInstanceId;
	TestNotEqual(TEXT("Applying immunity should resolve successfully."), Session->SubmitCommand(Command).EventType, EFinalBattleEventType::CommandRejected);

	Snapshot = Session->GetSnapshot();
	TeamDamageCardView = FindHandCardById(Snapshot, TeamDamageCardId);
	if (!TestNotNull(TEXT("Team damage card should remain in hand after applying immunity."), TeamDamageCardView))
	{
		return false;
	}

	Command.CommandType = EFinalBattleCommandType::PlayCard;
	Command.CardInstanceId = TeamDamageCardView->CardInstanceId;
	TestNotEqual(TEXT("Team damage card should resolve successfully."), Session->SubmitCommand(Command).EventType, EFinalBattleEventType::CommandRejected);

	const FFinalBattleSnapshot SnapshotAfterTeamDamage = Session->GetSnapshot();
	TestEqual(TEXT("Immunity should prevent the incoming team HP damage."), SnapshotAfterTeamDamage.TeamCurrentHP, 20);
	const FFinalBattleStatusViewData* ImmunityView = SnapshotAfterTeamDamage.TeamStatuses.FindByPredicate(
		[&](const FFinalBattleStatusViewData& StatusView)
		{
			return StatusView.StatusId == ImmunityStatus->StatusId;
		});
	TestNull(TEXT("Immunity should be consumed after preventing team HP damage."), ImmunityView);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleStatusResourceDaoShiAccumulatesAndConsumesTest,
	"Final.Battle.Status.Resources.DaoShiAccumulatesAndConsumes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleStatusResourceDaoShiAccumulatesAndConsumesTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattleStatusTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.status.resource_daoshi")));
	const FFinalCardId ApplyCardId(FName(TEXT("card.test.status.resource_daoshi_apply")));
	const FFinalCardId ConsumerCardId(FName(TEXT("card.test.status.resource_daoshi_consume")));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig(2);
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition(nullptr, 20);
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition(CharacterId);
	TStrongObjectPtr<UFinalStatusDefinition> DaoShiStatus = MakeResourceStatusDefinition(TEXT("status.test.resource.daoshi"), TEXT("刀势"), 6);
	TStrongObjectPtr<UFinalCardDefinition> ApplyCard = MakeApplyStatusCard(ApplyCardId, CharacterId, TEXT("Gain DaoShi"), DaoShiStatus.Get(), 2, EFinalBattleUnitTargetRule::Self);
	TStrongObjectPtr<UFinalCardDefinition> ConsumerCard = MakeDaoShiConsumerCard(ConsumerCardId, CharacterId, DaoShiStatus.Get());
	TStrongObjectPtr<UFinalBattleSession> Session = CreateSession(EncounterDefinition.Get(), RuleConfig.Get(), CharacterDefinition.Get(), { ApplyCard.Get(), ConsumerCard.Get() });

	FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* ApplyCardView = FindHandCardById(Snapshot, ApplyCardId);
	const FFinalBattleCardViewData* ConsumerCardView = FindHandCardById(Snapshot, ConsumerCardId);
	if (!TestNotNull(TEXT("DaoShi apply card should begin in hand."), ApplyCardView)
		|| !TestNotNull(TEXT("DaoShi consumer card should begin in hand."), ConsumerCardView))
	{
		return false;
	}

	FFinalBattleCommand Command;
	Command.CommandType = EFinalBattleCommandType::PlayCard;
	Command.CardInstanceId = ApplyCardView->CardInstanceId;
	TestNotEqual(TEXT("Applying DaoShi should resolve successfully."), Session->SubmitCommand(Command).EventType, EFinalBattleEventType::CommandRejected);

	Snapshot = Session->GetSnapshot();
	const FFinalBattleStatusViewData* DaoShiView = Snapshot.Statuses.FindByPredicate(
		[&](const FFinalBattleStatusViewData& StatusView)
		{
			return StatusView.StatusId == DaoShiStatus->StatusId
				&& StatusView.OwnerUnitId == Snapshot.Characters[0].RuntimeUnitId;
		});
	if (!TestNotNull(TEXT("DaoShi should exist after ApplyStatus."), DaoShiView))
	{
		return false;
	}
	TestEqual(TEXT("DaoShi should accumulate two stacks from ApplyStatus."), DaoShiView->CurrentStacks, 2);

	ConsumerCardView = FindHandCardById(Snapshot, ConsumerCardId);
	if (!TestNotNull(TEXT("DaoShi consumer card should remain in hand after setup."), ConsumerCardView))
	{
		return false;
	}

	Command.CommandType = EFinalBattleCommandType::PlayCard;
	Command.CardInstanceId = ConsumerCardView->CardInstanceId;
	Command.TargetUnitId = Snapshot.Enemies[0].RuntimeUnitId;
	TestNotEqual(TEXT("Consuming DaoShi should resolve successfully."), Session->SubmitCommand(Command).EventType, EFinalBattleEventType::CommandRejected);

	const FFinalBattleSnapshot SnapshotAfterConsume = Session->GetSnapshot();
	const FFinalBattleStatusViewData* DaoShiAfterConsume = SnapshotAfterConsume.Statuses.FindByPredicate(
		[&](const FFinalBattleStatusViewData& StatusView)
		{
			return StatusView.StatusId == DaoShiStatus->StatusId
				&& StatusView.OwnerUnitId == SnapshotAfterConsume.Characters[0].RuntimeUnitId;
		});
	if (!TestNotNull(TEXT("DaoShi should remain after consuming one of two stacks."), DaoShiAfterConsume))
	{
		return false;
	}
	TestEqual(TEXT("ConsumeStatusResource should remove exactly one DaoShi stack."), DaoShiAfterConsume->CurrentStacks, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleStatusResourceDuanYueZhanRequiresDaoShiForBonusBreakTest,
	"Final.Battle.Status.Resources.DuanYueZhanRequiresDaoShiForBonusBreak",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleStatusResourceDuanYueZhanRequiresDaoShiForBonusBreakTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattleStatusTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.status.resource_duanyue")));
	const FFinalCardId ApplyCardId(FName(TEXT("card.test.status.resource_duanyue_apply")));
	const FFinalCardId ConsumerCardId(FName(TEXT("card.test.status.resource_duanyue_consume")));

	auto RunScenario = [&](const bool bApplyResourceFirst)
	{
		TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig(2);
		TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition(nullptr, 20);
		TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
		TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition(CharacterId);
		TStrongObjectPtr<UFinalStatusDefinition> DaoShiStatus = MakeResourceStatusDefinition(TEXT("status.test.resource.duanyue_daoshi"), TEXT("刀势"), 6);
		TStrongObjectPtr<UFinalCardDefinition> ApplyCard = MakeApplyStatusCard(ApplyCardId, CharacterId, TEXT("Gain DaoShi"), DaoShiStatus.Get(), 1, EFinalBattleUnitTargetRule::Self);
		TStrongObjectPtr<UFinalCardDefinition> ConsumerCard = MakeDaoShiConsumerCard(ConsumerCardId, CharacterId, DaoShiStatus.Get());
		TStrongObjectPtr<UFinalBattleSession> Session = CreateSession(EncounterDefinition.Get(), RuleConfig.Get(), CharacterDefinition.Get(), { ApplyCard.Get(), ConsumerCard.Get() });

		FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
		if (bApplyResourceFirst)
		{
			const FFinalBattleCardViewData* ApplyCardView = FindHandCardById(Snapshot, ApplyCardId);
			if (!TestNotNull(TEXT("DaoShi apply card should begin in hand."), ApplyCardView))
			{
				return -1;
			}

			FFinalBattleCommand Command;
			Command.CommandType = EFinalBattleCommandType::PlayCard;
			Command.CardInstanceId = ApplyCardView->CardInstanceId;
			if (!TestNotEqual(TEXT("Applying DaoShi should resolve successfully."), Session->SubmitCommand(Command).EventType, EFinalBattleEventType::CommandRejected))
			{
				return -1;
			}
			Snapshot = Session->GetSnapshot();
		}

		const FFinalBattleCardViewData* ConsumerCardView = FindHandCardById(Snapshot, ConsumerCardId);
		if (!TestNotNull(TEXT("DaoShi consumer card should be available."), ConsumerCardView))
		{
			return -1;
		}

		FFinalBattleCommand Command;
		Command.CommandType = EFinalBattleCommandType::PlayCard;
		Command.CardInstanceId = ConsumerCardView->CardInstanceId;
		Command.TargetUnitId = Snapshot.Enemies[0].RuntimeUnitId;
		if (!TestNotEqual(TEXT("DaoShi consumer should resolve successfully."), Session->SubmitCommand(Command).EventType, EFinalBattleEventType::CommandRejected))
		{
			return -1;
		}

		return Session->GetSnapshot().Enemies[0].CurrentBreakValue;
	};

	const int32 BreakWithoutDaoShi = RunScenario(false);
	const int32 BreakWithDaoShi = RunScenario(true);
	if (!TestTrue(TEXT("Both DuanYue scenarios should execute."), BreakWithoutDaoShi >= 0 && BreakWithDaoShi >= 0))
	{
		return false;
	}

	TestEqual(TEXT("Base break without DaoShi should remain at 7."), BreakWithoutDaoShi, 7);
	TestEqual(TEXT("Bonus break should only apply when DaoShi was consumed."), BreakWithDaoShi, 5);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleStatusResourceYaoYinConsumerEffectsRequireConsumeTest,
	"Final.Battle.Status.Resources.YaoYinConsumerEffectsRequireConsume",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleStatusResourceYaoYinConsumerEffectsRequireConsumeTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattleStatusTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.status.resource_yaoyin")));
	const FFinalCardId ApplyCardId(FName(TEXT("card.test.status.resource_yaoyin_apply")));
	const FFinalCardId ConsumerCardId(FName(TEXT("card.test.status.resource_yaoyin_consume")));
	const FFinalCardId FillerCardId(FName(TEXT("card.test.status.resource_yaoyin_filler")));

	auto RunScenario = [&](const bool bApplyResourceFirst, int32& OutCurrentAP, int32& OutHandCount, int32& OutRemainingStacks)
	{
		TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig(bApplyResourceFirst ? 2 : 1);
		TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition(nullptr, 20);
		TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
		TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition(CharacterId);
		TStrongObjectPtr<UFinalStatusDefinition> YaoYinStatus = MakeResourceStatusDefinition(TEXT("status.test.resource.yaoyin"), TEXT("药引"), 9);
		TStrongObjectPtr<UFinalCardDefinition> ApplyCard = MakeApplyStatusCard(ApplyCardId, CharacterId, TEXT("Gain YaoYin"), YaoYinStatus.Get(), 1, EFinalBattleUnitTargetRule::Self);
		TStrongObjectPtr<UFinalCardDefinition> ConsumerCard = MakeYaoYinConsumerCard(ConsumerCardId, CharacterId, YaoYinStatus.Get());
		TStrongObjectPtr<UFinalCardDefinition> FillerCard = MakeDamageCard(FillerCardId, CharacterId, TEXT("Reserve"), 1, 4.0f);
		AddOpeningKeyword(ApplyCard.Get());
		AddOpeningKeyword(ConsumerCard.Get());
		TArray<UFinalCardDefinition*> DeckCards;
		if (bApplyResourceFirst)
		{
			DeckCards = { ApplyCard.Get(), ConsumerCard.Get(), FillerCard.Get() };
		}
		else
		{
			DeckCards = { ConsumerCard.Get(), FillerCard.Get() };
		}
		TStrongObjectPtr<UFinalBattleSession> Session = CreateSession(EncounterDefinition.Get(), RuleConfig.Get(), CharacterDefinition.Get(), DeckCards);

		FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
		if (bApplyResourceFirst)
		{
			const FFinalBattleCardViewData* ApplyCardView = FindHandCardById(Snapshot, ApplyCardId);
			if (!TestNotNull(TEXT("YaoYin apply card should begin in hand."), ApplyCardView))
			{
				return false;
			}

			FFinalBattleCommand Command;
			Command.CommandType = EFinalBattleCommandType::PlayCard;
			Command.CardInstanceId = ApplyCardView->CardInstanceId;
			if (!TestNotEqual(TEXT("Applying YaoYin should resolve successfully."), Session->SubmitCommand(Command).EventType, EFinalBattleEventType::CommandRejected))
			{
				return false;
			}
			Snapshot = Session->GetSnapshot();
		}

		const FFinalBattleCardViewData* ConsumerCardView = FindHandCardById(Snapshot, ConsumerCardId);
		if (!TestNotNull(TEXT("YaoYin consumer card should be available."), ConsumerCardView))
		{
			return false;
		}

		FFinalBattleCommand Command;
		Command.CommandType = EFinalBattleCommandType::PlayCard;
		Command.CardInstanceId = ConsumerCardView->CardInstanceId;
		if (!TestNotEqual(TEXT("YaoYin consumer should resolve successfully."), Session->SubmitCommand(Command).EventType, EFinalBattleEventType::CommandRejected))
		{
			return false;
		}

		const FFinalBattleSnapshot FinalSnapshot = Session->GetSnapshot();
		OutCurrentAP = FinalSnapshot.CurrentAP;
		OutHandCount = FinalSnapshot.HandCards.Num();
		if (const FFinalBattleStatusViewData* YaoYinView = FinalSnapshot.Statuses.FindByPredicate(
			[&](const FFinalBattleStatusViewData& StatusView)
			{
				return StatusView.StatusId == YaoYinStatus->StatusId
					&& StatusView.OwnerUnitId == FinalSnapshot.Characters[0].RuntimeUnitId;
			}))
		{
			OutRemainingStacks = YaoYinView->CurrentStacks;
		}
		else
		{
			OutRemainingStacks = 0;
		}

		return true;
	};

	int32 APWithoutResource = 0;
	int32 HandWithoutResource = 0;
	int32 RemainingWithoutResource = 0;
	if (!RunScenario(false, APWithoutResource, HandWithoutResource, RemainingWithoutResource))
	{
		return false;
	}

	int32 APWithResource = 0;
	int32 HandWithResource = 0;
	int32 RemainingWithResource = 0;
	if (!RunScenario(true, APWithResource, HandWithResource, RemainingWithResource))
	{
		return false;
	}

	TestEqual(TEXT("Without YaoYin, consumer should not gain AP."), APWithoutResource, 2);
	TestEqual(TEXT("Without YaoYin, consumer should not draw a card."), HandWithoutResource, 0);
	TestEqual(TEXT("Without YaoYin, no resource stacks should remain."), RemainingWithoutResource, 0);

	TestEqual(TEXT("With YaoYin, consumer should refund 1 AP."), APWithResource, 3);
	TestEqual(TEXT("With YaoYin, consumer should draw exactly one replacement card."), HandWithResource, 1);
	TestEqual(TEXT("With YaoYin, consumed resource should be removed."), RemainingWithResource, 0);
	return true;
}

#endif
