#include "Systems/FinalBattleResourceService.h"

#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Runtime/FinalBattleState.h"

void FFinalBattleResourceService::InitializeBattleResources(FFinalBattleState& BattleState, const UFinalBattleRuleConfig* RuleConfig) const
{
	BattleState.CurrentAP = RuleConfig ? RuleConfig->InitialAP : 0;
	BattleState.CurrentEP = 0;
	BattleState.MaxEP = RuleConfig ? RuleConfig->MaxEP : 0;
}

void FFinalBattleResourceService::ResetRoundResources(FFinalBattleState& BattleState, const int32 StartingAP) const
{
	BattleState.CurrentAP = FMath::Max(StartingAP, 0);
}

bool FFinalBattleResourceService::HasEnoughAP(const FFinalBattleState& BattleState, const int32 RequiredAP) const
{
	return BattleState.CurrentAP >= FMath::Max(RequiredAP, 0);
}

bool FFinalBattleResourceService::HasEnoughEP(const FFinalBattleState& BattleState, const int32 RequiredEP) const
{
	return BattleState.CurrentEP >= FMath::Max(RequiredEP, 0);
}

void FFinalBattleResourceService::SpendAP(FFinalBattleState& BattleState, const int32 APToSpend) const
{
	BattleState.CurrentAP = FMath::Max(BattleState.CurrentAP - FMath::Max(APToSpend, 0), 0);
}

void FFinalBattleResourceService::SpendEP(FFinalBattleState& BattleState, const int32 EPToSpend) const
{
	BattleState.CurrentEP = FMath::Max(BattleState.CurrentEP - FMath::Max(EPToSpend, 0), 0);
}

void FFinalBattleResourceService::GainCardPlayEP(FFinalBattleState& BattleState, const UFinalBattleRuleConfig* RuleConfig) const
{
	if (RuleConfig == nullptr)
	{
		return;
	}

	BattleState.CurrentEP = FMath::Min(BattleState.CurrentEP + RuleConfig->BaseCardEpGain, RuleConfig->MaxEP);
}

void FFinalBattleResourceService::GainEndTurnEP(FFinalBattleState& BattleState, const UFinalBattleRuleConfig* RuleConfig) const
{
	if (RuleConfig == nullptr)
	{
		return;
	}

	BattleState.CurrentEP = FMath::Min(BattleState.CurrentEP + RuleConfig->EndTurnEpGain, RuleConfig->MaxEP);
}
