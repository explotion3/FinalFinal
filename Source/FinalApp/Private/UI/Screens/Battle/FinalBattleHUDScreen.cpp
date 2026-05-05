#include "UI/Screens/Battle/FinalBattleHUDScreen.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Controllers/FinalBattleWidgetController.h"
#include "UI/Panels/Battle/FinalBattleHUDPanels.h"
#include "UI/Settings/FinalUIWidgetClassSettings.h"
#include "ViewModels/FinalBattleHUDViewModel.h"

template <>
UFinalBattleTopBarPanel* UFinalBattleHUDScreen::CreateConfiguredPanel<UFinalBattleTopBarPanel>(const TCHAR* WidgetName)
{
	return WidgetTree->ConstructWidget<UFinalBattleTopBarPanel>(UFinalUIWidgetClassSettings::GetBattleTopBarPanelClass(), WidgetName);
}

template <>
UFinalBattleResourcePanel* UFinalBattleHUDScreen::CreateConfiguredPanel<UFinalBattleResourcePanel>(const TCHAR* WidgetName)
{
	return WidgetTree->ConstructWidget<UFinalBattleResourcePanel>(UFinalUIWidgetClassSettings::GetBattleResourcePanelClass(), WidgetName);
}

template <>
UFinalRunFlowPromptPanel* UFinalBattleHUDScreen::CreateConfiguredPanel<UFinalRunFlowPromptPanel>(const TCHAR* WidgetName)
{
	return WidgetTree->ConstructWidget<UFinalRunFlowPromptPanel>(UFinalUIWidgetClassSettings::GetBattleRunFlowPromptPanelClass(), WidgetName);
}

template <>
UFinalBattleFeedbackPanel* UFinalBattleHUDScreen::CreateConfiguredPanel<UFinalBattleFeedbackPanel>(const TCHAR* WidgetName)
{
	return WidgetTree->ConstructWidget<UFinalBattleFeedbackPanel>(UFinalUIWidgetClassSettings::GetBattleFeedbackPanelClass(), WidgetName);
}

template <>
UFinalBattleContextPanel* UFinalBattleHUDScreen::CreateConfiguredPanel<UFinalBattleContextPanel>(const TCHAR* WidgetName)
{
	return WidgetTree->ConstructWidget<UFinalBattleContextPanel>(UFinalUIWidgetClassSettings::GetBattleContextPanelClass(), WidgetName);
}

template <>
UFinalBattleCharacterPanel* UFinalBattleHUDScreen::CreateConfiguredPanel<UFinalBattleCharacterPanel>(const TCHAR* WidgetName)
{
	return WidgetTree->ConstructWidget<UFinalBattleCharacterPanel>(UFinalUIWidgetClassSettings::GetBattleCharacterPanelClass(), WidgetName);
}

template <>
UFinalBattleEnemyPanel* UFinalBattleHUDScreen::CreateConfiguredPanel<UFinalBattleEnemyPanel>(const TCHAR* WidgetName)
{
	return WidgetTree->ConstructWidget<UFinalBattleEnemyPanel>(UFinalUIWidgetClassSettings::GetBattleEnemyPanelClass(), WidgetName);
}

template <>
UFinalBattleEnemyDetailPanel* UFinalBattleHUDScreen::CreateConfiguredPanel<UFinalBattleEnemyDetailPanel>(const TCHAR* WidgetName)
{
	return WidgetTree->ConstructWidget<UFinalBattleEnemyDetailPanel>(UFinalUIWidgetClassSettings::GetBattleEnemyDetailPanelClass(), WidgetName);
}

template <>
UFinalBattleCharacterDetailPanel* UFinalBattleHUDScreen::CreateConfiguredPanel<UFinalBattleCharacterDetailPanel>(const TCHAR* WidgetName)
{
	return WidgetTree->ConstructWidget<UFinalBattleCharacterDetailPanel>(UFinalUIWidgetClassSettings::GetBattleCharacterDetailPanelClass(), WidgetName);
}

template <>
UFinalBattleCardZoneDetailPanel* UFinalBattleHUDScreen::CreateConfiguredPanel<UFinalBattleCardZoneDetailPanel>(const TCHAR* WidgetName)
{
	return WidgetTree->ConstructWidget<UFinalBattleCardZoneDetailPanel>(UFinalUIWidgetClassSettings::GetBattleCardZoneDetailPanelClass(), WidgetName);
}

template <>
UFinalBattleHandPanel* UFinalBattleHUDScreen::CreateConfiguredPanel<UFinalBattleHandPanel>(const TCHAR* WidgetName)
{
	return WidgetTree->ConstructWidget<UFinalBattleHandPanel>(UFinalUIWidgetClassSettings::GetBattleHandPanelClass(), WidgetName);
}

template <>
UFinalBattleUltimatePanel* UFinalBattleHUDScreen::CreateConfiguredPanel<UFinalBattleUltimatePanel>(const TCHAR* WidgetName)
{
	return WidgetTree->ConstructWidget<UFinalBattleUltimatePanel>(UFinalUIWidgetClassSettings::GetBattleUltimatePanelClass(), WidgetName);
}

template <>
UFinalBattleRecentEventPanel* UFinalBattleHUDScreen::CreateConfiguredPanel<UFinalBattleRecentEventPanel>(const TCHAR* WidgetName)
{
	return WidgetTree->ConstructWidget<UFinalBattleRecentEventPanel>(UFinalUIWidgetClassSettings::GetBattleRecentEventPanelClass(), WidgetName);
}

template <>
UFinalBattleActionPanel* UFinalBattleHUDScreen::CreateConfiguredPanel<UFinalBattleActionPanel>(const TCHAR* WidgetName)
{
	return WidgetTree->ConstructWidget<UFinalBattleActionPanel>(UFinalUIWidgetClassSettings::GetBattleActionPanelClass(), WidgetName);
}

void UFinalBattleHUDScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	EnsureWidgetTree();
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (WidgetTree && WidgetTree->RootWidget)
	{
		WidgetTree->RootWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UFinalBattleHUDScreen::InitializeScreen(UFinalBattleHUDViewModel* InViewModel, UFinalBattleWidgetController* InController)
{
	BattleViewModel = InViewModel;
	BattleController = InController;
	SetPresentationContext(InController, InViewModel);
	InitializePanels();
}

void UFinalBattleHUDScreen::AddPanelToSlot(UOverlay* TargetSlot, UWidget* Panel) const
{
	if (TargetSlot == nullptr || Panel == nullptr || Panel->GetParent() != nullptr)
	{
		return;
	}

	if (UOverlaySlot* PanelSlot = TargetSlot->AddChildToOverlay(Panel))
	{
		PanelSlot->SetHorizontalAlignment(HAlign_Fill);
		PanelSlot->SetVerticalAlignment(VAlign_Fill);
	}
}

void UFinalBattleHUDScreen::EnsurePanelsInBlueprintSlots()
{
	if (TopBarSlot != nullptr && TopBarPanel == nullptr)
	{
		TopBarPanel = CreateConfiguredPanel<UFinalBattleTopBarPanel>(TEXT("TopBarPanel"));
		AddPanelToSlot(TopBarSlot, TopBarPanel);
	}

	if (ResourceSlot != nullptr && ResourcePanel == nullptr)
	{
		ResourcePanel = CreateConfiguredPanel<UFinalBattleResourcePanel>(TEXT("ResourcePanel"));
		AddPanelToSlot(ResourceSlot, ResourcePanel);
	}

	if (RunFlowPromptSlot != nullptr && RunFlowPromptPanel == nullptr)
	{
		RunFlowPromptPanel = CreateConfiguredPanel<UFinalRunFlowPromptPanel>(TEXT("RunFlowPromptPanel"));
		AddPanelToSlot(RunFlowPromptSlot, RunFlowPromptPanel);
	}

	if (FeedbackSlot != nullptr && FeedbackPanel == nullptr)
	{
		FeedbackPanel = CreateConfiguredPanel<UFinalBattleFeedbackPanel>(TEXT("FeedbackPanel"));
		AddPanelToSlot(FeedbackSlot, FeedbackPanel);
	}

	if (ContextSlot != nullptr && ContextPanel == nullptr)
	{
		ContextPanel = CreateConfiguredPanel<UFinalBattleContextPanel>(TEXT("ContextPanel"));
		AddPanelToSlot(ContextSlot, ContextPanel);
	}

	if (CharacterPanelSlot != nullptr && CharacterPanel == nullptr)
	{
		CharacterPanel = CreateConfiguredPanel<UFinalBattleCharacterPanel>(TEXT("CharacterPanel"));
		AddPanelToSlot(CharacterPanelSlot, CharacterPanel);
	}

	if (LegacyEnemyPanelSlot != nullptr && EnemyPanel == nullptr)
	{
		EnemyPanel = CreateConfiguredPanel<UFinalBattleEnemyPanel>(TEXT("EnemyPanel"));
		AddPanelToSlot(LegacyEnemyPanelSlot, EnemyPanel);
	}

	if (EnemyDetailSlot != nullptr && EnemyDetailPanel == nullptr)
	{
		EnemyDetailPanel = CreateConfiguredPanel<UFinalBattleEnemyDetailPanel>(TEXT("EnemyDetailPanel"));
		AddPanelToSlot(EnemyDetailSlot, EnemyDetailPanel);
	}

	if (CharacterDetailSlot != nullptr && CharacterDetailPanel == nullptr)
	{
		CharacterDetailPanel = CreateConfiguredPanel<UFinalBattleCharacterDetailPanel>(TEXT("CharacterDetailPanel"));
		AddPanelToSlot(CharacterDetailSlot, CharacterDetailPanel);
	}

	if (CardZoneDetailSlot != nullptr && CardZoneDetailPanel == nullptr)
	{
		CardZoneDetailPanel = CreateConfiguredPanel<UFinalBattleCardZoneDetailPanel>(TEXT("CardZoneDetailPanel"));
		AddPanelToSlot(CardZoneDetailSlot, CardZoneDetailPanel);
	}

	if (UltimateSlot != nullptr && UltimatePanel == nullptr)
	{
		UltimatePanel = CreateConfiguredPanel<UFinalBattleUltimatePanel>(TEXT("UltimatePanel"));
		AddPanelToSlot(UltimateSlot, UltimatePanel);
	}

	if (HandSlot != nullptr && HandPanel == nullptr)
	{
		HandPanel = CreateConfiguredPanel<UFinalBattleHandPanel>(TEXT("HandPanel"));
		AddPanelToSlot(HandSlot, HandPanel);
	}

	if (RecentEventSlot != nullptr && RecentEventPanel == nullptr)
	{
		RecentEventPanel = CreateConfiguredPanel<UFinalBattleRecentEventPanel>(TEXT("RecentEventPanel"));
		AddPanelToSlot(RecentEventSlot, RecentEventPanel);
	}

	if (ActionSlot != nullptr && ActionPanel == nullptr)
	{
		ActionPanel = CreateConfiguredPanel<UFinalBattleActionPanel>(TEXT("ActionPanel"));
		AddPanelToSlot(ActionSlot, ActionPanel);
	}
}

void UFinalBattleHUDScreen::EnsureWidgetTree()
{
	if (WidgetTree == nullptr)
	{
		return;
	}

	ScreenLayer = EFinalUIScreenLayer::HUD;

	const bool bHasBlueprintPanelSlots =
		TopBarPanel != nullptr ||
		ResourcePanel != nullptr ||
		RunFlowPromptPanel != nullptr ||
		FeedbackPanel != nullptr ||
		ContextPanel != nullptr ||
		CharacterPanel != nullptr ||
		EnemyPanel != nullptr ||
		EnemyDetailPanel != nullptr ||
		CharacterDetailPanel != nullptr ||
		CardZoneDetailPanel != nullptr ||
		HandPanel != nullptr ||
		UltimatePanel != nullptr ||
		RecentEventPanel != nullptr ||
		ActionPanel != nullptr;

	const bool bHasBlueprintLayoutSlots =
		TopBarSlot != nullptr ||
		ResourceSlot != nullptr ||
		RunFlowPromptSlot != nullptr ||
		FeedbackSlot != nullptr ||
		ContextSlot != nullptr ||
		CharacterPanelSlot != nullptr ||
		LegacyEnemyPanelSlot != nullptr ||
		EnemyDetailSlot != nullptr ||
		CharacterDetailSlot != nullptr ||
		CardZoneDetailSlot != nullptr ||
		UltimateSlot != nullptr ||
		HandSlot != nullptr ||
		RecentEventSlot != nullptr ||
		ActionSlot != nullptr;

	if (WidgetTree->RootWidget != nullptr && (bHasBlueprintPanelSlots || bHasBlueprintLayoutSlots))
	{
		EnsurePanelsInBlueprintSlots();
		return;
	}

	if (WidgetTree->RootWidget != nullptr)
	{
		WidgetTree->RootWidget = nullptr;
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("BattleHUDRoot"));
	RootCanvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	WidgetTree->RootWidget = RootCanvas;

	UOverlay* BattlefieldGlass = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("BattlefieldGlass"));
	BattlefieldGlass->SetVisibility(ESlateVisibility::HitTestInvisible);
	if (UCanvasPanelSlot* BattlefieldSlot = RootCanvas->AddChildToCanvas(BattlefieldGlass))
	{
		BattlefieldSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		BattlefieldSlot->SetOffsets(FMargin(0.0f));
	}

	UBorder* VignetteBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("InkVignette"));
	VignetteBorder->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.08f));
	VignetteBorder->SetPadding(FMargin(0.0f));
	VignetteBorder->SetVisibility(ESlateVisibility::HitTestInvisible);
	BattlefieldGlass->AddChildToOverlay(VignetteBorder);

	ResourcePanel = CreateConfiguredPanel<UFinalBattleResourcePanel>(TEXT("ResourcePanel"));
	if (UCanvasPanelSlot* ResourceCanvasSlot = RootCanvas->AddChildToCanvas(ResourcePanel))
	{
		ResourceCanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		ResourceCanvasSlot->SetOffsets(FMargin(0.0f));
		ResourceCanvasSlot->SetZOrder(80);
	}

	RunFlowPromptPanel = CreateConfiguredPanel<UFinalRunFlowPromptPanel>(TEXT("RunFlowPromptPanel"));
	if (UCanvasPanelSlot* PromptSlot = RootCanvas->AddChildToCanvas(RunFlowPromptPanel))
	{
		PromptSlot->SetAnchors(FAnchors(0.72f, 0.48f, 0.92f, 0.56f));
		PromptSlot->SetOffsets(FMargin(0.0f));
		PromptSlot->SetZOrder(65);
	}

	FeedbackPanel = CreateConfiguredPanel<UFinalBattleFeedbackPanel>(TEXT("FeedbackPanel"));
	if (UCanvasPanelSlot* FeedbackCanvasSlot = RootCanvas->AddChildToCanvas(FeedbackPanel))
	{
		FeedbackCanvasSlot->SetAnchors(FAnchors(0.32f, 0.90f, 0.68f, 0.98f));
		FeedbackCanvasSlot->SetOffsets(FMargin(0.0f));
		FeedbackCanvasSlot->SetZOrder(20);
	}

	ContextPanel = CreateConfiguredPanel<UFinalBattleContextPanel>(TEXT("ContextPanel"));
	if (UCanvasPanelSlot* ContextCanvasSlot = RootCanvas->AddChildToCanvas(ContextPanel))
	{
		ContextCanvasSlot->SetAnchors(FAnchors(0.825f, 0.02f, 0.985f, 0.18f));
		ContextCanvasSlot->SetOffsets(FMargin(0.0f));
		ContextCanvasSlot->SetZOrder(62);
	}

	CharacterPanel = CreateConfiguredPanel<UFinalBattleCharacterPanel>(TEXT("CharacterPanel"));
	if (UCanvasPanelSlot* CharacterSlot = RootCanvas->AddChildToCanvas(CharacterPanel))
	{
		CharacterSlot->SetAnchors(FAnchors(0.015f, 0.02f, 0.23f, 0.48f));
		CharacterSlot->SetOffsets(FMargin(0.0f));
	}

	EnemyPanel = CreateConfiguredPanel<UFinalBattleEnemyPanel>(TEXT("EnemyPanel"));
	if (UCanvasPanelSlot* EnemySlot = RootCanvas->AddChildToCanvas(EnemyPanel))
	{
		EnemySlot->SetAnchors(FAnchors(0.30f, 0.02f, 0.82f, 0.20f));
		EnemySlot->SetOffsets(FMargin(0.0f));
	}

	EnemyDetailPanel = CreateConfiguredPanel<UFinalBattleEnemyDetailPanel>(TEXT("EnemyDetailPanel"));
	if (UCanvasPanelSlot* EnemyDetailCanvasSlot = RootCanvas->AddChildToCanvas(EnemyDetailPanel))
	{
		EnemyDetailCanvasSlot->SetAnchors(FAnchors(0.70f, 0.18f, 0.985f, 0.60f));
		EnemyDetailCanvasSlot->SetOffsets(FMargin(0.0f));
		EnemyDetailCanvasSlot->SetZOrder(85);
	}

	CharacterDetailPanel = CreateConfiguredPanel<UFinalBattleCharacterDetailPanel>(TEXT("CharacterDetailPanel"));
	if (UCanvasPanelSlot* CharacterDetailCanvasSlot = RootCanvas->AddChildToCanvas(CharacterDetailPanel))
	{
		CharacterDetailCanvasSlot->SetAnchors(FAnchors(0.70f, 0.18f, 0.985f, 0.60f));
		CharacterDetailCanvasSlot->SetOffsets(FMargin(0.0f));
		CharacterDetailCanvasSlot->SetZOrder(86);
	}

	CardZoneDetailPanel = CreateConfiguredPanel<UFinalBattleCardZoneDetailPanel>(TEXT("CardZoneDetailPanel"));
	if (UCanvasPanelSlot* CardZoneDetailCanvasSlot = RootCanvas->AddChildToCanvas(CardZoneDetailPanel))
	{
		CardZoneDetailCanvasSlot->SetAnchors(FAnchors(0.24f, 0.14f, 0.76f, 0.78f));
		CardZoneDetailCanvasSlot->SetOffsets(FMargin(0.0f));
		CardZoneDetailCanvasSlot->SetZOrder(95);
	}

	UltimatePanel = CreateConfiguredPanel<UFinalBattleUltimatePanel>(TEXT("UltimatePanel"));
	if (UCanvasPanelSlot* UltimateCanvasSlot = RootCanvas->AddChildToCanvas(UltimatePanel))
	{
		UltimateCanvasSlot->SetAnchors(FAnchors(0.015f, 0.49f, 0.18f, 0.65f));
		UltimateCanvasSlot->SetOffsets(FMargin(0.0f));
	}

	HandPanel = CreateConfiguredPanel<UFinalBattleHandPanel>(TEXT("HandPanel"));
	if (UCanvasPanelSlot* HandCanvasSlot = RootCanvas->AddChildToCanvas(HandPanel))
	{
		HandCanvasSlot->SetAnchors(FAnchors(0.16f, 0.56f, 0.82f, 0.985f));
		HandCanvasSlot->SetOffsets(FMargin(0.0f));
		HandCanvasSlot->SetZOrder(40);
	}

	ActionPanel = CreateConfiguredPanel<UFinalBattleActionPanel>(TEXT("ActionPanel"));
	if (UCanvasPanelSlot* ActionCanvasSlot = RootCanvas->AddChildToCanvas(ActionPanel))
	{
		ActionCanvasSlot->SetAnchors(FAnchors(0.825f, 0.63f, 0.985f, 0.975f));
		ActionCanvasSlot->SetOffsets(FMargin(0.0f));
		ActionCanvasSlot->SetZOrder(60);
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

	if (ResourcePanel)
	{
		ResourcePanel->InitializePanel(BattleViewModel->GetResourceViewModel(), BattleController->GetResourcePanelController());
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

	if (EnemyDetailPanel)
	{
		EnemyDetailPanel->InitializePanel(BattleViewModel->GetEnemyDetailViewModel(), BattleController->GetEnemyDetailPanelController());
	}

	if (CharacterDetailPanel)
	{
		CharacterDetailPanel->InitializePanel(BattleViewModel->GetCharacterDetailViewModel(), BattleController->GetCharacterDetailPanelController());
	}

	if (CardZoneDetailPanel)
	{
		CardZoneDetailPanel->InitializePanel(BattleViewModel->GetCardZoneDetailViewModel(), BattleController->GetCardZoneDetailPanelController());
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
