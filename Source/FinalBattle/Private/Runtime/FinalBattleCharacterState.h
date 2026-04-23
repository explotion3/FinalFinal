#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "Runtime/FinalBattleRuntimeTriggerState.h"

class UFinalUltimateDefinition;

struct FFinalBattleCharacterState
{
	FName RuntimeUnitId = NAME_None;
	FFinalCharacterId CharacterId;
	FText DisplayName;
	int32 CurrentStress = 0;
	int32 StressCap = 0;
	bool bCollapsed = false;
	int32 CurrentAwakenCount = 0;
	int32 CurrentAwakenThreshold = 0;
	int32 CollapseCount = 0;
	int32 VitalShare = 0;
	int32 RuntimeAttack = 0;
	int32 RuntimeDefense = 0;
	float RuntimeBreakRate = 0.0f;
	FFinalUltimateId UltimateId;
	FText UltimateDisplayName;
	int32 UltimateCostEP = 0;
	UFinalUltimateDefinition* UltimateDefinition = nullptr;
	bool bUltimateUsedThisBattle = false;
	TArray<FFinalBattleRuntimeTriggerState> TriggerStates;
};
