#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"

struct FFinalBattleCharacterState
{
	FName RuntimeUnitId = NAME_None;
	FFinalCharacterId CharacterId;
	int32 CurrentStress = 0;
	bool bCollapsed = false;
	int32 CurrentAwakenCount = 0;
	int32 CollapseCount = 0;
	int32 RuntimeAttack = 0;
	int32 RuntimeDefense = 0;
	float RuntimeBreakRate = 0.0f;
};
