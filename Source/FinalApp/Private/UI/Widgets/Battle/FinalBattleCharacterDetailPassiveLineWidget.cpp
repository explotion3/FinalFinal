#include "UI/Widgets/Battle/FinalBattleCharacterDetailPassiveLineWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Styling/CoreStyle.h"

void UFinalBattleCharacterDetailPassiveLineWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
}

void UFinalBattleCharacterDetailPassiveLineWidget::ApplyPassiveLineView(const FFinalBattleHUDCharacterDetailPassiveEntry& ViewData)
{
	PassiveLineViewData = ViewData;
	EnsureWidgetTree();
	RefreshBoundWidgets();
	OnPassiveLineViewApplied(PassiveLineViewData);
}

void UFinalBattleCharacterDetailPassiveLineWidget::RefreshBoundWidgets()
{
	EnsureWidgetTree();

	if (PassiveNameText)
	{
		PassiveNameText->SetText(PassiveLineViewData.DisplayName);
	}

	if (StackText)
	{
		StackText->SetText(BuildStackText());
	}

	if (DurationText)
	{
		const bool bHasDuration = PassiveLineViewData.RemainingDuration > 0;
		DurationText->SetVisibility(bHasDuration ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		DurationText->SetText(bHasDuration ? BuildDurationText() : FText::GetEmpty());
	}

	if (SummaryText)
	{
		const bool bHasSummary = !PassiveLineViewData.SummaryText.IsEmpty();
		SummaryText->SetVisibility(bHasSummary ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		SummaryText->SetText(PassiveLineViewData.SummaryText);
	}
}

void UFinalBattleCharacterDetailPassiveLineWidget::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CharacterPassiveLineRoot"));
	UHorizontalBox* HeaderRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("CharacterPassiveLineHeader"));
	RootBox->AddChildToVerticalBox(HeaderRow);

	PassiveNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PassiveNameText"));
	PassiveNameText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 12));
	if (UHorizontalBoxSlot* NameSlot = HeaderRow->AddChildToHorizontalBox(PassiveNameText))
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

FText UFinalBattleCharacterDetailPassiveLineWidget::BuildStackText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleCharacterDetailPassiveLine", "StackText", "x{0}"),
		FText::AsNumber(PassiveLineViewData.CurrentStacks));
}

FText UFinalBattleCharacterDetailPassiveLineWidget::BuildDurationText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleCharacterDetailPassiveLine", "DurationText", "{0}"),
		FText::AsNumber(PassiveLineViewData.RemainingDuration));
}
