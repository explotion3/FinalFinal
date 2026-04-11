#include "World/FinalBattleGameMode.h"

#include "EngineUtils.h"
#include "World/FinalBattleDirector.h"
#include "World/FinalBattlePlayerController.h"

AFinalBattleGameMode::AFinalBattleGameMode()
{
	PlayerControllerClass = AFinalBattlePlayerController::StaticClass();
	BattleDirectorClass = AFinalBattleDirector::StaticClass();
}

void AFinalBattleGameMode::StartPlay()
{
	Super::StartPlay();

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	for (TActorIterator<AFinalBattleDirector> It(World); It; ++It)
	{
		ActiveBattleDirector = *It;
		break;
	}

	if (ActiveBattleDirector != nullptr || BattleDirectorClass == nullptr)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ActiveBattleDirector = World->SpawnActor<AFinalBattleDirector>(BattleDirectorClass, FTransform::Identity, SpawnParameters);
}
