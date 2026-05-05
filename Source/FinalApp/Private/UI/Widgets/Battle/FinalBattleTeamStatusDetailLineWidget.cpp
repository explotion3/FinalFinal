#include "UI/Widgets/Battle/FinalBattleTeamStatusDetailLineWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Styling/CoreStyle.h"

void UFinalBattleTeamStatusDetailLineWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
}

void UFinalBattleTeamStatusDetailLineWidget::ApplyTeamStatusDetailLineView(const FFinalBattleHUDTeamStatusEntry& ViewData)
{
	TeamStatusDetailLineViewData = ViewData;
	EnsureWidgetTree();
	RefreshBoundWidgets();
	OnTeamStatusDetailLineViewApplied(TeamStatusDetailLineViewData);
}

void UFinalBattleTeamStatusDetailLineWidget::RefreshBoundWidgets()
{
	EnsureWidgetTree();

	if (OwnerText)
	{
		OwnerText->SetText(TeamStatusDetailLineViewData.OwnerDisplayName);
	}

	if (StatusNameText)
	{
		StatusNameText->SetText(TeamStatusDetailLineViewData.DisplayName);
	}

	if (StackText)
	{
		StackText->SetText(BuildStackText());
	}

	if (DurationText)
	{
		const bool bHasDuration = TeamStatusDetailLineViewData.RemainingDuration > 0;
		DurationText->SetVisibility(bHasDuration ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		DurationText->SetText(BuildDurationText());
	}

	if (SummaryText)
	{
		SummaryText->SetVisibility(!TeamStatusDetailLineViewData.SummaryText.IsEmpty() ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		SummaryText->SetText(TeamStatusDetailLineViewData.SummaryText);
	}
}

void UFinalBattleTeamStatusDetailLineWidget::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TeamStatusDetailLineRoot"));
	UHorizontalBox* HeaderRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("TeamStatusDetailHeader"));
	OwnerText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("OwnerText"));
	OwnerText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 11));
	StatusNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusNameText"));
	StatusNameText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 12));
	StackText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StackText"));
	StackText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 11));
	DurationText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DurationText"));
	DurationText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 11));
	SummaryText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SummaryText"));
	SummaryText->SetAutoWrapText(true);
	SummaryText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 11));

	HeaderRow->AddChildToHorizontalBox(OwnerText);
	HeaderRow->AddChildToHorizontalBox(StatusNameText);
	HeaderRow->AddChildToHorizontalBox(StackText);
	HeaderRow->AddChildToHorizontalBox(DurationText);
	RootBox->AddChildToVerticalBox(HeaderRow);
	RootBox->AddChildToVerticalBox(SummaryText);
	WidgetTree->RootWidget = RootBox;
}

FText UFinalBattleTeamStatusDetailLineWidget::BuildStackText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleTeamStatusDetailLine", "StackText", " x{0}"),
		FText::AsNumber(TeamStatusDetailLineViewData.CurrentStacks));
}

FText UFinalBattleTeamStatusDetailLineWidget::BuildDurationText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleTeamStatusDetailLine", "DurationText", " ({0})"),
		FText::AsNumber(TeamStatusDetailLineViewData.RemainingDuration));
}
