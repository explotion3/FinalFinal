#include "UI/Screens/Battle/FinalBattleHUDScreen.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
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
		HandPanel != nullptr ||
		UltimatePanel != nullptr ||
		RecentEventPanel != nullptr ||
		ActionPanel != nullptr;

	if (WidgetTree->RootWidget != nullptr && bHasBlueprintPanelSlots)
	{
		return;
	}

	if (WidgetTree->RootWidget != nullptr)
	{
		WidgetTree->RootWidget = nullptr;
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("BattleHUDRoot"));
	WidgetTree->RootWidget = RootCanvas;

	UOverlay* BattlefieldGlass = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("BattlefieldGlass"));
	if (UCanvasPanelSlot* BattlefieldSlot = RootCanvas->AddChildToCanvas(BattlefieldGlass))
	{
		BattlefieldSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		BattlefieldSlot->SetOffsets(FMargin(0.0f));
	}

	UBorder* VignetteBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("InkVignette"));
	VignetteBorder->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.08f));
	VignetteBorder->SetPadding(FMargin(0.0f));
	BattlefieldGlass->AddChildToOverlay(VignetteBorder);

	TopBarPanel = CreateConfiguredPanel<UFinalBattleTopBarPanel>(TEXT("TopBarPanel"));
	if (UCanvasPanelSlot* TopBarSlot = RootCanvas->AddChildToCanvas(TopBarPanel))
	{
		TopBarSlot->SetAnchors(FAnchors(0.02f, 0.88f, 0.16f, 0.98f));
		TopBarSlot->SetOffsets(FMargin(0.0f));
	}

	ResourcePanel = CreateConfiguredPanel<UFinalBattleResourcePanel>(TEXT("ResourcePanel"));
	if (UCanvasPanelSlot* ResourceSlot = RootCanvas->AddChildToCanvas(ResourcePanel))
	{
		ResourceSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		ResourceSlot->SetOffsets(FMargin(0.0f));
		ResourceSlot->SetZOrder(80);
	}

	RunFlowPromptPanel = CreateConfiguredPanel<UFinalRunFlowPromptPanel>(TEXT("RunFlowPromptPanel"));
	if (UCanvasPanelSlot* PromptSlot = RootCanvas->AddChildToCanvas(RunFlowPromptPanel))
	{
		PromptSlot->SetAnchors(FAnchors(0.72f, 0.48f, 0.92f, 0.56f));
		PromptSlot->SetOffsets(FMargin(0.0f));
		PromptSlot->SetZOrder(65);
	}

	FeedbackPanel = CreateConfiguredPanel<UFinalBattleFeedbackPanel>(TEXT("FeedbackPanel"));
	if (UCanvasPanelSlot* FeedbackSlot = RootCanvas->AddChildToCanvas(FeedbackPanel))
	{
		FeedbackSlot->SetAnchors(FAnchors(0.32f, 0.90f, 0.68f, 0.98f));
		FeedbackSlot->SetOffsets(FMargin(0.0f));
		FeedbackSlot->SetZOrder(20);
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

	ContextPanel = CreateConfiguredPanel<UFinalBattleContextPanel>(TEXT("ContextPanel"));
	if (UCanvasPanelSlot* ContextSlot = RootCanvas->AddChildToCanvas(ContextPanel))
	{
		ContextSlot->SetAnchors(FAnchors(0.83f, 0.02f, 0.985f, 0.28f));
		ContextSlot->SetOffsets(FMargin(0.0f));
	}

	RecentEventPanel = CreateConfiguredPanel<UFinalBattleRecentEventPanel>(TEXT("RecentEventPanel"));
	if (UCanvasPanelSlot* RecentEventSlot = RootCanvas->AddChildToCanvas(RecentEventPanel))
	{
		RecentEventSlot->SetAnchors(FAnchors(0.38f, 0.205f, 0.70f, 0.29f));
		RecentEventSlot->SetOffsets(FMargin(0.0f));
	}

	UltimatePanel = CreateConfiguredPanel<UFinalBattleUltimatePanel>(TEXT("UltimatePanel"));
	if (UCanvasPanelSlot* UltimateSlot = RootCanvas->AddChildToCanvas(UltimatePanel))
	{
		UltimateSlot->SetAnchors(FAnchors(0.015f, 0.49f, 0.18f, 0.65f));
		UltimateSlot->SetOffsets(FMargin(0.0f));
	}

	HandPanel = CreateConfiguredPanel<UFinalBattleHandPanel>(TEXT("HandPanel"));
	if (UCanvasPanelSlot* HandSlot = RootCanvas->AddChildToCanvas(HandPanel))
	{
		HandSlot->SetAnchors(FAnchors(0.16f, 0.56f, 0.82f, 0.985f));
		HandSlot->SetOffsets(FMargin(0.0f));
		HandSlot->SetZOrder(40);
	}

	ActionPanel = CreateConfiguredPanel<UFinalBattleActionPanel>(TEXT("ActionPanel"));
	if (UCanvasPanelSlot* ActionSlot = RootCanvas->AddChildToCanvas(ActionPanel))
	{
		ActionSlot->SetAnchors(FAnchors(0.825f, 0.63f, 0.985f, 0.975f));
		ActionSlot->SetOffsets(FMargin(0.0f));
		ActionSlot->SetZOrder(60);
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
