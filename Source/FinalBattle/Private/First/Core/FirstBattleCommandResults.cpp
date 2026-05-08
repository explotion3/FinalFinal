#include "First/Core/FirstBattleCommandResults.h"

FFirstBattleCommandResult FFirstBattleCommandResults::Rejected(const FName ReasonTag, const FText& Message)
{
	FFirstBattleCommandResult Result;
	Result.ResultCode = EFirstBattleCommandResultCode::Rejected;
	Result.ReasonTag = ReasonTag;
	Result.Message = Message;
	return Result;
}

FFirstBattleCommandResult FFirstBattleCommandResults::Accepted(const FText& Message)
{
	FFirstBattleCommandResult Result;
	Result.ResultCode = EFirstBattleCommandResultCode::Accepted;
	Result.ReasonTag = NAME_None;
	Result.Message = Message;
	return Result;
}
