#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalEnemyDefinition.h"
#include "Battle/Definitions/FinalStatusDefinition.h"
#include "Battle/Definitions/FinalRuntimeTriggerDefinition.h"
#include "Battle/Conditions/FinalBattleConditionResolvedCard.h"
#include "Battle/Effects/FinalBattleEffectApplyCardModifiers.h"
#include "Battle/Effects/FinalBattleEffectApplyStatus.h"
#include "Battle/Effects/FinalBattleEffectDamage.h"
#include "Battle/Effects/FinalBattleEffectDrawCards.h"
#include "Battle/Effects/FinalBattleEffectGenerateCard.h"
#include "Run/Definitions/FinalRelicDefinition.h"
#include "Commands/FinalBattleCommand.h"
#include "Facade/FinalBattleSession.h"
#include "Facade/FinalBattleSessionTypes.h"
#include "Queries/FinalBattleSnapshot.h"
#include "UObject/StrongObjectPtr.h"

namespace FinalBattleCardProjectionTests
{
	TStrongObjectPtr<UFinalBattleRuleConfig> MakeRuleConfig(const int32 InitialHandSize = 1)
	{
		TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig(NewObject<UFinalBattleRuleConfig>(GetTransientPackage()));
		RuleConfig->RuleConfigId = FFinalRuleConfigId(FName(TEXT("rule.test.card_projection")));
		RuleConfig->InitialHandSize = InitialHandSize;
		RuleConfig->InitialAP = 3;
		RuleConfig->TurnStartDrawCount = InitialHandSize;
		return RuleConfig;
	}

	TStrongObjectPtr<UFinalEnemyDefinition> MakeEnemyDefinition(const int32 MaxHP = 20)
	{
		TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition(NewObject<UFinalEnemyDefinition>(GetTransientPackage()));
		EnemyDefinition->EnemyId = FFinalEnemyId(FName(TEXT("enemy.test.card_projection")));
		EnemyDefinition->DisplayName = FText::FromString(TEXT("Projection Dummy"));
		EnemyDefinition->MaxHP = MaxHP;
		EnemyDefinition->MaxBreakValue = 10;
		EnemyDefinition->BaseDamagePower = 0;
		EnemyDefinition->InitialInitiativeValue = 0;
		EnemyDefinition->IntentSelectRule = EFinalIntentSelectRule::Cycle;
		return EnemyDefinition;
	}

	TStrongObjectPtr<UFinalBattleEncounterDefinition> MakeEncounter(UFinalBattleRuleConfig* RuleConfig, UFinalEnemyDefinition* EnemyDefinition)
	{
		TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition(NewObject<UFinalBattleEncounterDefinition>(GetTransientPackage()));
		EncounterDefinition->EncounterId = FFinalEncounterId(FName(TEXT("encounter.test.card_projection")));
		EncounterDefinition->DisplayName = FText::FromString(TEXT("Projection Encounter"));
		EncounterDefinition->RuleConfig = RuleConfig;

		FFinalEnemyRosterEntry& Entry = EncounterDefinition->EnemyRoster.AddDefaulted_GetRef();
		Entry.EnemyDefinition = EnemyDefinition;
		Entry.PositionIndex = 0;
		Entry.SpawnWave = 1;
		return EncounterDefinition;
	}

	TStrongObjectPtr<UFinalCharacterDefinition> MakeCharacterDefinition()
	{
		TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition(NewObject<UFinalCharacterDefinition>(GetTransientPackage()));
		CharacterDefinition->CharacterId = FFinalCharacterId(FName(TEXT("character.test.card_projection")));
		CharacterDefinition->DisplayName = FText::FromString(TEXT("Projection Hero"));
		CharacterDefinition->BaseVitalShare = 20;
		CharacterDefinition->BaseStressCap = 12;
		CharacterDefinition->BaseAttack = 5;
		CharacterDefinition->BaseDefense = 0;
		CharacterDefinition->BaseBreakRate = 1.0f;
		CharacterDefinition->BaseCritChance = 0.0f;
		CharacterDefinition->BaseCritDamage = 1.5f;
		return CharacterDefinition;
	}

	TStrongObjectPtr<UFinalCharacterDefinition> MakeCharacterDefinitionWithId(const FFinalCharacterId& CharacterId, const FString& DisplayName)
	{
		TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition();
		CharacterDefinition->CharacterId = CharacterId;
		CharacterDefinition->DisplayName = FText::FromString(DisplayName);
		return CharacterDefinition;
	}

	UFinalBattleEffectDamage* AddFlatDamageEffect(UFinalCardDefinition* CardDefinition, const FName EffectId, const float Damage)
	{
		UFinalBattleEffectDamage* DamageEffect = NewObject<UFinalBattleEffectDamage>(CardDefinition);
		DamageEffect->EffectId = EffectId;
		DamageEffect->UnitTargetRule = EFinalBattleUnitTargetRule::SelectedEnemy;
		DamageEffect->Scalar.ScaleMode = EFinalBattleScalarMode::Flat;
		DamageEffect->Scalar.BaseValue = Damage;
		CardDefinition->Effects.Add(DamageEffect);
		return DamageEffect;
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
		AddFlatDamageEffect(CardDefinition.Get(), FName(TEXT("effect.base.damage")), Damage);
		return CardDefinition;
	}

	TStrongObjectPtr<UFinalCardDefinition> MakeRetainedDamageCard(
		const FFinalCardId& CardId,
		const FFinalCharacterId& CharacterId,
		const FString& DisplayName,
		const int32 BaseCostAP,
		const float Damage)
	{
		TStrongObjectPtr<UFinalCardDefinition> CardDefinition = MakeDamageCard(CardId, CharacterId, DisplayName, BaseCostAP, Damage);
		CardDefinition->Keywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Retain")));
		return CardDefinition;
	}

	TStrongObjectPtr<UFinalStatusDefinition> MakeProjectedStatusDefinition(
		const FName StatusName,
		const int32 OutgoingDamagePercentPerStack,
		const int32 ProjectedDamagePercentPerStack,
		const bool bProjectCurrentOwnedHandAttackCards,
		const bool bConsumeOnSuccessfulDamage,
		const bool bExpireAtPlayerTurnEnd,
		const bool bOnlyAffectAttackCards)
	{
		TStrongObjectPtr<UFinalStatusDefinition> StatusDefinition(NewObject<UFinalStatusDefinition>(GetTransientPackage()));
		StatusDefinition->StatusId = FFinalStatusId(StatusName);
		StatusDefinition->DisplayName = FText::FromName(StatusName);
		StatusDefinition->StatusCategory = EFinalStatusCategory::Buff;
		StatusDefinition->MaxStacks = 9;
		StatusDefinition->DurationType = EFinalStatusDurationType::PlayerTurns;
		StatusDefinition->ExpireWindow = bExpireAtPlayerTurnEnd ? EFinalStatusExpireWindow::PlayerTurnEnd : EFinalStatusExpireWindow::None;
		StatusDefinition->RuntimeModifiers.Reset();
		if (OutgoingDamagePercentPerStack != 0)
		{
			FFinalStatusRuntimeModifierDefinition& RuntimeModifier = StatusDefinition->RuntimeModifiers.AddDefaulted_GetRef();
			RuntimeModifier.OutgoingDamagePercentPerStack = OutgoingDamagePercentPerStack;
			RuntimeModifier.bOnlyAffectAttackCards = bOnlyAffectAttackCards;
		}
		StatusDefinition->ProjectedCardModifiers.Reset();
		if (bProjectCurrentOwnedHandAttackCards && ProjectedDamagePercentPerStack != 0)
		{
			FFinalStatusProjectedCardModifierDefinition& ProjectedModifier = StatusDefinition->ProjectedCardModifiers.AddDefaulted_GetRef();
			ProjectedModifier.TargetSource = EFinalTriggeredCardModifierTargetSource::CurrentOwnedHandCards;
			ProjectedModifier.bRequireCardType = true;
			ProjectedModifier.RequiredCardType = EFinalCardType::Attack;
			ProjectedModifier.OutgoingDamagePercentPerStack = ProjectedDamagePercentPerStack;
			ProjectedModifier.LifetimePolicy = EFinalStatusProjectedCardModifierLifetimePolicy::WhileStatusActive;
			ProjectedModifier.bExpireAtPlayerTurnEnd = bExpireAtPlayerTurnEnd;
		}
		StatusDefinition->ConsumptionRules.Reset();
		if (bConsumeOnSuccessfulDamage)
		{
			FFinalStatusConsumptionRuleDefinition& ConsumptionRule = StatusDefinition->ConsumptionRules.AddDefaulted_GetRef();
			ConsumptionRule.Window = EFinalStatusConsumptionWindow::SuccessfulOwnerDamage;
			ConsumptionRule.StacksToConsume = 1;
			ConsumptionRule.bRequireAttackCardDamage = bOnlyAffectAttackCards;
		}
		return StatusDefinition;
	}

	TStrongObjectPtr<UFinalCardDefinition> MakeApplyStatusCard(
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
		CardDefinition->BaseCostAP = 0;
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

	TStrongObjectPtr<UFinalCardDefinition> MakeGenerateAttackCard(
		const FFinalCardId& CardId,
		const FFinalCharacterId& CharacterId,
		const FString& DisplayName,
		UFinalCardDefinition* GeneratedCardDefinition)
	{
		TStrongObjectPtr<UFinalCardDefinition> CardDefinition(NewObject<UFinalCardDefinition>(GetTransientPackage()));
		CardDefinition->CardId = CardId;
		CardDefinition->OwnerUnitId = CharacterId.Value;
		CardDefinition->DisplayName = FText::FromString(DisplayName);
		CardDefinition->BaseCostAP = 0;
		CardDefinition->CardType = EFinalCardType::Skill;

		UFinalBattleEffectGenerateCard* GenerateEffect = NewObject<UFinalBattleEffectGenerateCard>(CardDefinition.Get());
		GenerateEffect->EffectId = FName(*FString::Printf(TEXT("effect.%s.generate"), *CardId.Value.ToString()));
		GenerateEffect->GeneratedCardDefinition = GeneratedCardDefinition;
		GenerateEffect->GeneratedCardId = GeneratedCardDefinition != nullptr ? GeneratedCardDefinition->CardId : FFinalCardId();
		GenerateEffect->GenerateCount = 1;
		GenerateEffect->bGeneratedCard = true;
		GenerateEffect->bTemporaryCard = true;
		CardDefinition->Effects.Add(GenerateEffect);
		return CardDefinition;
	}

	TStrongObjectPtr<UFinalCardDefinition> MakeSkillCard(
		const FFinalCardId& CardId,
		const FFinalCharacterId& CharacterId,
		const FString& DisplayName,
		const int32 BaseCostAP)
	{
		TStrongObjectPtr<UFinalCardDefinition> CardDefinition(NewObject<UFinalCardDefinition>(GetTransientPackage()));
		CardDefinition->CardId = CardId;
		CardDefinition->OwnerUnitId = CharacterId.Value;
		CardDefinition->DisplayName = FText::FromString(DisplayName);
		CardDefinition->BaseCostAP = BaseCostAP;
		CardDefinition->CardType = EFinalCardType::Skill;
		return CardDefinition;
	}

	TStrongObjectPtr<UFinalCardDefinition> MakeApplyCardModifiersSkillCard(
		const FFinalCardId& CardId,
		const FFinalCharacterId& CharacterId,
		const FString& DisplayName,
		const bool bDrawAfterModifier = false)
	{
		TStrongObjectPtr<UFinalCardDefinition> CardDefinition = MakeSkillCard(CardId, CharacterId, DisplayName, 1);

		UFinalBattleEffectApplyCardModifiers* ApplyCardModifiersEffect = NewObject<UFinalBattleEffectApplyCardModifiers>(CardDefinition.Get());
		ApplyCardModifiersEffect->EffectId = FName(*FString::Printf(TEXT("effect.%s.apply_card_modifiers"), *CardId.Value.ToString()));
		FFinalTriggeredCardModifierDefinition& AllyAttackModifier = ApplyCardModifiersEffect->CardModifiers.AddDefaulted_GetRef();
		AllyAttackModifier.TargetSource = EFinalTriggeredCardModifierTargetSource::CurrentAllyHandCards;
		AllyAttackModifier.bRequireCardType = true;
		AllyAttackModifier.RequiredCardType = EFinalCardType::Attack;
		AllyAttackModifier.CostDeltaAP = -1;
		AllyAttackModifier.OutgoingDamagePercentDelta = 20;
		AllyAttackModifier.DurationPolicy = EFinalTriggeredCardModifierDurationPolicy::UntilPlayed;
		AllyAttackModifier.bExpireAtPlayerTurnEnd = true;
		CardDefinition->Effects.Add(ApplyCardModifiersEffect);

		if (bDrawAfterModifier)
		{
			UFinalBattleEffectDrawCards* DrawCardsEffect = NewObject<UFinalBattleEffectDrawCards>(CardDefinition.Get());
			DrawCardsEffect->EffectId = FName(*FString::Printf(TEXT("effect.%s.draw_after_modifier"), *CardId.Value.ToString()));
			DrawCardsEffect->DrawCount = 1;
			CardDefinition->Effects.Add(DrawCardsEffect);
		}

		return CardDefinition;
	}

	TStrongObjectPtr<UFinalBattleSession> CreateSession(
		UFinalBattleEncounterDefinition* EncounterDefinition,
		UFinalBattleRuleConfig* RuleConfig,
		UFinalCharacterDefinition* CharacterDefinition,
		UFinalCardDefinition* CardDefinition,
		const FName SourceRunCardInstanceId)
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
		CardInit.SourceRunCardInstanceId = SourceRunCardInstanceId;
		InitContext.DeckDefinitions.Add(CardDefinition);

		TStrongObjectPtr<UFinalBattleSession> Session(NewObject<UFinalBattleSession>(GetTransientPackage()));
		Session->InitializeSession(EncounterDefinition, RuleConfig, InitContext);
		return Session;
	}

	TStrongObjectPtr<UFinalBattleSession> CreateSessionWithDeck(
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

	TStrongObjectPtr<UFinalBattleSession> CreateTeamSessionWithDeck(
		UFinalBattleEncounterDefinition* EncounterDefinition,
		UFinalBattleRuleConfig* RuleConfig,
		const TArray<UFinalCharacterDefinition*>& CharacterDefinitions,
		const TArray<UFinalCardDefinition*>& CardDefinitions)
	{
		FFinalBattleInitContext InitContext;
		InitContext.TeamCurrentHP = 40;

		for (UFinalCharacterDefinition* CharacterDefinition : CharacterDefinitions)
		{
			if (CharacterDefinition == nullptr)
			{
				continue;
			}

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
		}

		for (UFinalCardDefinition* CardDefinition : CardDefinitions)
		{
			if (CardDefinition == nullptr)
			{
				continue;
			}

			FFinalBattleCardInitData& CardInit = InitContext.DeckCards.AddDefaulted_GetRef();
			CardInit.CardDefinition = CardDefinition;
			CardInit.CardId = CardDefinition->CardId;
			CardInit.OwnerCharacterId = FFinalCharacterId(CardDefinition->OwnerUnitId);
			InitContext.DeckDefinitions.Add(CardDefinition);
		}

		TStrongObjectPtr<UFinalBattleSession> Session(NewObject<UFinalBattleSession>(GetTransientPackage()));
		Session->InitializeSession(EncounterDefinition, RuleConfig, InitContext);
		return Session;
	}

	TStrongObjectPtr<UFinalBattleSession> CreateSessionWithDeckAndRelics(
		UFinalBattleEncounterDefinition* EncounterDefinition,
		UFinalBattleRuleConfig* RuleConfig,
		UFinalCharacterDefinition* CharacterDefinition,
		const TArray<UFinalCardDefinition*>& CardDefinitions,
		const TArray<FName>& SourceRunCardInstanceIds,
		const TArray<FFinalBattleStartRelicInput>& BattleStartRelics)
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
			CardInit.SourceRunCardInstanceId = SourceRunCardInstanceIds.IsValidIndex(CardIndex) ? SourceRunCardInstanceIds[CardIndex] : NAME_None;
			InitContext.DeckDefinitions.Add(CardDefinition);
		}

		InitContext.BattleStartRelics = BattleStartRelics;

		TStrongObjectPtr<UFinalBattleSession> Session(NewObject<UFinalBattleSession>(GetTransientPackage()));
		Session->InitializeSession(EncounterDefinition, RuleConfig, InitContext);
		return Session;
	}

	TStrongObjectPtr<UFinalRelicDefinition> MakeTokenZeroDrawRelicDefinition()
	{
		TStrongObjectPtr<UFinalRelicDefinition> RelicDefinition(NewObject<UFinalRelicDefinition>(GetTransientPackage()));
		RelicDefinition->RelicId = FFinalRelicId(FName(TEXT("relic.test.token_zero_draw")));
		RelicDefinition->DisplayName = FText::FromString(TEXT("Token Zero Draw"));

		FFinalRuntimeTriggerDefinition& TriggerDefinition = RelicDefinition->RuntimeTriggers.AddDefaulted_GetRef();
		TriggerDefinition.Domain = EFinalRuntimeTriggerDomain::Battle;
		TriggerDefinition.Window = EFinalRuntimeTriggerWindow::PlayerCardResolved;
		TriggerDefinition.Limit = EFinalRuntimeTriggerLimit::OncePerPlayerTurn;

		UFinalBattleConditionResolvedCard* ResolvedCardCondition = NewObject<UFinalBattleConditionResolvedCard>(RelicDefinition.Get());
		ResolvedCardCondition->Requirement.bRequireCardCostAP = true;
		ResolvedCardCondition->Requirement.RequiredCardCostAP = 0;
		TriggerDefinition.Conditions.Add(ResolvedCardCondition);

		UFinalBattleEffectDrawCards* DrawCardsEffect = NewObject<UFinalBattleEffectDrawCards>(RelicDefinition.Get());
		DrawCardsEffect->EffectId = TEXT("effect.test.relic.token_zero_draw.draw");
		DrawCardsEffect->DrawCount = 1;
		TriggerDefinition.Effects.Add(DrawCardsEffect);

		FFinalTriggeredCardModifierDefinition& TriggeredModifier = TriggerDefinition.TriggeredCardModifiers.AddDefaulted_GetRef();
		TriggeredModifier.TargetSource = EFinalTriggeredCardModifierTargetSource::DrawnCardsFromExecutedEffects;
		TriggeredModifier.bRequireCardType = true;
		TriggeredModifier.RequiredCardType = EFinalCardType::Attack;
		TriggeredModifier.CostDeltaAP = -1;
		TriggeredModifier.OutgoingDamagePercentDelta = 20;
		TriggeredModifier.DurationPolicy = EFinalTriggeredCardModifierDurationPolicy::UntilPlayed;
		TriggeredModifier.bExpireAtPlayerTurnEnd = true;
		TriggeredModifier.bApplyToAllSameSourceRunCardInstances = true;
		return RelicDefinition;
	}

	FFinalBattleStartRelicInput MakeBattleStartRelicInput(const UFinalRelicDefinition* RelicDefinition)
	{
		FFinalBattleStartRelicInput RelicInput;
		RelicInput.RelicId = RelicDefinition != nullptr ? RelicDefinition->RelicId : FFinalRelicId();
		RelicInput.DisplayId = RelicDefinition != nullptr ? RelicDefinition->DisplayId : NAME_None;
		RelicInput.DisplayName = RelicDefinition != nullptr ? RelicDefinition->DisplayName : FText::GetEmpty();
		RelicInput.RuntimeTriggers = RelicDefinition != nullptr ? RelicDefinition->RuntimeTriggers : TArray<FFinalRuntimeTriggerDefinition>{};
		return RelicInput;
	}

	const FFinalBattleCardViewData* FindSingleHandCard(const FFinalBattleSnapshot& Snapshot)
	{
		return Snapshot.HandCards.Num() == 1 ? &Snapshot.HandCards[0] : nullptr;
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
	FFinalBattleCardProjectionRefreshPreservesModifiersTest,
	"Final.Battle.CardProjection.BaseRefreshPreservesModifiers",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleCardProjectionRefreshPreservesModifiersTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattleCardProjectionTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.card_projection_refresh")));
	const FName SourceRunCardInstanceId(TEXT("run.card.test.refresh"));
	const FFinalCardId BaseCardId(FName(TEXT("card.test.card_projection.base")));
	const FFinalCardId EvolvedCardId(FName(TEXT("card.test.card_projection.evolved")));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig();
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition();
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition();
	CharacterDefinition->CharacterId = CharacterId;
	TStrongObjectPtr<UFinalCardDefinition> BaseCardDefinition = MakeDamageCard(BaseCardId, CharacterId, TEXT("Projection Base Slash"), 2, 3.0f);
	TStrongObjectPtr<UFinalCardDefinition> EvolvedCardDefinition = MakeDamageCard(EvolvedCardId, CharacterId, TEXT("Projection Evolved Slash"), 4, 6.0f);

	TStrongObjectPtr<UFinalBattleSession> Session = CreateSession(
		EncounterDefinition.Get(),
		RuleConfig.Get(),
		CharacterDefinition.Get(),
		BaseCardDefinition.Get(),
		SourceRunCardInstanceId);

	FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* HandCard = FindSingleHandCard(Snapshot);
	if (!TestNotNull(TEXT("Projection refresh test should begin with one hand card."), HandCard))
	{
		return false;
	}

	FFinalBattleCardModifierRecord ModifierRecord;
	ModifierRecord.ModifierId = TEXT("modifier.refresh.preserve");
	ModifierRecord.DurationPolicy = EFinalBattleCardModifierDuration::ManualClear;
	ModifierRecord.ApplyOrder = 1;
	ModifierRecord.CostDeltaAP = -1;
	ModifierRecord.AddedKeywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Retain")));
	ModifierRecord.bOverrideConsumeOnPlay = true;
	ModifierRecord.bConsumeOnPlay = true;
	TestTrue(TEXT("The temporary projection modifier should be added to the hand card."), Session->AddCardModifier(HandCard->CardInstanceId, ModifierRecord));

	FFinalBattleCardProjectionView ProjectionBeforeRefresh = Session->GetCardProjectionView(HandCard->CardInstanceId);
	TestEqual(TEXT("The modifier should reduce the projected AP cost before refresh."), ProjectionBeforeRefresh.EffectiveCostAP, 1);
	TestTrue(TEXT("The modifier should set retained before refresh."), ProjectionBeforeRefresh.bRetained);
	TestTrue(TEXT("The modifier should set consume-on-play before refresh."), ProjectionBeforeRefresh.bConsumeOnPlay);

	FFinalBattleCardRefreshRequest RefreshRequest;
	RefreshRequest.SourceRunCardInstanceId = SourceRunCardInstanceId;
	RefreshRequest.NewCardId = EvolvedCardId;
	RefreshRequest.NewDefinition = EvolvedCardDefinition.Get();
	TestEqual(TEXT("Refreshing by run-card instance should update the active battle card once."), Session->RefreshCardsForRunCardInstance(RefreshRequest), 1);

	FFinalBattleCardProjectionView ProjectionAfterRefresh = Session->GetCardProjectionView(HandCard->CardInstanceId);
	TestEqual(TEXT("The refreshed card should now project the evolved card id."), ProjectionAfterRefresh.CardId.Value, EvolvedCardId.Value);
	TestEqual(TEXT("Refreshing the base definition should preserve the temporary AP modifier."), ProjectionAfterRefresh.EffectiveCostAP, 3);
	TestTrue(TEXT("Refreshing the base definition should preserve the temporary retain modifier."), ProjectionAfterRefresh.bRetained);
	TestTrue(TEXT("Refreshing the base definition should preserve the temporary consume-on-play modifier."), ProjectionAfterRefresh.bConsumeOnPlay);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleCardProjectionEffectPatchDrivesResolutionTest,
	"Final.Battle.CardProjection.EffectPatchDrivesProjectedResolution",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleCardProjectionEffectPatchDrivesResolutionTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattleCardProjectionTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.card_projection_patch")));
	const FName SourceRunCardInstanceId(TEXT("run.card.test.patch"));
	const FFinalCardId CardId(FName(TEXT("card.test.card_projection.patch")));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig();
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition();
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition();
	CharacterDefinition->CharacterId = CharacterId;
	TStrongObjectPtr<UFinalCardDefinition> CardDefinition = MakeDamageCard(CardId, CharacterId, TEXT("Projection Patch Slash"), 1, 3.0f);

	TStrongObjectPtr<UFinalBattleSession> Session = CreateSession(
		EncounterDefinition.Get(),
		RuleConfig.Get(),
		CharacterDefinition.Get(),
		CardDefinition.Get(),
		SourceRunCardInstanceId);

	FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* HandCard = FindSingleHandCard(Snapshot);
	if (!TestNotNull(TEXT("Effect patch test should begin with one hand card."), HandCard))
	{
		return false;
	}

	UFinalBattleEffectDamage* ReplacementEffect = NewObject<UFinalBattleEffectDamage>(GetTransientPackage());
	ReplacementEffect->EffectId = TEXT("effect.replacement.damage");
	ReplacementEffect->UnitTargetRule = EFinalBattleUnitTargetRule::SelectedEnemy;
	ReplacementEffect->Scalar.ScaleMode = EFinalBattleScalarMode::Flat;
	ReplacementEffect->Scalar.BaseValue = 9.0f;

	FFinalBattleCardModifierRecord ModifierRecord;
	ModifierRecord.ModifierId = TEXT("modifier.patch.replace_effect");
	ModifierRecord.DurationPolicy = EFinalBattleCardModifierDuration::ManualClear;

	FFinalBattleCardEffectPatch& EffectPatch = ModifierRecord.EffectPatches.AddDefaulted_GetRef();
	EffectPatch.TargetEffectId = TEXT("effect.base.damage");
	EffectPatch.Operation = EFinalBattleCardEffectPatchOperation::Replace;
	EffectPatch.EffectDefinition = ReplacementEffect;

	TestTrue(TEXT("Effect patch modifier should be accepted."), Session->AddCardModifier(HandCard->CardInstanceId, ModifierRecord));
	FFinalBattleCardProjectionView ProjectionView = Session->GetCardProjectionView(HandCard->CardInstanceId);
	TestEqual(TEXT("Replacing the base effect should still leave one projected effect."), ProjectionView.EffectCount, 1);

	const FFinalBattleSnapshot SnapshotBeforePlay = Session->GetSnapshot();
	if (!TestTrue(TEXT("Effect patch test requires a targetable enemy in the opening snapshot."), SnapshotBeforePlay.Enemies.Num() == 1))
	{
		return false;
	}

	FFinalBattleCommand PlayCardCommand;
	PlayCardCommand.CommandType = EFinalBattleCommandType::PlayCard;
	PlayCardCommand.CardInstanceId = HandCard->CardInstanceId;
	PlayCardCommand.TargetUnitId = SnapshotBeforePlay.Enemies[0].RuntimeUnitId;
	const FFinalBattleEvent PlayEvent = Session->SubmitCommand(PlayCardCommand);
	if (!TestTrue(TEXT("The patched card should still resolve as a playable card."), PlayEvent.EventType != EFinalBattleEventType::CommandRejected))
	{
		return false;
	}

	const FFinalBattleSnapshot SnapshotAfterPlay = Session->GetSnapshot();
	if (!TestTrue(TEXT("Effect patch test should keep exactly one enemy for damage verification."), SnapshotAfterPlay.Enemies.Num() == 1))
	{
		return false;
	}

	TestEqual(TEXT("The replaced damage effect should determine the resolved damage amount."), SnapshotAfterPlay.Enemies[0].CurrentHP, 11);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleCardProjectionUntilPlayedClearsAfterPlayTest,
	"Final.Battle.CardProjection.UntilPlayedClearsAfterPlay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleCardProjectionUntilPlayedClearsAfterPlayTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattleCardProjectionTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.card_projection_until_played")));
	const FName SourceRunCardInstanceId(TEXT("run.card.test.until_played"));
	const FFinalCardId CardId(FName(TEXT("card.test.card_projection.until_played")));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig();
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition();
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition();
	CharacterDefinition->CharacterId = CharacterId;
	TStrongObjectPtr<UFinalCardDefinition> CardDefinition = MakeDamageCard(CardId, CharacterId, TEXT("Projection Until Played Slash"), 2, 3.0f);

	TStrongObjectPtr<UFinalBattleSession> Session = CreateSession(
		EncounterDefinition.Get(),
		RuleConfig.Get(),
		CharacterDefinition.Get(),
		CardDefinition.Get(),
		SourceRunCardInstanceId);

	FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* HandCard = FindSingleHandCard(Snapshot);
	if (!TestNotNull(TEXT("UntilPlayed cleanup test should begin with one hand card."), HandCard))
	{
		return false;
	}

	FFinalBattleCardModifierRecord ModifierRecord;
	ModifierRecord.ModifierId = TEXT("modifier.until_played.cost");
	ModifierRecord.DurationPolicy = EFinalBattleCardModifierDuration::UntilPlayed;
	ModifierRecord.CostDeltaAP = -1;
	TestTrue(TEXT("UntilPlayed modifier should be accepted before play."), Session->AddCardModifier(HandCard->CardInstanceId, ModifierRecord));

	FFinalBattleCardProjectionView ProjectionBeforePlay = Session->GetCardProjectionView(HandCard->CardInstanceId);
	TestEqual(TEXT("UntilPlayed modifier should reduce the projected cost before play."), ProjectionBeforePlay.EffectiveCostAP, 1);
	TestEqual(TEXT("UntilPlayed modifier should count as one active modifier before play."), ProjectionBeforePlay.ModifierCount, 1);

	const FFinalBattleSnapshot SnapshotBeforePlay = Session->GetSnapshot();
	FFinalBattleCommand PlayCardCommand;
	PlayCardCommand.CommandType = EFinalBattleCommandType::PlayCard;
	PlayCardCommand.CardInstanceId = HandCard->CardInstanceId;
	PlayCardCommand.TargetUnitId = SnapshotBeforePlay.Enemies[0].RuntimeUnitId;
	const FFinalBattleEvent PlayEvent = Session->SubmitCommand(PlayCardCommand);
	TestTrue(TEXT("Playing the card should succeed so the UntilPlayed cleanup path runs."), PlayEvent.EventType != EFinalBattleEventType::CommandRejected);

	FFinalBattleCardProjectionView ProjectionAfterPlay = Session->GetCardProjectionView(HandCard->CardInstanceId);
	TestEqual(TEXT("UntilPlayed modifiers should be cleared immediately after the card resolves."), ProjectionAfterPlay.ModifierCount, 0);
	TestEqual(TEXT("After UntilPlayed cleanup the projected cost should return to the base cost."), ProjectionAfterPlay.EffectiveCostAP, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleCardProjectionFengRuiProjectsToHandAttackCardsTest,
	"Final.Battle.CardProjection.FengRuiProjectsToHandAttackCards",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleCardProjectionFengRuiProjectsToHandAttackCardsTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattleCardProjectionTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.fengrui.project")));
	const FFinalCardId FengRuiCardId(FName(TEXT("card.test.fengrui.apply")));
	const FFinalCardId AttackCardId(FName(TEXT("card.test.fengrui.attack")));
	const FFinalCardId SkillCardId(FName(TEXT("card.test.fengrui.skill")));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig(3);
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition(20);
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition();
	CharacterDefinition->CharacterId = CharacterId;

	TStrongObjectPtr<UFinalStatusDefinition> FengRuiStatus = MakeProjectedStatusDefinition(
		TEXT("status.test.fengrui"),
		0,
		20,
		true,
		true,
		true,
		true);
	TStrongObjectPtr<UFinalCardDefinition> FengRuiApplyCard = MakeApplyStatusCard(FengRuiCardId, CharacterId, TEXT("Apply FengRui"), FengRuiStatus.Get(), 1);
	TStrongObjectPtr<UFinalCardDefinition> AttackCard = MakeDamageCard(AttackCardId, CharacterId, TEXT("Attack Card"), 1, 5.0f);
	TStrongObjectPtr<UFinalCardDefinition> SkillCard = MakeApplyStatusCard(SkillCardId, CharacterId, TEXT("Skill Card"), FengRuiStatus.Get(), 0);

	TStrongObjectPtr<UFinalBattleSession> Session = CreateSessionWithDeck(
		EncounterDefinition.Get(),
		RuleConfig.Get(),
		CharacterDefinition.Get(),
		TArray<UFinalCardDefinition*>{ FengRuiApplyCard.Get(), AttackCard.Get(), SkillCard.Get() });

	FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* FengRuiHandCard = FindHandCardById(Snapshot, FengRuiCardId);
	if (!TestNotNull(TEXT("Should begin with the FengRui apply card in hand."), FengRuiHandCard))
	{
		return false;
	}

	FFinalBattleCommand ApplyCommand;
	ApplyCommand.CommandType = EFinalBattleCommandType::PlayCard;
	ApplyCommand.CardInstanceId = FengRuiHandCard->CardInstanceId;
	ApplyCommand.TargetUnitId = Snapshot.Enemies[0].RuntimeUnitId;
	const FFinalBattleEvent ApplyEvent = Session->SubmitCommand(ApplyCommand);
	if (!TestTrue(TEXT("Applying FengRui should resolve as a playable skill card."), ApplyEvent.EventType != EFinalBattleEventType::CommandRejected))
	{
		return false;
	}

	const FFinalBattleSnapshot AfterApplySnapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* AttackHandCard = FindHandCardById(AfterApplySnapshot, AttackCardId);
	const FFinalBattleCardViewData* NonAttackHandCard = FindHandCardById(AfterApplySnapshot, SkillCardId);
	if (!TestNotNull(TEXT("Attack card should remain in hand after FengRui is applied."), AttackHandCard)
		|| !TestNotNull(TEXT("Non-attack skill card should remain in hand after FengRui is applied."), NonAttackHandCard))
	{
		return false;
	}

	const FFinalBattleCardProjectionView AttackProjection = Session->GetCardProjectionView(AttackHandCard->CardInstanceId);
	const FFinalBattleCardProjectionView NonAttackProjection = Session->GetCardProjectionView(NonAttackHandCard->CardInstanceId);
	TestEqual(TEXT("FengRui should project +20% outgoing damage onto current hand attack cards."), AttackProjection.EffectiveOutgoingDamagePercent, 20);
	TestEqual(TEXT("FengRui should not project outgoing damage onto non-attack hand cards."), NonAttackProjection.EffectiveOutgoingDamagePercent, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleCardProjectionFengRuiStacksWithShiQiTest,
	"Final.Battle.CardProjection.FengRuiStacksWithShiQi",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleCardProjectionFengRuiStacksWithShiQiTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattleCardProjectionTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.fengrui.shiqi")));
	const FFinalCardId FengRuiCardId(FName(TEXT("card.test.fengrui.apply_stack")));
	const FFinalCardId ShiQiCardId(FName(TEXT("card.test.shiqi.apply")));
	const FFinalCardId AttackCardId(FName(TEXT("card.test.fengrui.stack_attack")));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig(3);
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition(20);
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition();
	CharacterDefinition->CharacterId = CharacterId;

	TStrongObjectPtr<UFinalStatusDefinition> FengRuiStatus = MakeProjectedStatusDefinition(
		TEXT("status.test.fengrui.stack"),
		0,
		20,
		true,
		true,
		true,
		true);
	TStrongObjectPtr<UFinalStatusDefinition> ShiQiStatus = MakeProjectedStatusDefinition(
		TEXT("status.test.shiqi.stack"),
		20,
		0,
		false,
		false,
		true,
		false);
	TStrongObjectPtr<UFinalCardDefinition> FengRuiApplyCard = MakeApplyStatusCard(FengRuiCardId, CharacterId, TEXT("Apply FengRui"), FengRuiStatus.Get(), 1);
	TStrongObjectPtr<UFinalCardDefinition> ShiQiApplyCard = MakeApplyStatusCard(ShiQiCardId, CharacterId, TEXT("Apply ShiQi"), ShiQiStatus.Get(), 1);
	TStrongObjectPtr<UFinalCardDefinition> AttackCard = MakeDamageCard(AttackCardId, CharacterId, TEXT("Stack Attack"), 1, 10.0f);

	TStrongObjectPtr<UFinalBattleSession> Session = CreateSessionWithDeck(
		EncounterDefinition.Get(),
		RuleConfig.Get(),
		CharacterDefinition.Get(),
		TArray<UFinalCardDefinition*>{ FengRuiApplyCard.Get(), ShiQiApplyCard.Get(), AttackCard.Get() });

	FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* FengRuiHandCard = FindHandCardById(Snapshot, FengRuiCardId);
	const FFinalBattleCardViewData* ShiQiHandCard = FindHandCardById(Snapshot, ShiQiCardId);
	if (!TestNotNull(TEXT("FengRui apply card should begin in hand."), FengRuiHandCard)
		|| !TestNotNull(TEXT("ShiQi apply card should begin in hand."), ShiQiHandCard))
	{
		return false;
	}

	FFinalBattleCommand ApplyFengRuiCommand;
	ApplyFengRuiCommand.CommandType = EFinalBattleCommandType::PlayCard;
	ApplyFengRuiCommand.CardInstanceId = FengRuiHandCard->CardInstanceId;
	ApplyFengRuiCommand.TargetUnitId = Snapshot.Enemies[0].RuntimeUnitId;
	TestTrue(TEXT("Playing the FengRui apply card should succeed."), Session->SubmitCommand(ApplyFengRuiCommand).EventType != EFinalBattleEventType::CommandRejected);

	Snapshot = Session->GetSnapshot();
	ShiQiHandCard = FindHandCardById(Snapshot, ShiQiCardId);
	FFinalBattleCommand ApplyShiQiCommand;
	ApplyShiQiCommand.CommandType = EFinalBattleCommandType::PlayCard;
	ApplyShiQiCommand.CardInstanceId = ShiQiHandCard->CardInstanceId;
	ApplyShiQiCommand.TargetUnitId = Snapshot.Enemies[0].RuntimeUnitId;
	TestTrue(TEXT("Playing the ShiQi apply card should succeed."), Session->SubmitCommand(ApplyShiQiCommand).EventType != EFinalBattleEventType::CommandRejected);

	Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* AttackHandCard = FindHandCardById(Snapshot, AttackCardId);
	if (!TestNotNull(TEXT("Attack card should remain in hand before the stacked damage check."), AttackHandCard))
	{
		return false;
	}

	const FFinalBattleCardProjectionView AttackProjection = Session->GetCardProjectionView(AttackHandCard->CardInstanceId);
	TestEqual(TEXT("FengRui should still only contribute +20% through card projection."), AttackProjection.EffectiveOutgoingDamagePercent, 20);

	FFinalBattleCommand AttackCommand;
	AttackCommand.CommandType = EFinalBattleCommandType::PlayCard;
	AttackCommand.CardInstanceId = AttackHandCard->CardInstanceId;
	AttackCommand.TargetUnitId = Snapshot.Enemies[0].RuntimeUnitId;
	const FFinalBattleEvent AttackEvent = Session->SubmitCommand(AttackCommand);
	if (!TestTrue(TEXT("The stacked FengRui + ShiQi attack should resolve successfully."), AttackEvent.EventType != EFinalBattleEventType::CommandRejected))
	{
		return false;
	}

	const FFinalBattleSnapshot AfterAttackSnapshot = Session->GetSnapshot();
	TestEqual(TEXT("Base 10 damage with +20% ShiQi and +20% FengRui should resolve to 14 total damage."), AfterAttackSnapshot.Enemies[0].CurrentHP, 6);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleCardProjectionFengRuiConsumesAndReprojectsTest,
	"Final.Battle.CardProjection.FengRuiConsumesAndReprojects",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleCardProjectionFengRuiConsumesAndReprojectsTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattleCardProjectionTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.fengrui.consume")));
	const FFinalCardId FengRuiCardId(FName(TEXT("card.test.fengrui.apply_consume")));
	const FFinalCardId AttackCardAId(FName(TEXT("card.test.fengrui.attack_a")));
	const FFinalCardId AttackCardBId(FName(TEXT("card.test.fengrui.attack_b")));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig(3);
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition(30);
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition();
	CharacterDefinition->CharacterId = CharacterId;

	TStrongObjectPtr<UFinalStatusDefinition> FengRuiStatus = MakeProjectedStatusDefinition(
		TEXT("status.test.fengrui.consume"),
		0,
		20,
		true,
		true,
		true,
		true);
	TStrongObjectPtr<UFinalCardDefinition> FengRuiApplyCard = MakeApplyStatusCard(FengRuiCardId, CharacterId, TEXT("Apply Double FengRui"), FengRuiStatus.Get(), 2);
	TStrongObjectPtr<UFinalCardDefinition> AttackCardA = MakeDamageCard(AttackCardAId, CharacterId, TEXT("Attack A"), 1, 5.0f);
	TStrongObjectPtr<UFinalCardDefinition> AttackCardB = MakeDamageCard(AttackCardBId, CharacterId, TEXT("Attack B"), 1, 5.0f);

	TStrongObjectPtr<UFinalBattleSession> Session = CreateSessionWithDeck(
		EncounterDefinition.Get(),
		RuleConfig.Get(),
		CharacterDefinition.Get(),
		TArray<UFinalCardDefinition*>{ FengRuiApplyCard.Get(), AttackCardA.Get(), AttackCardB.Get() });

	FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* FengRuiHandCard = FindHandCardById(Snapshot, FengRuiCardId);
	if (!TestNotNull(TEXT("Double FengRui apply card should begin in hand."), FengRuiHandCard))
	{
		return false;
	}

	FFinalBattleCommand ApplyCommand;
	ApplyCommand.CommandType = EFinalBattleCommandType::PlayCard;
	ApplyCommand.CardInstanceId = FengRuiHandCard->CardInstanceId;
	ApplyCommand.TargetUnitId = Snapshot.Enemies[0].RuntimeUnitId;
	TestTrue(TEXT("Applying two FengRui stacks should succeed."), Session->SubmitCommand(ApplyCommand).EventType != EFinalBattleEventType::CommandRejected);

	Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* AttackHandCardA = FindHandCardById(Snapshot, AttackCardAId);
	const FFinalBattleCardViewData* AttackHandCardB = FindHandCardById(Snapshot, AttackCardBId);
	if (!TestNotNull(TEXT("Attack A should be in hand after applying FengRui."), AttackHandCardA)
		|| !TestNotNull(TEXT("Attack B should be in hand after applying FengRui."), AttackHandCardB))
	{
		return false;
	}

	TestEqual(TEXT("Two FengRui stacks should project +40% onto Attack A."), Session->GetCardProjectionView(AttackHandCardA->CardInstanceId).EffectiveOutgoingDamagePercent, 40);
	TestEqual(TEXT("Two FengRui stacks should project +40% onto Attack B."), Session->GetCardProjectionView(AttackHandCardB->CardInstanceId).EffectiveOutgoingDamagePercent, 40);

	FFinalBattleCommand AttackCommand;
	AttackCommand.CommandType = EFinalBattleCommandType::PlayCard;
	AttackCommand.CardInstanceId = AttackHandCardA->CardInstanceId;
	AttackCommand.TargetUnitId = Snapshot.Enemies[0].RuntimeUnitId;
	TestTrue(TEXT("Playing the first FengRui-buffed attack should succeed."), Session->SubmitCommand(AttackCommand).EventType != EFinalBattleEventType::CommandRejected);

	Snapshot = Session->GetSnapshot();
	AttackHandCardB = FindHandCardById(Snapshot, AttackCardBId);
	if (!TestNotNull(TEXT("One attack card should remain in hand after consuming one FengRui stack."), AttackHandCardB))
	{
		return false;
	}

	TestEqual(TEXT("After one successful attack, the remaining hand attack should reproject to +20%."), Session->GetCardProjectionView(AttackHandCardB->CardInstanceId).EffectiveOutgoingDamagePercent, 20);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleCardProjectionFengRuiAppliesToGeneratedAttackCardsTest,
	"Final.Battle.CardProjection.FengRuiAppliesToGeneratedAttackCards",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleCardProjectionFengRuiAppliesToGeneratedAttackCardsTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattleCardProjectionTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.fengrui.generate")));
	const FFinalCardId FengRuiCardId(FName(TEXT("card.test.fengrui.apply_generate")));
	const FFinalCardId GenerateCardId(FName(TEXT("card.test.fengrui.generate_skill")));
	const FFinalCardId GeneratedAttackCardId(FName(TEXT("card.test.fengrui.generated_attack")));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig(2);
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition(20);
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition();
	CharacterDefinition->CharacterId = CharacterId;

	TStrongObjectPtr<UFinalStatusDefinition> FengRuiStatus = MakeProjectedStatusDefinition(
		TEXT("status.test.fengrui.generate"),
		0,
		20,
		true,
		true,
		true,
		true);
	TStrongObjectPtr<UFinalCardDefinition> FengRuiApplyCard = MakeApplyStatusCard(FengRuiCardId, CharacterId, TEXT("Apply FengRui"), FengRuiStatus.Get(), 1);
	TStrongObjectPtr<UFinalCardDefinition> GeneratedAttackCard = MakeDamageCard(GeneratedAttackCardId, CharacterId, TEXT("Generated Attack"), 1, 5.0f);
	TStrongObjectPtr<UFinalCardDefinition> GenerateAttackCard = MakeGenerateAttackCard(GenerateCardId, CharacterId, TEXT("Generate Attack"), GeneratedAttackCard.Get());

	TStrongObjectPtr<UFinalBattleSession> Session = CreateSessionWithDeck(
		EncounterDefinition.Get(),
		RuleConfig.Get(),
		CharacterDefinition.Get(),
		TArray<UFinalCardDefinition*>{ FengRuiApplyCard.Get(), GenerateAttackCard.Get() });

	FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* FengRuiHandCard = FindHandCardById(Snapshot, FengRuiCardId);
	const FFinalBattleCardViewData* GenerateHandCard = FindHandCardById(Snapshot, GenerateCardId);
	if (!TestNotNull(TEXT("FengRui apply card should begin in hand."), FengRuiHandCard)
		|| !TestNotNull(TEXT("Generate attack card should begin in hand."), GenerateHandCard))
	{
		return false;
	}

	FFinalBattleCommand ApplyCommand;
	ApplyCommand.CommandType = EFinalBattleCommandType::PlayCard;
	ApplyCommand.CardInstanceId = FengRuiHandCard->CardInstanceId;
	ApplyCommand.TargetUnitId = Snapshot.Enemies[0].RuntimeUnitId;
	TestTrue(TEXT("Applying FengRui before generation should succeed."), Session->SubmitCommand(ApplyCommand).EventType != EFinalBattleEventType::CommandRejected);

	Snapshot = Session->GetSnapshot();
	GenerateHandCard = FindHandCardById(Snapshot, GenerateCardId);
	FFinalBattleCommand GenerateCommand;
	GenerateCommand.CommandType = EFinalBattleCommandType::PlayCard;
	GenerateCommand.CardInstanceId = GenerateHandCard->CardInstanceId;
	GenerateCommand.TargetUnitId = Snapshot.Enemies[0].RuntimeUnitId;
	TestTrue(TEXT("Generating an attack card into hand while FengRui is active should succeed."), Session->SubmitCommand(GenerateCommand).EventType != EFinalBattleEventType::CommandRejected);

	const FFinalBattleSnapshot AfterGenerateSnapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* GeneratedAttackHandCard = FindHandCardById(AfterGenerateSnapshot, GeneratedAttackCardId);
	if (!TestNotNull(TEXT("Generated attack card should appear in hand."), GeneratedAttackHandCard))
	{
		return false;
	}

	TestEqual(TEXT("A generated attack card entering hand while FengRui is active should immediately gain +20% projected outgoing damage."), Session->GetCardProjectionView(GeneratedAttackHandCard->CardInstanceId).EffectiveOutgoingDamagePercent, 20);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleCardProjectionZeroCostRelicDrawnAttackGetsProjectedModifierTest,
	"Final.Battle.CardProjection.ZeroCostRelicDrawnAttackGetsProjectedModifier",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleCardProjectionZeroCostRelicDrawnAttackGetsProjectedModifierTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattleCardProjectionTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.relic_draw_attack")));
	const FFinalCardId TriggerCardId(FName(TEXT("card.test.relic.trigger_attack")));
	const FFinalCardId DrawnAttackCardId(FName(TEXT("card.test.relic.drawn_attack")));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig(1);
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition(20);
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition();
	CharacterDefinition->CharacterId = CharacterId;

	TStrongObjectPtr<UFinalCardDefinition> TriggerCard = MakeDamageCard(TriggerCardId, CharacterId, TEXT("Zero Cost Trigger"), 0, 2.0f);
	TriggerCard->Keywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Opening")));
	TStrongObjectPtr<UFinalCardDefinition> DrawnAttackCard = MakeDamageCard(DrawnAttackCardId, CharacterId, TEXT("Drawn Attack"), 1, 5.0f);
	TStrongObjectPtr<UFinalRelicDefinition> RelicDefinition = MakeTokenZeroDrawRelicDefinition();

	TStrongObjectPtr<UFinalBattleSession> Session = CreateSessionWithDeckAndRelics(
		EncounterDefinition.Get(),
		RuleConfig.Get(),
		CharacterDefinition.Get(),
		TArray<UFinalCardDefinition*>{ TriggerCard.Get(), DrawnAttackCard.Get() },
		TArray<FName>{ TEXT("run.card.trigger"), TEXT("run.card.drawn_attack") },
		TArray<FFinalBattleStartRelicInput>{ MakeBattleStartRelicInput(RelicDefinition.Get()) });

	FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* TriggerHandCard = FindHandCardById(Snapshot, TriggerCardId);
	if (!TestNotNull(TEXT("Opening 0 AP trigger card should begin in hand."), TriggerHandCard))
	{
		return false;
	}

	FFinalBattleCommand TriggerCommand;
	TriggerCommand.CommandType = EFinalBattleCommandType::PlayCard;
	TriggerCommand.CardInstanceId = TriggerHandCard->CardInstanceId;
	TriggerCommand.TargetUnitId = Snapshot.Enemies[0].RuntimeUnitId;
	TestTrue(TEXT("Playing the opening 0 AP trigger card should succeed."), Session->SubmitCommand(TriggerCommand).EventType != EFinalBattleEventType::CommandRejected);

	const FFinalBattleSnapshot AfterTriggerSnapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* DrawnAttackHandCard = FindHandCardById(AfterTriggerSnapshot, DrawnAttackCardId);
	if (!TestNotNull(TEXT("Relic draw should put the attack card into hand."), DrawnAttackHandCard))
	{
		return false;
	}

	const FFinalBattleCardProjectionView DrawnProjection = Session->GetCardProjectionView(DrawnAttackHandCard->CardInstanceId);
	TestEqual(TEXT("Relic-driven attack draw should reduce projected AP cost by 1."), DrawnProjection.EffectiveCostAP, 0);
	TestEqual(TEXT("Relic-driven attack draw should add +20% projected outgoing damage."), DrawnProjection.EffectiveOutgoingDamagePercent, 20);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleCardProjectionZeroCostRelicDrawnNonAttackDoesNotGetModifierTest,
	"Final.Battle.CardProjection.ZeroCostRelicDrawnNonAttackDoesNotGetModifier",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleCardProjectionZeroCostRelicDrawnNonAttackDoesNotGetModifierTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattleCardProjectionTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.relic_draw_non_attack")));
	const FFinalCardId TriggerCardId(FName(TEXT("card.test.relic.trigger_non_attack")));
	const FFinalCardId DrawnSkillCardId(FName(TEXT("card.test.relic.drawn_skill")));
	const FName StatusId(TEXT("status.test.relic.drawn_skill"));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig(1);
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition(20);
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition();
	CharacterDefinition->CharacterId = CharacterId;

	TStrongObjectPtr<UFinalCardDefinition> TriggerCard = MakeDamageCard(TriggerCardId, CharacterId, TEXT("Zero Cost Trigger"), 0, 2.0f);
	TriggerCard->Keywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Opening")));
	TStrongObjectPtr<UFinalStatusDefinition> DrawnSkillStatus = MakeProjectedStatusDefinition(StatusId, 0, 0, false, false, false, false);
	TStrongObjectPtr<UFinalCardDefinition> DrawnSkillCard = MakeApplyStatusCard(DrawnSkillCardId, CharacterId, TEXT("Drawn Skill"), DrawnSkillStatus.Get(), 1);
	TStrongObjectPtr<UFinalRelicDefinition> RelicDefinition = MakeTokenZeroDrawRelicDefinition();

	TStrongObjectPtr<UFinalBattleSession> Session = CreateSessionWithDeckAndRelics(
		EncounterDefinition.Get(),
		RuleConfig.Get(),
		CharacterDefinition.Get(),
		TArray<UFinalCardDefinition*>{ TriggerCard.Get(), DrawnSkillCard.Get() },
		TArray<FName>{ TEXT("run.card.trigger"), TEXT("run.card.drawn_skill") },
		TArray<FFinalBattleStartRelicInput>{ MakeBattleStartRelicInput(RelicDefinition.Get()) });

	FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* TriggerHandCard = FindHandCardById(Snapshot, TriggerCardId);
	if (!TestNotNull(TEXT("Opening 0 AP trigger card should begin in hand."), TriggerHandCard))
	{
		return false;
	}

	FFinalBattleCommand TriggerCommand;
	TriggerCommand.CommandType = EFinalBattleCommandType::PlayCard;
	TriggerCommand.CardInstanceId = TriggerHandCard->CardInstanceId;
	TriggerCommand.TargetUnitId = Snapshot.Enemies[0].RuntimeUnitId;
	TestTrue(TEXT("Playing the opening 0 AP trigger card should succeed."), Session->SubmitCommand(TriggerCommand).EventType != EFinalBattleEventType::CommandRejected);

	const FFinalBattleSnapshot AfterTriggerSnapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* DrawnSkillHandCard = FindHandCardById(AfterTriggerSnapshot, DrawnSkillCardId);
	if (!TestNotNull(TEXT("Relic draw should put the non-attack skill into hand."), DrawnSkillHandCard))
	{
		return false;
	}

	const FFinalBattleCardProjectionView DrawnProjection = Session->GetCardProjectionView(DrawnSkillHandCard->CardInstanceId);
	TestEqual(TEXT("Non-attack cards drawn by the relic should keep their projected AP cost."), DrawnProjection.EffectiveCostAP, DrawnSkillCard->BaseCostAP);
	TestEqual(TEXT("Non-attack cards drawn by the relic should not gain outgoing damage."), DrawnProjection.EffectiveOutgoingDamagePercent, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleCardProjectionZeroCostRelicModifierClearsOnPlayTest,
	"Final.Battle.CardProjection.ZeroCostRelicModifierClearsOnPlay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleCardProjectionZeroCostRelicModifierClearsOnPlayTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattleCardProjectionTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.relic_clear_on_play")));
	const FFinalCardId TriggerCardId(FName(TEXT("card.test.relic.trigger_clear_on_play")));
	const FFinalCardId DrawnAttackCardId(FName(TEXT("card.test.relic.drawn_attack_clear_on_play")));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig(1);
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition(20);
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition();
	CharacterDefinition->CharacterId = CharacterId;

	TStrongObjectPtr<UFinalCardDefinition> TriggerCard = MakeDamageCard(TriggerCardId, CharacterId, TEXT("Zero Cost Trigger"), 0, 2.0f);
	TriggerCard->Keywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Opening")));
	TStrongObjectPtr<UFinalCardDefinition> DrawnAttackCard = MakeDamageCard(DrawnAttackCardId, CharacterId, TEXT("Drawn Attack"), 1, 5.0f);
	TStrongObjectPtr<UFinalRelicDefinition> RelicDefinition = MakeTokenZeroDrawRelicDefinition();

	TStrongObjectPtr<UFinalBattleSession> Session = CreateSessionWithDeckAndRelics(
		EncounterDefinition.Get(),
		RuleConfig.Get(),
		CharacterDefinition.Get(),
		TArray<UFinalCardDefinition*>{ TriggerCard.Get(), DrawnAttackCard.Get() },
		TArray<FName>{ TEXT("run.card.trigger"), TEXT("run.card.drawn_attack") },
		TArray<FFinalBattleStartRelicInput>{ MakeBattleStartRelicInput(RelicDefinition.Get()) });

	FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* TriggerHandCard = FindHandCardById(Snapshot, TriggerCardId);
	if (!TestNotNull(TEXT("Opening 0 AP trigger card should begin in hand."), TriggerHandCard))
	{
		return false;
	}

	FFinalBattleCommand TriggerCommand;
	TriggerCommand.CommandType = EFinalBattleCommandType::PlayCard;
	TriggerCommand.CardInstanceId = TriggerHandCard->CardInstanceId;
	TriggerCommand.TargetUnitId = Snapshot.Enemies[0].RuntimeUnitId;
	TestTrue(TEXT("Playing the opening 0 AP trigger card should succeed."), Session->SubmitCommand(TriggerCommand).EventType != EFinalBattleEventType::CommandRejected);

	Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* DrawnAttackHandCard = FindHandCardById(Snapshot, DrawnAttackCardId);
	if (!TestNotNull(TEXT("Relic draw should put the attack card into hand."), DrawnAttackHandCard))
	{
		return false;
	}

	FFinalBattleCommand AttackCommand;
	AttackCommand.CommandType = EFinalBattleCommandType::PlayCard;
	AttackCommand.CardInstanceId = DrawnAttackHandCard->CardInstanceId;
	AttackCommand.TargetUnitId = Snapshot.Enemies[0].RuntimeUnitId;
	TestTrue(TEXT("Playing the relic-buffed attack should succeed."), Session->SubmitCommand(AttackCommand).EventType != EFinalBattleEventType::CommandRejected);

	const FFinalBattleCardProjectionView ProjectionAfterPlay = Session->GetCardProjectionView(DrawnAttackHandCard->CardInstanceId);
	TestEqual(TEXT("Relic modifier should clear immediately after the buffed attack is played."), ProjectionAfterPlay.ModifierCount, 0);
	TestEqual(TEXT("After play cleanup the projected cost should return to base."), ProjectionAfterPlay.EffectiveCostAP, DrawnAttackCard->BaseCostAP);
	TestEqual(TEXT("After play cleanup the projected outgoing damage bonus should be removed."), ProjectionAfterPlay.EffectiveOutgoingDamagePercent, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleCardProjectionZeroCostRelicModifierClearsAtTurnEndTest,
	"Final.Battle.CardProjection.ZeroCostRelicModifierClearsAtTurnEnd",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleCardProjectionZeroCostRelicModifierClearsAtTurnEndTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattleCardProjectionTests;

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.relic_clear_at_turn_end")));
	const FFinalCardId TriggerCardId(FName(TEXT("card.test.relic.trigger_clear_at_turn_end")));
	const FFinalCardId RetainedAttackCardId(FName(TEXT("card.test.relic.retained_attack")));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig(1);
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition(20);
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> CharacterDefinition = MakeCharacterDefinition();
	CharacterDefinition->CharacterId = CharacterId;

	TStrongObjectPtr<UFinalCardDefinition> TriggerCard = MakeDamageCard(TriggerCardId, CharacterId, TEXT("Zero Cost Trigger"), 0, 2.0f);
	TriggerCard->Keywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Opening")));
	TStrongObjectPtr<UFinalCardDefinition> RetainedAttackCard = MakeRetainedDamageCard(RetainedAttackCardId, CharacterId, TEXT("Retained Attack"), 1, 5.0f);
	TStrongObjectPtr<UFinalRelicDefinition> RelicDefinition = MakeTokenZeroDrawRelicDefinition();

	TStrongObjectPtr<UFinalBattleSession> Session = CreateSessionWithDeckAndRelics(
		EncounterDefinition.Get(),
		RuleConfig.Get(),
		CharacterDefinition.Get(),
		TArray<UFinalCardDefinition*>{ TriggerCard.Get(), RetainedAttackCard.Get() },
		TArray<FName>{ TEXT("run.card.trigger"), TEXT("run.card.retained_attack") },
		TArray<FFinalBattleStartRelicInput>{ MakeBattleStartRelicInput(RelicDefinition.Get()) });

	FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* TriggerHandCard = FindHandCardById(Snapshot, TriggerCardId);
	if (!TestNotNull(TEXT("Opening 0 AP trigger card should begin in hand."), TriggerHandCard))
	{
		return false;
	}

	FFinalBattleCommand TriggerCommand;
	TriggerCommand.CommandType = EFinalBattleCommandType::PlayCard;
	TriggerCommand.CardInstanceId = TriggerHandCard->CardInstanceId;
	TriggerCommand.TargetUnitId = Snapshot.Enemies[0].RuntimeUnitId;
	TestTrue(TEXT("Playing the opening 0 AP trigger card should succeed."), Session->SubmitCommand(TriggerCommand).EventType != EFinalBattleEventType::CommandRejected);

	Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* RetainedAttackHandCard = FindHandCardById(Snapshot, RetainedAttackCardId);
	if (!TestNotNull(TEXT("Relic draw should put the retained attack card into hand."), RetainedAttackHandCard))
	{
		return false;
	}

	const FFinalBattleCardProjectionView ProjectionBeforeEndTurn = Session->GetCardProjectionView(RetainedAttackHandCard->CardInstanceId);
	TestEqual(TEXT("Retained attack should be buffed before end turn cleanup."), ProjectionBeforeEndTurn.EffectiveOutgoingDamagePercent, 20);

	FFinalBattleCommand EndTurnCommand;
	EndTurnCommand.CommandType = EFinalBattleCommandType::EndTurn;
	TestTrue(TEXT("Ending the turn should succeed."), Session->SubmitCommand(EndTurnCommand).EventType != EFinalBattleEventType::CommandRejected);

	const FFinalBattleSnapshot AfterEndTurnSnapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* RetainedAttackAfterEndTurn = FindHandCardById(AfterEndTurnSnapshot, RetainedAttackCardId);
	if (!TestNotNull(TEXT("Retained attack should remain in hand after turn end cleanup."), RetainedAttackAfterEndTurn))
	{
		return false;
	}

	const FFinalBattleCardProjectionView ProjectionAfterEndTurn = Session->GetCardProjectionView(RetainedAttackAfterEndTurn->CardInstanceId);
	TestEqual(TEXT("Relic modifier should clear from retained cards at player turn end."), ProjectionAfterEndTurn.ModifierCount, 0);
	TestEqual(TEXT("Retained card cost should return to base after player turn end cleanup."), ProjectionAfterEndTurn.EffectiveCostAP, RetainedAttackCard->BaseCostAP);
	TestEqual(TEXT("Retained card damage bonus should be removed after player turn end cleanup."), ProjectionAfterEndTurn.EffectiveOutgoingDamagePercent, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleCardProjectionApplyCardModifiersAffectsCurrentAllyHandAttacksTest,
	"Final.Battle.CardProjection.ApplyCardModifiersAffectsCurrentAllyHandAttacks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleCardProjectionApplyCardModifiersAffectsCurrentAllyHandAttacksTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattleCardProjectionTests;

	const FFinalCharacterId SourceCharacterId(FName(TEXT("character.test.cross_modifier.source")));
	const FFinalCharacterId AllyCharacterId(FName(TEXT("character.test.cross_modifier.ally")));
	const FFinalCardId ModifierCardId(FName(TEXT("card.test.cross_modifier.apply")));
	const FFinalCardId SourceAttackCardId(FName(TEXT("card.test.cross_modifier.source_attack")));
	const FFinalCardId AllyAttackCardId(FName(TEXT("card.test.cross_modifier.ally_attack")));
	const FFinalCardId AllyRetainedAttackCardId(FName(TEXT("card.test.cross_modifier.ally_retained_attack")));
	const FFinalCardId AllySkillCardId(FName(TEXT("card.test.cross_modifier.ally_skill")));
	const FFinalCardId AllyDrawnAttackCardId(FName(TEXT("card.test.cross_modifier.ally_drawn_attack")));
	const FGameplayTag OpeningKeyword = FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Opening"));

	TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig = MakeRuleConfig(5);
	TStrongObjectPtr<UFinalEnemyDefinition> EnemyDefinition = MakeEnemyDefinition(40);
	TStrongObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition = MakeEncounter(RuleConfig.Get(), EnemyDefinition.Get());
	TStrongObjectPtr<UFinalCharacterDefinition> SourceCharacter = MakeCharacterDefinitionWithId(SourceCharacterId, TEXT("Source"));
	TStrongObjectPtr<UFinalCharacterDefinition> AllyCharacter = MakeCharacterDefinitionWithId(AllyCharacterId, TEXT("Ally"));

	TStrongObjectPtr<UFinalCardDefinition> ModifierCard = MakeApplyCardModifiersSkillCard(ModifierCardId, SourceCharacterId, TEXT("援锋成阵"), true);
	TStrongObjectPtr<UFinalCardDefinition> SourceAttackCard = MakeDamageCard(SourceAttackCardId, SourceCharacterId, TEXT("Source Attack"), 2, 4.0f);
	TStrongObjectPtr<UFinalCardDefinition> AllyAttackCard = MakeDamageCard(AllyAttackCardId, AllyCharacterId, TEXT("Ally Attack"), 2, 4.0f);
	TStrongObjectPtr<UFinalCardDefinition> AllyRetainedAttackCard = MakeRetainedDamageCard(AllyRetainedAttackCardId, AllyCharacterId, TEXT("Ally Retained Attack"), 2, 4.0f);
	TStrongObjectPtr<UFinalCardDefinition> AllySkillCard = MakeSkillCard(AllySkillCardId, AllyCharacterId, TEXT("Ally Skill"), 1);
	TStrongObjectPtr<UFinalCardDefinition> AllyDrawnAttackCard = MakeDamageCard(AllyDrawnAttackCardId, AllyCharacterId, TEXT("Ally Drawn Attack"), 2, 4.0f);

	ModifierCard->Keywords.AddTag(OpeningKeyword);
	SourceAttackCard->Keywords.AddTag(OpeningKeyword);
	AllyAttackCard->Keywords.AddTag(OpeningKeyword);
	AllyRetainedAttackCard->Keywords.AddTag(OpeningKeyword);
	AllySkillCard->Keywords.AddTag(OpeningKeyword);

	TStrongObjectPtr<UFinalBattleSession> Session = CreateTeamSessionWithDeck(
		EncounterDefinition.Get(),
		RuleConfig.Get(),
		TArray<UFinalCharacterDefinition*>{ SourceCharacter.Get(), AllyCharacter.Get() },
		TArray<UFinalCardDefinition*>{
			ModifierCard.Get(),
			SourceAttackCard.Get(),
			AllyAttackCard.Get(),
			AllyRetainedAttackCard.Get(),
			AllySkillCard.Get(),
			AllyDrawnAttackCard.Get()
		});

	FFinalBattleSnapshot Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* ModifierHandCard = FindHandCardById(Snapshot, ModifierCardId);
	if (!TestNotNull(TEXT("Opening modifier card should begin in hand."), ModifierHandCard))
	{
		return false;
	}

	FFinalBattleCommand ModifierCommand;
	ModifierCommand.CommandType = EFinalBattleCommandType::PlayCard;
	ModifierCommand.CardInstanceId = ModifierHandCard->CardInstanceId;
	TestTrue(TEXT("Playing the cross-character modifier card should succeed."), Session->SubmitCommand(ModifierCommand).EventType != EFinalBattleEventType::CommandRejected);

	Snapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* SourceAttackHandCard = FindHandCardById(Snapshot, SourceAttackCardId);
	const FFinalBattleCardViewData* AllyAttackHandCard = FindHandCardById(Snapshot, AllyAttackCardId);
	const FFinalBattleCardViewData* AllyRetainedAttackHandCard = FindHandCardById(Snapshot, AllyRetainedAttackCardId);
	const FFinalBattleCardViewData* AllySkillHandCard = FindHandCardById(Snapshot, AllySkillCardId);
	const FFinalBattleCardViewData* AllyDrawnAttackHandCard = FindHandCardById(Snapshot, AllyDrawnAttackCardId);
	if (!TestNotNull(TEXT("Source attack should remain in hand."), SourceAttackHandCard)
		|| !TestNotNull(TEXT("Ally attack should remain in hand."), AllyAttackHandCard)
		|| !TestNotNull(TEXT("Ally retained attack should remain in hand."), AllyRetainedAttackHandCard)
		|| !TestNotNull(TEXT("Ally skill should remain in hand."), AllySkillHandCard)
		|| !TestNotNull(TEXT("Ally drawn attack should enter hand after the modifier effect."), AllyDrawnAttackHandCard))
	{
		return false;
	}

	const FFinalBattleCardProjectionView SourceAttackProjection = Session->GetCardProjectionView(SourceAttackHandCard->CardInstanceId);
	const FFinalBattleCardProjectionView AllyAttackProjection = Session->GetCardProjectionView(AllyAttackHandCard->CardInstanceId);
	const FFinalBattleCardProjectionView AllyRetainedAttackProjection = Session->GetCardProjectionView(AllyRetainedAttackHandCard->CardInstanceId);
	const FFinalBattleCardProjectionView AllySkillProjection = Session->GetCardProjectionView(AllySkillHandCard->CardInstanceId);
	const FFinalBattleCardProjectionView AllyDrawnAttackProjection = Session->GetCardProjectionView(AllyDrawnAttackHandCard->CardInstanceId);

	TestEqual(TEXT("Source character's own attack should not be modified."), SourceAttackProjection.EffectiveCostAP, SourceAttackCard->BaseCostAP);
	TestEqual(TEXT("Source character's own attack damage should not be modified."), SourceAttackProjection.EffectiveOutgoingDamagePercent, 0);
	TestEqual(TEXT("Current ally attack should have AP reduced by 1."), AllyAttackProjection.EffectiveCostAP, AllyAttackCard->BaseCostAP - 1);
	TestEqual(TEXT("Current ally attack should gain +20% outgoing damage."), AllyAttackProjection.EffectiveOutgoingDamagePercent, 20);
	TestEqual(TEXT("Current retained ally attack should have AP reduced by 1."), AllyRetainedAttackProjection.EffectiveCostAP, AllyRetainedAttackCard->BaseCostAP - 1);
	TestEqual(TEXT("Current retained ally attack should gain +20% outgoing damage."), AllyRetainedAttackProjection.EffectiveOutgoingDamagePercent, 20);
	TestEqual(TEXT("Ally non-attack card should not be modified."), AllySkillProjection.EffectiveCostAP, AllySkillCard->BaseCostAP);
	TestEqual(TEXT("Ally non-attack card damage modifier should remain 0."), AllySkillProjection.EffectiveOutgoingDamagePercent, 0);
	TestEqual(TEXT("Attack drawn after the modifier pass should not receive the modifier."), AllyDrawnAttackProjection.EffectiveCostAP, AllyDrawnAttackCard->BaseCostAP);
	TestEqual(TEXT("Attack drawn after the modifier pass should not gain damage."), AllyDrawnAttackProjection.EffectiveOutgoingDamagePercent, 0);

	FFinalBattleCommand AllyAttackCommand;
	AllyAttackCommand.CommandType = EFinalBattleCommandType::PlayCard;
	AllyAttackCommand.CardInstanceId = AllyAttackHandCard->CardInstanceId;
	AllyAttackCommand.TargetUnitId = Snapshot.Enemies[0].RuntimeUnitId;
	TestTrue(TEXT("Playing the modified ally attack should succeed."), Session->SubmitCommand(AllyAttackCommand).EventType != EFinalBattleEventType::CommandRejected);

	const FFinalBattleCardProjectionView AllyAttackAfterPlayProjection = Session->GetCardProjectionView(AllyAttackHandCard->CardInstanceId);
	TestEqual(TEXT("UntilPlayed modifier should clear after modified ally attack is played."), AllyAttackAfterPlayProjection.ModifierCount, 0);
	TestEqual(TEXT("Modified ally attack cost should return to base after play cleanup."), AllyAttackAfterPlayProjection.EffectiveCostAP, AllyAttackCard->BaseCostAP);

	FFinalBattleCommand EndTurnCommand;
	EndTurnCommand.CommandType = EFinalBattleCommandType::EndTurn;
	TestTrue(TEXT("Ending the turn should succeed."), Session->SubmitCommand(EndTurnCommand).EventType != EFinalBattleEventType::CommandRejected);

	const FFinalBattleSnapshot AfterEndTurnSnapshot = Session->GetSnapshot();
	const FFinalBattleCardViewData* AllyRetainedAttackAfterEndTurn = FindHandCardById(AfterEndTurnSnapshot, AllyRetainedAttackCardId);
	if (!TestNotNull(TEXT("Retained ally attack should remain in hand after end turn."), AllyRetainedAttackAfterEndTurn))
	{
		return false;
	}

	const FFinalBattleCardProjectionView AllyRetainedAttackAfterEndTurnProjection = Session->GetCardProjectionView(AllyRetainedAttackAfterEndTurn->CardInstanceId);
	TestEqual(TEXT("Turn end should clear cross-character AP modifier from retained ally attack."), AllyRetainedAttackAfterEndTurnProjection.EffectiveCostAP, AllyRetainedAttackCard->BaseCostAP);
	TestEqual(TEXT("Turn end should clear cross-character damage modifier from retained ally attack."), AllyRetainedAttackAfterEndTurnProjection.EffectiveOutgoingDamagePercent, 0);
	return true;
}

#endif
