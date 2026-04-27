#pragma once

#include "CoreMinimal.h"
#include "UI/Screens/FinalScreenBase.h"
#include "FinalBattleHUDScreen.generated.h"

class UVerticalBox;
class UFinalBattleHUDViewModel;
class UFinalBattleWidgetController;
class UFinalBattleTopBarPanel;
class UFinalBattleResourcePanel;
class UFinalRunFlowPromptPanel;
class UFinalBattleFeedbackPanel;
class UFinalBattleContextPanel;
class UFinalBattleCharacterPanel;
class UFinalBattleEnemyPanel;
class UFinalBattleHandPanel;
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
	void InitializePanels();

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
	TObjectPtr<UFinalBattleHandPanel> HandPanel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UFinalBattleUltimatePanel> UltimatePanel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UFinalBattleRecentEventPanel> RecentEventPanel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UFinalBattleActionPanel> ActionPanel;
};
