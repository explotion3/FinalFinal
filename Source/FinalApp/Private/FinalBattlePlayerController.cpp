#include "World/FinalBattlePlayerController.h"

#include "App/FinalGameInstance.h"
#include "Engine/GameInstance.h"
#include "Subsystems/FinalBattleFlowSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogFinalBattlePlayerController, Log, All);

bool AFinalBattlePlayerController::StartTestBattle()
{
	if (UFinalGameInstance* FinalGameInstance = Cast<UFinalGameInstance>(GetGameInstance()))
	{
		return FinalGameInstance->StartTestBattle();
	}

	return false;
}

FText AFinalBattlePlayerController::GetLastTestBattleFailureReason() const
{
	if (const UFinalGameInstance* FinalGameInstance = Cast<UFinalGameInstance>(GetGameInstance()))
	{
		return FinalGameInstance->GetLastTestFailureReason();
	}

	return FText::FromString(TEXT("FinalGameInstance is unavailable."));
}

bool AFinalBattlePlayerController::SubmitBattleCommand(const FFinalBattleCommand& Command)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UFinalBattleFlowSubsystem* BattleFlow = GameInstance->GetSubsystem<UFinalBattleFlowSubsystem>())
		{
			return BattleFlow->SubmitBattleCommand(Command);
		}
	}

	return false;
}

bool AFinalBattlePlayerController::EndPlayerTurn()
{
	FFinalBattleCommand Command;
	Command.CommandType = EFinalBattleCommandType::EndTurn;
	return SubmitBattleCommand(Command);
}

void AFinalBattlePlayerController::FinalStartTestBattle()
{
	const bool bStarted = StartTestBattle();
	const FText Feedback = bStarted
		? FText::FromString(TEXT("Test battle started."))
		: GetLastTestBattleFailureReason();

	if (bStarted)
	{
		UE_LOG(LogFinalBattlePlayerController, Log, TEXT("%s"), *Feedback.ToString());
	}
	else
	{
		UE_LOG(LogFinalBattlePlayerController, Warning, TEXT("%s"), *Feedback.ToString());
	}
}
