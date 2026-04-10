#include "UI/Widgets/Battle/FinalBattleEnemyEntryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UI/Screens/Battle/FinalBattleHUDScreen.h"
#include "ViewModels/FinalBattleHUDViewModel.h"

void UFinalBattleEnemyEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		SelectButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("EnemyButton"));
		LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("EnemyLabel"));
		LabelText->SetAutoWrapText(true);
		SelectButton->AddChild(LabelText);
		WidgetTree->RootWidget = SelectButton;
	}

	if (SelectButton)
	{
		SelectButton->OnClicked.AddDynamic(this, &UFinalBattleEnemyEntryWidget::HandleButtonClicked);
	}
}

void UFinalBattleEnemyEntryWidget::Configure(UFinalBattleHUDScreen* InOwningScreen, const FFinalBattleHUDEnemyEntry& InEntry)
{
	OwningBattleHUDScreen = InOwningScreen;
	RuntimeUnitId = InEntry.RuntimeUnitId;
	CachedLabel = FText::Format(
		NSLOCTEXT("FinalBattleHUD", "EnemyEntryFormat", "{0}\nHP {1}  Shield {2}  Break {3}  Init {4}\n{5}"),
		InEntry.DisplayName,
		FText::AsNumber(InEntry.CurrentHP),
		FText::AsNumber(InEntry.CurrentShield),
		FText::AsNumber(InEntry.CurrentBreakValue),
		FText::AsNumber(InEntry.CurrentInitiative),
		InEntry.IntentText);
	bSelected = InEntry.bSelected;
	RebuildVisual();
}

void UFinalBattleEnemyEntryWidget::HandleButtonClicked()
{
	if (OwningBattleHUDScreen.IsValid())
	{
		OwningBattleHUDScreen->HandleEnemySelected(RuntimeUnitId);
	}
}

void UFinalBattleEnemyEntryWidget::RebuildVisual()
{
	if (LabelText)
	{
		if (bSelected)
		{
			LabelText->SetText(FText::Format(NSLOCTEXT("FinalBattleHUD", "SelectedEnemyPrefix", "[目标]\n{0}"), CachedLabel));
		}
		else
		{
			LabelText->SetText(CachedLabel);
		}
	}

	if (SelectButton)
	{
		SelectButton->SetBackgroundColor(bSelected ? FLinearColor(0.82f, 0.24f, 0.24f, 1.0f) : FLinearColor(0.18f, 0.18f, 0.18f, 1.0f));
	}
}
