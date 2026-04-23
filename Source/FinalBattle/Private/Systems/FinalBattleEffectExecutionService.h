#pragma once

#include "CoreMinimal.h"
#include "Systems/FinalBattleEffectExecutionTypes.h"

class UFinalBattleEffectDefinition;
class UFinalCardDefinition;
class UFinalUltimateDefinition;
class FFinalBattleTriggerService;
class FFinalBattleUnitService;
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
		const FFinalBattleUnitService& UnitService,
		FFinalBattleEffectExecutionSummary& Summary) const;

	int32 ApplyTeamIncomingDamageAndTriggers(
		FFinalBattleState& State,
		int32 TotalIncomingDamage,
		const FFinalBattleUnitService& UnitService,
		const FFinalBattleTriggerService& TriggerService,
		FFinalBattleEffectExecutionSummary& Summary) const;
};
