#include "UI/Widgets/Battle/FinalBattleCharacterEntryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "UI/ViewModels/Battle/FinalBattleHUDTypes.h"

namespace
{
FText BuildCharacterStatusSummary(const TArray<FText>& Texts, const FText& EmptyText)
{
	if (Texts.Num() == 0)
	{
		return EmptyText;
	}

	TArray<FString> Segments;
	for (int32 Index = 0; Index < FMath::Min(Texts.Num(), 2); ++Index)
	{
		Segments.Add(Texts[Index].ToString());
	}

	if (Texts.Num() > 2)
	{
		Segments.Add(FString::Printf(TEXT("+%d"), Texts.Num() - 2));
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
		RootBorder->SetPadding(FMargin(6.0f));
		LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CharacterEntryLabel"));
		LabelText->SetAutoWrapText(true);
		RootBorder->SetContent(LabelText);
		WidgetTree->RootWidget = RootBorder;
	}
}

void UFinalBattleCharacterEntryWidget::Configure(const FFinalBattleHUDCharacterEntry& InEntry)
{
	CachedLabel = FText::Format(
		NSLOCTEXT("FinalBattleHUD", "CharacterEntryWidgetFormat", "{0}\nStress {1}/{2} | Vital {3}\nAwk {4}/{5} | Col {6}\n{7} | {8}"),
		InEntry.DisplayName,
		FText::AsNumber(InEntry.CurrentStress),
		FText::AsNumber(InEntry.StressCap),
		FText::AsNumber(InEntry.VitalShare),
		FText::AsNumber(InEntry.CurrentAwakenCount),
		FText::AsNumber(InEntry.CurrentAwakenThreshold),
		FText::AsNumber(InEntry.CollapseCount),
		InEntry.StateText,
		BuildCharacterStatusSummary(InEntry.StatusTexts, NSLOCTEXT("FinalBattleHUD", "NoCharacterStatus", "无状态")));
	RebuildVisual();
}

void UFinalBattleCharacterEntryWidget::RebuildVisual()
{
	if (LabelText)
	{
		LabelText->SetText(CachedLabel);
	}
}
