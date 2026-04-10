#pragma once

#include "CoreMinimal.h"
#include "Commands/FinalRunCommand.h"
#include "Events/FinalRunEvent.h"
#include "Queries/FinalRunSnapshot.h"
#include "Requests/FinalBattleResult.h"
#include "Requests/FinalBattleStartRequest.h"
#include "Runtime/FinalRunState.h"
#include "UObject/Object.h"
#include "FinalRunSession.generated.h"

UCLASS(BlueprintType)
class FINALRUN_API UFinalRunSession : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Final|Run")
	void InitializeRun();

	UFUNCTION(BlueprintCallable, Category = "Final|Run")
	void ConfigureRunNodeGraph(const TArray<FFinalRunNodeDefinition>& NodeDefinitions, FName InCurrentNodeId);

	UFUNCTION(BlueprintCallable, Category = "Final|Run")
	void ConfigureBattleStartState(const FFinalEncounterId& EncounterId, const FFinalRuleConfigId& RuleConfigId, const TArray<FFinalRunPersistentCharacterState>& PartyStates, const TArray<FFinalCardId>& DeckCardIds, int32 InTeamCurrentHP = 0);

	UFUNCTION(BlueprintCallable, Category = "Final|Run")
	bool SubmitRunCommand(const FFinalRunCommand& Command);

	UFUNCTION(BlueprintCallable, Category = "Final|Run")
	bool ClaimPendingBattleReward();

	UFUNCTION(BlueprintCallable, Category = "Final|Run")
	bool AdvanceToNode(FName NodeId);

	UFUNCTION(BlueprintPure, Category = "Final|Run")
	bool HasValidBattleStartState() const;

	UFUNCTION(BlueprintPure, Category = "Final|Run")
	FFinalBattleStartRequest BuildBattleStartRequest() const;

	UFUNCTION(BlueprintCallable, Category = "Final|Run")
	void ApplyBattleResult(const FFinalBattleResult& Result);

	UFUNCTION(BlueprintPure, Category = "Final|Run")
	FFinalRunSnapshot GetSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "Final|Run")
	FFinalRunState GetRunState() const;

	UFUNCTION(BlueprintPure, Category = "Final|Run")
	TArray<FFinalRunEvent> GetRunLogEntries() const;

	UFUNCTION(BlueprintPure, Category = "Final|Run")
	TArray<FFinalRunEvent> GetRunEventsSince(int32 LastSeenEventSequence) const;

	UFUNCTION(BlueprintPure, Category = "Final|Run")
	int32 GetLatestRunEventSequence() const;

private:
	bool TryExecuteClaimPendingBattleReward(FFinalRunEvent& OutDetailEvent, EFinalRunCommandRejectReason& OutRejectReason, FText& OutFailureMessage);
	bool TryExecuteAdvanceToNode(const FName& TargetNodeId, FFinalRunEvent& OutDetailEvent, EFinalRunCommandRejectReason& OutRejectReason, FText& OutFailureMessage);
	const FFinalRunNodeDefinition* FindNodeDefinition(const FName& NodeId) const;
	TArray<FFinalRunNodeOptionViewData> BuildAvailableNextNodeViews() const;
	void ApplyBattleStartContextFromNode(const FFinalRunNodeDefinition& NodeDefinition);
	EFinalRunNodeType GetCurrentNodeType() const;
	void AppendEvent(const FFinalRunEvent& Event);

	UPROPERTY(Transient)
	FFinalRunState CurrentState;

	UPROPERTY(Transient)
	TArray<FFinalRunEvent> RunLogEntries;

	TArray<FFinalRunNodeDefinition> ConfiguredRunNodes;
	FName CurrentNodeId = NAME_None;
	EFinalRunFlowStage CurrentFlowStage = EFinalRunFlowStage::None;
	FName PendingRewardSourceNodeId = NAME_None;
	FFinalEncounterId PendingRewardSourceEncounterId;
	EFinalBattleOutcome PendingRewardBattleOutcome = EFinalBattleOutcome::None;
	int32 PendingRewardGold = 0;

	int32 LastEventSequence = 0;
};
