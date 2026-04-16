#pragma once

class UFinalBattleRuleConfig;
struct FFinalBattleState;

class FFinalBattleResourceService
{
public:
	void InitializeBattleResources(FFinalBattleState& BattleState, const UFinalBattleRuleConfig* RuleConfig) const;
	void ResetRoundResources(FFinalBattleState& BattleState, int32 StartingAP) const;
	bool HasEnoughAP(const FFinalBattleState& BattleState, int32 RequiredAP) const;
	bool HasEnoughEP(const FFinalBattleState& BattleState, int32 RequiredEP) const;
	void SpendAP(FFinalBattleState& BattleState, int32 APToSpend) const;
	void SpendEP(FFinalBattleState& BattleState, int32 EPToSpend) const;
	void GainCardPlayEP(FFinalBattleState& BattleState, const UFinalBattleRuleConfig* RuleConfig) const;
	void GainEndTurnEP(FFinalBattleState& BattleState, const UFinalBattleRuleConfig* RuleConfig) const;
};
