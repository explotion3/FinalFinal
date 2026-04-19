#include "Systems/FinalBattleTurnService.h"

#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Runtime/FinalBattleEnemyState.h"
#include "Runtime/FinalBattleState.h"
#include "Systems/FinalBattleCardService.h"
#include "Systems/FinalBattleRelicService.h"
#include "Systems/FinalBattleResourceService.h"
#include "Systems/FinalBattleStatusService.h"
#include "Systems/FinalEnemyIntentService.h"

namespace
{
const FName TeamPlayerUnitId(TEXT("team_player"));

const FFinalEnemyIntentService& GetEnemyIntentService()
{
	static const FFinalEnemyIntentService IntentService;
	return IntentService;
}
}

FFinalBattleEndTurnResult FFinalBattleTurnService::ResolveEndTurn(
	FFinalBattleState& BattleState,
	const UFinalBattleRuleConfig* RuleConfig,
	const FFinalBattleCardService& CardService,
	const FFinalBattleRelicService& RelicService,
	const FFinalBattleResourceService& ResourceService,
	const FFinalBattleStatusService& StatusService,
	TFunctionRef<FFinalBattleEnemyActionResult(FFinalBattleState&, FFinalBattleEnemyState&)> ExecuteEnemyAction) const
{
	FFinalBattleEndTurnResult Result;
	StatusService.ResolvePlayerTurnEndStatuses(BattleState);

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
	RelicService.ResetPlayerTurnTriggerCounts(BattleState);
	RelicService.ApplyPlayerTurnStartRelicEffects(BattleState, Result.GeneratedEvents);

	for (FFinalBattleEnemyState& EnemyState : BattleState.Enemies)
	{
		EnemyState.bActedThisRound = false;
	}

	const int32 TargetHandSize = RuleConfig
		? FMath::Max(RuleConfig->InitialHandSize, 0)
		: BattleState.DeckState.HandCardInstanceIds.Num();
	CardService.DrawUpToHandSize(BattleState, TargetHandSize);

	return Result;
}
