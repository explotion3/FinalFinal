#pragma once

#include "CoreMinimal.h"
#include "UI/Panels/FinalPanelWidgetBase.h"
#include "FinalBattleHUDPanels.generated.h"

class UBorder;
class UButton;
class UCanvasPanel;
class UHorizontalBox;
class UImage;
class UTextBlock;
class UVerticalBox;
class UFinalBattleTopBarPanelController;
class UFinalBattleResourcePanelController;
class UFinalBattleFeedbackPanelController;
class UFinalBattleContextPanelController;
class UFinalBattleCharacterPanelController;
class UFinalBattleEnemyPanelController;
class UFinalBattleHandPanelController;
class UFinalBattleUltimatePanelController;
class UFinalBattleRecentEventPanelController;
class UFinalBattleActionPanelController;
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
class UFinalBattleCardEntryWidget;
class UFinalRunFlowSubsystem;

struct FFinalBattleHandCardVisualState
{
	FGuid CardInstanceId;
	TWeakObjectPtr<UFinalBattleCardEntryWidget> Widget;
	int32 HandIndex = INDEX_NONE;
	FVector2D CurrentPosition = FVector2D::ZeroVector;
	FVector2D TargetPosition = FVector2D::ZeroVector;
	float CurrentAngle = 0.0f;
	float TargetAngle = 0.0f;
	float CurrentScale = 1.0f;
	float TargetScale = 1.0f;
	float HoverAlpha = 0.0f;
	int32 BaseZOrder = 0;
	bool bCanPlayHint = true;
	bool bEntering = false;
	bool bLeaving = false;
	bool bSnapToTargetOnNextArrange = false;
};

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
class FINALAPP_API UFinalBattleResourcePanel : public UFinalPanelWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void InitializePanel(UFinalBattleResourcePanelViewModel* InViewModel, UFinalBattleResourcePanelController* InController);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Resource")
	FSlateColor NormalEPColor = FSlateColor(FLinearColor::White);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Resource")
	FSlateColor FullEPColor = FSlateColor(FLinearColor(0.92f, 0.12f, 0.08f, 1.0f));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Resource")
	FLinearColor ActiveQiPipBaseColor = FLinearColor(0.75f, 0.04f, 0.04f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Resource")
	FLinearColor InactiveQiPipBaseColor = FLinearColor(0.35f, 0.35f, 0.35f, 0.5f);

private:
	UFUNCTION()
	void HandleViewModelChanged();

	void EnsureWidgetTree();
	void RefreshFromViewModel();

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleResourcePanelViewModel> PanelViewModel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> APLabelText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> APText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> EPLabelText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> EPText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> QiLabelText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> ResourceText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UImage> QIPipBase0;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UImage> QIPipBase1;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UImage> QIPipBase2;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UImage> QIPipBase3;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UImage> QIPipBase4;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UImage> QIPipBase5;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UImage> QIPipBase6;
};

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalRunFlowPromptPanel : public UFinalPanelWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "Final|RunFlow")
	void RefreshPrompt();

private:
	UFUNCTION()
	void HandleOpenFlowClicked();

	UFUNCTION()
	void HandleRunFlowStateChanged();

	void EnsureWidgetTree();
	UFinalRunFlowSubsystem* ResolveRunFlowSubsystem() const;
	bool ShouldShowPrompt() const;
	FText BuildPromptText() const;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UButton> OpenFlowButton;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> OpenFlowLabel;
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
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void InitializePanel(UFinalBattleHandPanelViewModel* InViewModel, UFinalBattleHandPanelController* InController);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout")
	FVector2D CardSize = FVector2D(260.0f, 380.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout")
	float PanelHeightOverride = 430.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout")
	float BottomPadding = 24.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout")
	float MinSpacing = 96.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout")
	float MaxSpacing = 242.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout")
	float CenterLift = 28.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout")
	float MaxFanAngle = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout")
	float CardScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout")
	float HoverScale = 1.32f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout")
	float HoverLift = 110.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout")
	float HoverAngle = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout")
	int32 HoverZOrder = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout")
	float HoverInterpSpeed = 14.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout", meta = (ClampMin = "0.0"))
	float UnplayableDropMin = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout", meta = (ClampMin = "0.0"))
	float UnplayableDropMax = 24.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UnplayableOpacity = 0.62f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout")
	FVector2D EnterStartOffset = FVector2D(-120.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout")
	FVector2D ExitTargetOffset = FVector2D(120.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout")
	float EnterInterpSpeed = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout")
	float MoveInterpSpeed = 14.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout")
	float ExitInterpSpeed = 16.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout")
	float RemoveDistanceTolerance = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout")
	bool bAnimateInitialHand = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Battle HUD|Hand Layout")
	bool bAllowOverlap = true;

private:
	UFUNCTION()
	void HandleViewModelChanged();

	UFUNCTION()
	void HandleCardHoverChanged(FGuid CardInstanceId, int32 HandIndex, bool bHovered);

	void EnsureWidgetTree();
	void RefreshFromViewModel();
	void ArrangeHandCards();
	bool UpdateHoverAlphas(float InDeltaTime);
	bool UpdateCardVisuals(float InDeltaTime);
	void ApplyCardVisualState(const FFinalBattleHandCardVisualState& VisualState);

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleHandPanelViewModel> PanelViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleHandPanelController> PanelController;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UCanvasPanel> HandCardCanvas;

	TMap<FGuid, FFinalBattleHandCardVisualState> CardVisuals;
	TArray<FGuid> OrderedCardInstanceIds;
	FGuid HoveredCardInstanceId;

	bool bHandLayoutDirty = false;
	bool bHasReceivedHandSnapshot = false;
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
