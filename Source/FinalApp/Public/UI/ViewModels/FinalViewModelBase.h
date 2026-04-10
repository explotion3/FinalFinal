#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FinalViewModelBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFinalViewModelChangedSignature);

UCLASS(Abstract, BlueprintType)
class FINALAPP_API UFinalViewModelBase : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Final|UI")
	FFinalViewModelChangedSignature OnViewModelChanged;

protected:
	void BroadcastViewModelChanged();
};
