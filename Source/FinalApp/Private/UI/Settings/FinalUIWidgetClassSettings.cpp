#include "UI/Settings/FinalUIWidgetClassSettings.h"

#include "UI/Panels/Battle/FinalBattleHUDPanels.h"
#include "UI/Screens/Battle/FinalBattleHUDScreen.h"
#include "UI/Widgets/Battle/FinalBattleCardEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleCharacterEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleEnemyEntryWidget.h"
#include "UI/Widgets/Battle/FinalBattleLogEntryWidget.h"
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

TSubclassOf<UFinalBattleTopBarPanel> UFinalUIWidgetClassSettings::GetBattleTopBarPanelClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleTopBarPanelClass);
}

TSubclassOf<UFinalBattleFeedbackPanel> UFinalUIWidgetClassSettings::GetBattleFeedbackPanelClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleFeedbackPanelClass);
}

TSubclassOf<UFinalBattleContextPanel> UFinalUIWidgetClassSettings::GetBattleContextPanelClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleContextPanelClass);
}

TSubclassOf<UFinalBattleCharacterPanel> UFinalUIWidgetClassSettings::GetBattleCharacterPanelClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleCharacterPanelClass);
}

TSubclassOf<UFinalBattleEnemyPanel> UFinalUIWidgetClassSettings::GetBattleEnemyPanelClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleEnemyPanelClass);
}

TSubclassOf<UFinalBattleHandPanel> UFinalUIWidgetClassSettings::GetBattleHandPanelClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleHandPanelClass);
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

TSubclassOf<UFinalBattleCharacterEntryWidget> UFinalUIWidgetClassSettings::GetBattleCharacterEntryWidgetClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleCharacterEntryWidgetClass);
}

TSubclassOf<UFinalBattleEnemyEntryWidget> UFinalUIWidgetClassSettings::GetBattleEnemyEntryWidgetClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleEnemyEntryWidgetClass);
}

TSubclassOf<UFinalBattleUltimateEntryWidget> UFinalUIWidgetClassSettings::GetBattleUltimateEntryWidgetClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleUltimateEntryWidgetClass);
}

TSubclassOf<UFinalBattleLogEntryWidget> UFinalUIWidgetClassSettings::GetBattleLogEntryWidgetClass()
{
	return ResolveConfiguredWidgetClass(GetDefault<UFinalUIWidgetClassSettings>()->BattleLogEntryWidgetClass);
}
