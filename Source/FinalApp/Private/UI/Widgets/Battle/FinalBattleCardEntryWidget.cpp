#include "UI/Widgets/Battle/FinalBattleCardEntryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UI/Screens/Battle/FinalBattleHUDScreen.h"
#include "ViewModels/FinalBattleHUDViewModel.h"

void UFinalBattleCardEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		CardButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("CardButton"));
		LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CardLabel"));
		LabelText->SetAutoWrapText(true);
		CardButton->AddChild(LabelText);
		WidgetTree->RootWidget = CardButton;
	}

	if (CardButton)
	{
		CardButton->OnClicked.AddDynamic(this, &UFinalBattleCardEntryWidget::HandleButtonClicked);
	}
}

void UFinalBattleCardEntryWidget::Configure(UFinalBattleHUDScreen* InOwningScreen, int32 InHandIndex, const FFinalBattleHUDCardEntry& InEntry)
{
	OwningBattleHUDScreen = InOwningScreen;
	HandIndex = InHandIndex;
	CachedLabel = FText::Format(
		NSLOCTEXT("FinalBattleHUD", "CardEntryFormat", "{0}\nAP {1}\n{2}"),
		InEntry.DisplayName,
		FText::AsNumber(InEntry.RuntimeCostAP),
		InEntry.RulesText);
	RebuildVisual();
}

void UFinalBattleCardEntryWidget::HandleButtonClicked()
{
	if (OwningBattleHUDScreen.IsValid())
	{
		OwningBattleHUDScreen->HandlePlayCard(HandIndex);
	}
}

void UFinalBattleCardEntryWidget::RebuildVisual()
{
	if (LabelText)
	{
		LabelText->SetText(CachedLabel);
	}

	if (CardButton)
	{
		CardButton->SetBackgroundColor(FLinearColor(0.17f, 0.23f, 0.34f, 1.0f));
	}
}
