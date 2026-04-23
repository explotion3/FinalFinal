#pragma once

#include "CoreMinimal.h"
#include "Battle/Effects/FinalBattleTargetedEffectDefinition.h"
#include "Types/FinalCoreTypes.h"
#include "FinalBattleEffectHeal.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleEffectHeal : public UFinalBattleTargetedEffectDefinition
{
	GENERATED_BODY()

public:
	UFinalBattleEffectHeal();

	// 治疗数值来源。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalBattleScalarValue Scalar;
};
