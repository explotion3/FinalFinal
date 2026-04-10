#include "UI/Widgets/Battle/FinalBattleUltimateEntryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UI/Screens/Battle/FinalBattleHUDScreen.h"
#include "ViewModels/FinalBattleHUDViewModel.h"

void UFinalBattleUltimateEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		UltimateButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("UltimateButton"));
		LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("UltimateLabel"));
		LabelText->SetAutoWrapText(true);
		UltimateButton->AddChild(LabelText);
		WidgetTree->RootWidget = UltimateButton;
	}

	if (UltimateButton)
	{
		UltimateButton->OnClicked.AddDynamic(this, &UFinalBattleUltimateEntryWidget::HandleButtonClicked);
	}
}

void UFinalBattleUltimateEntryWidget::Configure(UFinalBattleHUDScreen* InOwningScreen, int32 InCharacterIndex, const FFinalBattleHUDUltimateEntry& InEntry)
{
	OwningBattleHUDScreen = InOwningScreen;
	CharacterIndex = InCharacterIndex;
	CachedLabel = FText::Format(
		NSLOCTEXT("FinalBattleHUD", "UltimateEntryFormat", "{0}\n{1}"),
		InEntry.DisplayName,
		InEntry.StatusText);
	bEnabled = InEntry.bEnabled;
	RebuildVisual();
}

void UFinalBattleUltimateEntryWidget::HandleButtonClicked()
{
	if (bEnabled && OwningBattleHUDScreen.IsValid())
	{
		OwningBattleHUDScreen->HandlePlayUltimate(CharacterIndex);
	}
}

void UFinalBattleUltimateEntryWidget::RebuildVisual()
{
	if (LabelText)
	{
		LabelText->SetText(CachedLabel);
	}

	if (UltimateButton)
	{
		UltimateButton->SetIsEnabled(bEnabled);
		UltimateButton->SetBackgroundColor(bEnabled ? FLinearColor(0.28f, 0.45f, 0.23f, 1.0f) : FLinearColor(0.16f, 0.16f, 0.16f, 1.0f));
	}
}
