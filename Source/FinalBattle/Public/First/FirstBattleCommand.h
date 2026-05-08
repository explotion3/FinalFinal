#pragma once

#include "CoreMinimal.h"
#include "First/FirstBattleTypes.h"

struct FINALBATTLE_API FFirstBattleCommand
{
	EFirstBattleCommandType CommandType = EFirstBattleCommandType::EndTurn;
	FGuid CardInstanceId;
	FName TargetPartId = NAME_None;
};

struct FINALBATTLE_API FFirstBattleCommandResult
{
	EFirstBattleCommandResultCode ResultCode = EFirstBattleCommandResultCode::Rejected;
	FName ReasonTag = NAME_None;
	FText Message;

	bool IsAccepted() const
	{
		return ResultCode == EFirstBattleCommandResultCode::Accepted;
	}
};
