#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "Queries/FinalBattleQueryTypes.h"
#include "Run/Bridge/FinalBattleRelicBridge.h"
#include "FinalBattleSnapshot.generated.h"

USTRUCT(BlueprintType)
struct FINALBATTLE_API FFinalBattleSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FGuid BattleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FFinalEncounterId EncounterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FFinalRuleConfigId RuleConfigId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FText EncounterDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 CurrentRound = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 CurrentAP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 CurrentEP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 MaxEP = 0;

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
	FName CurrentTargetUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FFinalBattleDeckViewData DeckState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	TArray<FFinalBattleCharacterViewData> Characters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	TArray<FFinalBattleUltimateViewData> CharacterUltimates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	TArray<FFinalBattleStartRelicInput> ActiveRelics;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	TArray<FFinalBattleEnemyViewData> Enemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	TArray<FFinalBattleStatusViewData> TeamStatuses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	TArray<FFinalBattleCharacterStatusesViewData> CharacterStatuses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	TArray<FFinalBattleStatusViewData> Statuses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	TArray<FFinalBattleCardViewData> HandCards;
};
