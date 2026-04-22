#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"

class UFinalCardDefinition;

struct FFinalBattleCardRuntimeBehavior
{
	int32 RecycleCount = 0;
	bool bRetained = false;
	bool bConsumeOnPlay = false;
};

struct FFinalBattleCardInstance
{
	FGuid CardInstanceId;
	FFinalCardId CardId;
	FName RuntimeOwnerUnitId = NAME_None;
	int32 RuntimeCostAP = 0;
	FGameplayTagContainer RuntimeKeywords;
	UFinalCardDefinition* SourceDefinition = nullptr;
	FFinalBattleCardRuntimeBehavior RuntimeBehavior;
	bool bGeneratedCard = false;
	bool bTemporaryCard = false;
};
