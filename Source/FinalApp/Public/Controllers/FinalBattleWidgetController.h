#pragma once

#include "CoreMinimal.h"
#include "Events/FinalBattleEvent.h"
#include "Facade/FinalBattleSession.h"
#include "Queries/FinalBattleSnapshot.h"
#include "UI/Controllers/FinalWidgetControllerBase.h"
#include "ViewModels/FinalBattleHUDViewModel.h"
#include "FinalBattleWidgetController.generated.h"

class UFinalBattleFlowSubsystem;

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleWidgetController : public UFinalWidgetControllerBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void Initialize(UFinalBattleHUDViewModel* InViewModel);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void BindToBattleFlow(UFinalBattleFlowSubsystem* InBattleFlowSubsystem);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void UnbindFromBattleFlow();

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void RefreshFromSession(UFinalBattleSession* Session);

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleHUDViewModel* GetViewModel() const;

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	bool SelectEnemyByUnitId(FName RuntimeUnitId);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	bool PlayCardByHandIndex(int32 HandIndex);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	bool PlayUltimateByCharacterIndex(int32 CharacterIndex);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	bool EndTurn();

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	FName GetSelectedEnemyUnitId() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	FText GetLastInteractionFeedback() const;

	virtual void ShutdownController() override;

private:
	UFUNCTION()
	void HandleBattleSnapshotChanged(const FFinalBattleSnapshot& Snapshot);

	UFUNCTION()
	void HandleBattleEventBroadcast(const FFinalBattleEvent& BattleEvent);

	void RebuildPresentation();
	void RefreshSelectedEnemyFromSnapshot();
	FName ResolveDefaultTargetUnitId() const;
	bool SubmitBattleCommandWithFeedback(const FFinalBattleCommand& Command);

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleHUDViewModel> ViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleFlowSubsystem> BattleFlowSubsystem;

	UPROPERTY(Transient)
	FFinalBattleSnapshot CachedSnapshot;

	UPROPERTY(Transient)
	TArray<FFinalBattleEvent> CachedBattleEvents;

	UPROPERTY(Transient)
	FName SelectedEnemyUnitId = NAME_None;

	UPROPERTY(Transient)
	FText LastInteractionFeedback;

	UPROPERTY(Transient)
	FFinalBattleEvent LastInteractionEvent;
};
