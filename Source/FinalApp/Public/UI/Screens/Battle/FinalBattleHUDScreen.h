#pragma once

#include "CoreMinimal.h"
#include "UI/Screens/FinalScreenBase.h"
#include "FinalBattleHUDScreen.generated.h"

class UButton;
class UHorizontalBox;
class UScrollBox;
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

	void EnsureWidgetTree();
	void RefreshFromViewModel();
	void RebuildCharacterPanel();
	void RebuildEnemyPanel();
	void RebuildHandPanel();
	void RebuildUltimatePanel();
	void RebuildLogPanel();

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleHUDViewModel> BattleViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleWidgetController> BattleController;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HeaderText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> FeedbackText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> GapText;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> CharacterListBox;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> EnemyListBox;

	UPROPERTY(Transient)
	TObjectPtr<UHorizontalBox> HandCardBox;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> UltimateButtonBox;

	UPROPERTY(Transient)
	TObjectPtr<UScrollBox> LogScrollBox;

	UPROPERTY(Transient)
	TObjectPtr<UButton> EndTurnButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> EndTurnLabel;
};
