#include "Systems/FinalBattleTriggerService.h"

#include "Battle/Definitions/FinalBattleTriggerDefinition.h"
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
const FName RelicTeamHealthDamageGainShieldTag(TEXT("battle.relic.trigger.player_team_took_health_damage.gain_shield"));
const FName RelicPlayerCardResolvedDrawCardsTag(TEXT("battle.relic.trigger.player_card_resolved.draw_cards"));
const FName RelicPlayerCardResolvedGainShieldTag(TEXT("battle.relic.trigger.player_card_resolved.gain_shield"));

bool CanTrigger(const FFinalBattleRelicRuntimeTriggerState& TriggerState)
{
	switch (TriggerState.TriggerDefinition.Limit)
	{
	case EFinalRelicTriggerLimit::OncePerPlayerTurn:
		return TriggerState.TriggeredCountThisPlayerTurn <= 0;

	case EFinalRelicTriggerLimit::OncePerBattle:
		return TriggerState.TriggeredCountThisBattle <= 0;

	case EFinalRelicTriggerLimit::None:
	default:
		return true;
	}
}

void MarkTriggered(FFinalBattleRelicRuntimeTriggerState& TriggerState)
{
	++TriggerState.TriggeredCountThisPlayerTurn;
	++TriggerState.TriggeredCountThisBattle;
}

FFinalBattleEvent BuildTriggeredEvent(
	const FFinalRelicId& RelicId,
	const FName RelatedTag,
	const int32 PrimaryValue,
	const int32 SecondaryValue,
	const FText& Message)
{
	FFinalBattleEvent RelicEvent;
	RelicEvent.EventType = EFinalBattleEventType::RelicTriggered;
	RelicEvent.RelicId = RelicId;
	RelicEvent.RelatedTag = RelatedTag;
	RelicEvent.PrimaryValue = PrimaryValue;
	RelicEvent.SecondaryValue = SecondaryValue;
	RelicEvent.Message = Message;
	return RelicEvent;
}

bool IsValidRuntimeTriggerEffect(const FFinalRelicRuntimeTriggerEffectDefinition& EffectDefinition)
{
	return EffectDefinition.EffectType != EFinalRelicTriggerEffectType::None
		&& EffectDefinition.Value > 0;
}
}

void FFinalBattleTriggerService::HandleOwnerTookHealthDamage(
	FFinalBattleState& BattleState,
	const FFinalBattleUnitService& UnitService,
	const FFinalBattleEffectExecutionService& EffectExecutionService,
	FFinalBattleEffectExecutionSummary& InOutSummary) const
{
	for (const FFinalBattleCharacterState& CharacterState : BattleState.Characters)
	{
		if (CharacterState.bCollapsed)
		{
			continue;
		}

		for (const FFinalBattleTriggerDefinition& TriggerDefinition : CharacterState.BattleTriggers)
		{
			if (TriggerDefinition.TriggerWindow != EFinalBattleTriggerWindow::OwnerTookHealthDamage
				|| TriggerDefinition.Effects.IsEmpty())
			{
				continue;
			}

			EffectExecutionService.ExecuteEffectList(
				BattleState,
				TriggerDefinition.Effects,
				nullptr,
				nullptr,
				&CharacterState,
				nullptr,
				UnitService,
				InOutSummary);
		}
	}
}

void FFinalBattleTriggerService::HandlePlayerTeamTookHealthDamage(
	FFinalBattleState& BattleState,
	const int32 ActualHealthDamage,
	const FFinalBattleCardService& CardService,
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

		for (FFinalBattleRelicRuntimeTriggerState& TriggerState : RuntimeState.TriggerStates)
		{
			const FFinalRelicRuntimeTriggerDefinition& TriggerDefinition = TriggerState.TriggerDefinition;
			if (TriggerDefinition.Domain != EFinalRelicTriggerDomain::Battle
				|| TriggerDefinition.Window != EFinalRelicTriggerWindow::PlayerTeamTookHealthDamage
				|| !CanTrigger(TriggerState))
			{
				continue;
			}

			bool bAppliedAnyEffect = false;
			for (const FFinalRelicRuntimeTriggerEffectDefinition& EffectDefinition : TriggerDefinition.Effects)
			{
				if (!IsValidRuntimeTriggerEffect(EffectDefinition))
				{
					continue;
				}

				switch (EffectDefinition.EffectType)
				{
				case EFinalRelicTriggerEffectType::GainShield:
					BattleState.TeamShield += EffectDefinition.Value;
					OutGeneratedEvents.Add(BuildTriggeredEvent(
						RuntimeState.RelicId,
						RelicTeamHealthDamageGainShieldTag,
						EffectDefinition.Value,
						BattleState.TeamShield,
						FText::Format(
							NSLOCTEXT("FinalBattleTriggerService", "RelicTeamHealthDamageGainShield", "{0} triggered after actual health loss and granted {1} shield."),
							RuntimeState.DisplayName.IsEmpty() ? FText::FromName(RuntimeState.DisplayId) : RuntimeState.DisplayName,
							FText::AsNumber(EffectDefinition.Value))));
					bAppliedAnyEffect = true;
					break;

				case EFinalRelicTriggerEffectType::DrawCards:
					{
						const int32 DrawnCount = CardService.DrawCards(BattleState, EffectDefinition.Value);
						OutGeneratedEvents.Add(BuildTriggeredEvent(
							RuntimeState.RelicId,
							RelicPlayerCardResolvedDrawCardsTag,
							DrawnCount,
							BattleState.DeckState.HandCardInstanceIds.Num(),
							FText::Format(
								NSLOCTEXT("FinalBattleTriggerService", "RelicTeamHealthDamageDrawCards", "{0} triggered after actual health loss and drew {1} card(s)."),
								RuntimeState.DisplayName.IsEmpty() ? FText::FromName(RuntimeState.DisplayId) : RuntimeState.DisplayName,
								FText::AsNumber(DrawnCount))));
						bAppliedAnyEffect = true;
						break;
					}

				default:
					break;
				}
			}

			if (bAppliedAnyEffect)
			{
				MarkTriggered(TriggerState);
			}
		}
	}
}

void FFinalBattleTriggerService::HandlePlayerCardResolved(
	FFinalBattleState& BattleState,
	const FFinalBattleResolvedCardTriggerContext& CardContext,
	const FFinalBattleCardService& CardService,
	const FFinalBattleConditionService& ConditionService,
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

		for (FFinalBattleRelicRuntimeTriggerState& TriggerState : RuntimeState.TriggerStates)
		{
			const FFinalRelicRuntimeTriggerDefinition& TriggerDefinition = TriggerState.TriggerDefinition;
			if (TriggerDefinition.Domain != EFinalRelicTriggerDomain::Battle
				|| TriggerDefinition.Window != EFinalRelicTriggerWindow::PlayerCardResolved
				|| !CanTrigger(TriggerState)
				|| !ConditionService.SatisfiesResolvedCardCondition(TriggerDefinition.CardCondition, CardContext))
			{
				continue;
			}

			bool bAppliedAnyEffect = false;
			for (const FFinalRelicRuntimeTriggerEffectDefinition& EffectDefinition : TriggerDefinition.Effects)
			{
				if (!IsValidRuntimeTriggerEffect(EffectDefinition))
				{
					continue;
				}

				switch (EffectDefinition.EffectType)
				{
				case EFinalRelicTriggerEffectType::DrawCards:
					{
						const int32 DrawnCount = CardService.DrawCards(BattleState, EffectDefinition.Value);
						OutGeneratedEvents.Add(BuildTriggeredEvent(
							RuntimeState.RelicId,
							RelicPlayerCardResolvedDrawCardsTag,
							DrawnCount,
							BattleState.DeckState.HandCardInstanceIds.Num(),
							FText::Format(
								NSLOCTEXT("FinalBattleTriggerService", "RelicPlayerCardResolvedDrawCards", "{0} triggered after a card resolved and drew {1} card(s)."),
								RuntimeState.DisplayName.IsEmpty() ? FText::FromName(RuntimeState.DisplayId) : RuntimeState.DisplayName,
								FText::AsNumber(DrawnCount))));
						bAppliedAnyEffect = true;
						break;
					}

				case EFinalRelicTriggerEffectType::GainShield:
					BattleState.TeamShield += EffectDefinition.Value;
					OutGeneratedEvents.Add(BuildTriggeredEvent(
						RuntimeState.RelicId,
						RelicPlayerCardResolvedGainShieldTag,
						EffectDefinition.Value,
						BattleState.TeamShield,
						FText::Format(
							NSLOCTEXT("FinalBattleTriggerService", "RelicPlayerCardResolvedGainShield", "{0} triggered after a card resolved and granted {1} shield."),
							RuntimeState.DisplayName.IsEmpty() ? FText::FromName(RuntimeState.DisplayId) : RuntimeState.DisplayName,
							FText::AsNumber(EffectDefinition.Value))));
					bAppliedAnyEffect = true;
					break;

				default:
					break;
				}
			}

			if (bAppliedAnyEffect)
			{
				MarkTriggered(TriggerState);
			}
		}
	}
}
