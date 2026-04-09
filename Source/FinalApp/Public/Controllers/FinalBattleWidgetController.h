#pragma once

#include "CoreMinimal.h"
#include "Facade/FinalBattleSession.h"
#include "ViewModels/FinalBattleHUDViewModel.h"
#include "UObject/Object.h"
#include "FinalBattleWidgetController.generated.h"

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleWidgetController : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void Initialize(UFinalBattleHUDViewModel* InViewModel);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void RefreshFromSession(UFinalBattleSession* Session);

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalBattleHUDViewModel* GetViewModel() const;

private:
	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleHUDViewModel> ViewModel;
};
