#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FinalWidgetControllerBase.generated.h"

UCLASS(Abstract, BlueprintType)
class FINALAPP_API UFinalWidgetControllerBase : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	virtual void ShutdownController();
};
