#include "UI/Widgets/Battle/FinalBattleCharacterEntryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
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
		ContentBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CharacterEntryContent"));
		LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CharacterEntryLabel"));
		LabelText->SetAutoWrapText(true);
		BreakthroughProgressBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("CharacterEntryBreakthroughBar"));
		BreakthroughProgressBar->SetPercent(0.0f);
		BreakthroughProgressBar->SetFillColorAndOpacity(FLinearColor(0.31f, 0.72f, 0.98f, 1.0f));
		ContentBox->AddChildToVerticalBox(LabelText);
		if (UVerticalBoxSlot* ProgressSlot = ContentBox->AddChildToVerticalBox(BreakthroughProgressBar))
		{
			ProgressSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 0.0f));
		}
		RootBorder->SetContent(ContentBox);
		WidgetTree->RootWidget = RootBorder;
	}
}

void UFinalBattleCharacterEntryWidget::Configure(const FFinalBattleHUDCharacterEntry& InEntry)
{
	CachedLabel = FText::Format(
		NSLOCTEXT("FinalBattleHUD", "CharacterEntryWidgetFormat", "{0}  Lv.{1}\nStress {2}/{3} | Vital {4}\nBreakthrough {5}/{6}\nAwk {7}/{8} | Col {9}\n{10} | {11}"),
		InEntry.DisplayName,
		FText::AsNumber(InEntry.Level),
		FText::AsNumber(InEntry.CurrentStress),
		FText::AsNumber(InEntry.StressCap),
		FText::AsNumber(InEntry.VitalShare),
		FText::AsNumber(InEntry.BreakthroughValue),
		FText::AsNumber(InEntry.BreakthroughRequiredValue),
		FText::AsNumber(InEntry.CurrentAwakenCount),
		FText::AsNumber(InEntry.CurrentAwakenThreshold),
		FText::AsNumber(InEntry.CollapseCount),
		InEntry.StateText,
		BuildCharacterStatusSummary(InEntry.StatusTexts, NSLOCTEXT("FinalBattleHUD", "NoCharacterStatus", "无状态")));
	CachedBreakthroughFill = InEntry.BreakthroughFillNormalized;
	bCachedBreakthroughReady = InEntry.bBreakthroughReady;
	RebuildVisual();
}

void UFinalBattleCharacterEntryWidget::RebuildVisual()
{
	if (LabelText)
	{
		LabelText->SetText(CachedLabel);
	}

	if (BreakthroughProgressBar)
	{
		BreakthroughProgressBar->SetPercent(CachedBreakthroughFill);
		BreakthroughProgressBar->SetFillColorAndOpacity(
			bCachedBreakthroughReady
				? FLinearColor(0.97f, 0.79f, 0.26f, 1.0f)
				: FLinearColor(0.31f, 0.72f, 0.98f, 1.0f));
	}

	if (RootBorder)
	{
		RootBorder->SetBrushColor(
			bCachedBreakthroughReady
				? FLinearColor(0.28f, 0.22f, 0.09f, 0.98f)
				: FLinearColor(0.12f, 0.16f, 0.24f, 0.95f));
	}
}
