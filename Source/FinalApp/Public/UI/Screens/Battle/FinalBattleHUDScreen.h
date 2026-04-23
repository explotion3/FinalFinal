#pragma once

#include "CoreMinimal.h"
#include "UI/Screens/FinalScreenBase.h"
#include "FinalBattleHUDScreen.generated.h"

class UButton;
class UBorder;
class UHorizontalBox;
class UTextBlock;
class UVerticalBox;
class UFinalBattleHUDViewModel;
class UFinalBattleWidgetController;

UCLASS()
class FINALAPP_API UFinalBattleHUDScreen : public UFinalScreenBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void InitializeScreen(UFinalBattleHUDViewModel* InViewModel, UFinalBattleWidgetController* InController);

	void HandleEnemySelected(FName RuntimeUnitId);
	void HandlePlayCard(int32 HandIndex);
	void HandlePlayUltimate(int32 CharacterIndex);

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
	void RefreshTopBarSection(const struct FFinalBattleHUDPresentationData& Presentation);
	void RefreshFeedbackSection(const struct FFinalBattleHUDPresentationData& Presentation);
	void RefreshContextSection(const struct FFinalBattleHUDPresentationData& Presentation);
	void RefreshCharacterPanel(const struct FFinalBattleHUDPresentationData& Presentation);
	void RefreshEnemyPanel(const struct FFinalBattleHUDPresentationData& Presentation);
	void RefreshHandPanel(const struct FFinalBattleHUDPresentationData& Presentation);
	void RefreshUltimatePanel(const struct FFinalBattleHUDPresentationData& Presentation);
	void RefreshRecentEventPanel(const struct FFinalBattleHUDPresentationData& Presentation);
	void RefreshActionSection(const struct FFinalBattleHUDPresentationData& Presentation);

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleHUDViewModel> BattleViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleWidgetController> BattleController;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TopBarText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> FeedbackText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> AuxiliaryContextText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> GapText;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> GapBorder;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> CharacterListBox;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> EnemyListBox;

	UPROPERTY(Transient)
	TObjectPtr<UHorizontalBox> HandCardBox;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> UltimateButtonBox;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> RecentEventListBox;

	UPROPERTY(Transient)
	TObjectPtr<UButton> EndTurnButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> EndTurnLabel;

	UPROPERTY(Transient)
	TObjectPtr<UButton> OpenDebugButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> OpenDebugLabel;

	UPROPERTY(Transient)
	TObjectPtr<UButton> OpenEventLedgerButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> OpenEventLedgerLabel;
};
