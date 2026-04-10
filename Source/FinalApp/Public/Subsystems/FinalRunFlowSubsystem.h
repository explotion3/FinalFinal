#pragma once

#include "CoreMinimal.h"
#include "Events/FinalRunEvent.h"
#include "Queries/FinalRunSnapshot.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FinalRunFlowSubsystem.generated.h"

enum class EFinalRunPresentedOverlay : uint8
{
	None,
	BattleReward,
	NodeSelect,
	RewardNode,
	EventNode,
	ShopNode
};

UCLASS()
class FINALAPP_API UFinalRunFlowSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Final|RunFlow")
	void HandleRunSessionChanged();

	UFUNCTION(BlueprintCallable, Category = "Final|RunFlow")
	void RefreshRunFlow(bool bForce = false);

	UFUNCTION(BlueprintCallable, Category = "Final|RunFlow")
	bool ClaimPendingBattleReward();

	UFUNCTION(BlueprintCallable, Category = "Final|RunFlow")
	bool AdvanceToNode(FName NodeId);

	UFUNCTION(BlueprintPure, Category = "Final|RunFlow")
	FFinalRunSnapshot GetCurrentRunSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "Final|RunFlow")
	FFinalRunEvent GetLastProcessedRunEvent() const;

	UFUNCTION(BlueprintPure, Category = "Final|RunFlow")
	FText GetLastFlowMessage() const;

private:
	void ResetFlowState();
	void ApplyPresentationForSnapshot(const FFinalRunSnapshot& Snapshot, bool bForce);
	void CloseActiveFlowModal() const;
	void CloseActiveFlowOverlay() const;
	EFinalRunPresentedOverlay DetermineDesiredOverlay(const FFinalRunSnapshot& Snapshot) const;
	class UFinalRunSession* ResolveRunSession() const;
	class UFinalUISubsystem* ResolveUISubsystem() const;

	UPROPERTY(Transient)
	FFinalRunSnapshot CachedSnapshot;

	UPROPERTY(Transient)
	FFinalRunEvent LastProcessedRunEvent;

	UPROPERTY(Transient)
	FText LastFlowMessage;

	UPROPERTY(Transient)
	int32 LastSeenRunEventSequence = 0;

	EFinalRunPresentedOverlay PresentedOverlay = EFinalRunPresentedOverlay::None;
};
