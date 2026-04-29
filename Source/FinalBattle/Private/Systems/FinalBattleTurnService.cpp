#include "Systems/FinalBattleTurnService.h"

#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Runtime/FinalBattleEnemyState.h"
#include "Runtime/FinalBattleState.h"
#include "Systems/FinalBattleCardService.h"
#include "Systems/FinalBattleEffectExecutionService.h"
#include "Systems/FinalBattleEnemyActionService.h"
#include "Systems/FinalBattleRelicService.h"
#include "Systems/FinalBattleResourceService.h"
#include "Systems/FinalBattleStatusService.h"
#include "Systems/FinalBattleTriggerService.h"
#include "Systems/FinalBattleUnitService.h"
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
	const FFinalBattleEnemyActionService& EnemyActionService,
	const FFinalBattleTriggerService& TriggerService,
	const FFinalBattleUnitService& UnitService,
	const FFinalBattleEffectExecutionService& EffectExecutionService) const
{
	FFinalBattleEndTurnResult Result;
	CardService.ResolveEndTurnHandCleanup(BattleState);
	StatusService.ResolvePlayerTurnEndStatuses(BattleState);
	for (const FFinalBattleCharacterState& CharacterState : BattleState.Characters)
	{
		StatusService.ResyncProjectedHandCardModifiers(BattleState, CardService, CharacterState.RuntimeUnitId);
	}

	for (FFinalBattleEnemyState& EnemyState : BattleState.Enemies)
	{
		if (EnemyState.CurrentHP <= 0)
		{
			continue;
		}

		const FFinalBattleEnemyActionResult ActionResult = EnemyActionService.ResolveEnemyAction(
			BattleState,
			EnemyState,
			UnitService,
			TriggerService,
			EffectExecutionService);
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

	const int32 TurnStartDrawCount = RuleConfig
		? FMath::Max(RuleConfig->TurnStartDrawCount, 0)
		: 0;
	CardService.DrawCards(BattleState, TurnStartDrawCount);

	return Result;
}
