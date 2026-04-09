#pragma once

struct FFinalBattleState;

class FFinalBattleResourceService
{
public:
	void ResetRoundResources(FFinalBattleState& BattleState, int32 StartingAP) const;
};
