#include "UI/Settings/FinalUIWidgetClassSettings.h"

#include "UI/Panels/Battle/FinalBattleHUDPanels.h"
#include "UI/Root/FinalUIRootLayout.h"
#include "UI/Screens/Battle/FinalBattleHUDScreen.h"
#include "UI/Screens/Flow/FinalRunFlowOverlayScreen.h"
#include "UI/Screens/Flow/FinalRunGrowthChoiceOverlayScreen.h"
#include "UI/Screens/Flow/FinalRunEventNodeOverlayScreen.h"
#include "UI/Screens/Flow/FinalRunRewardOverlayScreen.h"
#include "UI/Screens/Flow/FinalRunShopNodeOverlayScreen.h"
#include "UI/Widgets/Battle/FinalBattleCardEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleCardZoneEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleCharacterDetailPassiveLineWidget.h"
#include "UI/Widgets/Battle/FinalBattleCharacterDetailStatusLineWidget.h"
#include "UI/Widgets/Battle/FinalBattleCharacterDetailWidget.h"
#include "UI/Widgets/Battle/FinalBattleCharacterEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleEnemyDetailStatusLineWidget.h"
#include "UI/Widgets/Battle/FinalBattleEnemyDetailWidget.h"
#include "UI/Widgets/Battle/FinalBattleEnemyEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleLogEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleTeamCharacterEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleTeamStatusDetailLineWidget.h"
#include "UI/Widgets/Battle/FinalBattleTeamStatusIconWidget.h"
#include "UI/Widgets/Battle/FinalBattleUltimateEntryWidget.h"

namespace
{
template <typename TWidget>
TSubclassOf<TWidget> ResolveConfiguredWidgetClass(const TSoftClassPtr<TWidget>& ConfiguredClass)
{
	if (!ConfiguredClass.IsNull())
	{
		if (UClass* LoadedClass = ConfiguredClass.LoadSynchronous())
		{
			if (LoadedClass->IsChildOf(TWidget::StaticClass()))
			{
				return LoadedClass;
			}
		}
	}

	return TWidget::StaticClass();
}
}

FName UFinalUIWidgetClassSettings::GetCategoryName() const
{
	return TEXT("Final");
}

FName UFinalUIWidgetClassSettings::GetSectionName() const
{
	return TEXT("UI");
}

TSubclassOf<UFinalBattleHUDScreen> UFinalUIWidgetClassSettings::GetBattleHUDScreenClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleHUDScreenClass);
}

TSubclassOf<UFinalUIRootLayout> UFinalUIWidgetClassSettings::GetRootLayoutClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->RootLayoutClass);
}

TSubclassOf<UFinalBattleTopBarPanel> UFinalUIWidgetClassSettings::GetBattleTopBarPanelClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleTopBarPanelClass);
}

TSubclassOf<UFinalBattleResourcePanel> UFinalUIWidgetClassSettings::GetBattleResourcePanelClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleResourcePanelClass);
}

TSubclassOf<UFinalRunFlowPromptPanel> UFinalUIWidgetClassSettings::GetBattleRunFlowPromptPanelClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleRunFlowPromptPanelClass);
}

TSubclassOf<UFinalBattleFeedbackPanel> UFinalUIWidgetClassSettings::GetBattleFeedbackPanelClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleFeedbackPanelClass);
}

TSubclassOf<UFinalBattleContextPanel> UFinalUIWidgetClassSettings::GetBattleContextPanelClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleContextPanelClass);
}

TSubclassOf<UFinalBattleTeamPanel> UFinalUIWidgetClassSettings::GetBattleTeamPanelClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleTeamPanelClass);
}

TSubclassOf<UFinalBattleTeamStatusDetailPanel> UFinalUIWidgetClassSettings::GetBattleTeamStatusDetailPanelClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleTeamStatusDetailPanelClass);
}

TSubclassOf<UFinalBattleCharacterPanel> UFinalUIWidgetClassSettings::GetBattleCharacterPanelClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleCharacterPanelClass);
}

TSubclassOf<UFinalBattleEnemyPanel> UFinalUIWidgetClassSettings::GetBattleEnemyPanelClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleEnemyPanelClass);
}

TSubclassOf<UFinalBattleEnemyDetailPanel> UFinalUIWidgetClassSettings::GetBattleEnemyDetailPanelClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleEnemyDetailPanelClass);
}

TSubclassOf<UFinalBattleCharacterDetailPanel> UFinalUIWidgetClassSettings::GetBattleCharacterDetailPanelClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleCharacterDetailPanelClass);
}

TSubclassOf<UFinalBattleHandPanel> UFinalUIWidgetClassSettings::GetBattleHandPanelClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleHandPanelClass);
}

TSubclassOf<UFinalBattleCardZoneDetailPanel> UFinalUIWidgetClassSettings::GetBattleCardZoneDetailPanelClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleCardZoneDetailPanelClass);
}

TSubclassOf<UFinalBattleUltimatePanel> UFinalUIWidgetClassSettings::GetBattleUltimatePanelClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleUltimatePanelClass);
}

TSubclassOf<UFinalBattleRecentEventPanel> UFinalUIWidgetClassSettings::GetBattleRecentEventPanelClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleRecentEventPanelClass);
}

TSubclassOf<UFinalBattleActionPanel> UFinalUIWidgetClassSettings::GetBattleActionPanelClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleActionPanelClass);
}

TSubclassOf<UFinalBattleCardEntryWidget> UFinalUIWidgetClassSettings::GetBattleCardEntryWidgetClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleCardEntryWidgetClass);
}

TSubclassOf<UFinalBattleCardZoneEntryWidget> UFinalUIWidgetClassSettings::GetBattleCardZoneEntryWidgetClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleCardZoneEntryWidgetClass);
}

TSubclassOf<UFinalBattleCharacterEntryWidget> UFinalUIWidgetClassSettings::GetBattleCharacterEntryWidgetClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleCharacterEntryWidgetClass);
}

TSubclassOf<UFinalBattleEnemyEntryWidget> UFinalUIWidgetClassSettings::GetBattleEnemyEntryWidgetClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleEnemyEntryWidgetClass);
}

TSubclassOf<UFinalBattleTeamCharacterEntryWidget> UFinalUIWidgetClassSettings::GetBattleTeamCharacterEntryWidgetClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleTeamCharacterEntryWidgetClass);
}

TSubclassOf<UFinalBattleTeamStatusIconWidget> UFinalUIWidgetClassSettings::GetBattleTeamStatusIconWidgetClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleTeamStatusIconWidgetClass);
}

TSubclassOf<UFinalBattleTeamStatusDetailLineWidget> UFinalUIWidgetClassSettings::GetBattleTeamStatusDetailLineWidgetClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleTeamStatusDetailLineWidgetClass);
}

TSubclassOf<UFinalBattleEnemyDetailWidget> UFinalUIWidgetClassSettings::GetBattleEnemyDetailWidgetClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleEnemyDetailWidgetClass);
}

TSubclassOf<UFinalBattleEnemyDetailStatusLineWidget> UFinalUIWidgetClassSettings::GetBattleEnemyDetailStatusLineWidgetClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleEnemyDetailStatusLineWidgetClass);
}

TSubclassOf<UFinalBattleCharacterDetailWidget> UFinalUIWidgetClassSettings::GetBattleCharacterDetailWidgetClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleCharacterDetailWidgetClass);
}

TSubclassOf<UFinalBattleCharacterDetailStatusLineWidget> UFinalUIWidgetClassSettings::GetBattleCharacterDetailStatusLineWidgetClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleCharacterDetailStatusLineWidgetClass);
}

TSubclassOf<UFinalBattleCharacterDetailPassiveLineWidget> UFinalUIWidgetClassSettings::GetBattleCharacterDetailPassiveLineWidgetClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleCharacterDetailPassiveLineWidgetClass);
}

TSubclassOf<UFinalBattleUltimateEntryWidget> UFinalUIWidgetClassSettings::GetBattleUltimateEntryWidgetClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleUltimateEntryWidgetClass);
}

TSubclassOf<UFinalBattleLogEntryWidget> UFinalUIWidgetClassSettings::GetBattleLogEntryWidgetClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleLogEntryWidgetClass);
}

TSubclassOf<UFinalRunFlowOverlayScreen> UFinalUIWidgetClassSettings::GetRunFlowOverlayScreenClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->RunFlowOverlayScreenClass);
}

TSubclassOf<UFinalRunGrowthChoiceOverlayScreen> UFinalUIWidgetClassSettings::GetRunGrowthChoiceOverlayScreenClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->RunGrowthChoiceOverlayScreenClass);
}

TSubclassOf<UFinalRunRewardOverlayScreen> UFinalUIWidgetClassSettings::GetRunRewardOverlayScreenClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->RunRewardOverlayScreenClass);
}

TSubclassOf<UFinalRunEventNodeOverlayScreen> UFinalUIWidgetClassSettings::GetRunEventNodeOverlayScreenClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->RunEventNodeOverlayScreenClass);
}

TSubclassOf<UFinalRunShopNodeOverlayScreen> UFinalUIWidgetClassSettings::GetRunShopNodeOverlayScreenClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->RunShopNodeOverlayScreenClass);
}

TSubclassOf<UFinalRunFlowOptionButton> UFinalUIWidgetClassSettings::GetRunFlowOptionButtonClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->RunFlowOptionButtonClass);
}

TSubclassOf<UFinalRunRouteNodeEntryWidget> UFinalUIWidgetClassSettings::GetRunRouteNodeEntryWidgetClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->RunRouteNodeEntryWidgetClass);
}

TSubclassOf<UFinalRunGrowthChoiceEntryWidget> UFinalUIWidgetClassSettings::GetRunGrowthChoiceEntryWidgetClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->RunGrowthChoiceEntryWidgetClass);
}

TSubclassOf<UFinalRunRewardCandidateEntryWidget> UFinalUIWidgetClassSettings::GetRunRewardCandidateEntryWidgetClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->RunRewardCandidateEntryWidgetClass);
}

TSubclassOf<UFinalRunEventOptionEntryWidget> UFinalUIWidgetClassSettings::GetRunEventOptionEntryWidgetClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->RunEventOptionEntryWidgetClass);
}

TSubclassOf<UFinalRunShopOfferEntryWidget> UFinalUIWidgetClassSettings::GetRunShopOfferEntryWidgetClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->RunShopOfferEntryWidgetClass);
}
