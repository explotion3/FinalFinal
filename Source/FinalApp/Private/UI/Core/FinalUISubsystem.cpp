#include "Subsystems/UI/FinalUISubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Controllers/FinalBattleWidgetController.h"
#include "Subsystems/FinalBattleFlowSubsystem.h"
#include "UI/Root/FinalUIRootLayout.h"
#include "UI/Screens/Battle/FinalBattleHUDScreen.h"
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
	ApplyGameplayHudInputMode();
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

void UFinalUISubsystem::ApplyGameplayHudInputMode() const
{
	if (PrimaryPlayerController == nullptr)
	{
		return;
	}

	UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(PrimaryPlayerController, nullptr, EMouseLockMode::DoNotLock, false, false);
	PrimaryPlayerController->SetShowMouseCursor(true);
}
