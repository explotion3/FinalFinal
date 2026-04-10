#pragma once

#include "CoreMinimal.h"
#include "UI/Screens/FinalScreenBase.h"
#include "FinalModalScreenBase.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class FINALAPP_API UFinalModalScreenBase : public UFinalScreenBase
{
	GENERATED_BODY()

public:
	UFinalModalScreenBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
