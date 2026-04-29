#include "Systems/FinalBattleRelicService.h"

#include "Runtime/FinalBattleCharacterState.h"
#include "Runtime/FinalBattleRelicRuntimeState.h"
#include "Runtime/FinalBattleState.h"
#include "Systems/FinalBattleConditionService.h"
#include "Systems/FinalBattleEffectExecutionService.h"
#include "Systems/FinalBattleTriggerService.h"
#include "Systems/FinalBattleUnitService.h"

namespace
{
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

bool IsSupportedBattleRuntimeTrigger(const FFinalRuntimeTriggerDefinition& TriggerDefinition)
{
	return TriggerDefinition.Domain == EFinalRuntimeTriggerDomain::Battle
		&& TriggerDefinition.Window != EFinalRuntimeTriggerWindow::None
		&& !TriggerDefinition.Effects.IsEmpty();
}
}

void FFinalBattleRelicService::InitializeRelics(
	FFinalBattleState& BattleState,
	TArray<FFinalBattleStartRelicInput> ActiveRelics,
	const FFinalBattleTriggerService& TriggerService,
	const FFinalBattleConditionService& ConditionService,
	const FFinalBattleEffectExecutionService& EffectExecutionService,
	const FFinalBattleUnitService& UnitService,
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

		TArray<FFinalRuntimeTriggerDefinition> ValidRuntimeTriggers;
		for (const FFinalRuntimeTriggerDefinition& TriggerDefinition : RelicInput.RuntimeTriggers)
		{
			if (!IsSupportedBattleRuntimeTrigger(TriggerDefinition))
			{
				continue;
			}

			ValidRuntimeTriggers.Add(TriggerDefinition);
		}

		if (ValidRuntimeTriggers.IsEmpty())
		{
			continue;
		}

		RelicInput.RuntimeTriggers = ValidRuntimeTriggers;
		BattleState.ActiveRelics.Add(RelicInput);

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

	TriggerService.HandleBattlePhaseRuntimeTriggers(
		BattleState,
		EFinalRuntimeTriggerWindow::BattleStart,
		ConditionService,
		EffectExecutionService,
		UnitService,
		OutGeneratedEvents);
}

void FFinalBattleRelicService::ApplyPlayerTurnStartRelicEffects(
	FFinalBattleState& BattleState,
	const FFinalBattleTriggerService& TriggerService,
	const FFinalBattleConditionService& ConditionService,
	const FFinalBattleEffectExecutionService& EffectExecutionService,
	const FFinalBattleUnitService& UnitService,
	TArray<FFinalBattleEvent>& OutGeneratedEvents) const
{
	TriggerService.HandleBattlePhaseRuntimeTriggers(
		BattleState,
		EFinalRuntimeTriggerWindow::PlayerTurnStart,
		ConditionService,
		EffectExecutionService,
		UnitService,
		OutGeneratedEvents);
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
