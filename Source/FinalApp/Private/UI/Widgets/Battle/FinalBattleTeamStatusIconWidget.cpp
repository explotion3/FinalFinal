#include "UI/Widgets/Battle/FinalBattleTeamStatusIconWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Controllers/Battle/FinalBattleHUDPanelControllers.h"
#include "Styling/CoreStyle.h"

void UFinalBattleTeamStatusIconWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
	if (StatusButton)
	{
		StatusButton->OnClicked.AddUniqueDynamic(this, &UFinalBattleTeamStatusIconWidget::HandleClicked);
	}
}

void UFinalBattleTeamStatusIconWidget::ApplyTeamStatusIconView(const FFinalBattleHUDTeamStatusEntry& ViewData)
{
	TeamStatusIconViewData = ViewData;
	EnsureWidgetTree();
	RefreshBoundWidgets();
	OnTeamStatusIconViewApplied(TeamStatusIconViewData);
}

void UFinalBattleTeamStatusIconWidget::RefreshBoundWidgets()
{
	EnsureWidgetTree();

	if (StatusNameText)
	{
		StatusNameText->SetText(TeamStatusIconViewData.DisplayName);
	}

	if (StackText)
	{
		StackText->SetText(BuildStackText());
	}
}

void UFinalBattleTeamStatusIconWidget::HandleClicked()
{
	if (UFinalBattleTeamPanelController* TeamController = Cast<UFinalBattleTeamPanelController>(GetWidgetController()))
	{
		TeamController->OpenTeamStatusDetail();
	}
}

void UFinalBattleTeamStatusIconWidget::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	StatusButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("StatusButton"));
	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("StatusIconRow"));
	StatusNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusNameText"));
	StatusNameText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 11));
	StackText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StackText"));
	StackText->SetFont(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 10));
	Row->AddChildToHorizontalBox(StatusNameText);
	Row->AddChildToHorizontalBox(StackText);
	StatusButton->AddChild(Row);
	WidgetTree->RootWidget = StatusButton;
}

FText UFinalBattleTeamStatusIconWidget::BuildStackText() const
{
	return FText::Format(
		NSLOCTEXT("FinalBattleTeamStatusIcon", "StackText", " x{0}"),
		FText::AsNumber(TeamStatusIconViewData.CurrentStacks));
}
