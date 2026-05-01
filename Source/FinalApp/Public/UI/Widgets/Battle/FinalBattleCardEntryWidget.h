#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalBattleCardEntryWidget.generated.h"

class UButton;
class URichTextBlock;
class UTextBlock;
class UFinalBattleHandPanelController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FFinalBattleCardHoverChangedSignature, FGuid, CardInstanceId, int32, HandIndex, bool, bHovered);

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleCardEntryWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	void Configure(UFinalBattleHandPanelController* InController, int32 InHandIndex, const struct FFinalBattleHUDCardEntry& InEntry);

	UPROPERTY(BlueprintAssignable, Category = "Final|Battle HUD|Card")
	FFinalBattleCardHoverChangedSignature OnCardHoverChanged;

private:
	UFUNCTION()
	void HandleButtonClicked();

	UFUNCTION()
	void HandleButtonHovered();

	UFUNCTION()
	void HandleButtonUnhovered();

	void RebuildVisual();

	TWeakObjectPtr<UFinalBattleHandPanelController> PanelController;

	FGuid CardInstanceId;
	int32 HandIndex = INDEX_NONE;
	FText CachedCostText;
	FText CachedNameText;
	FText CachedTypeText;
	FText CachedDescriptionText;
	FSlateColor CachedCostColor;
	bool bUsesFallbackLayout = false;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UButton> CardButton;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> CostText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> TypeText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<URichTextBlock> DescriptionText;
};
