#pragma once

#include "CoreMinimal.h"
#include "Queries/FinalRunQueryTypes.h"
#include "Requests/FinalBattleResult.h"
#include "FinalRunSnapshot.generated.h"

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalRunBattleBridgeViewData PendingBattle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 Gold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 RelicCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 DeckCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalBattleOutcome LastBattleOutcome = EFinalBattleOutcome::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalEncounterId LastResolvedEncounterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 LastBattleRewardGold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunCharacterViewData> Characters;
};
