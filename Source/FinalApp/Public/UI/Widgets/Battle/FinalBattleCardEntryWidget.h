#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalBattleCardEntryWidget.generated.h"

class UButton;
class URichTextBlock;
class UTextBlock;
class UFinalBattleHandPanelController;

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleCardEntryWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	void Configure(UFinalBattleHandPanelController* InController, int32 InHandIndex, const struct FFinalBattleHUDCardEntry& InEntry);

private:
	UFUNCTION()
	void HandleButtonClicked();

	void RebuildVisual();

	TWeakObjectPtr<UFinalBattleHandPanelController> PanelController;

	int32 HandIndex = INDEX_NONE;
	FText CachedCostText;
	FText CachedNameText;
	FText CachedTypeText;
	FText CachedDescriptionText;
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
