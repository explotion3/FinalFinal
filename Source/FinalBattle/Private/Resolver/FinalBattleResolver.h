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
	// 用遭遇、规则和初始化上下文构建一场战斗的权威运行时状态。
	void Initialize(FFinalBattleState& State, const UFinalBattleEncounterDefinition* EncounterDefinition, const UFinalBattleRuleConfig* RuleConfig, const FFinalBattleInitContext& InitContext) const;

	// 执行一条战斗命令，并产出最终写入日志的 BattleEvent。
	FFinalBattleEvent ExecuteCommand(FFinalBattleState& State, const FFinalBattleCommand& Command, const UFinalBattleRuleConfig* RuleConfig) const;

	// 基于当前权威状态构建对外只读的战斗快照。
	FFinalBattleSnapshot BuildSnapshot(const FFinalBattleState& State) const;

private:
	// 处理打牌命令。
	FFinalBattleEvent ExecutePlayCardCommand(FFinalBattleState& State, const FFinalBattleCommand& Command, const UFinalBattleRuleConfig* RuleConfig) const;

	// 处理释放奥义命令。
	FFinalBattleEvent ExecutePlayUltimateCommand(FFinalBattleState& State, const FFinalBattleCommand& Command, const UFinalBattleRuleConfig* RuleConfig) const;

	// 处理结束回合命令，并推进敌方行动与下一回合。
	FFinalBattleEvent ExecuteEndTurnCommand(FFinalBattleState& State, const FFinalBattleCommand& Command, const UFinalBattleRuleConfig* RuleConfig) const;

	// 处理切换当前目标命令。
	FFinalBattleEvent ExecuteSelectTargetCommand(FFinalBattleState& State, const FFinalBattleCommand& Command) const;

	// 处理当前不支持的命令类型。
	FFinalBattleEvent ExecuteUnsupportedCommand(FFinalBattleState& State) const;

	// 生成玩家单位的默认运行时 Id。
	static FName MakePlayerUnitId(int32 Index);

	// 生成敌方单位的默认运行时 Id。
	static FName MakeEnemyUnitId(int32 Index);
};
