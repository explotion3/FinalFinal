#include "Systems/FinalBattleTurnService.h"

#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Runtime/FinalBattleEnemyState.h"
#include "Runtime/FinalBattleState.h"
#include "Systems/FinalBattleCardService.h"
#include "Systems/FinalBattleConditionService.h"
#include "Systems/FinalBattleEffectExecutionService.h"
#include "Systems/FinalBattleEnemyActionService.h"
#include "Systems/FinalBattleInitiativeService.h"
#include "Systems/FinalBattlePassiveService.h"
#include "Systems/FinalBattleRelicService.h"
#include "Systems/FinalBattleResourceService.h"
#include "Systems/FinalBattleStatusService.h"
#include "Systems/FinalBattleTriggerService.h"
#include "Systems/FinalBattleUnitService.h"

namespace
{
const FName PassiveRemovedExpiredReasonTag(TEXT("passive.removed.expired"));

const FFinalBattlePassiveService& GetPassiveService()
{
	static const FFinalBattlePassiveService PassiveService;
	return PassiveService;
}

const FFinalBattleInitiativeService& GetInitiativeService()
{
	static const FFinalBattleInitiativeService InitiativeService;
	return InitiativeService;
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

	const FFinalBattleEnemyActionSequenceResult EnemyActionSequenceResult = GetInitiativeService().ResolveEndTurnEnemyActions(
		BattleState,
		EnemyActionService,
		UnitService,
		TriggerService,
		EffectExecutionService);
	Result.TotalDamageToTeam += EnemyActionSequenceResult.TotalDamageToTeam;
	Result.TotalEnemyShieldGained += EnemyActionSequenceResult.TotalEnemyShieldGained;
	Result.ResolvedEffectCount += EnemyActionSequenceResult.ResolvedEffectCount;
	Result.GeneratedEvents.Append(EnemyActionSequenceResult.GeneratedEvents);
	Result.bBattleLost = Result.bBattleLost || EnemyActionSequenceResult.bBattleLost;

	if (Result.bBattleLost || BattleState.TeamCurrentHP <= 0)
	{
		Result.bBattleLost = true;
		return Result;
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

	GetInitiativeService().ResetEnemyInitiativeForNewRound(BattleState);

	const int32 TurnStartDrawCount = RuleConfig
		? FMath::Max(RuleConfig->TurnStartDrawCount, 0)
		: 0;
	CardService.DrawCards(BattleState, TurnStartDrawCount);

	return Result;
}
