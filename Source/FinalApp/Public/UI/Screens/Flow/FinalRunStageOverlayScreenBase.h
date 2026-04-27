#pragma once

#include "CoreMinimal.h"
#include "Queries/FinalRunSnapshot.h"
#include "UI/Screens/FinalOverlayScreenBase.h"
#include "FinalRunStageOverlayScreenBase.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;

UCLASS(Abstract)
class FINALAPP_API UFinalRunStageOverlayScreenBase : public UFinalOverlayScreenBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	virtual void ConfigureFromRunSnapshot(const FFinalRunSnapshot& InSnapshot);

protected:
	void EnsureBaseWidgetTree(const FLinearColor& RootTint, const TCHAR* RootName, const TCHAR* ContentName);
	UTextBlock* CreateStageLabel(const TCHAR* Name, int32 FontSize) const;
	UButton* CreateStageButton(const TCHAR* ButtonName, const TCHAR* LabelName, const FText& LabelText, TObjectPtr<UTextBlock>& OutLabelText);
	FText BuildFeedbackText(const FText& DefaultText) const;
	void SetLastActionFeedback(const FText& InFeedbackText);
	class UFinalRunFlowSubsystem* ResolveRunFlowSubsystem() const;
	class UFinalUISubsystem* ResolveUISubsystem() const;

	const FFinalRunSnapshot& GetCachedSnapshot() const
	{
		return CachedSnapshot;
	}

	UPROPERTY(Transient)
	FFinalRunSnapshot CachedSnapshot;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> ContentBox;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SummaryText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GapText;

	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> FeedbackText;

	UPROPERTY(Transient)
	FText LastActionFeedback;
};
