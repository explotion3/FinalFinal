#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalBattleEnemyEntryWidget.generated.h"

class UButton;
class UTextBlock;
class UFinalBattleHUDScreen;
struct FFinalBattleHUDEnemyEntry;

UCLASS()
class FINALAPP_API UFinalBattleEnemyEntryWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	void Configure(UFinalBattleHUDScreen* InOwningScreen, const FFinalBattleHUDEnemyEntry& InEntry);

private:
	UFUNCTION()
	void HandleButtonClicked();

	void RebuildVisual();

	TWeakObjectPtr<UFinalBattleHUDScreen> OwningBattleHUDScreen;

	FName RuntimeUnitId = NAME_None;
	FText CachedLabel;
	bool bSelected = false;

	UPROPERTY(Transient)
	TObjectPtr<UButton> SelectButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> LabelText;
};
