#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FinalUISubsystem.generated.h"

class APlayerController;
class UFinalBattleHUDScreen;
class UFinalBattleHUDViewModel;
class UFinalBattleWidgetController;
class UFinalUIRootLayout;

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

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalUIRootLayout* GetRootLayout() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleHUDScreen* GetBattleHUDScreen() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleHUDViewModel* GetBattleHUDViewModel() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleWidgetController* GetBattleWidgetController() const;

private:
	void EnsureRootLayout();
	void EnsureBattleBridge();
	void ApplyGameplayHudInputMode() const;

	UPROPERTY(Transient)
	TObjectPtr<APlayerController> PrimaryPlayerController;

	UPROPERTY(Transient)
	TObjectPtr<UFinalUIRootLayout> RootLayout;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleHUDScreen> BattleHUDScreen;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleHUDViewModel> BattleHUDViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleWidgetController> BattleWidgetController;
};
