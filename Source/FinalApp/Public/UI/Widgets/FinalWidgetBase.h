#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FinalWidgetBase.generated.h"

class UFinalViewModelBase;
class UFinalWidgetControllerBase;

UCLASS(Abstract, BlueprintType, Blueprintable)
class FINALAPP_API UFinalWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	virtual void SetPresentationContext(UFinalWidgetControllerBase* InWidgetController, UFinalViewModelBase* InViewModel);

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalWidgetControllerBase* GetWidgetController() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UFinalViewModelBase* GetViewModel() const;

protected:
	UPROPERTY(Transient)
	TObjectPtr<UFinalWidgetControllerBase> WidgetController;

	UPROPERTY(Transient)
	TObjectPtr<UFinalViewModelBase> ViewModel;
};
