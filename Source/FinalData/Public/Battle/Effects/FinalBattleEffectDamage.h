#pragma once

#include "CoreMinimal.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "Types/FinalCoreTypes.h"
#include "FinalBattleEffectDamage.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleEffectDamage : public UFinalBattleEffectDefinition
{
	GENERATED_BODY()

public:
	UFinalBattleEffectDamage();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalBattleScalarValue Scalar;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (ClampMin = "1"))
	int32 HitCount = 1;
};
