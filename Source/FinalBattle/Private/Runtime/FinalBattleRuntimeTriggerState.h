#pragma once

#include "CoreMinimal.h"
#include "Battle/Definitions/FinalRuntimeTriggerDefinition.h"

struct FFinalBattleRuntimeTriggerState
{
	FFinalRuntimeTriggerDefinition TriggerDefinition;
	int32 TriggeredCountThisPlayerTurn = 0;
	int32 TriggeredCountThisBattle = 0;
};
