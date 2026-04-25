#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalBattleCharacterEntryWidget.generated.h"

class UBorder;
class UTextBlock;
struct FFinalBattleHUDCharacterEntry;

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleCharacterEntryWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	void Configure(const FFinalBattleHUDCharacterEntry& InEntry);

private:
	void RebuildVisual();

	FText CachedLabel;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UBorder> RootBorder;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> LabelText;
};
