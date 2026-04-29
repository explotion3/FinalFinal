#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "Run/Bridge/FinalBattleRelicPayload.h"

class UFinalCardDefinition;
class UFinalCharacterDefinition;
class UFinalUltimateDefinition;

struct FINALBATTLE_API FFinalBattleCharacterRuntimeStats
{
	FFinalCharacterId CharacterId;
	int32 VitalShare = 0;
	int32 StressCap = 0;
	int32 RuntimeAttack = 0;
	int32 RuntimeDefense = 0;
	float RuntimeBreakRate = 0.0f;
	float RuntimeCritChance = 0.0f;
	float RuntimeCritDamage = 1.5f;
};

struct FINALBATTLE_API FFinalBattleCharacterInitData
{
	UFinalCharacterDefinition* CharacterDefinition = nullptr;
	UFinalUltimateDefinition* UltimateDefinition = nullptr;
	int32 CurrentStress = 0;
	bool bCollapsed = false;
	int32 CurrentAwakenCount = 0;
	int32 CollapseCount = 0;
	int32 VitalShare = 0;
	int32 StressCap = 0;
	int32 RuntimeAttack = 0;
	int32 RuntimeDefense = 0;
	float RuntimeBreakRate = 0.0f;
	float RuntimeCritChance = 0.0f;
	float RuntimeCritDamage = 1.5f;
};

struct FINALBATTLE_API FFinalBattleInitContext
{
	int32 TeamCurrentHP = 0;
	TArray<FFinalBattleCharacterInitData> PartyMembers;
	TArray<UFinalCardDefinition*> DeckDefinitions;
	TArray<FFinalBattleStartRelicInput> BattleStartRelics;
};
