#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"

struct FFinalBattleStatusInstance
{
	FGuid StatusInstanceId;
	FFinalStatusId StatusId;
	FName OwnerUnitId = NAME_None;
	FName SourceUnitId = NAME_None;
	int32 CurrentStacks = 0;
	int32 RemainingDuration = 0;
};
