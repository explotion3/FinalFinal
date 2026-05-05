#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModels/Battle/FinalBattleHUDTypes.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalBattleCardZoneEntryWidget.generated.h"

class URichTextBlock;
class UTextBlock;

UCLASS(BlueprintType, Blueprintable)
class FINALAPP_API UFinalBattleCardZoneEntryWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable, Category = "Final|Battle HUD|Card Zone")
	void ApplyCardZoneEntryView(const FFinalBattleHUDCardZoneEntry& ViewData);

	UFUNCTION(BlueprintPure, Category = "Final|Battle HUD|Card Zone")
	const FFinalBattleHUDCardZoneEntry& GetCardZoneEntryViewData() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Final|Battle HUD|Card Zone")
	void OnCardZoneEntryViewApplied(const FFinalBattleHUDCardZoneEntry& ViewData);

private:
	void RefreshBoundWidgets();

	UPROPERTY(Transient)
	FFinalBattleHUDCardZoneEntry CachedViewData;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> TypeText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> CostText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> OwnerText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> KeywordText;

	UPROPERTY(Transient, meta=(BindWidgetOptional))
	TObjectPtr<URichTextBlock> RulesText;
};
