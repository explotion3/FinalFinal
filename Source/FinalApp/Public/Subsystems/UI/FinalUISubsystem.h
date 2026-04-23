#pragma once

#include "CoreMinimal.h"
#include "Queries/FinalRunSnapshot.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UI/Core/FinalUITypes.h"
#include "FinalUISubsystem.generated.h"

class APlayerController;
class UFinalBattleEventScreen;
class UFinalBattleHUDScreen;
class UFinalBattleHUDViewModel;
class UFinalBattleWidgetController;
class UFinalPrototypeRunDebugScreen;
class UFinalRunStageOverlayScreenBase;
class UFinalPlaceholderModalScreen;
class UFinalRunEventNodeOverlayScreen;
class UFinalRunNodeOverlayScreen;
class UFinalRunRewardNodeOverlayScreen;
class UFinalRunRewardOverlayScreen;
class UFinalRunShopNodeOverlayScreen;
class UFinalUIRootLayout;
class UFinalScreenBase;

UCLASS()
class FINALAPP_API UFinalUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void RegisterPrimaryPlayerController(APlayerController* InPlayerController);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void EnsureBattleHUD();

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void RefreshBattleHUD();

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void SetBattleHUDVisibility(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void OpenPrototypeRunDebugOverlay();

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void OpenBattleEventOverlay();

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void OpenOverlayScreen(UFinalScreenBase* Screen, bool bReplaceExisting = true);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void CloseOverlayScreen(UFinalScreenBase* Screen = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void OpenModalScreen(UFinalScreenBase* Screen, bool bReplaceExisting = true);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void CloseModalScreen(UFinalScreenBase* Screen = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void ShowBattleRewardOverlayPlaceholder();

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void ShowNodeProgressOverlayPlaceholder();

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void ShowNodeSelectOverlayPlaceholder();

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void ShowRewardNodeOverlayPlaceholder();

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void ShowEventNodeOverlayPlaceholder();

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void ShowShopNodeOverlayPlaceholder();

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void ShowPlaceholderModal(const FText& Title, const FText& Body);

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalScreenBase* GetActiveOverlayScreen() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalScreenBase* GetActiveModalScreen() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalUIRootLayout* GetRootLayout() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleHUDScreen* GetBattleHUDScreen() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalPrototypeRunDebugScreen* GetPrototypeRunDebugScreen() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleEventScreen* GetBattleEventScreen() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleHUDViewModel* GetBattleHUDViewModel() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleWidgetController* GetBattleWidgetController() const;

private:
	void EnsureRootLayout();
	void EnsureBattleBridge();
	void EnsurePrototypeDebugScreen();
	void EnsureBattleEventScreen();
	void EnsureFlowScreens();
	FFinalRunSnapshot GetCurrentRunSnapshot() const;
	void ConfigureAndOpenRunOverlay(UFinalRunStageOverlayScreenBase* Screen);
	void RebuildPersistentHUDLayer();
	void RebuildScreenLayer(EFinalUIScreenLayer Layer);
	void ApplyInputConfig(const FFinalUIInputConfig& InputConfig, UFinalScreenBase* FocusScreen) const;
	void ApplyTopScreenInputMode() const;
	void ApplyGameplayHudInputMode() const;

	UPROPERTY(Transient)
	TObjectPtr<APlayerController> PrimaryPlayerController;

	UPROPERTY(Transient)
	TObjectPtr<UFinalUIRootLayout> RootLayout;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleHUDScreen> BattleHUDScreen;

	UPROPERTY(Transient)
	TObjectPtr<UFinalPrototypeRunDebugScreen> PrototypeRunDebugScreen;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleEventScreen> BattleEventScreen;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleHUDViewModel> BattleHUDViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleWidgetController> BattleWidgetController;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFinalScreenBase>> OverlayScreenStack;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFinalScreenBase>> ModalScreenStack;

	UPROPERTY(Transient)
	TObjectPtr<UFinalRunRewardOverlayScreen> RewardOverlayScreen;

	UPROPERTY(Transient)
	TObjectPtr<UFinalRunNodeOverlayScreen> NodeOverlayScreen;

	UPROPERTY(Transient)
	TObjectPtr<UFinalRunRewardNodeOverlayScreen> RewardNodeOverlayScreen;

	UPROPERTY(Transient)
	TObjectPtr<UFinalRunEventNodeOverlayScreen> EventNodeOverlayScreen;

	UPROPERTY(Transient)
	TObjectPtr<UFinalRunShopNodeOverlayScreen> ShopNodeOverlayScreen;

	UPROPERTY(Transient)
	TObjectPtr<UFinalPlaceholderModalScreen> PlaceholderModalScreen;
};
