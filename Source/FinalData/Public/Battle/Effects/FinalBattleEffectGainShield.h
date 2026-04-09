#pragma once

#include "CoreMinimal.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "Types/FinalCoreTypes.h"
#include "FinalBattleEffectGainShield.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleEffectGainShield : public UFinalBattleEffectDefinition
{
	GENERATED_BODY()

public:
	UFinalBattleEffectGainShield();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalBattleScalarValue Scalar;
};
