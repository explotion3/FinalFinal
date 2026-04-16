#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Save/FinalRunSaveData.h"
#include "FinalRunSaveGame.generated.h"

UCLASS()
class FINALAPP_API UFinalRunSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Final|Save")
	FFinalRunSaveData RunSaveData;
};
