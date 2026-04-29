#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "Run/Bridge/FinalBattleGrowthFact.h"

struct FFinalBattleEffectExecutionSummary
{
	FFinalCharacterId SourceCharacterId;
	FFinalCardId SourceCardId;
	EFinalBattleGrowthCommandSource CommandSource = EFinalBattleGrowthCommandSource::None;
	bool bCausedByPlayerCommand = false;
	int32 TotalDamageToEnemies = 0;
	int32 TotalDamageToTeam = 0;
	int32 TotalBreakDamageToEnemies = 0;
	int32 TotalHealingToTeam = 0;
	int32 TotalEnemiesDefeated = 0;
	int32 TotalTeamShieldGained = 0;
	int32 TotalEnemyShieldGained = 0;
	int32 TotalStatusStacksApplied = 0;
	int32 TotalStatusStacksRemoved = 0;
	int32 TotalCardsDrawn = 0;
	int32 TotalAPGained = 0;
	int32 ResolvedEffectCount = 0;
};
