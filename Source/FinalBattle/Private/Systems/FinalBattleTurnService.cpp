#include "Systems/FinalBattleTurnService.h"

#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Runtime/FinalBattleEnemyState.h"
#include "Runtime/FinalBattleState.h"
#include "Systems/FinalBattleCardService.h"
#include "Systems/FinalBattleResourceService.h"
#include "Systems/FinalBattleStatusService.h"
#include "Systems/FinalEnemyIntentService.h"

namespace
{
const FName TeamPlayerUnitId(TEXT("team_player"));
const FName RelicGainAPTag(TEXT("battle.relic.effect.gain_ap"));
const FName RelicGainShieldTag(TEXT("battle.relic.effect.gain_shield"));
const FName RelicTurnStartGainAPTag(TEXT("battle.relic.effect.turn_start_gain_ap"));
const FName RelicTurnStartGainShieldTag(TEXT("battle.relic.effect.turn_start_gain_shield"));

const FFinalEnemyIntentService& GetEnemyIntentService()
{
	static const FFinalEnemyIntentService IntentService;
	return IntentService;
}

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
		: NSLOCTEXT("FinalBattleTurnService", "UnknownRelic", "Unknown Relic");
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
}

void FFinalBattleTurnService::ApplyBattleStartRelicEffects(
	FFinalBattleState& BattleState,
	TArray<FFinalBattleStartRelicInput> ActiveRelics,
	TArray<FFinalBattleEvent>& OutGeneratedEvents) const
{
	BattleState.ActiveRelics.Reset();

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
						NSLOCTEXT("FinalBattleTurnService", "RelicGainAP", "{0} triggered at battle start and granted {1} AP."),
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
						NSLOCTEXT("FinalBattleTurnService", "RelicGainShield", "{0} triggered at battle start and granted {1} shield."),
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

		if (ValidBattleStartEffects.Num() > 0 || ValidTurnStartEffects.Num() > 0)
		{
			RelicInput.BattleStartEffects = MoveTemp(ValidBattleStartEffects);
			RelicInput.PlayerTurnStartEffects = MoveTemp(ValidTurnStartEffects);
			BattleState.ActiveRelics.Add(MoveTemp(RelicInput));
		}
	}
}

void FFinalBattleTurnService::ApplyPlayerTurnStartRelicEffects(
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
						NSLOCTEXT("FinalBattleTurnService", "RelicTurnStartGainAP", "{0} triggered at player turn start and granted {1} AP."),
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
						NSLOCTEXT("FinalBattleTurnService", "RelicTurnStartGainShield", "{0} triggered at player turn start and granted {1} shield."),
						RelicDisplayName,
						FText::AsNumber(EffectInput.Value))));
				break;

			default:
				break;
			}
		}
	}
}

FFinalBattleEndTurnResult FFinalBattleTurnService::ResolveEndTurn(
	FFinalBattleState& BattleState,
	const UFinalBattleRuleConfig* RuleConfig,
	const FFinalBattleCardService& CardService,
	const FFinalBattleResourceService& ResourceService,
	const FFinalBattleStatusService& StatusService,
	TFunctionRef<FFinalBattleEnemyActionResult(FFinalBattleState&, FFinalBattleEnemyState&)> ExecuteEnemyAction) const
{
	FFinalBattleEndTurnResult Result;
	StatusService.TickStatusWindows(BattleState);

	for (FFinalBattleEnemyState& EnemyState : BattleState.Enemies)
	{
		if (EnemyState.CurrentHP <= 0)
		{
			continue;
		}

		const FFinalBattleEnemyActionResult ActionResult = ExecuteEnemyAction(BattleState, EnemyState);
		Result.TotalDamageToTeam += ActionResult.DamageToTeam;
		Result.TotalEnemyShieldGained += ActionResult.EnemyShieldGained;
		Result.ResolvedEffectCount += ActionResult.ResolvedEffectCount;

		EnemyState.bActedThisRound = true;

		FFinalBattleEvent EnemyActionEvent;
		EnemyActionEvent.EventType = EFinalBattleEventType::EnemyActed;
		EnemyActionEvent.SourceUnitId = EnemyState.RuntimeUnitId;
		EnemyActionEvent.TargetUnitId = TeamPlayerUnitId;
		EnemyActionEvent.RelatedTag = EnemyState.CurrentIntentId;
		EnemyActionEvent.PrimaryValue = ActionResult.DamageToTeam;
		EnemyActionEvent.SecondaryValue = ActionResult.EnemyShieldGained;
		EnemyActionEvent.Message = FText::Format(
			NSLOCTEXT("FinalBattleTurnService", "EnemyActed", "{0} resolved intent {1}."),
			EnemyState.DisplayName.IsEmpty() ? FText::FromName(EnemyState.RuntimeUnitId) : EnemyState.DisplayName,
			EnemyState.CurrentIntentText.IsEmpty() ? FText::FromString(TEXT("Attack")) : EnemyState.CurrentIntentText);
		Result.GeneratedEvents.Add(MoveTemp(EnemyActionEvent));

		GetEnemyIntentService().CommitCurrentIntentExecution(EnemyState, BattleState.CurrentRound);
		GetEnemyIntentService().RefreshIntent(EnemyState, BattleState.CurrentRound + 1);
	}

	ResourceService.GainEndTurnEP(BattleState, RuleConfig);

	if (BattleState.TeamCurrentHP <= 0)
	{
		Result.bBattleLost = true;
		return Result;
	}

	++BattleState.CurrentRound;
	ResourceService.ResetRoundResources(BattleState, RuleConfig ? RuleConfig->InitialAP : 0);
	ApplyPlayerTurnStartRelicEffects(BattleState, Result.GeneratedEvents);

	for (FFinalBattleEnemyState& EnemyState : BattleState.Enemies)
	{
		EnemyState.bActedThisRound = false;
	}

	const int32 TargetHandSize = RuleConfig
		? FMath::Max(RuleConfig->InitialHandSize, 0)
		: BattleState.DeckState.HandCardInstanceIds.Num();
	CardService.DrawUpToHandSize(BattleState, TargetHandSize);
	StatusService.TickStatusWindows(BattleState);

	return Result;
}
