#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalBattleLogEntryWidget.generated.h"

class UBorder;
class UTextBlock;
struct FFinalBattleHUDLogEntry;

UCLASS()
class FINALAPP_API UFinalBattleLogEntryWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	void Configure(const FFinalBattleHUDLogEntry& InEntry);

private:
	void RebuildVisual();

	FText CachedLabel;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> RootBorder;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> LabelText;
};
