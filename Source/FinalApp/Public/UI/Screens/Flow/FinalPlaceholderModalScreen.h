#pragma once

#include "CoreMinimal.h"
#include "UI/Screens/FinalModalScreenBase.h"
#include "FinalPlaceholderModalScreen.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class FINALAPP_API UFinalPlaceholderModalScreen : public UFinalModalScreenBase
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void ConfigureModal(const FText& InTitle, const FText& InBody);

private:
	UFUNCTION()
	void HandleCloseClicked();

	void EnsureWidgetTree();
	void RebuildVisual();

	UPROPERTY(Transient)
	FText CachedTitle;

	UPROPERTY(Transient)
	FText CachedBody;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> BodyText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CloseButtonText;
};
