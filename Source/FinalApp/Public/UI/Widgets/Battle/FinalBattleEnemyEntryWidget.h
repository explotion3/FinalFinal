#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalBattleEnemyEntryWidget.generated.h"

class UButton;
class UTextBlock;
class UFinalBattleEnemyPanelController;

UCLASS()
class FINALAPP_API UFinalBattleEnemyEntryWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	void Configure(UFinalBattleEnemyPanelController* InController, const struct FFinalBattleHUDEnemyEntry& InEntry);

private:
	UFUNCTION()
	void HandleButtonClicked();

	void RebuildVisual();

	TWeakObjectPtr<UFinalBattleEnemyPanelController> PanelController;

	FName RuntimeUnitId = NAME_None;
	FText CachedLabel;
	bool bSelected = false;

	UPROPERTY(Transient)
	TObjectPtr<UButton> SelectButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> LabelText;
};
