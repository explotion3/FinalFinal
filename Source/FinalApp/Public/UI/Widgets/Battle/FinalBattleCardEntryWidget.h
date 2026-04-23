#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalBattleCardEntryWidget.generated.h"

class UButton;
class UTextBlock;
class UFinalBattleHandPanelController;

UCLASS()
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
	FText CachedLabel;

	UPROPERTY(Transient)
	TObjectPtr<UButton> CardButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> LabelText;
};
