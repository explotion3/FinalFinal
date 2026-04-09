#pragma once

struct FFinalBattleState;

class FFinalBattleTurnService
{
public:
	void AdvanceToNextRound(FFinalBattleState& BattleState, int32 StartingAP) const;
};
