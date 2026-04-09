#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "FinalBattleQueryTypes.generated.h"

USTRUCT(BlueprintType)
struct FINALBATTLE_API FFinalBattleCharacterViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FName RuntimeUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FFinalCharacterId CharacterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 CurrentStress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	bool bCollapsed = false;
};

USTRUCT(BlueprintType)
struct FINALBATTLE_API FFinalBattleEnemyViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FName RuntimeUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FFinalEnemyId EnemyId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 CurrentHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 CurrentShield = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 CurrentBreakValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 CurrentInitiative = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FName CurrentPhaseTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FText IntentText;
};

USTRUCT(BlueprintType)
struct FINALBATTLE_API FFinalBattleCardViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FGuid CardInstanceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FFinalCardId CardId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 RuntimeCostAP = 0;
};
