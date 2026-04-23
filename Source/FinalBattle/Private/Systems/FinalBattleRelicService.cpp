#include "Systems/FinalBattleRelicService.h"

#include "Runtime/FinalBattleCharacterState.h"
#include "Runtime/FinalBattleRelicRuntimeState.h"
#include "Runtime/FinalBattleState.h"

namespace
{
const FName RelicGainAPTag(TEXT("battle.relic.effect.gain_ap"));
const FName RelicGainShieldTag(TEXT("battle.relic.effect.gain_shield"));
const FName RelicTurnStartGainAPTag(TEXT("battle.relic.effect.turn_start_gain_ap"));
const FName RelicTurnStartGainShieldTag(TEXT("battle.relic.effect.turn_start_gain_shield"));

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

bool IsSupportedBattleRuntimeTrigger(const FFinalRuntimeTriggerDefinition& TriggerDefinition)
{
	if (TriggerDefinition.Domain != EFinalRuntimeTriggerDomain::Battle
		|| TriggerDefinition.Window == EFinalRuntimeTriggerWindow::None
		|| TriggerDefinition.Effects.IsEmpty())
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
		TArray<FFinalRuntimeTriggerDefinition> ValidRuntimeTriggers;

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

		for (const FFinalRuntimeTriggerDefinition& TriggerDefinition : RelicInput.RuntimeTriggers)
		{
			if (!IsSupportedBattleRuntimeTrigger(TriggerDefinition))
			{
				continue;
			}

			ValidRuntimeTriggers.Add(TriggerDefinition);
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
			for (const FFinalRuntimeTriggerDefinition& TriggerDefinition : ValidRuntimeTriggers)
			{
				FFinalBattleRuntimeTriggerState& TriggerState = RuntimeState.TriggerStates.AddDefaulted_GetRef();
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
		for (FFinalBattleRuntimeTriggerState& TriggerState : RuntimeState.TriggerStates)
		{
			if (TriggerState.TriggerDefinition.Limit == EFinalRuntimeTriggerLimit::OncePerPlayerTurn)
			{
				TriggerState.TriggeredCountThisPlayerTurn = 0;
			}
		}
	}

	for (FFinalBattleCharacterState& CharacterState : BattleState.Characters)
	{
		for (FFinalBattleRuntimeTriggerState& TriggerState : CharacterState.TriggerStates)
		{
			if (TriggerState.TriggerDefinition.Limit == EFinalRuntimeTriggerLimit::OncePerPlayerTurn)
			{
				TriggerState.TriggeredCountThisPlayerTurn = 0;
			}
		}
	}
}
