#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "FinalBattleCommand.generated.h"

UENUM(BlueprintType)
enum class EFinalBattleCommandType : uint8
{
	PlayCard,
	PlayUltimate,
	EndTurn,
	SelectTarget
};

USTRUCT(BlueprintType)
struct FINALBATTLE_API FFinalBattleCommand
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	EFinalBattleCommandType CommandType = EFinalBattleCommandType::EndTurn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FGuid CardInstanceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FName TargetUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FName UltimateOwnerUnitId = NAME_None;
};
