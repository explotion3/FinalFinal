#pragma once

#include "CoreMinimal.h"
#include "Events/FinalRunEvent.h"
#include "Queries/FinalRunSnapshot.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FinalRunFlowSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFinalRunFlowStateChangedSignature);

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
	bool ClaimPendingBattleRewardById(FName RewardId);

	UFUNCTION(BlueprintCallable, Category = "Final|RunFlow")
	bool SkipPendingBattleReward();

	UFUNCTION(BlueprintCallable, Category = "Final|RunFlow")
	bool AdvanceToNode(FName NodeId);

	UFUNCTION(BlueprintCallable, Category = "Final|RunFlow")
	bool ResolveRewardNode();

	UFUNCTION(BlueprintCallable, Category = "Final|RunFlow")
	bool ResolveEventOption(FName OptionId);

	UFUNCTION(BlueprintCallable, Category = "Final|RunFlow")
	bool ResolveShopOffer(FName OfferId);

	UFUNCTION(BlueprintPure, Category = "Final|RunFlow")
	FFinalRunSnapshot GetCurrentRunSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "Final|RunFlow")
	FFinalRunEvent GetLastProcessedRunEvent() const;

	UFUNCTION(BlueprintPure, Category = "Final|RunFlow")
	FText GetLastFlowMessage() const;

	UPROPERTY(BlueprintAssignable, Category = "Final|RunFlow")
	FFinalRunFlowStateChangedSignature OnRunFlowStateChanged;

private:
	void ResetFlowState();
	void ApplyPresentationForSnapshot(const FFinalRunSnapshot& Snapshot, bool bForce);
	void CloseActiveFlowModal() const;
	void CloseActiveFlowOverlay() const;
	EFinalRunPresentedOverlay DetermineDesiredOverlay(const FFinalRunSnapshot& Snapshot) const;
	bool SubmitRunCommand(EFinalRunCommandType CommandType, FName PayloadId, const FText& MissingSessionMessage);
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
