#include "UI/Widgets/Battle/FinalBattleTeamCharacterEntryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Controllers/Battle/FinalBattleHUDPanelControllers.h"
#include "Styling/CoreStyle.h"

void UFinalBattleTeamCharacterEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
	if (InspectButton)
	{
		InspectButton->OnClicked.AddUniqueDynamic(this, &UFinalBattleTeamCharacterEntryWidget::HandleInspectClicked);
	}
}

FReply UFinalBattleTeamCharacterEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InspectButton == nullptr && bAllowInspectOnClick && TryInspectCharacter())
	{
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UFinalBattleTeamCharacterEntryWidget::ApplyTeamCharacterEntryView(const FFinalBattleHUDTeamCharacterEntry& ViewData)
{
	TeamCharacterEntryViewData = ViewData;
	EnsureWidgetTree();
	RefreshBoundWidgets();
	OnTeamCharacterEntryViewApplied(TeamCharacterEntryViewData);
}

void UFinalBattleTeamCharacterEntryWidget::HandleInspectClicked()
{
	TryInspectCharacter();
}

bool UFinalBattleTeamCharacterEntryWidget::TryInspectCharacter() const
{
	if (!bAllowInspectOnClick || TeamCharacterEntryViewData.RuntimeUnitId.IsNone())
	{
		return false;
	}

	if (UFinalBattleTeamPanelController* TeamPanelController = Cast<UFinalBattleTeamPanelController>(GetWidgetController()))
	{
		return TeamPanelController->InspectCharacterByUnitId(TeamCharacterEntryViewData.RuntimeUnitId);
	}

	return false;
}

void UFinalBattleTeamCharacterEntryWidget::RefreshBoundWidgets()
{
	EnsureWidgetTree();

	if (NameText)
	{
		NameText->SetText(TeamCharacterEntryViewData.DisplayName);
	}

	if (StressText)
	{
		StressText->SetText(BuildStressText());
	}

	if (StressBar)
	{
		StressBar->SetPercent(TeamCharacterEntryViewData.StressPercent);
	}

	if (BreakthroughText)
	{
		BreakthroughText->SetText(BuildBreakthroughText());
	}

	if (BreakthroughBar)
	{
		BreakthroughBar->SetPercent(TeamCharacterEntryViewData.BreakthroughPercent);
		BreakthroughBar->SetFillColorAndOpacity(
			TeamCharacterEntryViewData.bBreakthroughReady
				? FLinearColor(0.97f, 0.79f, 0.26f, 1.0f)
				: FLinearColor(0.31f, 0.72f, 0.98f, 1.0f));
	}

	if (CollapsedVisual)
	{
		CollapsedVisual->SetVisibility(TeamCharacterEntryViewData.bCollapsed ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (BreakthroughReadyVisual)
	{
		BreakthroughReadyVisual->SetVisibility(TeamCharacterEntryViewData.bBreakthroughReady ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UFinalBattleTeamCharacterEntryWidget::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("TeamCharacterEntryBorder"));
	RootBorder->SetBrushColor(FLinearColor(0.10f, 0.14f, 0.20f, 0.94f));
	RootBorder->SetPadding(FMargin(5.0f));

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TeamCharacterEntryRoot"));
	NameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NameText"));
	NameText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 12));
	StressText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StressText"));
	StressText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10));
	StressBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("StressBar"));
	StressBar->SetFillColorAndOpacity(FLinearColor(0.85f, 0.32f, 0.28f, 1.0f));
	BreakthroughText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("BreakthroughText"));
	BreakthroughText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10));
	BreakthroughBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("BreakthroughBar"));
	BreakthroughBar->SetFillColorAndOpacity(FLinearColor(0.31f, 0.72f, 0.98f, 1.0f));

	RootBox->AddChildToVerticalBox(NameText);
	RootBox->AddChildToVerticalBox(StressText);
	RootBox->AddChildToVerticalBox(StressBar);
	RootBox->AddChildToVerticalBox(BreakthroughText);
	RootBox->AddChildToVerticalBox(BreakthroughBar);
	RootBorder->SetContent(RootBox);
	WidgetTree->RootWidget = RootBorder;
}

FText UFinalBattleTeamCharacterEntryWidget::BuildStressText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleTeamCharacterEntry", "StressText", "{0}/{1} 压力"),
		FText::AsNumber(TeamCharacterEntryViewData.CurrentStress),
		FText::AsNumber(TeamCharacterEntryViewData.StressCap));
}

FText UFinalBattleTeamCharacterEntryWidget::BuildBreakthroughText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleTeamCharacterEntry", "BreakthroughText", "{0}/{1} 突破"),
		FText::AsNumber(TeamCharacterEntryViewData.BreakthroughValue),
		FText::AsNumber(TeamCharacterEntryViewData.BreakthroughRequiredValue));
}
