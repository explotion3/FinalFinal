#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalBattleUltimateEntryWidget.generated.h"

class UButton;
class UTextBlock;
class UFinalBattleUltimatePanelController;

UCLASS()
class FINALAPP_API UFinalBattleUltimateEntryWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	void Configure(UFinalBattleUltimatePanelController* InController, int32 InCharacterIndex, const struct FFinalBattleHUDUltimateEntry& InEntry);

private:
	UFUNCTION()
	void HandleButtonClicked();

	void RebuildVisual();

	TWeakObjectPtr<UFinalBattleUltimatePanelController> PanelController;

	int32 CharacterIndex = INDEX_NONE;
	FText CachedLabel;
	bool bEnabled = false;
	bool bBlockedByCollapse = false;
	bool bDefinitionReady = false;
	bool bUsedThisBattle = false;

	UPROPERTY(Transient)
	TObjectPtr<UButton> UltimateButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> LabelText;
};
