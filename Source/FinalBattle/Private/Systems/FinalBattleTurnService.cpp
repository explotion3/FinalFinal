#include "Systems/FinalBattleTurnService.h"

#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Runtime/FinalBattleEnemyState.h"
#include "Runtime/FinalBattleState.h"
#include "Systems/FinalBattleCardService.h"
#include "Systems/FinalBattleConditionService.h"
#include "Systems/FinalBattleEffectExecutionService.h"
#include "Systems/FinalBattleEnemyActionService.h"
#include "Systems/FinalBattlePassiveService.h"
#include "Systems/FinalBattleRelicService.h"
#include "Systems/FinalBattleResourceService.h"
#include "Systems/FinalBattleStatusService.h"
#include "Systems/FinalBattleTriggerService.h"
#include "Systems/FinalBattleUnitService.h"
#include "Systems/FinalEnemyIntentService.h"

namespace
{
const FName TeamPlayerUnitId(TEXT("team_player"));
const FName PassiveRemovedExpiredReasonTag(TEXT("passive.removed.expired"));

const FFinalBattlePassiveService& GetPassiveService()
{
	static const FFinalBattlePassiveService PassiveService;
	return PassiveService;
}

const FFinalEnemyIntentService& GetEnemyIntentService()
{
	static const FFinalEnemyIntentService IntentService;
	return IntentService;
}

FFinalBattleEvent BuildPassiveRemovedEvent(const FFinalBattlePassiveRemovalResult& RemovalResult)
{
	FFinalBattleEvent Event;
	Event.EventType = EFinalBattleEventType::PassiveRemoved;
	Event.PassiveInstanceId = RemovalResult.PassiveInstanceId;
	Event.PassiveId = RemovalResult.PassiveId;
	Event.SourceUnitId = RemovalResult.SourceUnitId;
	Event.TargetUnitId = RemovalResult.OwnerUnitId;
	Event.ReasonTag = RemovalResult.RemovalReasonTag.IsNone() ? PassiveRemovedExpiredReasonTag : RemovalResult.RemovalReasonTag;
	Event.PrimaryValue = RemovalResult.CurrentStacksBeforeRemoval;
	Event.SecondaryValue = RemovalResult.RemainingDurationBeforeRemoval;
	Event.Message = FText::Format(
		NSLOCTEXT("FinalBattleTurnService", "PassiveRemovedMessage", "被动失效：{0}。"),
		RemovalResult.DisplayName);
	return Event;
}
}

FFinalBattleEndTurnResult FFinalBattleTurnService::ResolveEndTurn(
	FFinalBattleState& BattleState,
	const UFinalBattleRuleConfig* RuleConfig,
	const FFinalBattleCardService& CardService,
	const FFinalBattleConditionService& ConditionService,
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
	const TArray<FFinalBattlePassiveRemovalResult> PassiveRemovalResults = GetPassiveService().ResolvePlayerTurnEndPassives(BattleState);
	for (const FFinalBattlePassiveRemovalResult& RemovalResult : PassiveRemovalResults)
	{
		Result.GeneratedEvents.Add(BuildPassiveRemovedEvent(RemovalResult));
	}
	const FFinalBattleDamageOverTimeResult DamageOverTimeResult = StatusService.ResolveDamageOverTimeAtTickWindow(
		BattleState,
		EFinalStatusDamageOverTimeTickWindow::PlayerTurnEndBeforeEnemyActions,
		UnitService,
		TriggerService,
		EffectExecutionService);
	Result.TotalDamageToTeam += DamageOverTimeResult.TotalDamageToTeam;
	for (const FFinalBattleCharacterState& CharacterState : BattleState.Characters)
	{
		StatusService.ResyncProjectedHandCardModifiers(BattleState, CardService, CharacterState.RuntimeUnitId);
	}

	if (BattleState.TeamCurrentHP <= 0)
	{
		Result.bBattleLost = true;
		return Result;
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
	GetPassiveService().ResetPlayerTurnTriggerCounts(BattleState);
	RelicService.ApplyPlayerTurnStartRelicEffects(
		BattleState,
		TriggerService,
		ConditionService,
		EffectExecutionService,
		UnitService,
		Result.GeneratedEvents);

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
