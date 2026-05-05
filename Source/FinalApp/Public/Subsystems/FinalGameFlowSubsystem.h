#pragma once

#include "CoreMinimal.h"
#include "Facade/FinalBattleSession.h"
#include "Facade/FinalRunSession.h"
#include "Queries/FinalBattleSnapshot.h"
#include "Run/Bridge/FinalBattleGrowthFact.h"
#include "Requests/FinalBattleResult.h"
#include "Save/FinalRunSaveData.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FinalGameFlowSubsystem.generated.h"

class UFinalCharacterGrowthConfig;
class UFinalBattleFlowSubsystem;

UCLASS()
class FINALAPP_API UFinalGameFlowSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Final|Flow")
	UFinalRunSession* BootstrapNewRun();

	UFUNCTION(BlueprintCallable, Category = "Final|Flow")
	UFinalBattleSession* StartBattleFromRunSession();

	bool TryAutoStartPreparedBattleFromRun();

	UFUNCTION(BlueprintCallable, Category = "Final|Flow")
	bool CompleteBattleAndApplyResult(const FFinalBattleResult& Result);

	UFUNCTION(BlueprintCallable, Category = "Final|Flow")
	bool CompleteResolvedBattle();

	UFUNCTION(BlueprintPure, Category = "Final|Flow")
	UFinalRunSession* GetRunSession() const;

	UFUNCTION(BlueprintPure, Category = "Final|Flow")
	UFinalBattleSession* GetActiveBattleSession() const;

	UFUNCTION(BlueprintPure, Category = "Final|Flow")
	FFinalBattleSnapshot GetCurrentBattleSnapshot() const;

	UFUNCTION(BlueprintCallable, Category = "Final|Flow|Save")
	bool RestoreRunSessionFromSaveData(const FFinalRunSaveData& SaveData, FText& OutFailureReason);

	UFUNCTION(BlueprintPure, Category = "Final|Flow")
	FText GetLastBattleFailureReason() const;

	void TryRefreshActiveBattleCharacterFromRunState(const FFinalCharacterId& CharacterId);
	int32 TryRefreshActiveBattleCardFromRunState(FName RunCardInstanceId);

private:
	bool BuildResolvedBattleResult(FFinalBattleResult& OutResult);
	UFUNCTION()
	void HandleBattleSnapshotChanged(const FFinalBattleSnapshot& Snapshot);
	void ProcessPendingBattleGrowthFacts(const FFinalBattleSnapshot& Snapshot);
	bool TryPresentPendingGrowthChoiceAtSafeWindow(const FFinalBattleSnapshot& Snapshot, bool bPendingCreatedThisTick);
	int32 ResolveBreakthroughGainFromFact(const FFinalBattleGrowthFact& Fact, const FFinalRunPersistentCharacterState& CharacterState) const;
	int32 ResolveBattleVictoryRewardForNode(EFinalRunNodeType NodeType, const UFinalCharacterGrowthConfig& GrowthConfig) const;
	bool BuildProjectedRuntimeStatsForCharacter(const FFinalCharacterId& CharacterId, FFinalBattleCharacterRuntimeStats& OutRuntimeStats) const;
	void BindToBattleFlowSubsystem(UFinalBattleFlowSubsystem* BattleFlowSubsystem);
	void UnbindFromBattleFlowSubsystem(UFinalBattleFlowSubsystem* BattleFlowSubsystem);

	UPROPERTY(Transient)
	TObjectPtr<UFinalRunSession> RunSession;

	UPROPERTY(Transient)
	FText LastFlowFailureReason;

	bool bAutoStartingPreparedBattle = false;
	int32 LastProcessedGrowthFactBatchSequence = 0;
	bool bPendingGrowthChoiceDeferredFromEnemyPhase = false;
	FName PresentedPendingGrowthChoiceKey = NAME_None;
};
