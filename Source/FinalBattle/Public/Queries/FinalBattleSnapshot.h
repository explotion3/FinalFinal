#pragma once

#include "CoreMinimal.h"
#include "Queries/FinalBattleQueryTypes.h"
#include "FinalBattleSnapshot.generated.h"

USTRUCT(BlueprintType)
struct FINALBATTLE_API FFinalBattleSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 CurrentRound = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 CurrentAP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 CurrentEP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 TeamCurrentHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 TeamMaxHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 TeamShield = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	bool bBattleEnded = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	bool bPlayerVictory = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	TArray<FFinalBattleCharacterViewData> Characters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	TArray<FFinalBattleEnemyViewData> Enemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	TArray<FFinalBattleCardViewData> HandCards;
};
