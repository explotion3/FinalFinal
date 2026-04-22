#pragma once

#include "CoreMinimal.h"

struct FFinalBattleCharacterState;
struct FFinalBattleCommand;
struct FFinalBattleEnemyState;
struct FFinalBattleState;

// Battle 私有单位查询服务。
// 职责：统一查询玩家角色、敌人、第一名存活敌人和命令目标。
class FFinalBattleUnitService
{
public:
	const FFinalBattleCharacterState* FindCharacterState(const FFinalBattleState& BattleState, FName RuntimeUnitId) const;
	FFinalBattleCharacterState* FindCharacterState(FFinalBattleState& BattleState, FName RuntimeUnitId) const;

	const FFinalBattleEnemyState* FindEnemyState(const FFinalBattleState& BattleState, FName RuntimeUnitId) const;
	FFinalBattleEnemyState* FindEnemyState(FFinalBattleState& BattleState, FName RuntimeUnitId) const;

	const FFinalBattleEnemyState* FindFirstAliveEnemy(const FFinalBattleState& BattleState) const;
	FFinalBattleEnemyState* FindFirstAliveEnemy(FFinalBattleState& BattleState) const;

	FName ResolveCommandTargetUnitId(const FFinalBattleState& BattleState, const FFinalBattleCommand& Command) const;
};
