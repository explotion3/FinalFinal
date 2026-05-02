#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "FinalBattleOverheadWidgetComponent.generated.h"

UCLASS(ClassGroup = (Final), meta = (BlueprintSpawnableComponent))
class FINALAPP_API UFinalBattleOverheadWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UFinalBattleOverheadWidgetComponent();
};
