#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "Run/Definitions/FinalRelicRuntimeTriggerTypes.h"

struct FFinalBattleRelicRuntimeTriggerState
{
	FFinalRelicRuntimeTriggerDefinition TriggerDefinition;
	int32 TriggeredCountThisPlayerTurn = 0;
	int32 TriggeredCountThisBattle = 0;
};

struct FFinalBattleRelicRuntimeState
{
	FFinalRelicId RelicId;
	FName DisplayId = NAME_None;
	FText DisplayName;
	TArray<FFinalRelicRuntimeTriggerDefinition> RuntimeTriggers;
	TArray<FFinalBattleRelicRuntimeTriggerState> TriggerStates;
};
