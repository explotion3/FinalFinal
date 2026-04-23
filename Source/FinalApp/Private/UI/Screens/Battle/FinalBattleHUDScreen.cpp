#include "UI/Screens/Battle/FinalBattleHUDScreen.h"

#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Controllers/FinalBattleWidgetController.h"
#include "UI/Panels/Battle/FinalBattleHUDPanels.h"
#include "ViewModels/FinalBattleHUDViewModel.h"

void UFinalBattleHUDScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
}

void UFinalBattleHUDScreen::InitializeScreen(UFinalBattleHUDViewModel* InViewModel, UFinalBattleWidgetController* InController)
{
	BattleViewModel = InViewModel;
	BattleController = InController;
	SetPresentationContext(InController, InViewModel);
	InitializePanels();
}

void UFinalBattleHUDScreen::EnsureWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	ScreenLayer = EFinalUIScreenLayer::HUD;

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("BattleHUDRoot"));
	WidgetTree->RootWidget = RootBox;

	TopBarPanel = WidgetTree->ConstructWidget<UFinalBattleTopBarPanel>(UFinalBattleTopBarPanel::StaticClass(), TEXT("TopBarPanel"));
	RootBox->AddChildToVerticalBox(TopBarPanel);

	FeedbackPanel = WidgetTree->ConstructWidget<UFinalBattleFeedbackPanel>(UFinalBattleFeedbackPanel::StaticClass(), TEXT("FeedbackPanel"));
	RootBox->AddChildToVerticalBox(FeedbackPanel);

	ContextPanel = WidgetTree->ConstructWidget<UFinalBattleContextPanel>(UFinalBattleContextPanel::StaticClass(), TEXT("ContextPanel"));
	RootBox->AddChildToVerticalBox(ContextPanel);

	UHorizontalBox* MiddleRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("MiddleRow"));
	RootBox->AddChildToVerticalBox(MiddleRow);

	CharacterPanel = WidgetTree->ConstructWidget<UFinalBattleCharacterPanel>(UFinalBattleCharacterPanel::StaticClass(), TEXT("CharacterPanel"));
	if (UHorizontalBoxSlot* CharacterSlot = MiddleRow->AddChildToHorizontalBox(CharacterPanel))
	{
		CharacterSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		CharacterSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));
	}

	EnemyPanel = WidgetTree->ConstructWidget<UFinalBattleEnemyPanel>(UFinalBattleEnemyPanel::StaticClass(), TEXT("EnemyPanel"));
	if (UHorizontalBoxSlot* EnemySlot = MiddleRow->AddChildToHorizontalBox(EnemyPanel))
	{
		EnemySlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	UHorizontalBox* BottomRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("BottomRow"));
	RootBox->AddChildToVerticalBox(BottomRow);

	RecentEventPanel = WidgetTree->ConstructWidget<UFinalBattleRecentEventPanel>(UFinalBattleRecentEventPanel::StaticClass(), TEXT("RecentEventPanel"));
	if (UHorizontalBoxSlot* EventSlot = BottomRow->AddChildToHorizontalBox(RecentEventPanel))
	{
		EventSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		EventSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));
	}

	HandPanel = WidgetTree->ConstructWidget<UFinalBattleHandPanel>(UFinalBattleHandPanel::StaticClass(), TEXT("HandPanel"));
	if (UHorizontalBoxSlot* HandSlot = BottomRow->AddChildToHorizontalBox(HandPanel))
	{
		HandSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		HandSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));
	}

	UVerticalBox* SideColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SideColumn"));
	if (UHorizontalBoxSlot* SideSlot = BottomRow->AddChildToHorizontalBox(SideColumn))
	{
		SideSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	UltimatePanel = WidgetTree->ConstructWidget<UFinalBattleUltimatePanel>(UFinalBattleUltimatePanel::StaticClass(), TEXT("UltimatePanel"));
	if (UVerticalBoxSlot* UltimateSlot = SideColumn->AddChildToVerticalBox(UltimatePanel))
	{
		UltimateSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
	}

	ActionPanel = WidgetTree->ConstructWidget<UFinalBattleActionPanel>(UFinalBattleActionPanel::StaticClass(), TEXT("ActionPanel"));
	SideColumn->AddChildToVerticalBox(ActionPanel);
}

void UFinalBattleHUDScreen::InitializePanels()
{
	if (BattleViewModel == nullptr || BattleController == nullptr)
	{
		return;
	}

	BattleViewModel->EnsurePanelViewModels();

	if (TopBarPanel)
	{
		TopBarPanel->InitializePanel(BattleViewModel->GetTopBarViewModel(), BattleController->GetTopBarPanelController());
	}

	if (FeedbackPanel)
	{
		FeedbackPanel->InitializePanel(BattleViewModel->GetFeedbackViewModel(), BattleController->GetFeedbackPanelController());
	}

	if (ContextPanel)
	{
		ContextPanel->InitializePanel(BattleViewModel->GetContextViewModel(), BattleController->GetContextPanelController());
	}

	if (CharacterPanel)
	{
		CharacterPanel->InitializePanel(BattleViewModel->GetCharacterViewModel(), BattleController->GetCharacterPanelController());
	}

	if (EnemyPanel)
	{
		EnemyPanel->InitializePanel(BattleViewModel->GetEnemyViewModel(), BattleController->GetEnemyPanelController());
	}

	if (HandPanel)
	{
		HandPanel->InitializePanel(BattleViewModel->GetHandViewModel(), BattleController->GetHandPanelController());
	}

	if (UltimatePanel)
	{
		UltimatePanel->InitializePanel(BattleViewModel->GetUltimateViewModel(), BattleController->GetUltimatePanelController());
	}

	if (RecentEventPanel)
	{
		RecentEventPanel->InitializePanel(BattleViewModel->GetRecentEventViewModel(), BattleController->GetRecentEventPanelController());
	}

	if (ActionPanel)
	{
		ActionPanel->InitializePanel(BattleViewModel->GetActionViewModel(), BattleController->GetActionPanelController());
	}
}
