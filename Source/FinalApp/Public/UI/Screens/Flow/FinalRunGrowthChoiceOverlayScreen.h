#pragma once

#include "CoreMinimal.h"
#include "UI/Screens/Flow/FinalRunStageOverlayScreenBase.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalRunGrowthChoiceOverlayScreen.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UVerticalBox;
class UWidget;

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalRunGrowthChoiceEntryViewData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunGrowth")
	FName ChoiceInstanceId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunGrowth")
	int32 ChoiceIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunGrowth")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunGrowth")
	FText ChoiceTypeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunGrowth")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunGrowth")
	FText DetailText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunGrowth")
	FText StateText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunGrowth")
	FName IconId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunGrowth")
	EFinalRunRewardVisualTier VisualTier = EFinalRunRewardVisualTier::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|RunGrowth")
	bool bEnabled = false;
};

class UFinalRunGrowthChoiceEntryWidget;
DECLARE_MULTICAST_DELEGATE_OneParam(FFinalRunGrowthChoiceClickedNative, UFinalRunGrowthChoiceEntryWidget*);

UCLASS()
class FINALAPP_API UFinalRunGrowthChoiceEntryWidget : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	void ApplyChoiceView(const FFinalRunGrowthChoiceEntryViewData& InViewData);
	const FFinalRunGrowthChoiceEntryViewData& GetChoiceViewData() const { return CachedViewData; }

	FFinalRunGrowthChoiceClickedNative OnChoiceClicked;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Final|RunGrowth")
	void OnChoiceViewApplied(const FFinalRunGrowthChoiceEntryViewData& ViewData);

private:
	UFUNCTION()
	void HandleClicked();

	void EnsureWidgetTree();
	void RefreshBoundWidgets();

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> ChoiceButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TypeText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DescriptionText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DetailText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StateText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> TierVisual;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> SelectedVisual;

	FFinalRunGrowthChoiceEntryViewData CachedViewData;
};

UCLASS()
class FINALAPP_API UFinalRunGrowthChoiceOverlayScreen : public UFinalRunStageOverlayScreenBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot) override;

	UFUNCTION(BlueprintCallable, Category = "Final|RunFlow")
	bool SelectChoiceByIndex(int32 ChoiceIndex);

	UFUNCTION(BlueprintCallable, Category = "Final|RunFlow")
	bool ConfirmCurrentChoice();

	UFUNCTION(BlueprintCallable, Category = "Final|RunFlow")
	bool SubmitChoiceByInstanceId(FName ChoiceInstanceId);

	UFUNCTION(BlueprintPure, Category = "Final|RunFlow")
	int32 GetSelectedChoiceIndex() const;

	UFUNCTION(BlueprintPure, Category = "Final|RunFlow")
	FName GetSelectedChoiceInstanceId() const;

private:
	UFUNCTION()
	void HandlePrimaryActionClicked();

	UFUNCTION()
	void HandleCloseClicked();

	void EnsureWidgetTree();
	void RebuildVisual();
	void RebuildChoiceList();
	void ClearChoiceList();
	void ClampSelectionIndex();
	void HandleGrowthChoiceEntryClicked(UFinalRunGrowthChoiceEntryWidget* ChoiceEntry);

	const FFinalRunGrowthChoiceInstance* GetSelectedChoice() const;
	const FFinalRunGrowthChoiceInstance* FindChoiceByInstanceId(FName ChoiceInstanceId) const;
	const FFinalRunCharacterViewData* GetTargetCharacter() const;
	FFinalRunGrowthChoiceEntryViewData BuildChoiceEntryData(int32 ChoiceIndex) const;
	FText BuildCharacterSummaryText() const;
	FText BuildSelectionSummaryText() const;
	FText BuildPrimaryActionText() const;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CharacterSummaryText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SelectionSummaryText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> GrowthChoiceListBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> PrimaryActionButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PrimaryActionButtonText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CloseButtonText;

	UPROPERTY(Transient)
	int32 SelectedChoiceIndex = INDEX_NONE;
};
