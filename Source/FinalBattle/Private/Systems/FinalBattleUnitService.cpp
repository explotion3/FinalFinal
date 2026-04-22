#include "Systems/FinalBattleUnitService.h"

#include "Commands/FinalBattleCommand.h"
#include "Runtime/FinalBattleCharacterState.h"
#include "Runtime/FinalBattleEnemyState.h"
#include "Runtime/FinalBattleState.h"

const FFinalBattleCharacterState* FFinalBattleUnitService::FindCharacterState(
	const FFinalBattleState& BattleState,
	const FName RuntimeUnitId) const
{
	return BattleState.Characters.FindByPredicate(
		[&RuntimeUnitId](const FFinalBattleCharacterState& Candidate)
		{
			return Candidate.RuntimeUnitId == RuntimeUnitId;
		});
}

FFinalBattleCharacterState* FFinalBattleUnitService::FindCharacterState(
	FFinalBattleState& BattleState,
	const FName RuntimeUnitId) const
{
	return BattleState.Characters.FindByPredicate(
		[&RuntimeUnitId](const FFinalBattleCharacterState& Candidate)
		{
			return Candidate.RuntimeUnitId == RuntimeUnitId;
		});
}

const FFinalBattleEnemyState* FFinalBattleUnitService::FindEnemyState(
	const FFinalBattleState& BattleState,
	const FName RuntimeUnitId) const
{
	return BattleState.Enemies.FindByPredicate(
		[&RuntimeUnitId](const FFinalBattleEnemyState& Candidate)
		{
			return Candidate.RuntimeUnitId == RuntimeUnitId;
		});
}

FFinalBattleEnemyState* FFinalBattleUnitService::FindEnemyState(
	FFinalBattleState& BattleState,
	const FName RuntimeUnitId) const
{
	return BattleState.Enemies.FindByPredicate(
		[&RuntimeUnitId](const FFinalBattleEnemyState& Candidate)
		{
			return Candidate.RuntimeUnitId == RuntimeUnitId;
		});
}

const FFinalBattleEnemyState* FFinalBattleUnitService::FindFirstAliveEnemy(const FFinalBattleState& BattleState) const
{
	return BattleState.Enemies.FindByPredicate(
		[](const FFinalBattleEnemyState& Candidate)
		{
			return Candidate.CurrentHP > 0;
		});
}

FFinalBattleEnemyState* FFinalBattleUnitService::FindFirstAliveEnemy(FFinalBattleState& BattleState) const
{
	return BattleState.Enemies.FindByPredicate(
		[](const FFinalBattleEnemyState& Candidate)
		{
			return Candidate.CurrentHP > 0;
		});
}

FName FFinalBattleUnitService::ResolveCommandTargetUnitId(
	const FFinalBattleState& BattleState,
	const FFinalBattleCommand& Command) const
{
	return Command.TargetUnitId != NAME_None ? Command.TargetUnitId : BattleState.CurrentTargetUnitId;
}
