#include "UI/Widgets/Battle/FinalBattleCharacterDetailStatusLineWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Styling/CoreStyle.h"

void UFinalBattleCharacterDetailStatusLineWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
}

void UFinalBattleCharacterDetailStatusLineWidget::ApplyStatusLineView(const FFinalBattleHUDCharacterDetailStatusEntry& ViewData)
{
	StatusLineViewData = ViewData;
	EnsureWidgetTree();
	RefreshBoundWidgets();
	OnStatusLineViewApplied(StatusLineViewData);
}

void UFinalBattleCharacterDetailStatusLineWidget::RefreshBoundWidgets()
{
	EnsureWidgetTree();

	if (StatusNameText)
	{
		StatusNameText->SetText(StatusLineViewData.DisplayName);
	}

	if (StackText)
	{
		StackText->SetText(BuildStackText());
	}

	if (DurationText)
	{
		const bool bHasDuration = StatusLineViewData.RemainingDuration > 0;
		DurationText->SetVisibility(bHasDuration ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		DurationText->SetText(bHasDuration ? BuildDurationText() : FText::GetEmpty());
	}

	if (SummaryText)
	{
		const bool bHasSummary = !StatusLineViewData.SummaryText.IsEmpty();
		SummaryText->SetVisibility(bHasSummary ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		SummaryText->SetText(StatusLineViewData.SummaryText);
	}
}

void UFinalBattleCharacterDetailStatusLineWidget::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CharacterStatusLineRoot"));
	UHorizontalBox* HeaderRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("CharacterStatusLineHeader"));
	RootBox->AddChildToVerticalBox(HeaderRow);

	StatusNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusNameText"));
	StatusNameText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 12));
	if (UHorizontalBoxSlot* NameSlot = HeaderRow->AddChildToHorizontalBox(StatusNameText))
	{
		NameSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	StackText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StackText"));
	StackText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 11));
	if (UHorizontalBoxSlot* StackSlot = HeaderRow->AddChildToHorizontalBox(StackText))
	{
		StackSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		StackSlot->SetPadding(FMargin(8.0f, 0.0f, 0.0f, 0.0f));
	}

	DurationText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DurationText"));
	DurationText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 11));
	if (UHorizontalBoxSlot* DurationSlot = HeaderRow->AddChildToHorizontalBox(DurationText))
	{
		DurationSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		DurationSlot->SetPadding(FMargin(8.0f, 0.0f, 0.0f, 0.0f));
	}

	SummaryText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SummaryText"));
	SummaryText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10));
	SummaryText->SetAutoWrapText(true);
	RootBox->AddChildToVerticalBox(SummaryText);

	WidgetTree->RootWidget = RootBox;
}

FText UFinalBattleCharacterDetailStatusLineWidget::BuildStackText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleCharacterDetailStatusLine", "StackText", "x{0}"),
		FText::AsNumber(StatusLineViewData.CurrentStacks));
}

FText UFinalBattleCharacterDetailStatusLineWidget::BuildDurationText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleCharacterDetailStatusLine", "DurationText", "{0}"),
		FText::AsNumber(StatusLineViewData.RemainingDuration));
}
