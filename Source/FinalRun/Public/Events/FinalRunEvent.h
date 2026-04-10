#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "Requests/FinalBattleResult.h"
#include "FinalRunEvent.generated.h"

UENUM(BlueprintType)
enum class EFinalRunEventType : uint8
{
	Info,
	RunInitialized,
	BattleStartConfigured,
	RunCommandAccepted,
	RunCommandRejected,
	BattleResultApplied
};

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 EventSequence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalRunEventType EventType = EFinalRunEventType::Info;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalEncounterId EncounterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FFinalRuleConfigId RuleConfigId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName PayloadId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalBattleOutcome BattleOutcome = EFinalBattleOutcome::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 TeamCurrentHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 RewardGold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText Message;
};
