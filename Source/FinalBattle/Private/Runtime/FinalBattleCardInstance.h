#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"

struct FFinalBattleCardInstance
{
	FGuid CardInstanceId;
	FFinalCardId CardId;
	FName RuntimeOwnerUnitId = NAME_None;
	int32 RuntimeCostAP = 0;
	FGameplayTagContainer RuntimeKeywords;
	bool bRetained = false;
};
