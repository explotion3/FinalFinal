#pragma once

#include "CoreMinimal.h"
#include "Systems/FinalBattleTurnService.h"

class FFinalBattleEffectExecutionService;
struct FFinalBattleEnemyState;
struct FFinalBattleState;

// Battle 私有敌人行动解析服务。
// 职责：执行单个敌人的当前 intent；没有可执行 intent 时走当前最小 fallback 普攻。
class FFinalBattleEnemyActionService
{
public:
	FFinalBattleEnemyActionResult ResolveEnemyAction(
		FFinalBattleState& BattleState,
		FFinalBattleEnemyState& EnemyState,
		const FFinalBattleEffectExecutionService& EffectExecutionService) const;
};
