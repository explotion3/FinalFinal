#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "Runtime/FinalBattleRuntimeTriggerState.h"

struct FFinalBattleRelicRuntimeState
{
	FFinalRelicId RelicId;
	FName DisplayId = NAME_None;
	FText DisplayName;
	TArray<FFinalBattleRuntimeTriggerState> TriggerStates;
};
