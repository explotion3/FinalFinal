#pragma once

struct FFinalBattleEnemyState;

class FFinalEnemyIntentService
{
public:
	void RefreshIntent(FFinalBattleEnemyState& EnemyState, int32 PreviewRound) const;
	void CommitCurrentIntentExecution(FFinalBattleEnemyState& EnemyState, int32 CurrentRound) const;
};
