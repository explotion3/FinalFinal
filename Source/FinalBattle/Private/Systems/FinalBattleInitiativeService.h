#pragma once

#include "CoreMinimal.h"
#include "Events/FinalBattleEvent.h"

class UFinalBattleRuleConfig;
class FFinalBattleEffectExecutionService;
class FFinalBattleEnemyActionService;
class FFinalBattleTriggerService;
class FFinalBattleUnitService;
struct FFinalBattleEnemyState;
struct FFinalBattleResolvedCardTriggerContext;
struct FFinalBattleState;

struct FFinalBattleEnemyActionSequenceResult
{
	TArray<FFinalBattleEvent> GeneratedEvents;
	int32 TotalDamageToTeam = 0;
	int32 TotalEnemyShieldGained = 0;
	int32 ResolvedEffectCount = 0;
	bool bBattleLost = false;
};

// Battle 私有先机服务。
// 职责：处理卡牌后的先机减少事件、先机行动队列、Break 行动覆盖与回合结束补行动。
class FFinalBattleInitiativeService
{
public:
	void ApplyBreakSkipActionOverride(FFinalBattleEnemyState& EnemyState, int32 PreviousBreakValue) const;

	FFinalBattleEnemyActionSequenceResult ResolveCardInitiativeEvents(
		FFinalBattleState& BattleState,
		const UFinalBattleRuleConfig* RuleConfig,
		const FFinalBattleResolvedCardTriggerContext& CardContext,
		bool bCollapsedCard,
		const FFinalBattleEnemyActionService& EnemyActionService,
		const FFinalBattleUnitService& UnitService,
		const FFinalBattleTriggerService& TriggerService,
		const FFinalBattleEffectExecutionService& EffectExecutionService) const;

	FFinalBattleEnemyActionSequenceResult ResolveUltimateInitiativeEvents(
		FFinalBattleState& BattleState,
		const UFinalBattleRuleConfig* RuleConfig,
		const FFinalBattleEnemyActionService& EnemyActionService,
		const FFinalBattleUnitService& UnitService,
		const FFinalBattleTriggerService& TriggerService,
		const FFinalBattleEffectExecutionService& EffectExecutionService) const;

	FFinalBattleEnemyActionSequenceResult ResolveEndTurnEnemyActions(
		FFinalBattleState& BattleState,
		const FFinalBattleEnemyActionService& EnemyActionService,
		const FFinalBattleUnitService& UnitService,
		const FFinalBattleTriggerService& TriggerService,
		const FFinalBattleEffectExecutionService& EffectExecutionService) const;

	void ResetEnemyInitiativeForNewRound(FFinalBattleState& BattleState) const;
};
