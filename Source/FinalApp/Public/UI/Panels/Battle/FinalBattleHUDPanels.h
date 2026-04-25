#pragma once

#include "CoreMinimal.h"
#include "UI/Panels/FinalPanelWidgetBase.h"
#include "FinalBattleHUDPanels.generated.h"

class UBorder;
class UButton;
class UHorizontalBox;
class UTextBlock;
class UVerticalBox;
class UFinalBattleTopBarPanelController;
class UFinalBattleFeedbackPanelController;
class UFinalBattleContextPanelController;
class UFinalBattleCharacterPanelController;
class UFinalBattleEnemyPanelController;
class UFinalBattleHandPanelController;
class UFinalBattleUltimatePanelController;
class UFinalBattleRecentEventPanelController;
class UFinalBattleActionPanelController;
class UFinalBattleTopBarPanelViewModel;
class UFinalBattleFeedbackPanelViewModel;
class UFinalBattleContextPanelViewModel;
class UFinalBattleCharacterPanelViewModel;
class UFinalBattleEnemyPanelViewModel;
class UFinalBattleHandPanelViewModel;
class UFinalBattleUltimatePanelViewModel;
class UFinalBattleRecentEventPanelViewModel;
class UFinalBattleActionPanelViewModel;

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleTopBarPanel : public UFinalPanelWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void InitializePanel(UFinalBattleTopBarPanelViewModel* InViewModel, UFinalBattleTopBarPanelController* InController);

private:
	UFUNCTION()
	void HandleViewModelChanged();

	void EnsureWidgetTree();
	void RefreshFromViewModel();

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleTopBarPanelViewModel> PanelViewModel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> TopBarText;
};

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleFeedbackPanel : public UFinalPanelWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void InitializePanel(UFinalBattleFeedbackPanelViewModel* InViewModel, UFinalBattleFeedbackPanelController* InController);

private:
	UFUNCTION()
	void HandleViewModelChanged();

	void EnsureWidgetTree();
	void RefreshFromViewModel();

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleFeedbackPanelViewModel> PanelViewModel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> FeedbackText;
};

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleContextPanel : public UFinalPanelWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void InitializePanel(UFinalBattleContextPanelViewModel* InViewModel, UFinalBattleContextPanelController* InController);

private:
	UFUNCTION()
	void HandleViewModelChanged();

	void EnsureWidgetTree();
	void RefreshFromViewModel();

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleContextPanelViewModel> PanelViewModel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> ContextText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UBorder> GapBorder;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> GapText;
};

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleCharacterPanel : public UFinalPanelWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void InitializePanel(UFinalBattleCharacterPanelViewModel* InViewModel, UFinalBattleCharacterPanelController* InController);

private:
	UFUNCTION()
	void HandleViewModelChanged();

	void EnsureWidgetTree();
	void RefreshFromViewModel();

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleCharacterPanelViewModel> PanelViewModel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> CharacterListBox;
};

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleEnemyPanel : public UFinalPanelWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void InitializePanel(UFinalBattleEnemyPanelViewModel* InViewModel, UFinalBattleEnemyPanelController* InController);

private:
	UFUNCTION()
	void HandleViewModelChanged();

	void EnsureWidgetTree();
	void RefreshFromViewModel();

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleEnemyPanelViewModel> PanelViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleEnemyPanelController> PanelController;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> EnemyListBox;
};

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleHandPanel : public UFinalPanelWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void InitializePanel(UFinalBattleHandPanelViewModel* InViewModel, UFinalBattleHandPanelController* InController);

private:
	UFUNCTION()
	void HandleViewModelChanged();

	void EnsureWidgetTree();
	void RefreshFromViewModel();

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleHandPanelViewModel> PanelViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleHandPanelController> PanelController;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UHorizontalBox> HandCardBox;
};

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleUltimatePanel : public UFinalPanelWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void InitializePanel(UFinalBattleUltimatePanelViewModel* InViewModel, UFinalBattleUltimatePanelController* InController);

private:
	UFUNCTION()
	void HandleViewModelChanged();

	void EnsureWidgetTree();
	void RefreshFromViewModel();

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleUltimatePanelViewModel> PanelViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleUltimatePanelController> PanelController;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> UltimateListBox;
};

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleRecentEventPanel : public UFinalPanelWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void InitializePanel(UFinalBattleRecentEventPanelViewModel* InViewModel, UFinalBattleRecentEventPanelController* InController);

private:
	UFUNCTION()
	void HandleViewModelChanged();

	void EnsureWidgetTree();
	void RefreshFromViewModel();

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleRecentEventPanelViewModel> PanelViewModel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> RecentEventListBox;
};

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleActionPanel : public UFinalPanelWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void InitializePanel(UFinalBattleActionPanelViewModel* InViewModel, UFinalBattleActionPanelController* InController);

private:
	UFUNCTION()
	void HandleViewModelChanged();

	UFUNCTION()
	void HandleEndTurnClicked();

	UFUNCTION()
	void HandleOpenDebugClicked();

	UFUNCTION()
	void HandleOpenEventLedgerClicked();

	void EnsureWidgetTree();
	void RefreshFromViewModel();

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleActionPanelViewModel> PanelViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleActionPanelController> PanelController;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UButton> EndTurnButton;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> EndTurnLabel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UButton> OpenDebugButton;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> OpenDebugLabel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UButton> OpenEventLedgerButton;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> OpenEventLedgerLabel;
};
