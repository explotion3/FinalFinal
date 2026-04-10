#include "UI/Widgets/Battle/FinalBattleUltimateEntryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UI/Screens/Battle/FinalBattleHUDScreen.h"
#include "ViewModels/FinalBattleHUDViewModel.h"

namespace
{
FText JoinTextArray(const TArray<FText>& Texts)
{
	TArray<FString> Segments;
	Segments.Reserve(Texts.Num());
	for (const FText& Entry : Texts)
	{
		if (!Entry.IsEmpty())
		{
			Segments.Add(Entry.ToString());
		}
	}

	return Segments.Num() > 0
		? FText::FromString(FString::Join(Segments, TEXT(" | ")))
		: FText::GetEmpty();
}
}

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
	bEnabled = InEntry.bEnabled;
	bBlockedByCollapse = InEntry.bBlockedByCollapse;
	bDefinitionReady = InEntry.bDefinitionReady;

	TArray<FText> DetailTexts;
	DetailTexts.Reserve(3);
	DetailTexts.Add(FText::Format(
		NSLOCTEXT("FinalBattleHUD", "UltimateCostText", "EP 消耗 {0}"),
		FText::AsNumber(InEntry.CostEP)));
	DetailTexts.Add(InEntry.StatusText);
	if (InEntry.bBlockedByCollapse)
	{
		DetailTexts.Add(NSLOCTEXT("FinalBattleHUD", "UltimateBlockedByCollapse", "角色崩溃中"));
	}
	else if (!InEntry.bDefinitionReady)
	{
		DetailTexts.Add(NSLOCTEXT("FinalBattleHUD", "UltimateDefinitionMissing", "定义未就绪"));
	}

	CachedLabel = FText::Format(
		NSLOCTEXT("FinalBattleHUD", "UltimateEntryFormat", "{0}\n{1}"),
		InEntry.DisplayName,
		JoinTextArray(DetailTexts));
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
		if (bEnabled)
		{
			UltimateButton->SetBackgroundColor(FLinearColor(0.28f, 0.45f, 0.23f, 1.0f));
		}
		else if (bBlockedByCollapse)
		{
			UltimateButton->SetBackgroundColor(FLinearColor(0.34f, 0.18f, 0.18f, 1.0f));
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
