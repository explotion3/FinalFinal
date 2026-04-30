#pragma once

#include "CoreMinimal.h"
#include "Queries/FinalRunSnapshot.h"
#include "UI/ViewModels/Battle/FinalBattleHUDTypes.h"
#include "UI/Controllers/FinalWidgetControllerBase.h"
#include "FinalBattleHUDPanelControllers.generated.h"

class UFinalBattleWidgetController;
class UFinalDataRegistry;
class UFinalBattleTopBarPanelViewModel;
class UFinalBattleResourcePanelViewModel;
class UFinalBattleFeedbackPanelViewModel;
class UFinalBattleContextPanelViewModel;
class UFinalBattleCharacterPanelViewModel;
class UFinalBattleEnemyPanelViewModel;
class UFinalBattleHandPanelViewModel;
class UFinalBattleUltimatePanelViewModel;
class UFinalBattleRecentEventPanelViewModel;
class UFinalBattleActionPanelViewModel;

struct FFinalBattleHUDCoordinatorData
{
	const FFinalBattleSnapshot* Snapshot = nullptr;
	const FFinalRunSnapshot* RunSnapshot = nullptr;
	const TArray<FFinalBattleEvent>* BattleEvents = nullptr;
	const UFinalDataRegistry* DataRegistry = nullptr;
	FName SelectedEnemyUnitId = NAME_None;
	FText LastInteractionFeedback;
	FFinalBattleEvent LastInteractionEvent;
};

FINALAPP_API FText ResolveBattleHUDEventFeedbackTitleText(
	const FFinalBattleEvent& Event,
	const FText& FallbackMessage,
	const TArray<FFinalBattleStartRelicInput>& ActiveRelics);

UCLASS(Abstract, BlueprintType)
class FINALAPP_API UFinalBattleHUDPanelControllerBase : public UFinalWidgetControllerBase
{
	GENERATED_BODY()

public:
	void InitializePanelController(UFinalBattleWidgetController* InCoordinator);

	virtual void RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData);

protected:
	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleWidgetController> Coordinator;
};

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleTopBarPanelController : public UFinalBattleHUDPanelControllerBase
{
	GENERATED_BODY()

public:
	void InitializeTopBar(UFinalBattleWidgetController* InCoordinator, UFinalBattleTopBarPanelViewModel* InViewModel);
	virtual void RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleTopBarPanelViewModel> ViewModel;
};

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleResourcePanelController : public UFinalBattleHUDPanelControllerBase
{
	GENERATED_BODY()

public:
	void InitializeResourcePanel(UFinalBattleWidgetController* InCoordinator, UFinalBattleResourcePanelViewModel* InViewModel);
	virtual void RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleResourcePanelViewModel> ViewModel;
};

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleFeedbackPanelController : public UFinalBattleHUDPanelControllerBase
{
	GENERATED_BODY()

public:
	void InitializeFeedback(UFinalBattleWidgetController* InCoordinator, UFinalBattleFeedbackPanelViewModel* InViewModel);
	virtual void RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleFeedbackPanelViewModel> ViewModel;
};

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleContextPanelController : public UFinalBattleHUDPanelControllerBase
{
	GENERATED_BODY()

public:
	void InitializeContext(UFinalBattleWidgetController* InCoordinator, UFinalBattleContextPanelViewModel* InViewModel);
	virtual void RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleContextPanelViewModel> ViewModel;
};

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleCharacterPanelController : public UFinalBattleHUDPanelControllerBase
{
	GENERATED_BODY()

public:
	void InitializeCharacterPanel(UFinalBattleWidgetController* InCoordinator, UFinalBattleCharacterPanelViewModel* InViewModel);
	virtual void RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleCharacterPanelViewModel> ViewModel;
};

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleEnemyPanelController : public UFinalBattleHUDPanelControllerBase
{
	GENERATED_BODY()

public:
	void InitializeEnemyPanel(UFinalBattleWidgetController* InCoordinator, UFinalBattleEnemyPanelViewModel* InViewModel);
	virtual void RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData) override;

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	bool SelectEnemyByUnitId(FName RuntimeUnitId);

private:
	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleEnemyPanelViewModel> ViewModel;
};

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleHandPanelController : public UFinalBattleHUDPanelControllerBase
{
	GENERATED_BODY()

public:
	void InitializeHandPanel(UFinalBattleWidgetController* InCoordinator, UFinalBattleHandPanelViewModel* InViewModel);
	virtual void RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData) override;

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	bool PlayCardByHandIndex(int32 HandIndex);

private:
	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleHandPanelViewModel> ViewModel;
};

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleUltimatePanelController : public UFinalBattleHUDPanelControllerBase
{
	GENERATED_BODY()

public:
	void InitializeUltimatePanel(UFinalBattleWidgetController* InCoordinator, UFinalBattleUltimatePanelViewModel* InViewModel);
	virtual void RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData) override;

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	bool PlayUltimateByCharacterIndex(int32 CharacterIndex);

private:
	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleUltimatePanelViewModel> ViewModel;
};

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleRecentEventPanelController : public UFinalBattleHUDPanelControllerBase
{
	GENERATED_BODY()

public:
	void InitializeRecentEventPanel(UFinalBattleWidgetController* InCoordinator, UFinalBattleRecentEventPanelViewModel* InViewModel);
	virtual void RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleRecentEventPanelViewModel> ViewModel;
};

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleActionPanelController : public UFinalBattleHUDPanelControllerBase
{
	GENERATED_BODY()

public:
	void InitializeActionPanel(UFinalBattleWidgetController* InCoordinator, UFinalBattleActionPanelViewModel* InViewModel);
	virtual void RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData) override;

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	bool EndTurn();

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void OpenDebugOverlay();

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void OpenEventLedgerOverlay();

private:
	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleActionPanelViewModel> ViewModel;
};
