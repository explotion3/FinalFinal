#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "Runtime/FinalBattleRuntimeTriggerState.h"
#include "Types/FinalCoreTypes.h"

struct FFinalBattlePassiveInstance
{
	FGuid PassiveInstanceId;
	FFinalPassiveId PassiveId;
	FName DisplayId = NAME_None;
	FText DisplayName;
	FName OwnerUnitId = NAME_None;
	FName SourceUnitId = NAME_None;
	int32 CurrentStacks = 0;
	int32 RemainingDuration = 0;
	EFinalPassiveDurationType DurationType = EFinalPassiveDurationType::Battle;
	int32 AppliedSequence = 0;
	TArray<FFinalBattleRuntimeTriggerState> TriggerStates;
};
