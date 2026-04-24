#include "UI/Screens/Battle/FinalBattleHUDScreen.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
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

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("BattleHUDRoot"));
	WidgetTree->RootWidget = RootCanvas;

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("BattleHUDShell"));
	if (UCanvasPanelSlot* RootSlot = RootCanvas->AddChildToCanvas(RootBox))
	{
		RootSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		RootSlot->SetOffsets(FMargin(12.0f));
	}

	UHorizontalBox* TopOverlayBar = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("TopOverlayBar"));
	USizeBox* TopOverlaySizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("TopOverlaySizeBox"));
	TopOverlaySizeBox->SetHeightOverride(144.0f);
	TopOverlaySizeBox->SetContent(TopOverlayBar);
	if (UVerticalBoxSlot* TopOverlaySlot = RootBox->AddChildToVerticalBox(TopOverlaySizeBox))
	{
		TopOverlaySlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));
	}

	UVerticalBox* TopOverlayLeft = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TopOverlayLeft"));
	if (UHorizontalBoxSlot* LeftSlot = TopOverlayBar->AddChildToHorizontalBox(TopOverlayLeft))
	{
		FSlateChildSize LeftFill(ESlateSizeRule::Fill);
		LeftFill.Value = 0.52f;
		LeftSlot->SetSize(LeftFill);
		LeftSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));
	}

	TopBarPanel = WidgetTree->ConstructWidget<UFinalBattleTopBarPanel>(UFinalBattleTopBarPanel::StaticClass(), TEXT("TopBarPanel"));
	TopOverlayLeft->AddChildToVerticalBox(TopBarPanel);

	FeedbackPanel = WidgetTree->ConstructWidget<UFinalBattleFeedbackPanel>(UFinalBattleFeedbackPanel::StaticClass(), TEXT("FeedbackPanel"));
	if (UVerticalBoxSlot* FeedbackSlot = TopOverlayLeft->AddChildToVerticalBox(FeedbackPanel))
	{
		FeedbackSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
	}

	UVerticalBox* TopOverlayRight = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TopOverlayRight"));
	if (UHorizontalBoxSlot* RightSlot = TopOverlayBar->AddChildToHorizontalBox(TopOverlayRight))
	{
		FSlateChildSize RightFill(ESlateSizeRule::Fill);
		RightFill.Value = 0.48f;
		RightSlot->SetSize(RightFill);
	}

	ContextPanel = WidgetTree->ConstructWidget<UFinalBattleContextPanel>(UFinalBattleContextPanel::StaticClass(), TEXT("ContextPanel"));
	TopOverlayRight->AddChildToVerticalBox(ContextPanel);

	RecentEventPanel = WidgetTree->ConstructWidget<UFinalBattleRecentEventPanel>(UFinalBattleRecentEventPanel::StaticClass(), TEXT("RecentEventPanel"));
	USizeBox* RecentEventSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RecentEventSizeBox"));
	RecentEventSizeBox->SetHeightOverride(92.0f);
	RecentEventSizeBox->SetContent(RecentEventPanel);
	if (UVerticalBoxSlot* EventSlot = TopOverlayRight->AddChildToVerticalBox(RecentEventSizeBox))
	{
		EventSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
	}

	UHorizontalBox* BattlefieldRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("BattlefieldRow"));
	if (UVerticalBoxSlot* BattlefieldSlot = RootBox->AddChildToVerticalBox(BattlefieldRow))
	{
		BattlefieldSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		BattlefieldSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));
	}

	CharacterPanel = WidgetTree->ConstructWidget<UFinalBattleCharacterPanel>(UFinalBattleCharacterPanel::StaticClass(), TEXT("CharacterPanel"));
	USizeBox* CharacterRail = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("CharacterRail"));
	CharacterRail->SetMinDesiredWidth(220.0f);
	CharacterRail->SetMaxDesiredWidth(300.0f);
	CharacterRail->SetHeightOverride(0.0f);
	CharacterRail->SetContent(CharacterPanel);
	if (UHorizontalBoxSlot* CharacterSlot = BattlefieldRow->AddChildToHorizontalBox(CharacterRail))
	{
		FSlateChildSize CharacterFill(ESlateSizeRule::Fill);
		CharacterFill.Value = 0.24f;
		CharacterSlot->SetSize(CharacterFill);
		CharacterSlot->SetPadding(FMargin(0.0f, 0.0f, 12.0f, 0.0f));
	}

	UBorder* BattlefieldPlaceholder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BattlefieldPlaceholder"));
	BattlefieldPlaceholder->SetBrushColor(FLinearColor(0.02f, 0.05f, 0.08f, 0.25f));
	BattlefieldPlaceholder->SetPadding(FMargin(24.0f));
	USpacer* BattlefieldSpacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), TEXT("BattlefieldSpacer"));
	BattlefieldSpacer->SetSize(FVector2D(640.0f, 360.0f));
	BattlefieldPlaceholder->SetContent(BattlefieldSpacer);
	if (UHorizontalBoxSlot* CenterSlot = BattlefieldRow->AddChildToHorizontalBox(BattlefieldPlaceholder))
	{
		FSlateChildSize CenterFill(ESlateSizeRule::Fill);
		CenterFill.Value = 0.52f;
		CenterSlot->SetSize(CenterFill);
		CenterSlot->SetPadding(FMargin(0.0f, 0.0f, 12.0f, 0.0f));
	}

	EnemyPanel = WidgetTree->ConstructWidget<UFinalBattleEnemyPanel>(UFinalBattleEnemyPanel::StaticClass(), TEXT("EnemyPanel"));
	USizeBox* EnemyRail = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("EnemyRail"));
	EnemyRail->SetMinDesiredWidth(260.0f);
	EnemyRail->SetMaxDesiredWidth(340.0f);
	EnemyRail->SetContent(EnemyPanel);
	if (UHorizontalBoxSlot* EnemySlot = BattlefieldRow->AddChildToHorizontalBox(EnemyRail))
	{
		FSlateChildSize EnemyFill(ESlateSizeRule::Fill);
		EnemyFill.Value = 0.24f;
		EnemySlot->SetSize(EnemyFill);
	}

	UHorizontalBox* BottomCommandBar = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("BottomCommandBar"));
	USizeBox* BottomCommandSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("BottomCommandSizeBox"));
	BottomCommandSizeBox->SetHeightOverride(184.0f);
	BottomCommandSizeBox->SetContent(BottomCommandBar);
	if (UVerticalBoxSlot* BottomCommandSlot = RootBox->AddChildToVerticalBox(BottomCommandSizeBox))
	{
		BottomCommandSlot->SetPadding(FMargin(0.0f));
	}

	UVerticalBox* BottomLeftColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("BottomLeftColumn"));
	USizeBox* BottomLeftSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("BottomLeftSizeBox"));
	BottomLeftSizeBox->SetMinDesiredWidth(180.0f);
	BottomLeftSizeBox->SetMaxDesiredWidth(240.0f);
	BottomLeftSizeBox->SetContent(BottomLeftColumn);
	if (UHorizontalBoxSlot* BottomLeftSlot = BottomCommandBar->AddChildToHorizontalBox(BottomLeftSizeBox))
	{
		FSlateChildSize BottomLeftFill(ESlateSizeRule::Fill);
		BottomLeftFill.Value = 0.18f;
		BottomLeftSlot->SetSize(BottomLeftFill);
		BottomLeftSlot->SetPadding(FMargin(0.0f, 0.0f, 12.0f, 0.0f));
	}

	UltimatePanel = WidgetTree->ConstructWidget<UFinalBattleUltimatePanel>(UFinalBattleUltimatePanel::StaticClass(), TEXT("UltimatePanel"));
	BottomLeftColumn->AddChildToVerticalBox(UltimatePanel);

	HandPanel = WidgetTree->ConstructWidget<UFinalBattleHandPanel>(UFinalBattleHandPanel::StaticClass(), TEXT("HandPanel"));
	USizeBox* HandZone = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("HandZone"));
	HandZone->SetHeightOverride(172.0f);
	HandZone->SetContent(HandPanel);
	if (UHorizontalBoxSlot* HandSlot = BottomCommandBar->AddChildToHorizontalBox(HandZone))
	{
		FSlateChildSize HandFill(ESlateSizeRule::Fill);
		HandFill.Value = 0.62f;
		HandSlot->SetSize(HandFill);
		HandSlot->SetPadding(FMargin(0.0f, 0.0f, 12.0f, 0.0f));
	}

	ActionPanel = WidgetTree->ConstructWidget<UFinalBattleActionPanel>(UFinalBattleActionPanel::StaticClass(), TEXT("ActionPanel"));
	USizeBox* ActionZone = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("ActionZone"));
	ActionZone->SetMinDesiredWidth(180.0f);
	ActionZone->SetMaxDesiredWidth(220.0f);
	ActionZone->SetContent(ActionPanel);
	if (UHorizontalBoxSlot* ActionSlot = BottomCommandBar->AddChildToHorizontalBox(ActionZone))
	{
		FSlateChildSize ActionFill(ESlateSizeRule::Fill);
		ActionFill.Value = 0.20f;
		ActionSlot->SetSize(ActionFill);
	}
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
