#pragma once

#include "CoreMinimal.h"
#include "First/FirstBattleCommand.h"

struct FFirstBattleCommandResults
{
	static FFirstBattleCommandResult Rejected(FName ReasonTag, const FText& Message);
	static FFirstBattleCommandResult Accepted(const FText& Message);
};
