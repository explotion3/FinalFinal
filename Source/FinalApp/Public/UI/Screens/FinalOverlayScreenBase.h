#pragma once

#include "CoreMinimal.h"
#include "UI/Screens/FinalScreenBase.h"
#include "FinalOverlayScreenBase.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class FINALAPP_API UFinalOverlayScreenBase : public UFinalScreenBase
{
	GENERATED_BODY()

public:
	UFinalOverlayScreenBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
