#pragma once

#include "CoreMinimal.h"
#include "Events/FinalBattleEvent.h"
#include "Run/Bridge/FinalBattleRelicPayload.h"

class UFinalBattleRuleConfig;
class FFinalBattleCardService;
class FFinalBattleResourceService;
class FFinalBattleStatusService;
struct FFinalBattleEnemyState;
struct FFinalBattleState;

struct FFinalBattleEnemyActionResult
{
	int32 DamageToTeam = 0;
	int32 EnemyShieldGained = 0;
	int32 ResolvedEffectCount = 0;
};

struct FFinalBattleEndTurnResult
{
	TArray<FFinalBattleEvent> GeneratedEvents;
	int32 TotalDamageToTeam = 0;
	int32 TotalEnemyShieldGained = 0;
	int32 ResolvedEffectCount = 0;
	bool bBattleLost = false;
};

class FFinalBattleTurnService
{
public:
	void ApplyBattleStartRelicEffects(
		FFinalBattleState& BattleState,
		TArray<FFinalBattleStartRelicInput> ActiveRelics,
		TArray<FFinalBattleEvent>& OutGeneratedEvents) const;
	void ApplyPlayerTurnStartRelicEffects(FFinalBattleState& BattleState, TArray<FFinalBattleEvent>& OutGeneratedEvents) const;
	FFinalBattleEndTurnResult ResolveEndTurn(
		FFinalBattleState& BattleState,
		const UFinalBattleRuleConfig* RuleConfig,
		const FFinalBattleCardService& CardService,
		const FFinalBattleResourceService& ResourceService,
		const FFinalBattleStatusService& StatusService,
		TFunctionRef<FFinalBattleEnemyActionResult(FFinalBattleState&, FFinalBattleEnemyState&)> ExecuteEnemyAction) const;
};
