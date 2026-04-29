#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Facade/FinalBattleSessionTypes.h"
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
	FName SourceRunCardInstanceId = NAME_None;
	FName RuntimeOwnerUnitId = NAME_None;
	TObjectPtr<UFinalCardDefinition> BaseDefinition = nullptr;
	TObjectPtr<UFinalCardDefinition> ProjectedDefinition = nullptr;
	int32 RuntimeCostAP = 0;
	FGameplayTagContainer RuntimeKeywords;
	FFinalBattleCardRuntimeBehavior RuntimeBehavior;
	int32 RuntimeOutgoingDamagePercent = 0;
	TArray<FFinalBattleCardModifierRecord> ModifierRecords;
	bool bGeneratedCard = false;
	bool bTemporaryCard = false;
};
