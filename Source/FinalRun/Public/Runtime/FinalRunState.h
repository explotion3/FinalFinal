#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "Requests/FinalBattleResult.h"
#include "Runtime/FinalRunPersistentCharacterState.h"
#include "FinalRunState.generated.h"

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalEncounterId CurrentEncounterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalRuleConfigId CurrentRuleConfigId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 TeamCurrentHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 Gold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bHasPendingBattleStart = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalEncounterId LastResolvedEncounterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalBattleOutcome LastBattleOutcome = EFinalBattleOutcome::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 LastBattleRewardGold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunPersistentCharacterState> Characters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalCardId> RunDeck;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRelicId> Relics;
};
