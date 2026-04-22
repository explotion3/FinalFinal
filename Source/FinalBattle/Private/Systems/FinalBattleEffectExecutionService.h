#pragma once

#include "CoreMinimal.h"
#include "Systems/FinalBattleEffectExecutionTypes.h"

class UFinalBattleEffectDefinition;
class UFinalCardDefinition;
class UFinalUltimateDefinition;
struct FFinalBattleCharacterState;
struct FFinalBattleCommand;
struct FFinalBattleEnemyState;
struct FFinalBattleState;

class FFinalBattleEffectExecutionService
{
public:
	bool HasSupportedEffectList(const TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects) const;
	bool HasSupportedEffect(const UFinalCardDefinition* CardDefinition) const;
	bool HasSupportedEffect(const UFinalUltimateDefinition* UltimateDefinition) const;

	bool ExecuteEffectList(
		FFinalBattleState& State,
		const TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects,
		const FFinalBattleCommand* Command,
		const UFinalCardDefinition* SourceCardDefinition,
		const FFinalBattleCharacterState* SourceCharacterState,
		FFinalBattleEnemyState* SourceEnemyState,
		FFinalBattleEffectExecutionSummary& Summary) const;

	int32 ApplyTeamIncomingDamageAndTriggers(
		FFinalBattleState& State,
		int32 TotalIncomingDamage,
		FFinalBattleEffectExecutionSummary& Summary) const;
};
