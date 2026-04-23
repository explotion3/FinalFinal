#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalBattleCharacterEntryWidget.generated.h"

class UBorder;
class UTextBlock;
struct FFinalBattleHUDCharacterEntry;

UCLASS()
class FINALAPP_API UFinalBattleCharacterEntryWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	void Configure(const FFinalBattleHUDCharacterEntry& InEntry);

private:
	void RebuildVisual();

	FText CachedLabel;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> RootBorder;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> LabelText;
};
