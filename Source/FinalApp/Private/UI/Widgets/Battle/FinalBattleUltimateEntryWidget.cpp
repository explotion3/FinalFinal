#include "UI/Widgets/Battle/FinalBattleUltimateEntryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Controllers/Battle/FinalBattleHUDPanelControllers.h"
#include "UI/ViewModels/Battle/FinalBattleHUDTypes.h"

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

void UFinalBattleUltimateEntryWidget::Configure(UFinalBattleUltimatePanelController* InController, int32 InCharacterIndex, const FFinalBattleHUDUltimateEntry& InEntry)
{
	PanelController = InController;
	CharacterIndex = InCharacterIndex;
	bEnabled = InEntry.bEnabled;
	bBlockedByCollapse = InEntry.bBlockedByCollapse;
	bDefinitionReady = InEntry.bDefinitionReady;
	bUsedThisBattle = InEntry.bUsedThisBattle;

	CachedLabel = FText::Format(
		NSLOCTEXT("FinalBattleHUD", "UltimateEntryFormat", "{0}\nEP {1} | {2}"),
		InEntry.DisplayName,
		FText::AsNumber(InEntry.CostEP),
		InEntry.StatusText);
	RebuildVisual();
}

void UFinalBattleUltimateEntryWidget::HandleButtonClicked()
{
	if (bEnabled && PanelController.IsValid())
	{
		PanelController->PlayUltimateByCharacterIndex(CharacterIndex);
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
		if (bEnabled)
		{
			UltimateButton->SetBackgroundColor(FLinearColor(0.28f, 0.45f, 0.23f, 1.0f));
		}
		else if (bBlockedByCollapse)
		{
			UltimateButton->SetBackgroundColor(FLinearColor(0.34f, 0.18f, 0.18f, 1.0f));
		}
		else if (bUsedThisBattle)
		{
			UltimateButton->SetBackgroundColor(FLinearColor(0.18f, 0.24f, 0.34f, 1.0f));
		}
		else if (!bDefinitionReady)
		{
			UltimateButton->SetBackgroundColor(FLinearColor(0.34f, 0.26f, 0.14f, 1.0f));
		}
		else
		{
			UltimateButton->SetBackgroundColor(FLinearColor(0.16f, 0.16f, 0.16f, 1.0f));
		}
	}
}
