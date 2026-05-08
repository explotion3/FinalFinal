#pragma once

#include "CoreMinimal.h"
#include "First/FirstBattleTypes.h"

class UFirstCardDefinition;

struct FINALBATTLE_API FFirstCardDefinitionCompiler
{
	static FFirstCardInstance CompileCardDefinition(const UFirstCardDefinition* Definition);
	static FFirstCardInstance CompileCardDefinition(const UFirstCardDefinition* Definition, const FGuid& CardInstanceId);
};
