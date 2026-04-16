#pragma once

#include "CoreMinimal.h"
#include "Commands/FinalRunCommand.h"
#include "Events/FinalRunEvent.h"
#include "Queries/FinalRunSnapshot.h"
#include "Requests/FinalBattleResult.h"
#include "Requests/FinalBattleStartRequest.h"
#include "Runtime/FinalRunState.h"
#include "Save/FinalRunSaveData.h"
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

	UFUNCTION(BlueprintPure, Category = "Final|Run|Save")
	FFinalRunSaveData ExportSaveData() const;

	UFUNCTION(BlueprintCallable, Category = "Final|Run|Save")
	bool RestoreFromSaveData(const FFinalRunSaveData& SaveData, FText& OutFailureReason);

private:
	bool TryExecuteClaimPendingBattleReward(FFinalRunEvent& OutDetailEvent, EFinalRunCommandRejectReason& OutRejectReason, FText& OutFailureMessage);
	bool TryExecuteResolveRewardNode(FFinalRunEvent& OutDetailEvent, EFinalRunCommandRejectReason& OutRejectReason, FText& OutFailureMessage);
	bool TryExecuteResolveEventNode(const FName& OptionId, FFinalRunEvent& OutDetailEvent, EFinalRunCommandRejectReason& OutRejectReason, FText& OutFailureMessage);
	bool TryExecuteResolveShopNode(const FName& OfferId, FFinalRunEvent& OutDetailEvent, EFinalRunCommandRejectReason& OutRejectReason, FText& OutFailureMessage);
	bool TryExecuteAdvanceToNode(const FName& TargetNodeId, FFinalRunEvent& OutDetailEvent, EFinalRunCommandRejectReason& OutRejectReason, FText& OutFailureMessage);
	const FFinalRunNodeDefinition* FindNodeDefinition(const FName& NodeId) const;
	TArray<FFinalRunNodeOptionViewData> BuildAvailableNextNodeViews() const;
	FFinalRunPendingRewardNodeViewData BuildPendingRewardNodeView() const;
	FFinalRunPendingEventNodeViewData BuildPendingEventNodeView() const;
	FFinalRunPendingShopNodeViewData BuildPendingShopNodeView() const;
	void ApplyNodeContextFromNode(const FFinalRunNodeDefinition& NodeDefinition);
	void ClearBattleStartContext();
	void PopulateNodeEventMetadata(FFinalRunEvent& Event, const FFinalRunNodeDefinition& NodeDefinition) const;
	void PopulateNodeViewMetadata(FFinalRunNodeOptionViewData& View, const FFinalRunNodeDefinition& NodeDefinition) const;
	void MarkCurrentNodeResolved();
	bool HasPendingBattleReward() const;
	int32 GetPendingBattleRewardGold() const;
	FText GetCurrentNodeStateMessage() const;
	EFinalRunNodeType GetCurrentNodeType() const;
	void AppendEvent(const FFinalRunEvent& Event);

	UPROPERTY(Transient)
	FFinalRunState CurrentState;

	UPROPERTY(Transient)
	TArray<FFinalRunEvent> RunLogEntries;

	TArray<FFinalRunNodeDefinition> ConfiguredRunNodes;
	TSet<FName> VisitedNodeIds;
	TSet<FName> ResolvedNodeIds;
	FName CurrentNodeId = NAME_None;
	EFinalRunFlowStage CurrentFlowStage = EFinalRunFlowStage::None;
	FName PendingRewardSourceNodeId = NAME_None;
	FFinalEncounterId PendingRewardSourceEncounterId;
	EFinalBattleOutcome PendingRewardBattleOutcome = EFinalBattleOutcome::None;
	TArray<FFinalRunRewardEntry> PendingRewardEntries;

	int32 LastEventSequence = 0;
};
