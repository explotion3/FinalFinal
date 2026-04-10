#pragma once

#include "CoreMinimal.h"
#include "Events/FinalBattleEvent.h"
#include "Facade/FinalBattleSession.h"
#include "ViewModels/FinalBattleHUDViewModel.h"
#include "UObject/Object.h"
#include "FinalBattleWidgetController.generated.h"

class UFinalBattleFlowSubsystem;

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleWidgetController : public UObject
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

private:
	UFUNCTION()
	void HandleBattleSnapshotChanged(const FFinalBattleSnapshot& Snapshot);

	UFUNCTION()
	void HandleBattleEventBroadcast(const FFinalBattleEvent& BattleEvent);

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleHUDViewModel> ViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleFlowSubsystem> BattleFlowSubsystem;
};
