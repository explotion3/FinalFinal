#include "Resolver/FinalBattleResolver.h"

#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalEnemyIntentDefinition.h"
#include "Battle/Definitions/FinalUltimateDefinition.h"
#include "Runtime/FinalBattleCardInstance.h"
#include "Runtime/FinalBattleCharacterState.h"
#include "Runtime/FinalBattleEnemyState.h"
#include "Runtime/FinalBattleState.h"
#include "Systems/FinalBattleCardService.h"
#include "Systems/FinalBattleEventService.h"
#include "Systems/FinalBattleEffectExecutionService.h"
#include "Systems/FinalBattleEffectExecutionTypes.h"
#include "Systems/FinalBattleInitializationService.h"
#include "Systems/FinalBattleRelicService.h"
#include "Systems/FinalBattleResourceService.h"
#include "Systems/FinalBattleStatusService.h"
#include "Systems/FinalBattleTurnService.h"
#include "Systems/FinalEnemyIntentService.h"

namespace
{
const FName TeamPlayerUnitId(TEXT("team_player"));
const FName RejectBattleResolvedTag(TEXT("battle.already_resolved"));
const FName RejectCardInstanceMissingTag(TEXT("battle.card_instance_missing"));
const FName RejectCardDefinitionMissingTag(TEXT("battle.card_definition_missing"));
const FName RejectCardNotInHandTag(TEXT("battle.card_not_in_hand"));
const FName RejectNotEnoughAPTag(TEXT("battle.not_enough_ap"));
const FName RejectUnsupportedCardEffectsTag(TEXT("battle.unsupported_card_effects"));
const FName RejectUltimateOwnerMissingTag(TEXT("battle.ultimate_owner_missing"));
const FName RejectUltimateBlockedByCollapseTag(TEXT("battle.ultimate_blocked_by_collapse"));
const FName RejectUltimateAlreadyUsedTag(TEXT("battle.ultimate_already_used"));
const FName RejectUltimateDefinitionMissingTag(TEXT("battle.ultimate_definition_unavailable"));
const FName RejectNotEnoughEPTag(TEXT("battle.not_enough_ep"));
const FName RejectInvalidTargetTag(TEXT("battle.invalid_target"));
const FName RejectUnsupportedCommandTag(TEXT("battle.unsupported_command"));


const FFinalBattleCharacterState* FindCharacterState(const FFinalBattleState& State, const FName RuntimeUnitId)
{
	return State.Characters.FindByPredicate(
		[&RuntimeUnitId](const FFinalBattleCharacterState& Candidate)
		{
			return Candidate.RuntimeUnitId == RuntimeUnitId;
		});
}

FFinalBattleEnemyState* FindEnemyState(FFinalBattleState& State, const FName RuntimeUnitId)
{
	return State.Enemies.FindByPredicate(
		[&RuntimeUnitId](const FFinalBattleEnemyState& Candidate)
		{
			return Candidate.RuntimeUnitId == RuntimeUnitId;
		});
}

bool AreAllEnemiesDefeated(const FFinalBattleState& State)
{
	if (State.Enemies.Num() == 0)
	{
		return false;
	}

	for (const FFinalBattleEnemyState& EnemyState : State.Enemies)
	{
		if (EnemyState.CurrentHP > 0)
		{
			return false;
		}
	}

	return true;
}

bool CanActivateUltimate(const FFinalBattleState& State, const FFinalBattleCharacterState& CharacterState)
{
	return CharacterState.UltimateDefinition != nullptr
		&& !CharacterState.bCollapsed
		&& !CharacterState.bUltimateUsedThisBattle
		&& CharacterState.UltimateId.IsValid()
		&& State.CurrentEP >= CharacterState.UltimateCostEP;
}

void SetRejectReason(FFinalBattleEvent& Event, const EFinalBattleCommandRejectReason RejectReason, const FName ReasonTag)
{
	Event.RejectReason = RejectReason;
	Event.ReasonTag = ReasonTag;
}

FFinalBattleEvent BuildRejectedCommandEvent(
	const EFinalBattleCommandRejectReason RejectReason,
	const FName ReasonTag,
	const FText& Message)
{
	FFinalBattleEvent Event;
	Event.EventType = EFinalBattleEventType::CommandRejected;
	SetRejectReason(Event, RejectReason, ReasonTag);
	Event.Message = Message;
	return Event;
}

FName ResolveCommandTargetUnitId(const FFinalBattleState& State, const FFinalBattleCommand& Command)
{
	return Command.TargetUnitId != NAME_None ? Command.TargetUnitId : State.CurrentTargetUnitId;
}

FFinalBattlePhaseProgressViewData BuildPhaseProgress(const FFinalBattleEnemyState& EnemyState)
{
	FFinalBattlePhaseProgressViewData PhaseProgress;
	PhaseProgress.TotalPhases = EnemyState.PhaseSequence.Num();

	if (!EnemyState.PhaseSequence.IsValidIndex(EnemyState.CurrentPhaseIndex) || EnemyState.MaxHP <= 0)
	{
		return PhaseProgress;
	}

	PhaseProgress.CurrentPhaseNumber = EnemyState.CurrentPhaseIndex + 1;

	const float CurrentHpPercent = FMath::Clamp(static_cast<float>(EnemyState.CurrentHP) / static_cast<float>(EnemyState.MaxHP), 0.0f, 1.0f);
	const float UpperBound = EnemyState.CurrentPhaseIndex > 0
		? EnemyState.PhaseSequence[EnemyState.CurrentPhaseIndex - 1].MaxHpPercent
		: 1.0f;
	const float LowerBound = EnemyState.CurrentPhaseIndex + 1 < EnemyState.PhaseSequence.Num()
		? EnemyState.PhaseSequence[EnemyState.CurrentPhaseIndex + 1].MaxHpPercent
		: 0.0f;
	const float PhaseSpan = FMath::Max(UpperBound - LowerBound, KINDA_SMALL_NUMBER);
	PhaseProgress.ProgressWithinPhase = FMath::Clamp((UpperBound - CurrentHpPercent) / PhaseSpan, 0.0f, 1.0f);
	return PhaseProgress;
}


void MarkBattleResolved(FFinalBattleState& State, const bool bPlayerVictory)
{
	State.bBattleEnded = true;
	State.bPlayerVictory = bPlayerVictory;
}

const FFinalEnemyIntentService& GetEnemyIntentService()
{
	static const FFinalEnemyIntentService IntentService;
	return IntentService;
}

const FFinalBattleEventService& GetEventService()
{
	static const FFinalBattleEventService EventService;
	return EventService;
}

const FFinalBattleCardService& GetCardService()
{
	static const FFinalBattleCardService CardService;
	return CardService;
}

const FFinalBattleRelicService& GetRelicService()
{
	static const FFinalBattleRelicService RelicService;
	return RelicService;
}

const FFinalBattleResourceService& GetResourceService()
{
	static const FFinalBattleResourceService ResourceService;
	return ResourceService;
}

const FFinalBattleTurnService& GetTurnService()
{
	static const FFinalBattleTurnService TurnService;
	return TurnService;
}

const FFinalBattleStatusService& GetStatusService()
{
	static const FFinalBattleStatusService StatusService;
	return StatusService;
}

const FFinalBattleEffectExecutionService& GetEffectExecutionService()
{
	static const FFinalBattleEffectExecutionService EffectExecutionService;
	return EffectExecutionService;
}

const FFinalBattleInitializationService& GetInitializationService()
{
	static const FFinalBattleInitializationService InitializationService;
	return InitializationService;
}

}

void FFinalBattleResolver::Initialize(FFinalBattleState& State, const UFinalBattleEncounterDefinition* EncounterDefinition, const UFinalBattleRuleConfig* RuleConfig, const FFinalBattleInitContext& InitContext) const
{
	GetInitializationService().InitializeBattle(
		State,
		EncounterDefinition,
		RuleConfig,
		InitContext,
		GetCardService(),
		GetEventService(),
		GetRelicService(),
		GetResourceService(),
		GetEnemyIntentService());
}

FFinalBattleEvent FFinalBattleResolver::ExecutePlayCardCommand(FFinalBattleState& State, const FFinalBattleCommand& Command, const UFinalBattleRuleConfig* RuleConfig) const
{
	FFinalBattleEvent Event;

	FFinalBattleCardInstance* CardInstance = GetCardService().FindCardInstance(State, Command.CardInstanceId);
	if (CardInstance == nullptr)
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::CardInstanceNotFound,
			RejectCardInstanceMissingTag,
			FText::FromString(TEXT("Card instance was not found.")));
		Event.CardInstanceId = Command.CardInstanceId;
		return GetEventService().FinalizeBattleEvent(State, Event);
	}

	if (CardInstance->SourceDefinition == nullptr)
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::CardDefinitionMissing,
			RejectCardDefinitionMissingTag,
			FText::FromString(TEXT("Card definition is missing for the selected card.")));
		Event.CardInstanceId = Command.CardInstanceId;
		Event.CardId = CardInstance->CardId;
		return GetEventService().FinalizeBattleEvent(State, Event);
	}

	if (!GetCardService().IsCardInHand(State, Command.CardInstanceId))
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::CardNotInHand,
			RejectCardNotInHandTag,
			FText::FromString(TEXT("Card instance is not in hand.")));
		Event.CardInstanceId = Command.CardInstanceId;
		Event.CardId = CardInstance->CardId;
		return GetEventService().FinalizeBattleEvent(State, Event);
	}

	if (!GetResourceService().HasEnoughAP(State, CardInstance->RuntimeCostAP))
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::NotEnoughAP,
			RejectNotEnoughAPTag,
			FText::FromString(TEXT("Not enough AP to play the selected card.")));
		Event.CardInstanceId = Command.CardInstanceId;
		Event.CardId = CardInstance->CardId;
		Event.PrimaryValue = CardInstance->RuntimeCostAP;
		Event.SecondaryValue = State.CurrentAP;
		return GetEventService().FinalizeBattleEvent(State, Event);
	}

	if (!GetEffectExecutionService().HasSupportedEffect(CardInstance->SourceDefinition))
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::UnsupportedCardEffects,
			RejectUnsupportedCardEffectsTag,
			FText::FromString(TEXT("Selected card has no supported effects.")));
		Event.CardInstanceId = Command.CardInstanceId;
		Event.CardId = CardInstance->CardId;
		return GetEventService().FinalizeBattleEvent(State, Event);
	}

	const FFinalBattleCharacterState* OwnerCharacterState = FindCharacterState(State, CardInstance->RuntimeOwnerUnitId);
	const UFinalCardDefinition* SourceCardDefinition = CardInstance->SourceDefinition;
	const FName ResolvedRuntimeOwnerUnitId = CardInstance->RuntimeOwnerUnitId;
	const FGuid ResolvedCardInstanceId = CardInstance->CardInstanceId;
	const FFinalCardId ResolvedCardId = CardInstance->CardId;

	FFinalBattleResolvedCardTriggerContext RelicCardContext;
	RelicCardContext.RuntimeOwnerUnitId = ResolvedRuntimeOwnerUnitId;
	RelicCardContext.CardId = ResolvedCardId;
	RelicCardContext.CardType = SourceCardDefinition->CardType;
	RelicCardContext.RuntimeCostAP = CardInstance->RuntimeCostAP;
	RelicCardContext.RuntimeKeywords = CardInstance->RuntimeKeywords;
	RelicCardContext.bGeneratedCard = CardInstance->bGeneratedCard;

	GetResourceService().SpendAP(State, CardInstance->RuntimeCostAP);
	GetResourceService().GainCardPlayEP(State, RuleConfig);
	GetCardService().MoveHandCardAfterPlay(State, Command.CardInstanceId);

	FFinalBattleEffectExecutionSummary Summary;
	GetEffectExecutionService().ExecuteEffectList(State, SourceCardDefinition->Effects, &Command, SourceCardDefinition, OwnerCharacterState, nullptr, Summary);

	TArray<FFinalBattleEvent> RelicEvents;
	GetRelicService().HandlePlayerCardResolved(State, RelicCardContext, GetCardService(), RelicEvents);
	for (const FFinalBattleEvent& RelicEvent : RelicEvents)
	{
		GetEventService().AppendBattleEvent(State, RelicEvent);
	}

	Event.EventType = EFinalBattleEventType::CardResolved;
	Event.SourceUnitId = ResolvedRuntimeOwnerUnitId;
	Event.TargetUnitId = ResolveCommandTargetUnitId(State, Command);
	Event.CardInstanceId = ResolvedCardInstanceId;
	Event.CardId = ResolvedCardId;
	Event.PrimaryValue = Summary.TotalDamageToEnemies;
	Event.SecondaryValue = Summary.TotalCardsDrawn;

	if (AreAllEnemiesDefeated(State))
	{
		MarkBattleResolved(State, true);
		Event.Message = FText::Format(
			NSLOCTEXT("FinalBattleResolver", "CardPlayVictory", "Resolved {0} effects. Damage {1}, Break {2}, Heal {3}, Shield {4}, Draw {5}, AP {6}. Battle won."),
			FText::AsNumber(Summary.ResolvedEffectCount),
			FText::AsNumber(Summary.TotalDamageToEnemies),
			FText::AsNumber(Summary.TotalBreakDamageToEnemies),
			FText::AsNumber(Summary.TotalHealingToTeam),
			FText::AsNumber(Summary.TotalTeamShieldGained),
			FText::AsNumber(Summary.TotalCardsDrawn),
			FText::AsNumber(Summary.TotalAPGained));
		return GetEventService().FinalizeBattleEvent(State, Event);
	}

	Event.Message = FText::Format(
		NSLOCTEXT("FinalBattleResolver", "CardPlayAccepted", "Resolved {0} effects. Damage {1}, Break {2}, Heal {3}, Shield {4}, Draw {5}, AP {6}."),
		FText::AsNumber(Summary.ResolvedEffectCount),
		FText::AsNumber(Summary.TotalDamageToEnemies),
		FText::AsNumber(Summary.TotalBreakDamageToEnemies),
		FText::AsNumber(Summary.TotalHealingToTeam),
		FText::AsNumber(Summary.TotalTeamShieldGained),
		FText::AsNumber(Summary.TotalCardsDrawn),
		FText::AsNumber(Summary.TotalAPGained));
	return GetEventService().FinalizeBattleEvent(State, Event);
}

FFinalBattleEvent FFinalBattleResolver::ExecutePlayUltimateCommand(FFinalBattleState& State, const FFinalBattleCommand& Command, const UFinalBattleRuleConfig* RuleConfig) const
{
	FFinalBattleEvent Event;

	const FFinalBattleCharacterState* OwnerCharacterState = FindCharacterState(State, Command.UltimateOwnerUnitId);
	if (OwnerCharacterState == nullptr)
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::UltimateOwnerNotFound,
			RejectUltimateOwnerMissingTag,
			FText::FromString(TEXT("Ultimate owner was not found.")));
		Event.SourceUnitId = Command.UltimateOwnerUnitId;
		return GetEventService().FinalizeBattleEvent(State, Event);
	}

	Event.SourceUnitId = OwnerCharacterState->RuntimeUnitId;
	Event.UltimateId = OwnerCharacterState->UltimateId;

	if (OwnerCharacterState->bCollapsed)
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::UltimateBlockedByCollapse,
			RejectUltimateBlockedByCollapseTag,
			FText::FromString(TEXT("Collapsed characters cannot use ultimates.")));
		return GetEventService().FinalizeBattleEvent(State, Event);
	}

	if (OwnerCharacterState->bUltimateUsedThisBattle)
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::UltimateAlreadyUsed,
			RejectUltimateAlreadyUsedTag,
			FText::FromString(TEXT("Ultimate was already used this battle.")));
		return GetEventService().FinalizeBattleEvent(State, Event);
	}

	if (OwnerCharacterState->UltimateDefinition == nullptr || !GetEffectExecutionService().HasSupportedEffect(OwnerCharacterState->UltimateDefinition))
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::UltimateDefinitionUnavailable,
			RejectUltimateDefinitionMissingTag,
			FText::FromString(TEXT("Ultimate definition is unavailable or has no supported effects.")));
		return GetEventService().FinalizeBattleEvent(State, Event);
	}

	if (!GetResourceService().HasEnoughEP(State, OwnerCharacterState->UltimateCostEP))
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::NotEnoughEP,
			RejectNotEnoughEPTag,
			FText::FromString(TEXT("Not enough EP to use the selected ultimate.")));
		Event.PrimaryValue = OwnerCharacterState->UltimateCostEP;
		Event.SecondaryValue = State.CurrentEP;
		return GetEventService().FinalizeBattleEvent(State, Event);
	}

	GetResourceService().SpendEP(State, OwnerCharacterState->UltimateCostEP);

	FFinalBattleEffectExecutionSummary Summary;
	GetEffectExecutionService().ExecuteEffectList(State, OwnerCharacterState->UltimateDefinition->Effects, &Command, nullptr, OwnerCharacterState, nullptr, Summary);

	Event.EventType = EFinalBattleEventType::UltimateResolved;
	Event.TargetUnitId = ResolveCommandTargetUnitId(State, Command);
	Event.PrimaryValue = Summary.TotalDamageToEnemies;
	Event.SecondaryValue = Summary.TotalTeamShieldGained;
	if (FFinalBattleCharacterState* MutableOwnerCharacterState = State.Characters.FindByPredicate(
		[&Command](const FFinalBattleCharacterState& Candidate)
		{
			return Candidate.RuntimeUnitId == Command.UltimateOwnerUnitId;
		}))
	{
		MutableOwnerCharacterState->bUltimateUsedThisBattle = true;
	}

	if (AreAllEnemiesDefeated(State))
	{
		MarkBattleResolved(State, true);
		Event.Message = FText::Format(
			NSLOCTEXT("FinalBattleResolver", "UltimateVictory", "Ultimate resolved. Damage {0}, Break {1}, Heal {2}, Shield {3}, AP {4}. Battle won."),
			FText::AsNumber(Summary.TotalDamageToEnemies),
			FText::AsNumber(Summary.TotalBreakDamageToEnemies),
			FText::AsNumber(Summary.TotalHealingToTeam),
			FText::AsNumber(Summary.TotalTeamShieldGained),
			FText::AsNumber(Summary.TotalAPGained));
		return GetEventService().FinalizeBattleEvent(State, Event);
	}

	Event.Message = FText::Format(
		NSLOCTEXT("FinalBattleResolver", "UltimateResolved", "Ultimate resolved. Damage {0}, Break {1}, Heal {2}, Shield {3}, AP {4}."),
		FText::AsNumber(Summary.TotalDamageToEnemies),
		FText::AsNumber(Summary.TotalBreakDamageToEnemies),
		FText::AsNumber(Summary.TotalHealingToTeam),
		FText::AsNumber(Summary.TotalTeamShieldGained),
		FText::AsNumber(Summary.TotalAPGained));
	return GetEventService().FinalizeBattleEvent(State, Event);
}

FFinalBattleEvent FFinalBattleResolver::ExecuteEndTurnCommand(FFinalBattleState& State, const FFinalBattleCommand& Command, const UFinalBattleRuleConfig* RuleConfig) const
{
	FFinalBattleEvent Event;

	const FFinalBattleEndTurnResult EndTurnResult = GetTurnService().ResolveEndTurn(
		State,
		RuleConfig,
		GetCardService(),
		GetRelicService(),
		GetResourceService(),
		GetStatusService(),
		[](FFinalBattleState& MutableState, FFinalBattleEnemyState& EnemyState) -> FFinalBattleEnemyActionResult
		{
			FFinalBattleEnemyActionResult ActionResult;
			FFinalBattleEffectExecutionSummary Summary;

			if (EnemyState.CurrentIntentDefinition && GetEffectExecutionService().HasSupportedEffectList(EnemyState.CurrentIntentDefinition->Effects))
			{
				GetEffectExecutionService().ExecuteEffectList(MutableState, EnemyState.CurrentIntentDefinition->Effects, nullptr, nullptr, nullptr, &EnemyState, Summary);
			}
			else
			{
				const int32 HpDamage = GetEffectExecutionService().ApplyTeamIncomingDamageAndTriggers(
					MutableState,
					FMath::Max(EnemyState.RuntimeDamagePower, 0),
					Summary);
				Summary.TotalDamageToTeam += HpDamage;
			}

			ActionResult.DamageToTeam = Summary.TotalDamageToTeam;
			ActionResult.EnemyShieldGained = Summary.TotalEnemyShieldGained;
			ActionResult.ResolvedEffectCount = Summary.ResolvedEffectCount;
			return ActionResult;
		});

	for (const FFinalBattleEvent& GeneratedEvent : EndTurnResult.GeneratedEvents)
	{
		GetEventService().AppendBattleEvent(State, GeneratedEvent);
	}

	if (EndTurnResult.bBattleLost)
	{
		MarkBattleResolved(State, false);
		Event.EventType = EFinalBattleEventType::BattleResolved;
		Event.TargetUnitId = TeamPlayerUnitId;
		Event.PrimaryValue = EndTurnResult.TotalDamageToTeam;
		Event.SecondaryValue = EndTurnResult.TotalEnemyShieldGained;
		Event.Message = FText::Format(
			NSLOCTEXT("FinalBattleResolver", "EndTurnDefeat", "Enemies resolved {0} effects. Team damage {1}, enemy shield {2}. Battle lost."),
			FText::AsNumber(EndTurnResult.ResolvedEffectCount),
			FText::AsNumber(EndTurnResult.TotalDamageToTeam),
			FText::AsNumber(EndTurnResult.TotalEnemyShieldGained));
		return GetEventService().FinalizeBattleEvent(State, Event);
	}

	Event.EventType = EFinalBattleEventType::TurnTransition;
	Event.TargetUnitId = TeamPlayerUnitId;
	Event.PrimaryValue = EndTurnResult.TotalDamageToTeam;
	Event.SecondaryValue = EndTurnResult.TotalEnemyShieldGained;
	Event.Message = FText::Format(
		NSLOCTEXT("FinalBattleResolver", "EndTurnAdvanced", "Turn advanced. Enemies resolved {0} effects. Team damage {1}, enemy shield {2}, team shield now {3}."),
		FText::AsNumber(EndTurnResult.ResolvedEffectCount),
		FText::AsNumber(EndTurnResult.TotalDamageToTeam),
		FText::AsNumber(EndTurnResult.TotalEnemyShieldGained),
		FText::AsNumber(State.TeamShield));
	Event.Round = State.CurrentRound;
	return GetEventService().FinalizeBattleEvent(State, Event);
}

FFinalBattleEvent FFinalBattleResolver::ExecuteSelectTargetCommand(FFinalBattleState& State, const FFinalBattleCommand& Command) const
{
	FFinalBattleEvent Event;

	FFinalBattleEnemyState* SelectedEnemy = FindEnemyState(State, Command.TargetUnitId);
	if (SelectedEnemy == nullptr || SelectedEnemy->CurrentHP <= 0)
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::InvalidTarget,
			RejectInvalidTargetTag,
			FText::FromString(TEXT("Target is not a valid living enemy.")));
		Event.TargetUnitId = Command.TargetUnitId;
		return GetEventService().FinalizeBattleEvent(State, Event);
	}

	State.CurrentTargetUnitId = Command.TargetUnitId;
	Event.EventType = EFinalBattleEventType::TargetChanged;
	Event.TargetUnitId = Command.TargetUnitId;
	Event.Message = FText::FromString(TEXT("Target updated."));
	return GetEventService().FinalizeBattleEvent(State, Event);
}

FFinalBattleEvent FFinalBattleResolver::ExecuteUnsupportedCommand(FFinalBattleState& State) const
{
	FFinalBattleEvent Event = BuildRejectedCommandEvent(
		EFinalBattleCommandRejectReason::UnsupportedCommand,
		RejectUnsupportedCommandTag,
		FText::FromString(TEXT("Unsupported command.")));
	return GetEventService().FinalizeBattleEvent(State, Event);
}

FFinalBattleEvent FFinalBattleResolver::ExecuteCommand(FFinalBattleState& State, const FFinalBattleCommand& Command, const UFinalBattleRuleConfig* RuleConfig) const
{
	FFinalBattleEvent Event;

	if (State.bBattleEnded)
	{
		Event = BuildRejectedCommandEvent(
			EFinalBattleCommandRejectReason::BattleAlreadyResolved,
			RejectBattleResolvedTag,
			FText::FromString(TEXT("Battle is already resolved.")));
		return GetEventService().FinalizeBattleEvent(State, Event);
	}

	switch (Command.CommandType)
	{
	case EFinalBattleCommandType::PlayCard:
		return ExecutePlayCardCommand(State, Command, RuleConfig);

	case EFinalBattleCommandType::PlayUltimate:
		return ExecutePlayUltimateCommand(State, Command, RuleConfig);

	case EFinalBattleCommandType::EndTurn:
		return ExecuteEndTurnCommand(State, Command, RuleConfig);

	case EFinalBattleCommandType::SelectTarget:
		return ExecuteSelectTargetCommand(State, Command);

	default:
		return ExecuteUnsupportedCommand(State);
	}
}

FFinalBattleSnapshot FFinalBattleResolver::BuildSnapshot(const FFinalBattleState& State) const
{
	FFinalBattleSnapshot Snapshot;
	Snapshot.BattleId = State.BattleId;
	Snapshot.EncounterId = State.EncounterId;
	Snapshot.RuleConfigId = State.RuleConfigId;
	Snapshot.EncounterDisplayName = State.EncounterDisplayName;
	Snapshot.CurrentRound = State.CurrentRound;
	Snapshot.CurrentAP = State.CurrentAP;
	Snapshot.CurrentEP = State.CurrentEP;
	Snapshot.MaxEP = State.MaxEP;
	Snapshot.TeamCurrentHP = State.TeamCurrentHP;
	Snapshot.TeamMaxHP = State.TeamMaxHP;
	Snapshot.TeamShield = State.TeamShield;
	Snapshot.bBattleEnded = State.bBattleEnded;
	Snapshot.bPlayerVictory = State.bPlayerVictory;
	Snapshot.CurrentTargetUnitId = State.CurrentTargetUnitId;
	Snapshot.DeckState.DrawPileCount = State.DeckState.DrawPileCardInstanceIds.Num();
	Snapshot.DeckState.HandCount = State.DeckState.HandCardInstanceIds.Num();
	Snapshot.DeckState.DiscardPileCount = State.DeckState.DiscardPileCardInstanceIds.Num();
	Snapshot.DeckState.OngoingZoneCount = State.DeckState.OngoingZoneCardInstanceIds.Num();
	Snapshot.DeckState.ConsumePileCount = State.DeckState.ConsumePileCardInstanceIds.Num();

	for (const FFinalBattleCharacterState& CharacterState : State.Characters)
	{
		FFinalBattleCharacterViewData CharacterView;
		CharacterView.RuntimeUnitId = CharacterState.RuntimeUnitId;
		CharacterView.CharacterId = CharacterState.CharacterId;
		CharacterView.DisplayName = CharacterState.DisplayName;
		CharacterView.CurrentStress = CharacterState.CurrentStress;
		CharacterView.StressCap = CharacterState.StressCap;
		CharacterView.bCollapsed = CharacterState.bCollapsed;
		CharacterView.CurrentAwakenCount = CharacterState.CurrentAwakenCount;
		CharacterView.CurrentAwakenThreshold = CharacterState.CurrentAwakenThreshold;
		CharacterView.CollapseCount = CharacterState.CollapseCount;
		CharacterView.VitalShare = CharacterState.VitalShare;
		Snapshot.Characters.Add(MoveTemp(CharacterView));

		FFinalBattleUltimateViewData UltimateView;
		UltimateView.OwnerUnitId = CharacterState.RuntimeUnitId;
		UltimateView.CharacterId = CharacterState.CharacterId;
		UltimateView.UltimateId = CharacterState.UltimateId;
		UltimateView.DisplayName = CharacterState.UltimateDisplayName;
		UltimateView.CostEP = CharacterState.UltimateCostEP;
		UltimateView.bDefinitionReady = CharacterState.UltimateDefinition != nullptr;
		UltimateView.bBlockedByCollapse = CharacterState.bCollapsed;
		UltimateView.bCanActivate = CanActivateUltimate(State, CharacterState);
		UltimateView.bUsedThisBattle = CharacterState.bUltimateUsedThisBattle;
		Snapshot.CharacterUltimates.Add(MoveTemp(UltimateView));
	}

	Snapshot.ActiveRelics = State.ActiveRelics;

	for (const FFinalBattleEnemyState& EnemyState : State.Enemies)
	{
		FFinalBattleEnemyViewData EnemyView;
		EnemyView.RuntimeUnitId = EnemyState.RuntimeUnitId;
		EnemyView.EnemyId = EnemyState.EnemyId;
		EnemyView.DisplayName = EnemyState.DisplayName;
		EnemyView.PositionIndex = EnemyState.PositionIndex;
		EnemyView.MaxHP = EnemyState.MaxHP;
		EnemyView.CurrentHP = EnemyState.CurrentHP;
		EnemyView.CurrentShield = EnemyState.CurrentShield;
		EnemyView.MaxBreakValue = EnemyState.MaxBreakValue;
		EnemyView.CurrentBreakValue = EnemyState.CurrentBreakValue;
		EnemyView.CurrentInitiative = EnemyState.CurrentInitiative;
		EnemyView.CurrentPhaseTag = EnemyState.CurrentPhaseTag;
		EnemyView.CurrentIntentId = EnemyState.CurrentIntentId;
		EnemyView.PhaseProgress = BuildPhaseProgress(EnemyState);
		EnemyView.IntentText = EnemyState.CurrentIntentText;
		EnemyView.bActedThisRound = EnemyState.bActedThisRound;
		Snapshot.Enemies.Add(MoveTemp(EnemyView));
	}

	GetStatusService().BuildStatusSnapshotData(State, Snapshot.CharacterStatuses, Snapshot.TeamStatuses, Snapshot.Statuses);
	GetCardService().BuildHandCardViews(State, Snapshot.HandCards);

	return Snapshot;
}

