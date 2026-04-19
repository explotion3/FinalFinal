#include "Systems/FinalBattleRelicService.h"

#include "Runtime/FinalBattleRelicRuntimeState.h"
#include "Runtime/FinalBattleState.h"
#include "Systems/FinalBattleCardService.h"

namespace
{
const FName RelicGainAPTag(TEXT("battle.relic.effect.gain_ap"));
const FName RelicGainShieldTag(TEXT("battle.relic.effect.gain_shield"));
const FName RelicTurnStartGainAPTag(TEXT("battle.relic.effect.turn_start_gain_ap"));
const FName RelicTurnStartGainShieldTag(TEXT("battle.relic.effect.turn_start_gain_shield"));
const FName RelicTeamHealthDamageGainShieldTag(TEXT("battle.relic.trigger.player_team_took_health_damage.gain_shield"));
const FName RelicPlayerCardResolvedDrawCardsTag(TEXT("battle.relic.trigger.player_card_resolved.draw_cards"));

FText ResolveBattleRelicDisplayName(const FFinalBattleStartRelicInput& RelicInput)
{
	if (!RelicInput.DisplayName.IsEmpty())
	{
		return RelicInput.DisplayName;
	}

	if (!RelicInput.DisplayId.IsNone())
	{
		return FText::FromName(RelicInput.DisplayId);
	}

	return RelicInput.RelicId.IsValid()
		? FText::FromName(RelicInput.RelicId.Value)
		: NSLOCTEXT("FinalBattleRelicService", "UnknownRelic", "Unknown Relic");
}

FFinalBattleEvent BuildRelicTriggeredEvent(
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

bool IsSupportedBattleRuntimeTrigger(const FFinalRelicRuntimeTriggerDefinition& TriggerDefinition)
{
	if (TriggerDefinition.Domain != EFinalRelicTriggerDomain::Battle
		|| TriggerDefinition.Window == EFinalRelicTriggerWindow::None
		|| TriggerDefinition.Effects.IsEmpty())
	{
		return false;
	}

	for (const FFinalRelicRuntimeTriggerEffectDefinition& EffectDefinition : TriggerDefinition.Effects)
	{
		if (IsValidRuntimeTriggerEffect(EffectDefinition))
		{
			return true;
		}
	}

	return false;
}

bool CanTriggerRuntimeRelicEffect(const FFinalBattleRelicRuntimeTriggerState& TriggerState)
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

bool SatisfiesCardCondition(
	const FFinalRelicRuntimeCardConditionDefinition& CardCondition,
	const FFinalBattleResolvedCardTriggerContext& CardContext)
{
	if (CardCondition.bRequireCardCostAP && CardContext.RuntimeCostAP != CardCondition.RequiredCardCostAP)
	{
		return false;
	}

	if (CardCondition.bRequireCardType && CardContext.CardType != CardCondition.RequiredCardType)
	{
		return false;
	}

	if (CardCondition.RequiredKeyword.IsValid() && !CardContext.RuntimeKeywords.HasTagExact(CardCondition.RequiredKeyword))
	{
		return false;
	}

	return true;
}
}

void FFinalBattleRelicService::InitializeRelics(
	FFinalBattleState& BattleState,
	TArray<FFinalBattleStartRelicInput> ActiveRelics,
	TArray<FFinalBattleEvent>& OutGeneratedEvents) const
{
	BattleState.ActiveRelics.Reset();
	BattleState.RelicRuntimeStates.Reset();

	for (FFinalBattleStartRelicInput& RelicInput : ActiveRelics)
	{
		if (!RelicInput.RelicId.IsValid())
		{
			continue;
		}

		if (RelicInput.DisplayId.IsNone())
		{
			RelicInput.DisplayId = RelicInput.RelicId.Value;
		}

		if (RelicInput.DisplayName.IsEmpty())
		{
			RelicInput.DisplayName = ResolveBattleRelicDisplayName(RelicInput);
		}

		TArray<FFinalBattleStartRelicEffectInput> ValidBattleStartEffects;
		TArray<FFinalBattlePlayerTurnStartRelicEffectInput> ValidTurnStartEffects;
		TArray<FFinalRelicRuntimeTriggerDefinition> ValidRuntimeTriggers;

		for (const FFinalBattleStartRelicEffectInput& EffectInput : RelicInput.BattleStartEffects)
		{
			if (EffectInput.Value <= 0 || EffectInput.EffectType == EFinalRelicBattleStartEffectType::None)
			{
				continue;
			}

			switch (EffectInput.EffectType)
			{
			case EFinalRelicBattleStartEffectType::GainAP:
				BattleState.CurrentAP += EffectInput.Value;
				OutGeneratedEvents.Add(BuildRelicTriggeredEvent(
					RelicInput.RelicId,
					RelicGainAPTag,
					EffectInput.Value,
					BattleState.CurrentAP,
					FText::Format(
						NSLOCTEXT("FinalBattleRelicService", "RelicGainAP", "{0} triggered at battle start and granted {1} AP."),
						RelicInput.DisplayName,
						FText::AsNumber(EffectInput.Value))));
				break;

			case EFinalRelicBattleStartEffectType::GainShield:
				BattleState.TeamShield += EffectInput.Value;
				OutGeneratedEvents.Add(BuildRelicTriggeredEvent(
					RelicInput.RelicId,
					RelicGainShieldTag,
					EffectInput.Value,
					BattleState.TeamShield,
					FText::Format(
						NSLOCTEXT("FinalBattleRelicService", "RelicGainShield", "{0} triggered at battle start and granted {1} shield."),
						RelicInput.DisplayName,
						FText::AsNumber(EffectInput.Value))));
				break;

			default:
				continue;
			}

			ValidBattleStartEffects.Add(EffectInput);
		}

		for (const FFinalBattlePlayerTurnStartRelicEffectInput& EffectInput : RelicInput.PlayerTurnStartEffects)
		{
			if (EffectInput.Value <= 0 || EffectInput.EffectType == EFinalRelicPlayerTurnStartEffectType::None)
			{
				continue;
			}

			ValidTurnStartEffects.Add(EffectInput);
		}

		for (const FFinalRelicRuntimeTriggerDefinition& TriggerDefinition : RelicInput.RuntimeTriggers)
		{
			if (!IsSupportedBattleRuntimeTrigger(TriggerDefinition))
			{
				continue;
			}

			FFinalRelicRuntimeTriggerDefinition ValidTrigger = TriggerDefinition;
			ValidTrigger.Effects.Reset();
			for (const FFinalRelicRuntimeTriggerEffectDefinition& EffectDefinition : TriggerDefinition.Effects)
			{
				if (IsValidRuntimeTriggerEffect(EffectDefinition))
				{
					ValidTrigger.Effects.Add(EffectDefinition);
				}
			}
			ValidRuntimeTriggers.Add(MoveTemp(ValidTrigger));
		}

		if (ValidBattleStartEffects.IsEmpty()
			&& ValidTurnStartEffects.IsEmpty()
			&& ValidRuntimeTriggers.IsEmpty())
		{
			continue;
		}

		RelicInput.BattleStartEffects = MoveTemp(ValidBattleStartEffects);
		RelicInput.PlayerTurnStartEffects = MoveTemp(ValidTurnStartEffects);
		RelicInput.RuntimeTriggers = ValidRuntimeTriggers;
		BattleState.ActiveRelics.Add(RelicInput);

		if (!ValidRuntimeTriggers.IsEmpty())
		{
			FFinalBattleRelicRuntimeState& RuntimeState = BattleState.RelicRuntimeStates.AddDefaulted_GetRef();
			RuntimeState.RelicId = RelicInput.RelicId;
			RuntimeState.DisplayId = RelicInput.DisplayId;
			RuntimeState.DisplayName = RelicInput.DisplayName;
			RuntimeState.RuntimeTriggers = MoveTemp(ValidRuntimeTriggers);

			for (const FFinalRelicRuntimeTriggerDefinition& TriggerDefinition : RuntimeState.RuntimeTriggers)
			{
				FFinalBattleRelicRuntimeTriggerState& TriggerState = RuntimeState.TriggerStates.AddDefaulted_GetRef();
				TriggerState.TriggerDefinition = TriggerDefinition;
			}
		}
	}
}

void FFinalBattleRelicService::ApplyPlayerTurnStartRelicEffects(
	FFinalBattleState& BattleState,
	TArray<FFinalBattleEvent>& OutGeneratedEvents) const
{
	for (const FFinalBattleStartRelicInput& RelicInput : BattleState.ActiveRelics)
	{
		if (!RelicInput.RelicId.IsValid() || RelicInput.PlayerTurnStartEffects.IsEmpty())
		{
			continue;
		}

		const FText RelicDisplayName = ResolveBattleRelicDisplayName(RelicInput);
		for (const FFinalBattlePlayerTurnStartRelicEffectInput& EffectInput : RelicInput.PlayerTurnStartEffects)
		{
			if (EffectInput.Value <= 0 || EffectInput.EffectType == EFinalRelicPlayerTurnStartEffectType::None)
			{
				continue;
			}

			switch (EffectInput.EffectType)
			{
			case EFinalRelicPlayerTurnStartEffectType::GainAP:
				BattleState.CurrentAP += EffectInput.Value;
				OutGeneratedEvents.Add(BuildRelicTriggeredEvent(
					RelicInput.RelicId,
					RelicTurnStartGainAPTag,
					EffectInput.Value,
					BattleState.CurrentAP,
					FText::Format(
						NSLOCTEXT("FinalBattleRelicService", "RelicTurnStartGainAP", "{0} triggered at player turn start and granted {1} AP."),
						RelicDisplayName,
						FText::AsNumber(EffectInput.Value))));
				break;

			case EFinalRelicPlayerTurnStartEffectType::GainShield:
				BattleState.TeamShield += EffectInput.Value;
				OutGeneratedEvents.Add(BuildRelicTriggeredEvent(
					RelicInput.RelicId,
					RelicTurnStartGainShieldTag,
					EffectInput.Value,
					BattleState.TeamShield,
					FText::Format(
						NSLOCTEXT("FinalBattleRelicService", "RelicTurnStartGainShield", "{0} triggered at player turn start and granted {1} shield."),
						RelicDisplayName,
						FText::AsNumber(EffectInput.Value))));
				break;

			default:
				break;
			}
		}
	}
}

void FFinalBattleRelicService::ResetPlayerTurnTriggerCounts(FFinalBattleState& BattleState) const
{
	for (FFinalBattleRelicRuntimeState& RuntimeState : BattleState.RelicRuntimeStates)
	{
		for (FFinalBattleRelicRuntimeTriggerState& TriggerState : RuntimeState.TriggerStates)
		{
			if (TriggerState.TriggerDefinition.Limit == EFinalRelicTriggerLimit::OncePerPlayerTurn)
			{
				TriggerState.TriggeredCountThisPlayerTurn = 0;
			}
		}
	}
}

void FFinalBattleRelicService::HandlePlayerTeamTookHealthDamage(
	FFinalBattleState& BattleState,
	const int32 ActualHealthDamage,
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
				|| !CanTriggerRuntimeRelicEffect(TriggerState))
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
					OutGeneratedEvents.Add(BuildRelicTriggeredEvent(
						RuntimeState.RelicId,
						RelicTeamHealthDamageGainShieldTag,
						EffectDefinition.Value,
						BattleState.TeamShield,
						FText::Format(
							NSLOCTEXT("FinalBattleRelicService", "RelicTeamHealthDamageGainShield", "{0} triggered after actual health loss and granted {1} shield."),
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
				++TriggerState.TriggeredCountThisPlayerTurn;
				++TriggerState.TriggeredCountThisBattle;
			}
		}
	}
}

void FFinalBattleRelicService::HandlePlayerCardResolved(
	FFinalBattleState& BattleState,
	const FFinalBattleResolvedCardTriggerContext& CardContext,
	const FFinalBattleCardService& CardService,
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
				|| !CanTriggerRuntimeRelicEffect(TriggerState)
				|| !SatisfiesCardCondition(TriggerDefinition.CardCondition, CardContext))
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
						OutGeneratedEvents.Add(BuildRelicTriggeredEvent(
							RuntimeState.RelicId,
							RelicPlayerCardResolvedDrawCardsTag,
							DrawnCount,
							BattleState.DeckState.HandCardInstanceIds.Num(),
							FText::Format(
								NSLOCTEXT("FinalBattleRelicService", "RelicPlayerCardResolvedDrawCards", "{0} triggered after a card resolved and drew {1} card(s)."),
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
				++TriggerState.TriggeredCountThisPlayerTurn;
				++TriggerState.TriggeredCountThisBattle;
			}
		}
	}
}
