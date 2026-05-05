#pragma once

#include "CoreMinimal.h"
#include "Events/FinalBattleEvent.h"
#include "Facade/FinalBattleSession.h"
#include "Queries/FinalBattleSnapshot.h"
#include "Controllers/Battle/FinalBattleHUDPanelControllers.h"
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
	bool RequestPlayCardByHandIndex(int32 HandIndex);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	bool PlayCardByHandIndex(int32 HandIndex);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	bool PlayUltimateByCharacterIndex(int32 CharacterIndex);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	bool EndTurn();

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	FName GetSelectedEnemyUnitId() const;

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	bool InspectEnemyByUnitId(FName RuntimeUnitId);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void ClearInspectedEnemy();

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	FName GetInspectedEnemyUnitId() const;

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	bool InspectCharacterByUnitId(FName RuntimeUnitId);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void ClearInspectedCharacter();

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	FName GetInspectedCharacterUnitId() const;

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void InspectCardZone(EFinalBattleCardZone Zone);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void SetSelectedCardZone(EFinalBattleCardZone Zone);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void ClearCardZoneDetail();

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	bool IsCardZoneDetailOpen() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	EFinalBattleCardZone GetSelectedCardZone() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	FText GetLastInteractionFeedback() const;

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void OpenDebugOverlay();

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void OpenEventLedgerOverlay();

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleTopBarPanelController* GetTopBarPanelController() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleResourcePanelController* GetResourcePanelController() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleFeedbackPanelController* GetFeedbackPanelController() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleContextPanelController* GetContextPanelController() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleCharacterPanelController* GetCharacterPanelController() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleEnemyPanelController* GetEnemyPanelController() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleEnemyDetailPanelController* GetEnemyDetailPanelController() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleCharacterDetailPanelController* GetCharacterDetailPanelController() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleHandPanelController* GetHandPanelController() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleCardZoneDetailPanelController* GetCardZoneDetailPanelController() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleUltimatePanelController* GetUltimatePanelController() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleRecentEventPanelController* GetRecentEventPanelController() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleActionPanelController* GetActionPanelController() const;

	virtual void ShutdownController() override;

private:
	UFUNCTION()
	void HandleBattleSnapshotChanged(const FFinalBattleSnapshot& Snapshot);

	UFUNCTION()
	void HandleBattleEventBroadcast(const FFinalBattleEvent& BattleEvent);

	void RebuildPresentation();
	void EnsurePanelControllers();
	void RefreshSelectedEnemyFromSnapshot();
	void RefreshInspectedEnemyFromSnapshot();
	void RefreshInspectedCharacterFromSnapshot();
	void RefreshInspectedCardZoneFromSnapshot();
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
	FName InspectedEnemyUnitId = NAME_None;

	UPROPERTY(Transient)
	FName InspectedCharacterUnitId = NAME_None;

	UPROPERTY(Transient)
	bool bCardZoneDetailOpen = false;

	UPROPERTY(Transient)
	EFinalBattleCardZone SelectedCardZone = EFinalBattleCardZone::DrawPile;

	UPROPERTY(Transient)
	FText LastInteractionFeedback;

	UPROPERTY(Transient)
	FFinalBattleEvent LastInteractionEvent;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleTopBarPanelController> TopBarPanelController;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleResourcePanelController> ResourcePanelController;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleFeedbackPanelController> FeedbackPanelController;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleContextPanelController> ContextPanelController;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleCharacterPanelController> CharacterPanelController;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleEnemyPanelController> EnemyPanelController;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleEnemyDetailPanelController> EnemyDetailPanelController;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleCharacterDetailPanelController> CharacterDetailPanelController;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleHandPanelController> HandPanelController;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleCardZoneDetailPanelController> CardZoneDetailPanelController;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleUltimatePanelController> UltimatePanelController;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleRecentEventPanelController> RecentEventPanelController;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleActionPanelController> ActionPanelController;
};
