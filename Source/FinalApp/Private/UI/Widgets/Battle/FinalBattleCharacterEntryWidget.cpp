#include "UI/Widgets/Battle/FinalBattleCharacterEntryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "UI/ViewModels/Battle/FinalBattleHUDTypes.h"

namespace
{
FText JoinCharacterStatusTextArray(const TArray<FText>& Texts, const FText& EmptyText)
{
	if (Texts.Num() == 0)
	{
		return EmptyText;
	}

	TArray<FString> Segments;
	for (const FText& Entry : Texts)
	{
		Segments.Add(Entry.ToString());
	}

	return FText::FromString(FString::Join(Segments, TEXT(" | ")));
}
}

void UFinalBattleCharacterEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("CharacterEntryBorder"));
		RootBorder->SetBrushColor(FLinearColor(0.12f, 0.16f, 0.24f, 0.95f));
		RootBorder->SetPadding(FMargin(8.0f));
		LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CharacterEntryLabel"));
		LabelText->SetAutoWrapText(true);
		RootBorder->SetContent(LabelText);
		WidgetTree->RootWidget = RootBorder;
	}
}

void UFinalBattleCharacterEntryWidget::Configure(const FFinalBattleHUDCharacterEntry& InEntry)
{
	CachedLabel = FText::Format(
		NSLOCTEXT("FinalBattleHUD", "CharacterEntryWidgetFormat", "{0}\nStress {1}/{2} | Vital {3}\nAwaken {4}/{5} | Collapse {6}\n{7}\nStatus: {8}"),
		InEntry.DisplayName,
		FText::AsNumber(InEntry.CurrentStress),
		FText::AsNumber(InEntry.StressCap),
		FText::AsNumber(InEntry.VitalShare),
		FText::AsNumber(InEntry.CurrentAwakenCount),
		FText::AsNumber(InEntry.CurrentAwakenThreshold),
		FText::AsNumber(InEntry.CollapseCount),
		InEntry.StateText,
		JoinCharacterStatusTextArray(InEntry.StatusTexts, NSLOCTEXT("FinalBattleHUD", "NoCharacterStatus", "无")));
	RebuildVisual();
}

void UFinalBattleCharacterEntryWidget::RebuildVisual()
{
	if (LabelText)
	{
		LabelText->SetText(CachedLabel);
	}
}
