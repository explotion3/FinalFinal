#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"
#include "FinalUIWidgetClassSettings.generated.h"

class UFinalBattleActionPanel;
class UFinalBattleCardEntryWidget;
class UFinalBattleCharacterEntryWidget;
class UFinalBattleCharacterPanel;
class UFinalBattleContextPanel;
class UFinalBattleEnemyEntryWidget;
class UFinalBattleEnemyPanel;
class UFinalBattleFeedbackPanel;
class UFinalBattleHUDScreen;
class UFinalBattleHandPanel;
class UFinalBattleLogEntryWidget;
class UFinalBattleRecentEventPanel;
class UFinalBattleResourcePanel;
class UFinalBattleTopBarPanel;
class UFinalBattleUltimateEntryWidget;
class UFinalBattleUltimatePanel;
class UFinalRunFlowOptionButton;
class UFinalRunFlowOverlayScreen;
class UFinalRunGrowthChoiceOverlayScreen;
class UFinalRunFlowPromptPanel;

UCLASS(Config=Game, DefaultConfig, DisplayName="Final UI Widget Classes")
class FINALAPP_API UFinalUIWidgetClassSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override;
	virtual FName GetSectionName() const override;

	static TSubclassOf<UFinalBattleHUDScreen> GetBattleHUDScreenClass();
	static TSubclassOf<UFinalBattleTopBarPanel> GetBattleTopBarPanelClass();
	static TSubclassOf<UFinalBattleResourcePanel> GetBattleResourcePanelClass();
	static TSubclassOf<UFinalRunFlowPromptPanel> GetBattleRunFlowPromptPanelClass();
	static TSubclassOf<UFinalBattleFeedbackPanel> GetBattleFeedbackPanelClass();
	static TSubclassOf<UFinalBattleContextPanel> GetBattleContextPanelClass();
	static TSubclassOf<UFinalBattleCharacterPanel> GetBattleCharacterPanelClass();
	static TSubclassOf<UFinalBattleEnemyPanel> GetBattleEnemyPanelClass();
	static TSubclassOf<UFinalBattleHandPanel> GetBattleHandPanelClass();
	static TSubclassOf<UFinalBattleUltimatePanel> GetBattleUltimatePanelClass();
	static TSubclassOf<UFinalBattleRecentEventPanel> GetBattleRecentEventPanelClass();
	static TSubclassOf<UFinalBattleActionPanel> GetBattleActionPanelClass();
	static TSubclassOf<UFinalBattleCardEntryWidget> GetBattleCardEntryWidgetClass();
	static TSubclassOf<UFinalBattleCharacterEntryWidget> GetBattleCharacterEntryWidgetClass();
	static TSubclassOf<UFinalBattleEnemyEntryWidget> GetBattleEnemyEntryWidgetClass();
	static TSubclassOf<UFinalBattleUltimateEntryWidget> GetBattleUltimateEntryWidgetClass();
	static TSubclassOf<UFinalBattleLogEntryWidget> GetBattleLogEntryWidgetClass();
	static TSubclassOf<UFinalRunFlowOverlayScreen> GetRunFlowOverlayScreenClass();
	static TSubclassOf<UFinalRunGrowthChoiceOverlayScreen> GetRunGrowthChoiceOverlayScreenClass();
	static TSubclassOf<UFinalRunFlowOptionButton> GetRunFlowOptionButtonClass();

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Screen")
	TSoftClassPtr<UFinalBattleHUDScreen> BattleHUDScreenClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Panels")
	TSoftClassPtr<UFinalBattleTopBarPanel> BattleTopBarPanelClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Panels")
	TSoftClassPtr<UFinalBattleResourcePanel> BattleResourcePanelClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Panels")
	TSoftClassPtr<UFinalRunFlowPromptPanel> BattleRunFlowPromptPanelClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Panels")
	TSoftClassPtr<UFinalBattleFeedbackPanel> BattleFeedbackPanelClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Panels")
	TSoftClassPtr<UFinalBattleContextPanel> BattleContextPanelClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Panels")
	TSoftClassPtr<UFinalBattleCharacterPanel> BattleCharacterPanelClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Panels")
	TSoftClassPtr<UFinalBattleEnemyPanel> BattleEnemyPanelClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Panels")
	TSoftClassPtr<UFinalBattleHandPanel> BattleHandPanelClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Panels")
	TSoftClassPtr<UFinalBattleUltimatePanel> BattleUltimatePanelClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Panels")
	TSoftClassPtr<UFinalBattleRecentEventPanel> BattleRecentEventPanelClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Panels")
	TSoftClassPtr<UFinalBattleActionPanel> BattleActionPanelClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Entries")
	TSoftClassPtr<UFinalBattleCardEntryWidget> BattleCardEntryWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Entries")
	TSoftClassPtr<UFinalBattleCharacterEntryWidget> BattleCharacterEntryWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Entries")
	TSoftClassPtr<UFinalBattleEnemyEntryWidget> BattleEnemyEntryWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Entries")
	TSoftClassPtr<UFinalBattleUltimateEntryWidget> BattleUltimateEntryWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Entries")
	TSoftClassPtr<UFinalBattleLogEntryWidget> BattleLogEntryWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Run Flow")
	TSoftClassPtr<UFinalRunFlowOverlayScreen> RunFlowOverlayScreenClass;

	UPROPERTY(Config, EditAnywhere, Category="Run Flow")
	TSoftClassPtr<UFinalRunGrowthChoiceOverlayScreen> RunGrowthChoiceOverlayScreenClass;

	UPROPERTY(Config, EditAnywhere, Category="Run Flow")
	TSoftClassPtr<UFinalRunFlowOptionButton> RunFlowOptionButtonClass;
};
