#include "Systems/FinalBattleTriggerService.h"

#include "Runtime/FinalBattleCharacterState.h"
#include "Runtime/FinalBattleRelicRuntimeState.h"
#include "Runtime/FinalBattleState.h"
#include "Systems/FinalBattleConditionService.h"
#include "Systems/FinalBattleEffectExecutionService.h"
#include "Systems/FinalBattleEffectExecutionTypes.h"
#include "Systems/FinalBattleUnitService.h"

namespace
{
const FName OwnerTookHealthDamageTag(TEXT("battle.trigger.owner_took_health_damage"));
const FName PlayerTeamTookHealthDamageTag(TEXT("battle.trigger.player_team_took_health_damage"));
const FName PlayerCardResolvedTag(TEXT("battle.trigger.player_card_resolved"));

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

bool IsValidBattleRuntimeTrigger(const FFinalRuntimeTriggerDefinition& TriggerDefinition)
{
	return TriggerDefinition.Domain == EFinalRuntimeTriggerDomain::Battle
		&& TriggerDefinition.Window != EFinalRuntimeTriggerWindow::None
		&& !TriggerDefinition.Effects.IsEmpty();
}

FText BuildRelicTriggerMessage(
	const FText& DisplayName,
	const EFinalRuntimeTriggerWindow Window)
{
	switch (Window)
	{
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
}

void FFinalBattleTriggerService::HandleOwnerTookHealthDamage(
	FFinalBattleState& BattleState,
	const FFinalBattleUnitService& UnitService,
	const FFinalBattleConditionService& ConditionService,
	const FFinalBattleEffectExecutionService& EffectExecutionService,
	FFinalBattleEffectExecutionSummary& InOutSummary) const
{
	for (FFinalBattleCharacterState& CharacterState : BattleState.Characters)
	{
		if (CharacterState.bCollapsed)
		{
			continue;
		}

		for (FFinalBattleRuntimeTriggerState& TriggerState : CharacterState.TriggerStates)
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
				&CharacterState,
				nullptr,
				ConditionService,
				EffectExecutionService,
				UnitService,
				TriggerSummary))
			{
				continue;
			}

			MarkTriggered(TriggerState);
			InOutSummary.TotalDamageToEnemies += TriggerSummary.TotalDamageToEnemies;
			InOutSummary.TotalDamageToTeam += TriggerSummary.TotalDamageToTeam;
			InOutSummary.TotalBreakDamageToEnemies += TriggerSummary.TotalBreakDamageToEnemies;
			InOutSummary.TotalHealingToTeam += TriggerSummary.TotalHealingToTeam;
			InOutSummary.TotalTeamShieldGained += TriggerSummary.TotalTeamShieldGained;
			InOutSummary.TotalEnemyShieldGained += TriggerSummary.TotalEnemyShieldGained;
			InOutSummary.TotalStatusStacksApplied += TriggerSummary.TotalStatusStacksApplied;
			InOutSummary.TotalStatusStacksRemoved += TriggerSummary.TotalStatusStacksRemoved;
			InOutSummary.TotalCardsDrawn += TriggerSummary.TotalCardsDrawn;
			InOutSummary.TotalAPGained += TriggerSummary.TotalAPGained;
			InOutSummary.ResolvedEffectCount += TriggerSummary.ResolvedEffectCount;
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
