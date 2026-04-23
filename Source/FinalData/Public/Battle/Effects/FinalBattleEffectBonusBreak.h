#pragma once

#include "CoreMinimal.h"
#include "Battle/Effects/FinalBattleTargetedEffectDefinition.h"
#include "Types/FinalCoreTypes.h"
#include "FinalBattleEffectBonusBreak.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleEffectBonusBreak : public UFinalBattleTargetedEffectDefinition
{
	GENERATED_BODY()

public:
	UFinalBattleEffectBonusBreak();

	// 额外削韧数值来源。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalBattleScalarValue Scalar;
};
