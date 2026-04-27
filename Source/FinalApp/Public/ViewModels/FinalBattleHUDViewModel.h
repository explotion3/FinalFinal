#pragma once

#include "CoreMinimal.h"
#include "Events/FinalBattleEvent.h"
#include "Queries/FinalBattleSnapshot.h"
#include "UI/ViewModels/Battle/FinalBattleHUDPanelViewModels.h"
#include "UI/ViewModels/FinalViewModelBase.h"
#include "FinalBattleHUDViewModel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFinalPhaseChangedPresentationSignature, const FFinalBattleEvent&, BattleEvent);

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleHUDViewModel : public UFinalViewModelBase
{
	GENERATED_BODY()

public:
	void EnsurePanelViewModels();

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void ApplySnapshot(const FFinalBattleSnapshot& InSnapshot);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void ApplyBattleEvent(const FFinalBattleEvent& BattleEvent);

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	FFinalBattleSnapshot GetSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	FFinalBattleEvent GetLatestPhaseChangedEvent() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleTopBarPanelViewModel* GetTopBarViewModel() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleResourcePanelViewModel* GetResourceViewModel() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleFeedbackPanelViewModel* GetFeedbackViewModel() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleContextPanelViewModel* GetContextViewModel() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleCharacterPanelViewModel* GetCharacterViewModel() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleEnemyPanelViewModel* GetEnemyViewModel() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleHandPanelViewModel* GetHandViewModel() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleUltimatePanelViewModel* GetUltimateViewModel() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleRecentEventPanelViewModel* GetRecentEventViewModel() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleActionPanelViewModel* GetActionViewModel() const;

	UPROPERTY(BlueprintAssignable, Category = "Final|UI")
	FFinalPhaseChangedPresentationSignature OnPhaseChangedPresentation;

private:
	UPROPERTY(Transient)
	FFinalBattleSnapshot Snapshot;

	UPROPERTY(Transient)
	FFinalBattleEvent LatestPhaseChangedEvent;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleTopBarPanelViewModel> TopBarViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleResourcePanelViewModel> ResourceViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleFeedbackPanelViewModel> FeedbackViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleContextPanelViewModel> ContextViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleCharacterPanelViewModel> CharacterViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleEnemyPanelViewModel> EnemyViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleHandPanelViewModel> HandViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleUltimatePanelViewModel> UltimateViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleRecentEventPanelViewModel> RecentEventViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleActionPanelViewModel> ActionViewModel;
};
