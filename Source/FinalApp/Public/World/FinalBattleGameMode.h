#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FinalBattleGameMode.generated.h"

class AFinalBattleDirector;

UCLASS()
class FINALAPP_API AFinalBattleGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFinalBattleGameMode();

	virtual void StartPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Final|Battle")
	TSubclassOf<AFinalBattleDirector> BattleDirectorClass;

	UPROPERTY(Transient)
	TObjectPtr<AFinalBattleDirector> ActiveBattleDirector;
};
