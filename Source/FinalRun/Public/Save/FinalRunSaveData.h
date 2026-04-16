#pragma once

#include "CoreMinimal.h"
#include "Events/FinalRunEvent.h"
#include "Queries/FinalRunQueryTypes.h"
#include "Requests/FinalBattleResult.h"
#include "Run/Definitions/FinalRunNodeDefinition.h"
#include "Run/Rewards/FinalRunRewardTypes.h"
#include "Runtime/FinalRunState.h"
#include "FinalRunSaveData.generated.h"

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunSaveData
{
	GENERATED_BODY()

	static constexpr int32 CurrentSaveVersion = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Final|Run|Save")
	int32 SaveVersion = CurrentSaveVersion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Final|Run|Save")
	FFinalRunState RunState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Final|Run|Save")
	TArray<FFinalRunEvent> RunLogEntries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Final|Run|Save")
	int32 LastEventSequence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Final|Run|Save")
	TArray<FFinalRunNodeDefinition> ConfiguredRunNodes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Final|Run|Save")
	FName CurrentNodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Final|Run|Save")
	TArray<FName> VisitedNodeIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Final|Run|Save")
	TArray<FName> ResolvedNodeIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Final|Run|Save")
	EFinalRunFlowStage CurrentFlowStage = EFinalRunFlowStage::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Final|Run|Save")
	FName PendingRewardSourceNodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Final|Run|Save")
	FFinalEncounterId PendingRewardSourceEncounterId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Final|Run|Save")
	EFinalBattleOutcome PendingRewardBattleOutcome = EFinalBattleOutcome::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Final|Run|Save")
	TArray<FFinalRunRewardEntry> PendingRewardEntries;

	bool IsSupportedVersion() const;
	bool IsStructurallyValid(FText* OutFailureReason = nullptr) const;
	int32 GetMaxRunLogEventSequence() const;
};
