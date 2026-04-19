#pragma once

#include "CoreMinimal.h"
#include "Events/FinalBattleEvent.h"

class UFinalBattleRuleConfig;
class FFinalBattleCardService;
class FFinalBattleRelicService;
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
	FFinalBattleEndTurnResult ResolveEndTurn(
		FFinalBattleState& BattleState,
		const UFinalBattleRuleConfig* RuleConfig,
		const FFinalBattleCardService& CardService,
		const FFinalBattleRelicService& RelicService,
		const FFinalBattleResourceService& ResourceService,
		const FFinalBattleStatusService& StatusService,
		TFunctionRef<FFinalBattleEnemyActionResult(FFinalBattleState&, FFinalBattleEnemyState&)> ExecuteEnemyAction) const;
};
