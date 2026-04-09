#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "Runtime/FinalRunPersistentCharacterState.h"
#include "FinalBattleResult.generated.h"

UENUM(BlueprintType)
enum class EFinalBattleOutcome : uint8
{
	None,
	Victory,
	Defeat,
	Escape
};

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalBattleResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalEncounterId EncounterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalBattleOutcome Outcome = EFinalBattleOutcome::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 TeamCurrentHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 RewardGold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunPersistentCharacterState> UpdatedCharacterStates;
};
