#include "Systems/FinalBattleEnemyActionService.h"

#include "Battle/Definitions/FinalEnemyIntentDefinition.h"
#include "Runtime/FinalBattleEnemyState.h"
#include "Runtime/FinalBattleState.h"
#include "Systems/FinalBattleEffectExecutionService.h"
#include "Systems/FinalBattleEffectExecutionTypes.h"

FFinalBattleEnemyActionResult FFinalBattleEnemyActionService::ResolveEnemyAction(
	FFinalBattleState& BattleState,
	FFinalBattleEnemyState& EnemyState,
	const FFinalBattleEffectExecutionService& EffectExecutionService) const
{
	FFinalBattleEnemyActionResult ActionResult;
	FFinalBattleEffectExecutionSummary Summary;

	if (EnemyState.CurrentIntentDefinition
		&& EffectExecutionService.HasSupportedEffectList(EnemyState.CurrentIntentDefinition->Effects))
	{
		EffectExecutionService.ExecuteEffectList(
			BattleState,
			EnemyState.CurrentIntentDefinition->Effects,
			nullptr,
			nullptr,
			nullptr,
			&EnemyState,
			Summary);
	}
	else
	{
		const int32 HpDamage = EffectExecutionService.ApplyTeamIncomingDamageAndTriggers(
			BattleState,
			FMath::Max(EnemyState.RuntimeDamagePower, 0),
			Summary);
		Summary.TotalDamageToTeam += HpDamage;
	}

	ActionResult.DamageToTeam = Summary.TotalDamageToTeam;
	ActionResult.EnemyShieldGained = Summary.TotalEnemyShieldGained;
	ActionResult.ResolvedEffectCount = Summary.ResolvedEffectCount;
	return ActionResult;
}
