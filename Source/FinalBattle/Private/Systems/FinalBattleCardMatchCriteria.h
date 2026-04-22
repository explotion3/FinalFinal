#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"

struct FFinalBattleCardMatchCriteria
{
	FName RuntimeOwnerUnitId = NAME_None;
	FFinalCardId RequiredCardId;
	FGameplayTag RequiredKeyword;
	bool bGeneratedOnly = false;
};
