#pragma once

#include "CoreMinimal.h"

class FFinalBattleCardService;
class FFinalBattleEventService;
class FFinalBattleRelicService;
class FFinalBattleResourceService;
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
		const FFinalBattleEventService& EventService,
		const FFinalBattleRelicService& RelicService,
		const FFinalBattleResourceService& ResourceService,
		const FFinalBattleUnitService& UnitService,
		const FFinalEnemyIntentService& EnemyIntentService) const;
};
