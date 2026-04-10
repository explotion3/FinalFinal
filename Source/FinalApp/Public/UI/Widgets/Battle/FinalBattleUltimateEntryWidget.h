#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalBattleUltimateEntryWidget.generated.h"

class UButton;
class UTextBlock;
class UFinalBattleHUDScreen;
struct FFinalBattleHUDUltimateEntry;

UCLASS()
class FINALAPP_API UFinalBattleUltimateEntryWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	void Configure(UFinalBattleHUDScreen* InOwningScreen, int32 InCharacterIndex, const FFinalBattleHUDUltimateEntry& InEntry);

private:
	UFUNCTION()
	void HandleButtonClicked();

	void RebuildVisual();

	TWeakObjectPtr<UFinalBattleHUDScreen> OwningBattleHUDScreen;

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
