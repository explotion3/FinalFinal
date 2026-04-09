#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"

class UFinalCardDefinition;

struct FFinalBattleCardInstance
{
	FGuid CardInstanceId;
	FFinalCardId CardId;
	FName RuntimeOwnerUnitId = NAME_None;
	int32 RuntimeCostAP = 0;
	FGameplayTagContainer RuntimeKeywords;
	UFinalCardDefinition* SourceDefinition = nullptr;
	bool bRetained = false;
};
