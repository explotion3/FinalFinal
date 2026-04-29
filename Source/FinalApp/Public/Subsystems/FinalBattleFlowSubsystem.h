#pragma once

#include "CoreMinimal.h"
#include "Commands/FinalBattleCommand.h"
#include "Events/FinalBattleEvent.h"
#include "Facade/FinalBattleSession.h"
#include "Facade/FinalBattleSessionTypes.h"
#include "Queries/FinalBattleSnapshot.h"
#include "Requests/FinalBattleStartRequest.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FinalBattleFlowSubsystem.generated.h"

class UFinalBattleEncounterDefinition;
class UFinalBattleRuleConfig;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFinalBattleSnapshotChangedSignature, const FFinalBattleSnapshot&, Snapshot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFinalBattleEventBroadcastSignature, const FFinalBattleEvent&, BattleEvent);

UCLASS()
class FINALAPP_API UFinalBattleFlowSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Final|Battle")
	UFinalBattleSession* CreateBattleSessionFromStartRequest(const FFinalBattleStartRequest& StartRequest);

	UFUNCTION(BlueprintCallable, Category = "Final|Battle")
	bool SubmitBattleCommand(const FFinalBattleCommand& Command);

	UFUNCTION(BlueprintCallable, Category = "Final|Battle")
	void ClearActiveBattleSession();

	UFUNCTION(BlueprintPure, Category = "Final|Battle")
	FFinalBattleSnapshot GetCurrentSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "Final|Battle")
	UFinalBattleSession* GetActiveBattleSession() const;

	UFUNCTION(BlueprintPure, Category = "Final|Battle")
	FText GetLastFailureReason() const;

	UFUNCTION(BlueprintPure, Category = "Final|Battle")
	FFinalBattleEvent GetLastCommandEvent() const;

	UFUNCTION(BlueprintPure, Category = "Final|Battle")
	TArray<FFinalBattleEvent> GetBattleLogEntries() const;

	UFUNCTION(BlueprintPure, Category = "Final|Battle")
	TArray<FFinalBattleEvent> GetBattleEventsSince(int32 LastSeenEventSequence) const;

	UFUNCTION(BlueprintPure, Category = "Final|Battle")
	int32 GetLatestBattleEventSequence() const;

	bool RefreshCharacterRuntimeStats(const FFinalBattleCharacterRuntimeStats& RuntimeStats);

	UPROPERTY(BlueprintAssignable, Category = "Final|Battle")
	FFinalBattleSnapshotChangedSignature OnBattleSnapshotChanged;

	UPROPERTY(BlueprintAssignable, Category = "Final|Battle")
	FFinalBattleEventBroadcastSignature OnBattleEventBroadcast;

private:
	bool BuildInitContext(const FFinalBattleStartRequest& StartRequest, FFinalBattleInitContext& OutInitContext);
	void BroadcastPendingBattleEvents();
	void BroadcastSnapshot();

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleSession> ActiveBattleSession;

	UPROPERTY(Transient)
	FFinalBattleStartRequest LastStartRequest;

	UPROPERTY(Transient)
	FText LastFailureReason;

	UPROPERTY(Transient)
	FFinalBattleEvent LastCommandEvent;

	UPROPERTY(Transient)
	int32 BroadcastBattleLogCount = 0;
};
