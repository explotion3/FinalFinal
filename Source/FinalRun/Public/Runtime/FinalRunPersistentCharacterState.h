#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "FinalRunPersistentCharacterState.generated.h"

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunPersistentCharacterState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalCharacterId CharacterId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Final|Run|Growth")
    int32 Level = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Final|Run|Growth")
    int32 BreakthroughValue = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Final|Run|Growth")
    int32 BreakthroughRequiredValue = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Final|Run|Growth")
    int32 RootBone = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Final|Run|Growth")
    int32 Insight = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Final|Run|Growth")
    int32 KillingIntent = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Final|Run|Growth")
    bool bHasPendingGrowthChoice = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 CurrentStress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bCollapsed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 CurrentAwakenCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 CollapseCount = 0;
};
