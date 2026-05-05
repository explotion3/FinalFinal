#pragma once

#include "CoreMinimal.h"
#include "UI/Screens/FinalScreenBase.h"
#include "FinalBattleHUDScreen.generated.h"

class UVerticalBox;
class UOverlay;
class UWidget;
class UFinalBattleHUDViewModel;
class UFinalBattleWidgetController;
class UFinalBattleTopBarPanel;
class UFinalBattleResourcePanel;
class UFinalRunFlowPromptPanel;
class UFinalBattleFeedbackPanel;
class UFinalBattleContextPanel;
class UFinalBattleCharacterPanel;
class UFinalBattleEnemyPanel;
class UFinalBattleEnemyDetailPanel;
class UFinalBattleCharacterDetailPanel;
class UFinalBattleHandPanel;
class UFinalBattleCardZoneDetailPanel;
class UFinalBattleUltimatePanel;
class UFinalBattleRecentEventPanel;
class UFinalBattleActionPanel;

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleHUDScreen : public UFinalScreenBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	void InitializeScreen(UFinalBattleHUDViewModel* InViewModel, UFinalBattleWidgetController* InController);

private:
	void EnsureWidgetTree();
	void EnsurePanelsInBlueprintSlots();
	void InitializePanels();
	void AddPanelToSlot(UOverlay* Slot, UWidget* Panel) const;

	template <typename TPanel>
	TPanel* CreateConfiguredPanel(const TCHAR* WidgetName);

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleHUDViewModel> BattleViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleWidgetController> BattleController;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UFinalBattleTopBarPanel> TopBarPanel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UFinalBattleResourcePanel> ResourcePanel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UFinalRunFlowPromptPanel> RunFlowPromptPanel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UFinalBattleFeedbackPanel> FeedbackPanel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UFinalBattleContextPanel> ContextPanel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UFinalBattleCharacterPanel> CharacterPanel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UFinalBattleEnemyPanel> EnemyPanel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UFinalBattleEnemyDetailPanel> EnemyDetailPanel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UFinalBattleCharacterDetailPanel> CharacterDetailPanel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UFinalBattleCardZoneDetailPanel> CardZoneDetailPanel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UFinalBattleHandPanel> HandPanel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UFinalBattleUltimatePanel> UltimatePanel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UFinalBattleRecentEventPanel> RecentEventPanel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UFinalBattleActionPanel> ActionPanel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> TopBarSlot;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> ResourceSlot;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> RunFlowPromptSlot;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> FeedbackSlot;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> ContextSlot;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> CharacterPanelSlot;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> LegacyEnemyPanelSlot;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> EnemyDetailSlot;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> CharacterDetailSlot;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> CardZoneDetailSlot;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> UltimateSlot;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> HandSlot;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> RecentEventSlot;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> ActionSlot;
};
