#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "FinalBattleEvent.generated.h"

UENUM(BlueprintType)
enum class EFinalBattleEventType : uint8
{
	Info,
	SessionStarted,
	CommandAccepted,
	CommandRejected,
	StateChanged,
	CardResolved,
	UltimateResolved,
	TargetChanged,
	EnemyActed,
	TurnTransition,
	PhaseChanged,
	BattleResolved
};

UENUM(BlueprintType)
enum class EFinalBattleCommandRejectReason : uint8
{
	None,
	BattleAlreadyResolved,
	BattleNotInitialized,
	CardInstanceNotFound,
	CardDefinitionMissing,
	CardNotInHand,
	NotEnoughAP,
	UnsupportedCardEffects,
	UltimateOwnerNotFound,
	UltimateBlockedByCollapse,
	UltimateAlreadyUsed,
	UltimateDefinitionUnavailable,
	NotEnoughEP,
	InvalidTarget,
	UnsupportedCommand
};

USTRUCT(BlueprintType)
struct FINALBATTLE_API FFinalBattleEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 EventSequence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	EFinalBattleEventType EventType = EFinalBattleEventType::Info;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	EFinalBattleCommandRejectReason RejectReason = EFinalBattleCommandRejectReason::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FGuid BattleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FText Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 Round = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FName SourceUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FName TargetUnitId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FName RelatedTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FName ReasonTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FGuid CardInstanceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FFinalCardId CardId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FFinalUltimateId UltimateId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	FFinalStatusId StatusId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 PrimaryValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	int32 SecondaryValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	bool bBattleEnded = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle")
	bool bPlayerVictory = false;
};
