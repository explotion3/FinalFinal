#pragma once

#include "CoreMinimal.h"

class UFinalCardDefinition;
class UFinalCharacterDefinition;
class UFinalUltimateDefinition;

struct FINALBATTLE_API FFinalBattleCharacterInitData
{
	UFinalCharacterDefinition* CharacterDefinition = nullptr;
	UFinalUltimateDefinition* UltimateDefinition = nullptr;
	int32 CurrentStress = 0;
	bool bCollapsed = false;
	int32 CurrentAwakenCount = 0;
	int32 CollapseCount = 0;
};

struct FINALBATTLE_API FFinalBattleInitContext
{
	int32 TeamCurrentHP = 0;
	TArray<FFinalBattleCharacterInitData> PartyMembers;
	TArray<UFinalCardDefinition*> DeckDefinitions;
};
