#include "Systems/FinalBattleTriggerService.h"

#include "Battle/Definitions/FinalCardDefinition.h"
#include "Runtime/FinalBattleCardInstance.h"
#include "Runtime/FinalBattleCharacterState.h"
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
const FName PlayerTeamTookHealthDamageTag(TEXT("battle.trigger.player_team_took_health_damage"));
const FName PlayerCardResolvedTag(TEXT("battle.trigger.player_card_resolved"));
const FName RelicTriggeredModifierIdPrefix(TEXT("relic.trigger"));

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

void ApplyTriggeredCardModifierDefinition(
	FFinalBattleState& BattleState,
	const FFinalRelicId& RelicId,
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

	FFinalBattleCardModifierRecord ModifierRecord;
	ModifierRecord.ModifierId = FName(*FString::Printf(
		TEXT("%s.%s.%s"),
		*RelicTriggeredModifierIdPrefix.ToString(),
		*RelicId.Value.ToString(),
		*DrawnCardInstanceId.ToString(EGuidFormats::DigitsWithHyphensLower)));
	ModifierRecord.SourceType = EFinalBattleCardModifierSourceType::Relic;
	ModifierRecord.DurationPolicy = ConvertTriggeredModifierDuration(ModifierDefinition.DurationPolicy);
	ModifierRecord.bExpireAtPlayerTurnEnd = ModifierDefinition.bExpireAtPlayerTurnEnd;
	ModifierRecord.ApplyOrder = 2000;
	ModifierRecord.CostDeltaAP = ModifierDefinition.CostDeltaAP;
	ModifierRecord.OutgoingDamagePercentDelta = ModifierDefinition.OutgoingDamagePercentDelta;

	for (const FGuid& TargetCardInstanceId : TargetCardInstanceIds)
	{
		GetCardService().RemoveCardModifier(BattleState, TargetCardInstanceId, BattleState.RuntimeProjectionOwner, ModifierRecord.ModifierId);
		GetCardService().AddCardModifier(BattleState, TargetCardInstanceId, BattleState.RuntimeProjectionOwner, ModifierRecord);
	}
}

void ApplyTriggeredCardModifiers(
	FFinalBattleState& BattleState,
	const FFinalRelicId& RelicId,
	const FFinalRuntimeTriggerDefinition& TriggerDefinition,
	const FFinalBattleEffectExecutionSummary& TriggerSummary)
{
	if (TriggerDefinition.TriggeredCardModifiers.IsEmpty() || TriggerSummary.DrawnCardInstanceIds.IsEmpty())
	{
		return;
	}

	for (const FFinalTriggeredCardModifierDefinition& ModifierDefinition : TriggerDefinition.TriggeredCardModifiers)
	{
		if (ModifierDefinition.TargetSource != EFinalTriggeredCardModifierTargetSource::DrawnCardsFromExecutedEffects)
		{
			continue;
		}

		if (ModifierDefinition.CostDeltaAP == 0 && ModifierDefinition.OutgoingDamagePercentDelta == 0)
		{
			continue;
		}

		for (const FGuid& DrawnCardInstanceId : TriggerSummary.DrawnCardInstanceIds)
		{
			ApplyTriggeredCardModifierDefinition(BattleState, RelicId, ModifierDefinition, DrawnCardInstanceId);
		}
	}
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
			InOutSummary.DrawnCardInstanceIds.Append(TriggerSummary.DrawnCardInstanceIds);
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

			ApplyTriggeredCardModifiers(BattleState, RuntimeState.RelicId, TriggerDefinition, TriggerSummary);
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
