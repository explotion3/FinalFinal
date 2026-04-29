#pragma once

#include "CoreMinimal.h"

class FFinalBattleCardService;
class FFinalBattleConditionService;
class FFinalBattleEventService;
class FFinalBattleEffectExecutionService;
class FFinalBattleRelicService;
class FFinalBattleResourceService;
class FFinalBattleTriggerService;
class FFinalBattleUnitService;
class FFinalEnemyIntentService;
class UFinalBattleEncounterDefinition;
class UFinalBattleRuleConfig;
struct FFinalBattleInitContext;
struct FFinalBattleState;

// Battle 私有初始化服务。
// 职责：把遭遇、规则和 init context 展开为 FFinalBattleState 的初始权威状态。
class FFinalBattleInitializationService
{
public:
	void InitializeBattle(
		FFinalBattleState& State,
		const UFinalBattleEncounterDefinition* EncounterDefinition,
		const UFinalBattleRuleConfig* RuleConfig,
		const FFinalBattleInitContext& InitContext,
		const FFinalBattleCardService& CardService,
		const FFinalBattleConditionService& ConditionService,
		const FFinalBattleEventService& EventService,
		const FFinalBattleEffectExecutionService& EffectExecutionService,
		const FFinalBattleRelicService& RelicService,
		const FFinalBattleResourceService& ResourceService,
		const FFinalBattleTriggerService& TriggerService,
		const FFinalBattleUnitService& UnitService,
		const FFinalEnemyIntentService& EnemyIntentService) const;
};
