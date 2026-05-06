#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UObject/SoftObjectPtr.h"
#include "FinalUIWidgetClassSettings.generated.h"

class UFinalBattleActionPanel;
class UFinalBattleCardEntryWidget;
class UFinalBattleCardZoneEntryWidget;
class UFinalBattleCardZoneDetailPanel;
class UFinalBattleCharacterEntryWidget;
class UFinalBattleCharacterDetailPanel;
class UFinalBattleCharacterDetailPassiveLineWidget;
class UFinalBattleCharacterDetailStatusLineWidget;
class UFinalBattleCharacterDetailWidget;
class UFinalBattleCharacterPanel;
class UFinalBattleContextPanel;
class UFinalBattleEnemyEntryWidget;
class UFinalBattleEnemyDetailPanel;
class UFinalBattleEnemyDetailStatusLineWidget;
class UFinalBattleEnemyDetailWidget;
class UFinalBattleEnemyPanel;
class UFinalBattleFeedbackPanel;
class UFinalBattleHUDScreen;
class UFinalBattleHandPanel;
class UFinalBattleLogEntryWidget;
class UFinalBattleRecentEventPanel;
class UFinalBattleResourcePanel;
class UFinalBattleTeamCharacterEntryWidget;
class UFinalBattleTeamPanel;
class UFinalBattleTeamStatusDetailLineWidget;
class UFinalBattleTeamStatusDetailPanel;
class UFinalBattleTeamStatusIconWidget;
class UFinalBattleTopBarPanel;
class UFinalBattleUltimateEntryWidget;
class UFinalBattleUltimatePanel;
class UFinalRunFlowOptionButton;
class UFinalRunFlowOverlayScreen;
class UFinalRunGrowthChoiceEntryWidget;
class UFinalRunGrowthChoiceOverlayScreen;
class UFinalRunFlowPromptPanel;
class UFinalRunEventNodeOverlayScreen;
class UFinalRunEventOptionEntryWidget;
class UFinalRunRewardCandidateEntryWidget;
class UFinalRunRewardOverlayScreen;
class UFinalRunRouteNodeEntryWidget;
class UFinalRunShopNodeOverlayScreen;
class UFinalRunShopOfferEntryWidget;
class UFinalUIRootLayout;

UCLASS(Config=Game, DefaultConfig, DisplayName="Final UI Widget Classes")
class FINALAPP_API UFinalUIWidgetClassSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override;
	virtual FName GetSectionName() const override;

	static TSubclassOf<UFinalBattleHUDScreen> GetBattleHUDScreenClass();
	static TSubclassOf<UFinalUIRootLayout> GetRootLayoutClass();
	static TSubclassOf<UFinalBattleTopBarPanel> GetBattleTopBarPanelClass();
	static TSubclassOf<UFinalBattleResourcePanel> GetBattleResourcePanelClass();
	static TSubclassOf<UFinalRunFlowPromptPanel> GetBattleRunFlowPromptPanelClass();
	static TSubclassOf<UFinalBattleFeedbackPanel> GetBattleFeedbackPanelClass();
	static TSubclassOf<UFinalBattleContextPanel> GetBattleContextPanelClass();
	static TSubclassOf<UFinalBattleTeamPanel> GetBattleTeamPanelClass();
	static TSubclassOf<UFinalBattleTeamStatusDetailPanel> GetBattleTeamStatusDetailPanelClass();
	static TSubclassOf<UFinalBattleCharacterPanel> GetBattleCharacterPanelClass();
	static TSubclassOf<UFinalBattleEnemyPanel> GetBattleEnemyPanelClass();
	static TSubclassOf<UFinalBattleEnemyDetailPanel> GetBattleEnemyDetailPanelClass();
	static TSubclassOf<UFinalBattleCharacterDetailPanel> GetBattleCharacterDetailPanelClass();
	static TSubclassOf<UFinalBattleHandPanel> GetBattleHandPanelClass();
	static TSubclassOf<UFinalBattleCardZoneDetailPanel> GetBattleCardZoneDetailPanelClass();
	static TSubclassOf<UFinalBattleUltimatePanel> GetBattleUltimatePanelClass();
	static TSubclassOf<UFinalBattleRecentEventPanel> GetBattleRecentEventPanelClass();
	static TSubclassOf<UFinalBattleActionPanel> GetBattleActionPanelClass();
	static TSubclassOf<UFinalBattleCardEntryWidget> GetBattleCardEntryWidgetClass();
	static TSubclassOf<UFinalBattleCardZoneEntryWidget> GetBattleCardZoneEntryWidgetClass();
	static TSubclassOf<UFinalBattleCharacterEntryWidget> GetBattleCharacterEntryWidgetClass();
	static TSubclassOf<UFinalBattleEnemyEntryWidget> GetBattleEnemyEntryWidgetClass();
	static TSubclassOf<UFinalBattleTeamCharacterEntryWidget> GetBattleTeamCharacterEntryWidgetClass();
	static TSubclassOf<UFinalBattleTeamStatusIconWidget> GetBattleTeamStatusIconWidgetClass();
	static TSubclassOf<UFinalBattleTeamStatusDetailLineWidget> GetBattleTeamStatusDetailLineWidgetClass();
	static TSubclassOf<UFinalBattleEnemyDetailWidget> GetBattleEnemyDetailWidgetClass();
	static TSubclassOf<UFinalBattleEnemyDetailStatusLineWidget> GetBattleEnemyDetailStatusLineWidgetClass();
	static TSubclassOf<UFinalBattleCharacterDetailWidget> GetBattleCharacterDetailWidgetClass();
	static TSubclassOf<UFinalBattleCharacterDetailStatusLineWidget> GetBattleCharacterDetailStatusLineWidgetClass();
	static TSubclassOf<UFinalBattleCharacterDetailPassiveLineWidget> GetBattleCharacterDetailPassiveLineWidgetClass();
	static TSubclassOf<UFinalBattleUltimateEntryWidget> GetBattleUltimateEntryWidgetClass();
	static TSubclassOf<UFinalBattleLogEntryWidget> GetBattleLogEntryWidgetClass();
	static TSubclassOf<UFinalRunFlowOverlayScreen> GetRunFlowOverlayScreenClass();
	static TSubclassOf<UFinalRunGrowthChoiceOverlayScreen> GetRunGrowthChoiceOverlayScreenClass();
	static TSubclassOf<UFinalRunRewardOverlayScreen> GetRunRewardOverlayScreenClass();
	static TSubclassOf<UFinalRunEventNodeOverlayScreen> GetRunEventNodeOverlayScreenClass();
	static TSubclassOf<UFinalRunShopNodeOverlayScreen> GetRunShopNodeOverlayScreenClass();
	static TSubclassOf<UFinalRunFlowOptionButton> GetRunFlowOptionButtonClass();
	static TSubclassOf<UFinalRunRouteNodeEntryWidget> GetRunRouteNodeEntryWidgetClass();
	static TSubclassOf<UFinalRunGrowthChoiceEntryWidget> GetRunGrowthChoiceEntryWidgetClass();
	static TSubclassOf<UFinalRunRewardCandidateEntryWidget> GetRunRewardCandidateEntryWidgetClass();
	static TSubclassOf<UFinalRunEventOptionEntryWidget> GetRunEventOptionEntryWidgetClass();
	static TSubclassOf<UFinalRunShopOfferEntryWidget> GetRunShopOfferEntryWidgetClass();

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Screen")
	TSoftClassPtr<UFinalBattleHUDScreen> BattleHUDScreenClass;

	UPROPERTY(Config, EditAnywhere, Category="Root Layout")
	TSoftClassPtr<UFinalUIRootLayout> RootLayoutClass;

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
	TSoftClassPtr<UFinalBattleTeamPanel> BattleTeamPanelClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Panels")
	TSoftClassPtr<UFinalBattleTeamStatusDetailPanel> BattleTeamStatusDetailPanelClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Panels")
	TSoftClassPtr<UFinalBattleCharacterPanel> BattleCharacterPanelClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Panels")
	TSoftClassPtr<UFinalBattleEnemyPanel> BattleEnemyPanelClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Panels")
	TSoftClassPtr<UFinalBattleEnemyDetailPanel> BattleEnemyDetailPanelClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Panels")
	TSoftClassPtr<UFinalBattleCharacterDetailPanel> BattleCharacterDetailPanelClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Panels")
	TSoftClassPtr<UFinalBattleHandPanel> BattleHandPanelClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Panels")
	TSoftClassPtr<UFinalBattleCardZoneDetailPanel> BattleCardZoneDetailPanelClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Panels")
	TSoftClassPtr<UFinalBattleUltimatePanel> BattleUltimatePanelClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Panels")
	TSoftClassPtr<UFinalBattleRecentEventPanel> BattleRecentEventPanelClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Panels")
	TSoftClassPtr<UFinalBattleActionPanel> BattleActionPanelClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Entries")
	TSoftClassPtr<UFinalBattleCardEntryWidget> BattleCardEntryWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Entries")
	TSoftClassPtr<UFinalBattleCardZoneEntryWidget> BattleCardZoneEntryWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Entries")
	TSoftClassPtr<UFinalBattleCharacterEntryWidget> BattleCharacterEntryWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Entries")
	TSoftClassPtr<UFinalBattleEnemyEntryWidget> BattleEnemyEntryWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Entries")
	TSoftClassPtr<UFinalBattleTeamCharacterEntryWidget> BattleTeamCharacterEntryWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Entries")
	TSoftClassPtr<UFinalBattleTeamStatusIconWidget> BattleTeamStatusIconWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Entries")
	TSoftClassPtr<UFinalBattleTeamStatusDetailLineWidget> BattleTeamStatusDetailLineWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Entries")
	TSoftClassPtr<UFinalBattleEnemyDetailWidget> BattleEnemyDetailWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Entries")
	TSoftClassPtr<UFinalBattleEnemyDetailStatusLineWidget> BattleEnemyDetailStatusLineWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Entries")
	TSoftClassPtr<UFinalBattleCharacterDetailWidget> BattleCharacterDetailWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Entries")
	TSoftClassPtr<UFinalBattleCharacterDetailStatusLineWidget> BattleCharacterDetailStatusLineWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Entries")
	TSoftClassPtr<UFinalBattleCharacterDetailPassiveLineWidget> BattleCharacterDetailPassiveLineWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Entries")
	TSoftClassPtr<UFinalBattleUltimateEntryWidget> BattleUltimateEntryWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Battle HUD|Entries")
	TSoftClassPtr<UFinalBattleLogEntryWidget> BattleLogEntryWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Run Flow")
	TSoftClassPtr<UFinalRunFlowOverlayScreen> RunFlowOverlayScreenClass;

	UPROPERTY(Config, EditAnywhere, Category="Run Flow")
	TSoftClassPtr<UFinalRunGrowthChoiceOverlayScreen> RunGrowthChoiceOverlayScreenClass;

	UPROPERTY(Config, EditAnywhere, Category="Run Flow")
	TSoftClassPtr<UFinalRunRewardOverlayScreen> RunRewardOverlayScreenClass;

	UPROPERTY(Config, EditAnywhere, Category="Run Flow")
	TSoftClassPtr<UFinalRunEventNodeOverlayScreen> RunEventNodeOverlayScreenClass;

	UPROPERTY(Config, EditAnywhere, Category="Run Flow")
	TSoftClassPtr<UFinalRunShopNodeOverlayScreen> RunShopNodeOverlayScreenClass;

	UPROPERTY(Config, EditAnywhere, Category="Run Flow")
	TSoftClassPtr<UFinalRunFlowOptionButton> RunFlowOptionButtonClass;

	UPROPERTY(Config, EditAnywhere, Category="Run Flow")
	TSoftClassPtr<UFinalRunRouteNodeEntryWidget> RunRouteNodeEntryWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Run Flow")
	TSoftClassPtr<UFinalRunGrowthChoiceEntryWidget> RunGrowthChoiceEntryWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Run Flow")
	TSoftClassPtr<UFinalRunRewardCandidateEntryWidget> RunRewardCandidateEntryWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Run Flow")
	TSoftClassPtr<UFinalRunEventOptionEntryWidget> RunEventOptionEntryWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Run Flow")
	TSoftClassPtr<UFinalRunShopOfferEntryWidget> RunShopOfferEntryWidgetClass;
};
