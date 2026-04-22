#pragma once

#include "CoreMinimal.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "Battle/Effects/FinalBattleHandCardRequirement.h"
#include "Types/FinalCoreTypes.h"
#include "FinalBattleEffectGainShield.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleEffectGainShield : public UFinalBattleEffectDefinition
{
	GENERATED_BODY()

public:
	UFinalBattleEffectGainShield();

	// 护盾数值来源。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalBattleScalarValue Scalar;

	// 生效前要求当前手牌满足的条件。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalBattleHandCardRequirement HandCardRequirement;
};
