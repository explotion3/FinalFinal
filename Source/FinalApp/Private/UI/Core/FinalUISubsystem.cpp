#include "Subsystems/UI/FinalUISubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Controllers/FinalBattleWidgetController.h"
#include "Facade/FinalRunSession.h"
#include "Subsystems/FinalBattleFlowSubsystem.h"
#include "Subsystems/FinalGameFlowSubsystem.h"
#include "Subsystems/FinalRunFlowSubsystem.h"
#include "UI/Root/FinalUIRootLayout.h"
#include "UI/Screens/FinalScreenBase.h"
#include "UI/Screens/Battle/FinalBattleHUDScreen.h"
#include "UI/Screens/Flow/FinalPlaceholderModalScreen.h"
#include "UI/Screens/Flow/FinalRunEventNodeOverlayScreen.h"
#include "UI/Screens/Flow/FinalRunNodeOverlayScreen.h"
#include "UI/Screens/Flow/FinalRunRewardOverlayScreen.h"
#include "UI/Screens/Flow/FinalRunRewardNodeOverlayScreen.h"
#include "UI/Screens/Flow/FinalRunShopNodeOverlayScreen.h"
#include "UI/Screens/Flow/FinalRunStageOverlayScreenBase.h"
#include "ViewModels/FinalBattleHUDViewModel.h"

void UFinalUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UFinalUISubsystem::Deinitialize()
{
	if (BattleWidgetController)
	{
		BattleWidgetController->ShutdownController();
	}

	if (RootLayout)
	{
		RootLayout->RemoveFromParent();
	}

	BattleHUDScreen = nullptr;
	BattleWidgetController = nullptr;
	BattleHUDViewModel = nullptr;
	OverlayScreenStack.Reset();
	ModalScreenStack.Reset();
	RewardOverlayScreen = nullptr;
	NodeOverlayScreen = nullptr;
	RewardNodeOverlayScreen = nullptr;
	EventNodeOverlayScreen = nullptr;
	ShopNodeOverlayScreen = nullptr;
	PlaceholderModalScreen = nullptr;
	RootLayout = nullptr;
	PrimaryPlayerController = nullptr;

	Super::Deinitialize();
}

void UFinalUISubsystem::RegisterPrimaryPlayerController(APlayerController* InPlayerController)
{
	if (InPlayerController == nullptr)
	{
		return;
	}

	PrimaryPlayerController = InPlayerController;
	EnsureRootLayout();
	EnsureBattleBridge();
	EnsureBattleHUD();
	EnsureFlowScreens();
	ApplyGameplayHudInputMode();

	if (UFinalRunFlowSubsystem* RunFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalRunFlowSubsystem>() : nullptr)
	{
		RunFlowSubsystem->RefreshRunFlow(true);
	}
}

void UFinalUISubsystem::EnsureBattleHUD()
{
	if (PrimaryPlayerController == nullptr)
	{
		return;
	}

	EnsureRootLayout();
	EnsureBattleBridge();

	if (RootLayout == nullptr)
	{
		return;
	}

	if (BattleHUDScreen == nullptr)
	{
		BattleHUDScreen = CreateWidget<UFinalBattleHUDScreen>(PrimaryPlayerController, UFinalBattleHUDScreen::StaticClass());
		if (BattleHUDScreen)
		{
			BattleHUDScreen->InitializeScreen(BattleHUDViewModel, BattleWidgetController);
		}
	}

	if (BattleHUDScreen)
	{
		RootLayout->SetPersistentHUD(BattleHUDScreen);
		BattleHUDScreen->SetVisibility(ESlateVisibility::Visible);
		RefreshBattleHUD();
	}
}

void UFinalUISubsystem::RefreshBattleHUD()
{
	if (BattleWidgetController == nullptr || GetGameInstance() == nullptr)
	{
		return;
	}

	if (UFinalBattleFlowSubsystem* BattleFlowSubsystem = GetGameInstance()->GetSubsystem<UFinalBattleFlowSubsystem>())
	{
		BattleWidgetController->BindToBattleFlow(BattleFlowSubsystem);
	}
}

void UFinalUISubsystem::SetBattleHUDVisibility(bool bVisible)
{
	if (BattleHUDScreen)
	{
		BattleHUDScreen->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UFinalUISubsystem::OpenOverlayScreen(UFinalScreenBase* Screen, const bool bReplaceExisting)
{
	if (Screen == nullptr)
	{
		return;
	}

	EnsureRootLayout();
	if (RootLayout == nullptr || Screen->GetScreenLayer() != EFinalUIScreenLayer::Overlay)
	{
		return;
	}

	OverlayScreenStack.Remove(Screen);
	if (bReplaceExisting)
	{
		for (UFinalScreenBase* ExistingScreen : OverlayScreenStack)
		{
			if (ExistingScreen)
			{
				ExistingScreen->HandleScreenClosed();
			}
		}
		OverlayScreenStack.Reset();
	}

	OverlayScreenStack.Add(Screen);
	Screen->HandleScreenOpened();
	RebuildScreenLayer(EFinalUIScreenLayer::Overlay);
	ApplyTopScreenInputMode();
}

void UFinalUISubsystem::CloseOverlayScreen(UFinalScreenBase* Screen)
{
	if (OverlayScreenStack.Num() == 0)
	{
		return;
	}

	UFinalScreenBase* ScreenToClose = Screen;
	if (ScreenToClose == nullptr)
	{
		ScreenToClose = OverlayScreenStack.Last().Get();
	}
	if (ScreenToClose == nullptr)
	{
		return;
	}

	if (OverlayScreenStack.Remove(ScreenToClose) > 0)
	{
		ScreenToClose->HandleScreenClosed();
		RebuildScreenLayer(EFinalUIScreenLayer::Overlay);
		ApplyTopScreenInputMode();
	}
}

void UFinalUISubsystem::OpenModalScreen(UFinalScreenBase* Screen, const bool bReplaceExisting)
{
	if (Screen == nullptr)
	{
		return;
	}

	EnsureRootLayout();
	if (RootLayout == nullptr || Screen->GetScreenLayer() != EFinalUIScreenLayer::Modal)
	{
		return;
	}

	ModalScreenStack.Remove(Screen);
	if (bReplaceExisting)
	{
		for (UFinalScreenBase* ExistingScreen : ModalScreenStack)
		{
			if (ExistingScreen)
			{
				ExistingScreen->HandleScreenClosed();
			}
		}
		ModalScreenStack.Reset();
	}

	ModalScreenStack.Add(Screen);
	Screen->HandleScreenOpened();
	RebuildScreenLayer(EFinalUIScreenLayer::Modal);
	ApplyTopScreenInputMode();
}

void UFinalUISubsystem::CloseModalScreen(UFinalScreenBase* Screen)
{
	if (ModalScreenStack.Num() == 0)
	{
		return;
	}

	UFinalScreenBase* ScreenToClose = Screen;
	if (ScreenToClose == nullptr)
	{
		ScreenToClose = ModalScreenStack.Last().Get();
	}
	if (ScreenToClose == nullptr)
	{
		return;
	}

	if (ModalScreenStack.Remove(ScreenToClose) > 0)
	{
		ScreenToClose->HandleScreenClosed();
		RebuildScreenLayer(EFinalUIScreenLayer::Modal);
		ApplyTopScreenInputMode();
	}
}

void UFinalUISubsystem::ShowBattleRewardOverlayPlaceholder()
{
	EnsureFlowScreens();
	ConfigureAndOpenRunOverlay(RewardOverlayScreen);
}

void UFinalUISubsystem::ShowNodeProgressOverlayPlaceholder()
{
	ShowNodeSelectOverlayPlaceholder();
}

void UFinalUISubsystem::ShowNodeSelectOverlayPlaceholder()
{
	EnsureFlowScreens();
	ConfigureAndOpenRunOverlay(NodeOverlayScreen);
}

void UFinalUISubsystem::ShowRewardNodeOverlayPlaceholder()
{
	EnsureFlowScreens();
	ConfigureAndOpenRunOverlay(RewardNodeOverlayScreen);
}

void UFinalUISubsystem::ShowEventNodeOverlayPlaceholder()
{
	EnsureFlowScreens();
	ConfigureAndOpenRunOverlay(EventNodeOverlayScreen);
}

void UFinalUISubsystem::ShowShopNodeOverlayPlaceholder()
{
	EnsureFlowScreens();
	ConfigureAndOpenRunOverlay(ShopNodeOverlayScreen);
}

void UFinalUISubsystem::ShowPlaceholderModal(const FText& Title, const FText& Body)
{
	EnsureFlowScreens();
	if (PlaceholderModalScreen == nullptr)
	{
		return;
	}

	PlaceholderModalScreen->ConfigureModal(Title, Body);
	OpenModalScreen(PlaceholderModalScreen, true);
}

UFinalScreenBase* UFinalUISubsystem::GetActiveOverlayScreen() const
{
	return OverlayScreenStack.Num() > 0 ? OverlayScreenStack.Last() : nullptr;
}

UFinalScreenBase* UFinalUISubsystem::GetActiveModalScreen() const
{
	return ModalScreenStack.Num() > 0 ? ModalScreenStack.Last() : nullptr;
}

UFinalUIRootLayout* UFinalUISubsystem::GetRootLayout() const
{
	return RootLayout;
}

UFinalBattleHUDScreen* UFinalUISubsystem::GetBattleHUDScreen() const
{
	return BattleHUDScreen;
}

UFinalBattleHUDViewModel* UFinalUISubsystem::GetBattleHUDViewModel() const
{
	return BattleHUDViewModel;
}

UFinalBattleWidgetController* UFinalUISubsystem::GetBattleWidgetController() const
{
	return BattleWidgetController;
}

void UFinalUISubsystem::EnsureRootLayout()
{
	if (PrimaryPlayerController == nullptr || RootLayout != nullptr)
	{
		return;
	}

	RootLayout = CreateWidget<UFinalUIRootLayout>(PrimaryPlayerController, UFinalUIRootLayout::StaticClass());
	if (RootLayout)
	{
		RootLayout->AddToViewport(0);
	}
}

void UFinalUISubsystem::EnsureBattleBridge()
{
	if (BattleHUDViewModel == nullptr)
	{
		BattleHUDViewModel = NewObject<UFinalBattleHUDViewModel>(this);
	}

	if (BattleWidgetController == nullptr)
	{
		BattleWidgetController = NewObject<UFinalBattleWidgetController>(this);
		BattleWidgetController->Initialize(BattleHUDViewModel);
	}

	if (GetGameInstance())
	{
		if (UFinalBattleFlowSubsystem* BattleFlowSubsystem = GetGameInstance()->GetSubsystem<UFinalBattleFlowSubsystem>())
		{
			BattleWidgetController->BindToBattleFlow(BattleFlowSubsystem);
		}
	}
}

void UFinalUISubsystem::EnsureFlowScreens()
{
	if (PrimaryPlayerController == nullptr)
	{
		return;
	}

	if (RewardOverlayScreen == nullptr)
	{
		RewardOverlayScreen = CreateWidget<UFinalRunRewardOverlayScreen>(PrimaryPlayerController, UFinalRunRewardOverlayScreen::StaticClass());
	}

	if (NodeOverlayScreen == nullptr)
	{
		NodeOverlayScreen = CreateWidget<UFinalRunNodeOverlayScreen>(PrimaryPlayerController, UFinalRunNodeOverlayScreen::StaticClass());
	}

	if (RewardNodeOverlayScreen == nullptr)
	{
		RewardNodeOverlayScreen = CreateWidget<UFinalRunRewardNodeOverlayScreen>(PrimaryPlayerController, UFinalRunRewardNodeOverlayScreen::StaticClass());
	}

	if (EventNodeOverlayScreen == nullptr)
	{
		EventNodeOverlayScreen = CreateWidget<UFinalRunEventNodeOverlayScreen>(PrimaryPlayerController, UFinalRunEventNodeOverlayScreen::StaticClass());
	}

	if (ShopNodeOverlayScreen == nullptr)
	{
		ShopNodeOverlayScreen = CreateWidget<UFinalRunShopNodeOverlayScreen>(PrimaryPlayerController, UFinalRunShopNodeOverlayScreen::StaticClass());
	}

	if (PlaceholderModalScreen == nullptr)
	{
		PlaceholderModalScreen = CreateWidget<UFinalPlaceholderModalScreen>(PrimaryPlayerController, UFinalPlaceholderModalScreen::StaticClass());
	}
}

FFinalRunSnapshot UFinalUISubsystem::GetCurrentRunSnapshot() const
{
	if (const UFinalGameFlowSubsystem* GameFlowSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalGameFlowSubsystem>() : nullptr)
	{
		if (const UFinalRunSession* RunSession = GameFlowSubsystem->GetRunSession())
		{
			return RunSession->GetSnapshot();
		}
	}

	return FFinalRunSnapshot{};
}

void UFinalUISubsystem::ConfigureAndOpenRunOverlay(UFinalRunStageOverlayScreenBase* Screen)
{
	if (Screen == nullptr)
	{
		return;
	}

	Screen->ConfigureFromRunSnapshot(GetCurrentRunSnapshot());
	OpenOverlayScreen(Screen, true);
}

void UFinalUISubsystem::RebuildScreenLayer(const EFinalUIScreenLayer Layer)
{
	if (RootLayout == nullptr)
	{
		return;
	}

	RootLayout->ClearLayer(Layer);

	UFinalScreenBase* ActiveScreen = nullptr;
	switch (Layer)
	{
	case EFinalUIScreenLayer::Overlay:
		ActiveScreen = GetActiveOverlayScreen();
		break;

	case EFinalUIScreenLayer::Modal:
		ActiveScreen = GetActiveModalScreen();
		break;

	default:
		break;
	}

	if (ActiveScreen)
	{
		RootLayout->AddScreenToLayer(ActiveScreen, Layer);
		ActiveScreen->SetVisibility(ESlateVisibility::Visible);
	}
}

void UFinalUISubsystem::ApplyInputConfig(const FFinalUIInputConfig& InputConfig, UFinalScreenBase* FocusScreen) const
{
	if (PrimaryPlayerController == nullptr)
	{
		return;
	}

	switch (InputConfig.InputMode)
	{
	case EFinalUIInputMode::GameOnly:
		UWidgetBlueprintLibrary::SetInputMode_GameOnly(PrimaryPlayerController);
		break;

	case EFinalUIInputMode::UIOnly:
		UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(PrimaryPlayerController, FocusScreen, EMouseLockMode::DoNotLock, InputConfig.bHideCursorDuringCapture);
		break;

	case EFinalUIInputMode::GameAndUI:
	default:
		UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(PrimaryPlayerController, FocusScreen, EMouseLockMode::DoNotLock, InputConfig.bHideCursorDuringCapture, false);
		break;
	}

	PrimaryPlayerController->SetShowMouseCursor(InputConfig.bShowMouseCursor);
}

void UFinalUISubsystem::ApplyTopScreenInputMode() const
{
	if (UFinalScreenBase* ActiveModalScreen = GetActiveModalScreen())
	{
		ApplyInputConfig(ActiveModalScreen->GetDesiredInputConfig(), ActiveModalScreen);
		return;
	}

	if (UFinalScreenBase* ActiveOverlayScreen = GetActiveOverlayScreen())
	{
		ApplyInputConfig(ActiveOverlayScreen->GetDesiredInputConfig(), ActiveOverlayScreen);
		return;
	}

	ApplyGameplayHudInputMode();
}

void UFinalUISubsystem::ApplyGameplayHudInputMode() const
{
	if (PrimaryPlayerController == nullptr)
	{
		return;
	}

	UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(PrimaryPlayerController, nullptr, EMouseLockMode::DoNotLock, false, false);
	PrimaryPlayerController->SetShowMouseCursor(true);
}
