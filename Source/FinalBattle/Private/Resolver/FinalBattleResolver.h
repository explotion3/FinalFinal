#pragma once

#include "CoreMinimal.h"
#include "Commands/FinalBattleCommand.h"
#include "Events/FinalBattleEvent.h"
#include "Facade/FinalBattleSessionTypes.h"
#include "Queries/FinalBattleSnapshot.h"

class UFinalBattleEncounterDefinition;
class UFinalBattleRuleConfig;
struct FFinalBattleState;

class FFinalBattleResolver
{
public:
	void Initialize(FFinalBattleState& State, const UFinalBattleEncounterDefinition* EncounterDefinition, const UFinalBattleRuleConfig* RuleConfig, const FFinalBattleInitContext& InitContext) const;
	FFinalBattleEvent ExecuteCommand(FFinalBattleState& State, const FFinalBattleCommand& Command, const UFinalBattleRuleConfig* RuleConfig) const;
	FFinalBattleSnapshot BuildSnapshot(const FFinalBattleState& State) const;

private:
	FFinalBattleEvent ExecutePlayCardCommand(FFinalBattleState& State, const FFinalBattleCommand& Command, const UFinalBattleRuleConfig* RuleConfig) const;
	FFinalBattleEvent ExecutePlayUltimateCommand(FFinalBattleState& State, const FFinalBattleCommand& Command, const UFinalBattleRuleConfig* RuleConfig) const;
	FFinalBattleEvent ExecuteEndTurnCommand(FFinalBattleState& State, const FFinalBattleCommand& Command, const UFinalBattleRuleConfig* RuleConfig) const;
	FFinalBattleEvent ExecuteSelectTargetCommand(FFinalBattleState& State, const FFinalBattleCommand& Command) const;
	FFinalBattleEvent ExecuteUnsupportedCommand(FFinalBattleState& State) const;

	static FName MakePlayerUnitId(int32 Index);
	static FName MakeEnemyUnitId(int32 Index);
};
