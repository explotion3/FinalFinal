#pragma once

#include "CoreMinimal.h"
#include "UI/Screens/FinalScreenBase.h"
#include "FinalBattleHUDScreen.generated.h"

class UHorizontalBox;
class UVerticalBox;
class UFinalBattleHUDViewModel;
class UFinalBattleWidgetController;
class UFinalBattleTopBarPanel;
class UFinalBattleFeedbackPanel;
class UFinalBattleContextPanel;
class UFinalBattleCharacterPanel;
class UFinalBattleEnemyPanel;
class UFinalBattleHandPanel;
class UFinalBattleUltimatePanel;
class UFinalBattleRecentEventPanel;
class UFinalBattleActionPanel;

UCLASS()
class FINALAPP_API UFinalBattleHUDScreen : public UFinalScreenBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	void InitializeScreen(UFinalBattleHUDViewModel* InViewModel, UFinalBattleWidgetController* InController);

private:
	void EnsureWidgetTree();
	void InitializePanels();

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleHUDViewModel> BattleViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleWidgetController> BattleController;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleTopBarPanel> TopBarPanel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleFeedbackPanel> FeedbackPanel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleContextPanel> ContextPanel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleCharacterPanel> CharacterPanel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleEnemyPanel> EnemyPanel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleHandPanel> HandPanel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleUltimatePanel> UltimatePanel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleRecentEventPanel> RecentEventPanel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleActionPanel> ActionPanel;
};
