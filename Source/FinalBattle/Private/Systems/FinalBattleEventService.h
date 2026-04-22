#pragma once

#include "CoreMinimal.h"
#include "Events/FinalBattleEvent.h"

struct FFinalBattleState;

class FFinalBattleEventService
{
public:
	void AppendBattleEvent(FFinalBattleState& State, const FFinalBattleEvent& Event) const;
	FFinalBattleEvent FinalizeBattleEvent(FFinalBattleState& State, const FFinalBattleEvent& Event) const;
};
