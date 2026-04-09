#pragma once

#include "CoreMinimal.h"
#include "FinalBattleEvent.generated.h"

UENUM(BlueprintType)
enum class EFinalBattleEventType : uint8
{
	Info,
	CommandAccepted,
	CommandRejected,
	StateChanged,
	PhaseChanged
};

USTRUCT(BlueprintType)
struct FINALBATTLE_API FFinalBattleEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	EFinalBattleEventType EventType = EFinalBattleEventType::Info;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FText Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 Round = 0;
};
