#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalBattleCharacterEntryWidget.generated.h"

class UBorder;
class UProgressBar;
class UTextBlock;
class UVerticalBox;
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
	float CachedBreakthroughFill = 0.0f;
	bool bCachedBreakthroughReady = false;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UBorder> RootBorder;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> LabelText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UProgressBar> BreakthroughProgressBar;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> ContentBox;
};
