#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Core/FinalUITypes.h"
#include "FinalUIRootLayout.generated.h"

class UCanvasPanel;
class UOverlay;
class UFinalScreenBase;

UCLASS(BlueprintType)
class FINALAPP_API UFinalUIRootLayout : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void SetPersistentHUD(UFinalScreenBase* InScreen);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void ClearLayer(EFinalUIScreenLayer Layer);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void AddScreenToLayer(UFinalScreenBase* Screen, EFinalUIScreenLayer Layer);

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	UOverlay* GetLayerWidget(EFinalUIScreenLayer Layer) const;

private:
	void EnsureLayoutTree();
	bool HasBoundLayerWidgets() const;
	UOverlay* ResolveLayer(EFinalUIScreenLayer Layer) const;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> HUDLayer;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> OverlayLayer;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> ModalLayer;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> TooltipLayer;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UOverlay> ToastLayer;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> RootCanvas;

	UPROPERTY(Transient)
	TObjectPtr<UOverlay> HUDLayerWidget;

	UPROPERTY(Transient)
	TObjectPtr<UOverlay> OverlayLayerWidget;

	UPROPERTY(Transient)
	TObjectPtr<UOverlay> ModalLayerWidget;

	UPROPERTY(Transient)
	TObjectPtr<UOverlay> TooltipLayerWidget;

	UPROPERTY(Transient)
	TObjectPtr<UOverlay> ToastLayerWidget;
};
