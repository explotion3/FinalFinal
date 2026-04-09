#include "World/FinalBattleGameMode.h"

#include "World/FinalBattlePlayerController.h"

AFinalBattleGameMode::AFinalBattleGameMode()
{
	PlayerControllerClass = AFinalBattlePlayerController::StaticClass();
}
