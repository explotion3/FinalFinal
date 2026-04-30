#include "Systems/FinalBattleTriggerService.h"

#include "Battle/Definitions/FinalCardDefinition.h"
#include "Runtime/FinalBattleCardInstance.h"
#include "Runtime/FinalBattleCharacterState.h"
#include "Runtime/FinalBattlePassiveInstance.h"
#include "Runtime/FinalBattleRelicRuntimeState.h"
#include "Runtime/FinalBattleState.h"
#include "Systems/FinalBattleCardService.h"
#include "Systems/FinalBattleConditionService.h"
#include "Systems/FinalBattleEffectExecutionService.h"
#include "Systems/FinalBattleEffectExecutionTypes.h"
#include "Systems/FinalBattleUnitService.h"

namespace
{
const FName OwnerTookHealthDamageTag(TEXT("battle.trigger.owner_took_health_damage"));
const FName BattleStartTag(TEXT("battle.trigger.battle_start"));
const FName PlayerTurnStartTag(TEXT("battle.trigger.player_turn_start"));
const FName PlayerTeamTookHealthDamageTag(TEXT("battle.trigger.player_team_took_health_damage"));
const FName PlayerCardResolvedTag(TEXT("battle.trigger.player_card_resolved"));
const FName RelicTriggeredModifierIdPrefix(TEXT("relic.trigger"));
const FName PassiveTriggeredModifierIdPrefix(TEXT("passive.trigger"));

const FFinalBattleCardService& GetCardService()
{
	static const FFinalBattleCardService CardService;
	return CardService;
}

bool CanTrigger(const FFinalBattleRuntimeTriggerState& TriggerState)
{
	switch (TriggerState.TriggerDefinition.Limit)
	{
	case EFinalRuntimeTriggerLimit::OncePerPlayerTurn:
		return TriggerState.TriggeredCountThisPlayerTurn <= 0;

	case EFinalRuntimeTriggerLimit::OncePerBattle:
		return TriggerState.TriggeredCountThisBattle <= 0;

	case EFinalRuntimeTriggerLimit::None:
	default:
		return true;
	}
}

void MarkTriggered(FFinalBattleRuntimeTriggerState& TriggerState)
{
	++TriggerState.TriggeredCountThisPlayerTurn;
	++TriggerState.TriggeredCountThisBattle;
}

FName ResolveTriggerWindowTag(const EFinalRuntimeTriggerWindow Window)
{
	switch (Window)
	{
	case EFinalRuntimeTriggerWindow::BattleStart:
		return BattleStartTag;

	case EFinalRuntimeTriggerWindow::PlayerTurnStart:
		return PlayerTurnStartTag;

	case EFinalRuntimeTriggerWindow::OwnerTookHealthDamage:
		return OwnerTookHealthDamageTag;

	case EFinalRuntimeTriggerWindow::PlayerTeamTookHealthDamage:
		return PlayerTeamTookHealthDamageTag;

	case EFinalRuntimeTriggerWindow::PlayerCardResolved:
		return PlayerCardResolvedTag;

	default:
		return NAME_None;
	}
}

FFinalBattleEvent BuildTriggeredEvent(
	const FFinalRelicId& RelicId,
	const FName RelatedTag,
	const FFinalBattleEffectExecutionSummary& Summary,
	const FText& Message)
{
	FFinalBattleEvent RelicEvent;
	RelicEvent.EventType = EFinalBattleEventType::RelicTriggered;
	RelicEvent.RelicId = RelicId;
	RelicEvent.RelatedTag = RelatedTag;
	RelicEvent.PrimaryValue = Summary.TotalCardsDrawn > 0
		? Summary.TotalCardsDrawn
		: (Summary.TotalTeamShieldGained > 0 ? Summary.TotalTeamShieldGained : Summary.ResolvedEffectCount);
	RelicEvent.SecondaryValue = Summary.TotalTeamShieldGained > 0
		? Summary.TotalTeamShieldGained
		: Summary.TotalDamageToEnemies;
	RelicEvent.Message = Message;
	return RelicEvent;
}

FFinalBattleEvent BuildPassiveTriggeredEvent(
	const FFinalBattlePassiveInstance& PassiveInstance,
	const FName RelatedTag)
{
	FFinalBattleEvent PassiveEvent;
	PassiveEvent.EventType = EFinalBattleEventType::PassiveTriggered;
	PassiveEvent.PassiveInstanceId = PassiveInstance.PassiveInstanceId;
	PassiveEvent.PassiveId = PassiveInstance.PassiveId;
	PassiveEvent.SourceUnitId = PassiveInstance.SourceUnitId;
	PassiveEvent.TargetUnitId = PassiveInstance.OwnerUnitId;
	PassiveEvent.RelatedTag = RelatedTag;
	PassiveEvent.PrimaryValue = PassiveInstance.CurrentStacks;
	PassiveEvent.SecondaryValue = PassiveInstance.RemainingDuration;
	PassiveEvent.Message = FText::Format(
		NSLOCTEXT("FinalBattleTriggerService", "PassiveTriggeredMessage", "被动触发：{0}。"),
		PassiveInstance.DisplayName.IsEmpty() ? FText::FromName(PassiveInstance.PassiveId.Value) : PassiveInstance.DisplayName);
	return PassiveEvent;
}

bool IsValidBattleRuntimeTrigger(const FFinalRuntimeTriggerDefinition& TriggerDefinition)
{
	return TriggerDefinition.Domain == EFinalRuntimeTriggerDomain::Battle
		&& TriggerDefinition.Window != EFinalRuntimeTriggerWindow::None
		&& (!TriggerDefinition.Effects.IsEmpty() || !TriggerDefinition.TriggeredCardModifiers.IsEmpty());
}

EFinalBattleCardModifierDuration ConvertTriggeredModifierDuration(
	const EFinalTriggeredCardModifierDurationPolicy DurationPolicy)
{
	switch (DurationPolicy)
	{
	case EFinalTriggeredCardModifierDurationPolicy::UntilPlayed:
		return EFinalBattleCardModifierDuration::UntilPlayed;

	case EFinalTriggeredCardModifierDurationPolicy::EndOfTurn:
		return EFinalBattleCardModifierDuration::EndOfTurn;

	case EFinalTriggeredCardModifierDurationPolicy::EndOfRound:
		return EFinalBattleCardModifierDuration::EndOfRound;

	case EFinalTriggeredCardModifierDurationPolicy::EndOfBattle:
		return EFinalBattleCardModifierDuration::EndOfBattle;

	case EFinalTriggeredCardModifierDurationPolicy::ManualClear:
	default:
		return EFinalBattleCardModifierDuration::ManualClear;
	}
}

bool MatchesTriggeredModifierCardFilter(
	const FFinalBattleCardInstance& CardInstance,
	const FFinalTriggeredCardModifierDefinition& ModifierDefinition)
{
	if (!ModifierDefinition.bRequireCardType)
	{
		return true;
	}

	const UFinalCardDefinition* EffectiveDefinition = CardInstance.ProjectedDefinition != nullptr
		? CardInstance.ProjectedDefinition
		: CardInstance.BaseDefinition;
	return EffectiveDefinition != nullptr
		&& EffectiveDefinition->CardType == ModifierDefinition.RequiredCardType;
}

void CollectTriggeredModifierTargetIds(
	const FFinalBattleState& BattleState,
	const FFinalBattleCardInstance& SourceCardInstance,
	const FFinalTriggeredCardModifierDefinition& ModifierDefinition,
	TArray<FGuid>& OutTargetCardInstanceIds)
{
	OutTargetCardInstanceIds.Reset();

	if (ModifierDefinition.bApplyToAllSameSourceRunCardInstances
		&& !SourceCardInstance.SourceRunCardInstanceId.IsNone())
	{
		for (const FFinalBattleCardInstance& Candidate : BattleState.CardInstances)
		{
			if (Candidate.SourceRunCardInstanceId == SourceCardInstance.SourceRunCardInstanceId)
			{
				OutTargetCardInstanceIds.AddUnique(Candidate.CardInstanceId);
			}
		}
		return;
	}

	OutTargetCardInstanceIds.Add(SourceCardInstance.CardInstanceId);
}

struct FTriggeredCardModifierSourceContext
{
	EFinalBattleCardModifierSourceType SourceType = EFinalBattleCardModifierSourceType::System;
	FName ModifierIdPrefix = NAME_None;
	FString SourceInstanceToken;
};

FName BuildTriggeredModifierId(
	const FTriggeredCardModifierSourceContext& SourceContext,
	const FGuid& TargetCardInstanceId)
{
	return FName(*FString::Printf(
		TEXT("%s.%s.%s"),
		*SourceContext.ModifierIdPrefix.ToString(),
		*SourceContext.SourceInstanceToken,
		*TargetCardInstanceId.ToString(EGuidFormats::DigitsWithHyphensLower)));
}

void ApplyTriggeredCardModifierRecord(
	FFinalBattleState& BattleState,
	const FTriggeredCardModifierSourceContext& SourceContext,
	const FFinalTriggeredCardModifierDefinition& ModifierDefinition,
	const TArray<FGuid>& TargetCardInstanceIds)
{
	if (TargetCardInstanceIds.IsEmpty())
	{
		return;
	}

	FFinalBattleCardModifierRecord ModifierRecord;
	ModifierRecord.SourceType = SourceContext.SourceType;
	ModifierRecord.DurationPolicy = ConvertTriggeredModifierDuration(ModifierDefinition.DurationPolicy);
	ModifierRecord.bExpireAtPlayerTurnEnd = ModifierDefinition.bExpireAtPlayerTurnEnd;
	ModifierRecord.ApplyOrder = 2000;
	ModifierRecord.CostDeltaAP = ModifierDefinition.CostDeltaAP;
	ModifierRecord.OutgoingDamagePercentDelta = ModifierDefinition.OutgoingDamagePercentDelta;

	for (const FGuid& TargetCardInstanceId : TargetCardInstanceIds)
	{
		ModifierRecord.ModifierId = BuildTriggeredModifierId(SourceContext, TargetCardInstanceId);
		GetCardService().RemoveCardModifier(BattleState, TargetCardInstanceId, BattleState.RuntimeProjectionOwner, ModifierRecord.ModifierId);
		GetCardService().AddCardModifier(BattleState, TargetCardInstanceId, BattleState.RuntimeProjectionOwner, ModifierRecord);
	}
}

void ApplyTriggeredCardModifierFromDrawnCard(
	FFinalBattleState& BattleState,
	const FTriggeredCardModifierSourceContext& SourceContext,
	const FFinalTriggeredCardModifierDefinition& ModifierDefinition,
	const FGuid& DrawnCardInstanceId)
{
	FFinalBattleCardInstance* DrawnCardInstance = GetCardService().FindCardInstance(BattleState, DrawnCardInstanceId);
	if (DrawnCardInstance == nullptr || !MatchesTriggeredModifierCardFilter(*DrawnCardInstance, ModifierDefinition))
	{
		return;
	}

	TArray<FGuid> TargetCardInstanceIds;
	CollectTriggeredModifierTargetIds(BattleState, *DrawnCardInstance, ModifierDefinition, TargetCardInstanceIds);
	if (TargetCardInstanceIds.IsEmpty())
	{
		return;
	}

	ApplyTriggeredCardModifierRecord(BattleState, SourceContext, ModifierDefinition, TargetCardInstanceIds);
}

void ApplyTriggeredCardModifierToCurrentOwnedHandCards(
	FFinalBattleState& BattleState,
	const FTriggeredCardModifierSourceContext& SourceContext,
	const FFinalTriggeredCardModifierDefinition& ModifierDefinition,
	const FFinalBattleCharacterState& SourceCharacterState)
{
	TArray<FGuid> TargetCardInstanceIds;
	for (const FGuid& HandCardInstanceId : BattleState.DeckState.HandCardInstanceIds)
	{
		const FFinalBattleCardInstance* HandCardInstance = GetCardService().FindCardInstance(BattleState, HandCardInstanceId);
		if (HandCardInstance == nullptr || HandCardInstance->RuntimeOwnerUnitId != SourceCharacterState.RuntimeUnitId)
		{
			continue;
		}

		if (!MatchesTriggeredModifierCardFilter(*HandCardInstance, ModifierDefinition))
		{
			continue;
		}

		TargetCardInstanceIds.AddUnique(HandCardInstance->CardInstanceId);
	}

	ApplyTriggeredCardModifierRecord(BattleState, SourceContext, ModifierDefinition, TargetCardInstanceIds);
}

void ApplyTriggeredCardModifierToCurrentAllyHandCards(
	FFinalBattleState& BattleState,
	const FTriggeredCardModifierSourceContext& SourceContext,
	const FFinalTriggeredCardModifierDefinition& ModifierDefinition,
	const FFinalBattleCharacterState& SourceCharacterState)
{
	TArray<FGuid> TargetCardInstanceIds;
	for (const FGuid& HandCardInstanceId : BattleState.DeckState.HandCardInstanceIds)
	{
		const FFinalBattleCardInstance* HandCardInstance = GetCardService().FindCardInstance(BattleState, HandCardInstanceId);
		if (HandCardInstance == nullptr
			|| HandCardInstance->RuntimeOwnerUnitId.IsNone()
			|| HandCardInstance->RuntimeOwnerUnitId == SourceCharacterState.RuntimeUnitId)
		{
			continue;
		}

		const bool bOwnedByPlayerCharacter = BattleState.Characters.ContainsByPredicate(
			[HandCardInstance](const FFinalBattleCharacterState& CharacterState)
			{
				return CharacterState.RuntimeUnitId == HandCardInstance->RuntimeOwnerUnitId;
			});
		if (!bOwnedByPlayerCharacter)
		{
			continue;
		}

		if (!MatchesTriggeredModifierCardFilter(*HandCardInstance, ModifierDefinition))
		{
			continue;
		}

		TargetCardInstanceIds.AddUnique(HandCardInstance->CardInstanceId);
	}

	ApplyTriggeredCardModifierRecord(BattleState, SourceContext, ModifierDefinition, TargetCardInstanceIds);
}

void ApplyTriggeredCardModifiers(
	FFinalBattleState& BattleState,
	const FTriggeredCardModifierSourceContext& SourceContext,
	const FFinalRuntimeTriggerDefinition& TriggerDefinition,
	const FFinalBattleEffectExecutionSummary& TriggerSummary,
	const FFinalBattleCharacterState* SourceCharacterState)
{
	if (TriggerDefinition.TriggeredCardModifiers.IsEmpty())
	{
		return;
	}

	for (const FFinalTriggeredCardModifierDefinition& ModifierDefinition : TriggerDefinition.TriggeredCardModifiers)
	{
		if (ModifierDefinition.CostDeltaAP == 0 && ModifierDefinition.OutgoingDamagePercentDelta == 0)
		{
			continue;
		}

		switch (ModifierDefinition.TargetSource)
		{
		case EFinalTriggeredCardModifierTargetSource::DrawnCardsFromExecutedEffects:
			for (const FGuid& DrawnCardInstanceId : TriggerSummary.DrawnCardInstanceIds)
			{
				ApplyTriggeredCardModifierFromDrawnCard(BattleState, SourceContext, ModifierDefinition, DrawnCardInstanceId);
			}
			break;

		case EFinalTriggeredCardModifierTargetSource::CurrentOwnedHandCards:
			if (SourceCharacterState != nullptr)
			{
				ApplyTriggeredCardModifierToCurrentOwnedHandCards(BattleState, SourceContext, ModifierDefinition, *SourceCharacterState);
			}
			break;

		case EFinalTriggeredCardModifierTargetSource::CurrentAllyHandCards:
			if (SourceCharacterState != nullptr)
			{
				ApplyTriggeredCardModifierToCurrentAllyHandCards(BattleState, SourceContext, ModifierDefinition, *SourceCharacterState);
			}
			break;

		case EFinalTriggeredCardModifierTargetSource::None:
		default:
			break;
		}
	}
}

FText BuildRelicTriggerMessage(
	const FText& DisplayName,
	const EFinalRuntimeTriggerWindow Window)
{
	switch (Window)
	{
	case EFinalRuntimeTriggerWindow::BattleStart:
		return FText::Format(
			NSLOCTEXT("FinalBattleTriggerService", "RelicBattleStart", "{0} triggered at battle start."),
			DisplayName);

	case EFinalRuntimeTriggerWindow::PlayerTurnStart:
		return FText::Format(
			NSLOCTEXT("FinalBattleTriggerService", "RelicPlayerTurnStart", "{0} triggered at player turn start."),
			DisplayName);

	case EFinalRuntimeTriggerWindow::PlayerTeamTookHealthDamage:
		return FText::Format(
			NSLOCTEXT("FinalBattleTriggerService", "RelicPlayerTeamTookHealthDamage", "{0} triggered after actual health loss."),
			DisplayName);

	case EFinalRuntimeTriggerWindow::PlayerCardResolved:
		return FText::Format(
			NSLOCTEXT("FinalBattleTriggerService", "RelicPlayerCardResolved", "{0} triggered after a card resolved."),
			DisplayName);

	default:
		return FText::Format(
			NSLOCTEXT("FinalBattleTriggerService", "RelicTriggerGeneric", "{0} triggered in the current battle window."),
			DisplayName);
	}
}

bool ExecuteRuntimeTriggerEffects(
	FFinalBattleState& BattleState,
	const FFinalRuntimeTriggerDefinition& TriggerDefinition,
	const FFinalBattleCharacterState* SourceCharacterState,
	const FFinalBattleResolvedCardTriggerContext* ResolvedCardContext,
	const FFinalBattleConditionService& ConditionService,
	const FFinalBattleEffectExecutionService& EffectExecutionService,
	const FFinalBattleUnitService& UnitService,
	FFinalBattleEffectExecutionSummary& OutSummary)
{
	FFinalBattleConditionEvaluationContext ConditionContext;
	ConditionContext.BattleState = &BattleState;
	ConditionContext.CardService = nullptr;
	ConditionContext.ResolvedCardContext = ResolvedCardContext;
	ConditionContext.SourceOwnerUnitId = SourceCharacterState != nullptr
		? SourceCharacterState->RuntimeUnitId
		: (ResolvedCardContext != nullptr ? ResolvedCardContext->RuntimeOwnerUnitId : NAME_None);

	if (!ConditionService.SatisfiesConditions(TriggerDefinition.Conditions, ConditionContext))
	{
		return false;
	}

	if (TriggerDefinition.Effects.IsEmpty())
	{
		return true;
	}

	return EffectExecutionService.ExecuteEffectList(
		BattleState,
		TriggerDefinition.Effects,
		nullptr,
		nullptr,
		SourceCharacterState,
		nullptr,
		UnitService,
		OutSummary);
}

void AccumulateExecutionSummary(
	FFinalBattleEffectExecutionSummary& InOutSummary,
	const FFinalBattleEffectExecutionSummary& TriggerSummary)
{
	InOutSummary.TotalDamageToEnemies += TriggerSummary.TotalDamageToEnemies;
	InOutSummary.TotalDamageToTeam += TriggerSummary.TotalDamageToTeam;
	InOutSummary.TotalBreakDamageToEnemies += TriggerSummary.TotalBreakDamageToEnemies;
	InOutSummary.TotalHealingToTeam += TriggerSummary.TotalHealingToTeam;
	InOutSummary.TotalEnemiesDefeated += TriggerSummary.TotalEnemiesDefeated;
	InOutSummary.TotalCriticalHits += TriggerSummary.TotalCriticalHits;
	InOutSummary.TotalCriticalBonusDamage += TriggerSummary.TotalCriticalBonusDamage;
	InOutSummary.TotalTeamShieldGained += TriggerSummary.TotalTeamShieldGained;
	InOutSummary.TotalEnemyShieldGained += TriggerSummary.TotalEnemyShieldGained;
	InOutSummary.TotalStatusStacksApplied += TriggerSummary.TotalStatusStacksApplied;
	InOutSummary.TotalStatusStacksRemoved += TriggerSummary.TotalStatusStacksRemoved;
	InOutSummary.TotalCardsDrawn += TriggerSummary.TotalCardsDrawn;
	InOutSummary.DrawnCardInstanceIds.Append(TriggerSummary.DrawnCardInstanceIds);
	InOutSummary.TotalAPGained += TriggerSummary.TotalAPGained;
	InOutSummary.ResolvedEffectCount += TriggerSummary.ResolvedEffectCount;
}
}

void FFinalBattleTriggerService::HandleBattlePhaseRuntimeTriggers(
	FFinalBattleState& BattleState,
	const EFinalRuntimeTriggerWindow Window,
	const FFinalBattleConditionService& ConditionService,
	const FFinalBattleEffectExecutionService& EffectExecutionService,
	const FFinalBattleUnitService& UnitService,
	TArray<FFinalBattleEvent>& OutGeneratedEvents) const
{
	for (FFinalBattleRelicRuntimeState& RuntimeState : BattleState.RelicRuntimeStates)
	{
		for (FFinalBattleRuntimeTriggerState& TriggerState : RuntimeState.TriggerStates)
		{
			const FFinalRuntimeTriggerDefinition& TriggerDefinition = TriggerState.TriggerDefinition;
			if (TriggerDefinition.Window != Window
				|| !IsValidBattleRuntimeTrigger(TriggerDefinition)
				|| !CanTrigger(TriggerState))
			{
				continue;
			}

			FFinalBattleEffectExecutionSummary TriggerSummary;
			if (!ExecuteRuntimeTriggerEffects(
				BattleState,
				TriggerDefinition,
				nullptr,
				nullptr,
				ConditionService,
				EffectExecutionService,
				UnitService,
				TriggerSummary))
			{
				continue;
			}

			const FTriggeredCardModifierSourceContext SourceContext{
				EFinalBattleCardModifierSourceType::Relic,
				RelicTriggeredModifierIdPrefix,
				RuntimeState.RelicId.Value.ToString()
			};
			ApplyTriggeredCardModifiers(BattleState, SourceContext, TriggerDefinition, TriggerSummary, nullptr);
			MarkTriggered(TriggerState);
			OutGeneratedEvents.Add(BuildTriggeredEvent(
				RuntimeState.RelicId,
				ResolveTriggerWindowTag(TriggerDefinition.Window),
				TriggerSummary,
				BuildRelicTriggerMessage(RuntimeState.DisplayName, TriggerDefinition.Window)));
		}
	}
}

void FFinalBattleTriggerService::HandleOwnerTookHealthDamage(
	FFinalBattleState& BattleState,
	const FFinalBattleUnitService& UnitService,
	const FFinalBattleConditionService& ConditionService,
	const FFinalBattleEffectExecutionService& EffectExecutionService,
	FFinalBattleEffectExecutionSummary& InOutSummary,
	TArray<FFinalBattleEvent>& OutGeneratedEvents) const
{
	for (FFinalBattlePassiveInstance& PassiveInstance : BattleState.PassiveInstances)
	{
		if (PassiveInstance.CurrentStacks <= 0)
		{
			continue;
		}

		const FFinalBattleCharacterState* OwnerCharacterState = UnitService.FindCharacterState(BattleState, PassiveInstance.OwnerUnitId);
		if (OwnerCharacterState == nullptr || OwnerCharacterState->bCollapsed)
		{
			continue;
		}

		for (FFinalBattleRuntimeTriggerState& TriggerState : PassiveInstance.TriggerStates)
		{
			const FFinalRuntimeTriggerDefinition& TriggerDefinition = TriggerState.TriggerDefinition;
			if (TriggerDefinition.Window != EFinalRuntimeTriggerWindow::OwnerTookHealthDamage
				|| !IsValidBattleRuntimeTrigger(TriggerDefinition)
				|| !CanTrigger(TriggerState))
			{
				continue;
			}

			FFinalBattleEffectExecutionSummary TriggerSummary;
			if (!ExecuteRuntimeTriggerEffects(
				BattleState,
				TriggerDefinition,
				OwnerCharacterState,
				nullptr,
				ConditionService,
				EffectExecutionService,
				UnitService,
				TriggerSummary))
			{
				continue;
			}

			MarkTriggered(TriggerState);
			AccumulateExecutionSummary(InOutSummary, TriggerSummary);
			OutGeneratedEvents.Add(BuildPassiveTriggeredEvent(PassiveInstance, ResolveTriggerWindowTag(TriggerDefinition.Window)));
		}
	}
}

void FFinalBattleTriggerService::HandlePlayerTeamTookHealthDamage(
	FFinalBattleState& BattleState,
	const int32 ActualHealthDamage,
	const FFinalBattleConditionService& ConditionService,
	const FFinalBattleEffectExecutionService& EffectExecutionService,
	const FFinalBattleUnitService& UnitService,
	TArray<FFinalBattleEvent>& OutGeneratedEvents) const
{
	if (ActualHealthDamage <= 0)
	{
		return;
	}

	for (FFinalBattleRelicRuntimeState& RuntimeState : BattleState.RelicRuntimeStates)
	{
		if (!RuntimeState.RelicId.IsValid())
		{
			continue;
		}

		for (FFinalBattleRuntimeTriggerState& TriggerState : RuntimeState.TriggerStates)
		{
			const FFinalRuntimeTriggerDefinition& TriggerDefinition = TriggerState.TriggerDefinition;
			if (TriggerDefinition.Window != EFinalRuntimeTriggerWindow::PlayerTeamTookHealthDamage
				|| !IsValidBattleRuntimeTrigger(TriggerDefinition)
				|| !CanTrigger(TriggerState))
			{
				continue;
			}

			FFinalBattleEffectExecutionSummary TriggerSummary;
			if (!ExecuteRuntimeTriggerEffects(
				BattleState,
				TriggerDefinition,
				nullptr,
				nullptr,
				ConditionService,
				EffectExecutionService,
				UnitService,
				TriggerSummary))
			{
				continue;
			}

			const FTriggeredCardModifierSourceContext SourceContext{
				EFinalBattleCardModifierSourceType::Relic,
				RelicTriggeredModifierIdPrefix,
				RuntimeState.RelicId.Value.ToString()
			};
			ApplyTriggeredCardModifiers(BattleState, SourceContext, TriggerDefinition, TriggerSummary, nullptr);
			MarkTriggered(TriggerState);
			OutGeneratedEvents.Add(BuildTriggeredEvent(
				RuntimeState.RelicId,
				ResolveTriggerWindowTag(TriggerDefinition.Window),
				TriggerSummary,
				BuildRelicTriggerMessage(
					RuntimeState.DisplayName.IsEmpty() ? FText::FromName(RuntimeState.DisplayId) : RuntimeState.DisplayName,
					TriggerDefinition.Window)));
		}
	}
}

void FFinalBattleTriggerService::HandlePlayerCardResolved(
	FFinalBattleState& BattleState,
	const FFinalBattleResolvedCardTriggerContext& CardContext,
	const FFinalBattleConditionService& ConditionService,
	const FFinalBattleEffectExecutionService& EffectExecutionService,
	const FFinalBattleUnitService& UnitService,
	TArray<FFinalBattleEvent>& OutGeneratedEvents) const
{
	if (!CardContext.CardId.IsValid() || CardContext.RuntimeOwnerUnitId.IsNone())
	{
		return;
	}

	for (FFinalBattleRelicRuntimeState& RuntimeState : BattleState.RelicRuntimeStates)
	{
		if (!RuntimeState.RelicId.IsValid())
		{
			continue;
		}

		for (FFinalBattleRuntimeTriggerState& TriggerState : RuntimeState.TriggerStates)
		{
			const FFinalRuntimeTriggerDefinition& TriggerDefinition = TriggerState.TriggerDefinition;
			if (TriggerDefinition.Window != EFinalRuntimeTriggerWindow::PlayerCardResolved
				|| !IsValidBattleRuntimeTrigger(TriggerDefinition)
				|| !CanTrigger(TriggerState))
			{
				continue;
			}

			const FFinalBattleCharacterState* SourceCharacterState = UnitService.FindCharacterState(BattleState, CardContext.RuntimeOwnerUnitId);
			FFinalBattleEffectExecutionSummary TriggerSummary;
			if (!ExecuteRuntimeTriggerEffects(
				BattleState,
				TriggerDefinition,
				SourceCharacterState,
				&CardContext,
				ConditionService,
				EffectExecutionService,
				UnitService,
				TriggerSummary))
			{
				continue;
			}

			const FTriggeredCardModifierSourceContext SourceContext{
				EFinalBattleCardModifierSourceType::Relic,
				RelicTriggeredModifierIdPrefix,
				RuntimeState.RelicId.Value.ToString()
			};
			ApplyTriggeredCardModifiers(BattleState, SourceContext, TriggerDefinition, TriggerSummary, SourceCharacterState);
			MarkTriggered(TriggerState);
			OutGeneratedEvents.Add(BuildTriggeredEvent(
				RuntimeState.RelicId,
				ResolveTriggerWindowTag(TriggerDefinition.Window),
				TriggerSummary,
				BuildRelicTriggerMessage(
					RuntimeState.DisplayName.IsEmpty() ? FText::FromName(RuntimeState.DisplayId) : RuntimeState.DisplayName,
					TriggerDefinition.Window)));
		}
	}

	for (FFinalBattlePassiveInstance& PassiveInstance : BattleState.PassiveInstances)
	{
		if (PassiveInstance.CurrentStacks <= 0)
		{
			continue;
		}

		const FFinalBattleCharacterState* SourceCharacterState = UnitService.FindCharacterState(BattleState, PassiveInstance.OwnerUnitId);
		if (SourceCharacterState == nullptr || SourceCharacterState->bCollapsed)
		{
			continue;
		}

		for (FFinalBattleRuntimeTriggerState& TriggerState : PassiveInstance.TriggerStates)
		{
			const FFinalRuntimeTriggerDefinition& TriggerDefinition = TriggerState.TriggerDefinition;
			if (TriggerDefinition.Window != EFinalRuntimeTriggerWindow::PlayerCardResolved
				|| !IsValidBattleRuntimeTrigger(TriggerDefinition)
				|| !CanTrigger(TriggerState))
			{
				continue;
			}

			FFinalBattleEffectExecutionSummary TriggerSummary;
			if (!ExecuteRuntimeTriggerEffects(
				BattleState,
				TriggerDefinition,
				SourceCharacterState,
				&CardContext,
				ConditionService,
				EffectExecutionService,
				UnitService,
				TriggerSummary))
			{
				continue;
			}

			const FTriggeredCardModifierSourceContext SourceContext{
				EFinalBattleCardModifierSourceType::Passive,
				PassiveTriggeredModifierIdPrefix,
				PassiveInstance.PassiveInstanceId.ToString(EGuidFormats::DigitsWithHyphensLower)
			};
			ApplyTriggeredCardModifiers(BattleState, SourceContext, TriggerDefinition, TriggerSummary, SourceCharacterState);
			MarkTriggered(TriggerState);
			OutGeneratedEvents.Add(BuildPassiveTriggeredEvent(PassiveInstance, ResolveTriggerWindowTag(TriggerDefinition.Window)));
		}
	}
}
