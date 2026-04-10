#pragma once

#include "CoreMinimal.h"
#include "FinalUITypes.generated.h"

UENUM(BlueprintType)
enum class EFinalUIScreenLayer : uint8
{
	HUD,
	Overlay,
	Modal,
	Tooltip,
	Toast
};

UENUM(BlueprintType)
enum class EFinalUIInputMode : uint8
{
	GameOnly,
	GameAndUI,
	UIOnly
};

USTRUCT(BlueprintType)
struct FINALAPP_API FFinalUIInputConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	EFinalUIInputMode InputMode = EFinalUIInputMode::GameAndUI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bShowMouseCursor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|UI")
	bool bHideCursorDuringCapture = false;
};
