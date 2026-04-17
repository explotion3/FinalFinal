#pragma once

#include "Commandlets/Commandlet.h"
#include "FinalPrototypeContentBootstrapCommandlet.generated.h"

UCLASS()
class FINALEDITOR_API UFinalPrototypeContentBootstrapCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UFinalPrototypeContentBootstrapCommandlet();

	virtual int32 Main(const FString& Params) override;
};
