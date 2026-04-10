#pragma once

#include "CoreMinimal.h"
#include "UI/Core/FinalUITypes.h"
#include "UI/Widgets/FinalWidgetBase.h"
#include "FinalScreenBase.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class FINALAPP_API UFinalScreenBase : public UFinalWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Final|UI")
	EFinalUIScreenLayer GetScreenLayer() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|UI")
	EFinalUIScreenLayer ScreenLayer = EFinalUIScreenLayer::HUD;
};
