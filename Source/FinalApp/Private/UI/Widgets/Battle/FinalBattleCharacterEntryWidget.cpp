#include "UI/Widgets/Battle/FinalBattleCharacterEntryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
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
	EnsureWidgetTree();
}

void UFinalBattleCharacterEntryWidget::Configure(const FFinalBattleHUDCharacterEntry& InEntry)
{
	ApplyCharacterEntryView(InEntry);
}

void UFinalBattleCharacterEntryWidget::ApplyCharacterEntryView(const FFinalBattleHUDCharacterEntry& ViewData)
{
	CharacterEntryViewData = ViewData;
	EnsureWidgetTree();
	RefreshBoundWidgets();
	OnCharacterEntryViewApplied(CharacterEntryViewData);
}

void UFinalBattleCharacterEntryWidget::EnsureWidgetTree()
{
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
		StressBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("CharacterEntryStressBar"));
		StressBar->SetPercent(0.0f);
		StressBar->SetFillColorAndOpacity(FLinearColor(0.85f, 0.32f, 0.28f, 1.0f));
		if (UVerticalBoxSlot* StressSlot = ContentBox->AddChildToVerticalBox(StressBar))
		{
			StressSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 0.0f));
		}
		if (UVerticalBoxSlot* ProgressSlot = ContentBox->AddChildToVerticalBox(BreakthroughProgressBar))
		{
			ProgressSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 0.0f));
		}
		RootBorder->SetContent(ContentBox);
		WidgetTree->RootWidget = RootBorder;
	}
}

void UFinalBattleCharacterEntryWidget::RefreshBoundWidgets()
{
	EnsureWidgetTree();

	if (ContentRoot)
	{
		ContentRoot->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (LabelText)
	{
		LabelText->SetText(BuildFallbackLabel());
	}

	if (NameText)
	{
		NameText->SetText(CharacterEntryViewData.DisplayName);
	}

	if (LevelText)
	{
		LevelText->SetText(BuildLevelText());
	}

	if (StressText)
	{
		StressText->SetText(BuildStressText());
	}

	if (StressBar)
	{
		StressBar->SetPercent(CharacterEntryViewData.StressPercent);
	}

	if (BreakthroughText)
	{
		BreakthroughText->SetText(BuildBreakthroughText());
	}

	if (BreakthroughBar)
	{
		BreakthroughBar->SetPercent(CharacterEntryViewData.BreakthroughFillNormalized);
		BreakthroughBar->SetFillColorAndOpacity(
			CharacterEntryViewData.bBreakthroughReady
				? FLinearColor(0.97f, 0.79f, 0.26f, 1.0f)
				: FLinearColor(0.31f, 0.72f, 0.98f, 1.0f));
	}

	if (BreakthroughProgressBar)
	{
		BreakthroughProgressBar->SetPercent(CharacterEntryViewData.BreakthroughFillNormalized);
		BreakthroughProgressBar->SetFillColorAndOpacity(
			CharacterEntryViewData.bBreakthroughReady
				? FLinearColor(0.97f, 0.79f, 0.26f, 1.0f)
				: FLinearColor(0.31f, 0.72f, 0.98f, 1.0f));
	}

	if (StateText)
	{
		StateText->SetText(CharacterEntryViewData.StateText);
	}

	if (StatusText)
	{
		StatusText->SetText(BuildStatusText());
	}

	RefreshStatusBox();

	if (CollapsedVisual)
	{
		CollapsedVisual->SetVisibility(CharacterEntryViewData.bCollapsed ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (BreakthroughReadyVisual)
	{
		BreakthroughReadyVisual->SetVisibility(CharacterEntryViewData.bBreakthroughReady ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (RootBorder)
	{
		RootBorder->SetBrushColor(
			CharacterEntryViewData.bCollapsed
				? FLinearColor(0.20f, 0.08f, 0.08f, 0.98f)
				: (CharacterEntryViewData.bBreakthroughReady
				? FLinearColor(0.28f, 0.22f, 0.09f, 0.98f)
				: FLinearColor(0.12f, 0.16f, 0.24f, 0.95f)));
	}
}

FText UFinalBattleCharacterEntryWidget::BuildFallbackLabel() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleHUD", "CharacterEntryWidgetLightFormat", "{0}  {1}\n{2}\n{3}\n{4} | {5}"),
		CharacterEntryViewData.DisplayName,
		BuildLevelText(),
		BuildStressText(),
		BuildBreakthroughText(),
		CharacterEntryViewData.StateText,
		BuildStatusText());
}

FText UFinalBattleCharacterEntryWidget::BuildLevelText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleHUD", "CharacterEntryLevelFormat", "Lv.{0}"),
		FText::AsNumber(CharacterEntryViewData.Level));
}

FText UFinalBattleCharacterEntryWidget::BuildStressText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleHUD", "CharacterEntryStressFormat", "压力 {0}/{1}"),
		FText::AsNumber(CharacterEntryViewData.CurrentStress),
		FText::AsNumber(CharacterEntryViewData.StressCap));
}

FText UFinalBattleCharacterEntryWidget::BuildBreakthroughText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleHUD", "CharacterEntryBreakthroughFormat", "突破 {0}/{1}"),
		FText::AsNumber(CharacterEntryViewData.BreakthroughValue),
		FText::AsNumber(CharacterEntryViewData.BreakthroughRequiredValue));
}

FText UFinalBattleCharacterEntryWidget::BuildStatusText() const
{
	return BuildCharacterStatusSummary(CharacterEntryViewData.StatusTexts, NSLOCTEXT("FinalBattleHUD", "NoCharacterStatus", "无状态"));
}

void UFinalBattleCharacterEntryWidget::RefreshStatusBox()
{
	if (StatusBox == nullptr)
	{
		return;
	}

	StatusBox->ClearChildren();

	const int32 VisibleCount = FMath::Min(static_cast<int32>(CharacterEntryViewData.StatusTexts.Num()), 2);
	for (int32 StatusIndex = 0; StatusIndex < VisibleCount; ++StatusIndex)
	{
		UTextBlock* StatusLabel = WidgetTree
			? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass())
			: NewObject<UTextBlock>(this);
		if (StatusLabel == nullptr)
		{
			continue;
		}

		StatusLabel->SetText(CharacterEntryViewData.StatusTexts[StatusIndex]);
		StatusBox->AddChild(StatusLabel);
	}

	if (CharacterEntryViewData.StatusTexts.Num() > VisibleCount)
	{
		UTextBlock* OverflowLabel = WidgetTree
			? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass())
			: NewObject<UTextBlock>(this);
		if (OverflowLabel)
		{
			OverflowLabel->SetText(FText::Format(
				NSLOCTEXT("FinalBattleHUD", "CharacterEntryStatusOverflow", "+{0}"),
				FText::AsNumber(CharacterEntryViewData.StatusTexts.Num() - VisibleCount)));
			StatusBox->AddChild(OverflowLabel);
		}
	}
}
