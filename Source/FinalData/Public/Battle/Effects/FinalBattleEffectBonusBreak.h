#pragma once

#include "CoreMinimal.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "Battle/Effects/FinalBattleStatusConsumeRequirement.h"
#include "Types/FinalCoreTypes.h"
#include "FinalBattleEffectBonusBreak.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleEffectBonusBreak : public UFinalBattleEffectDefinition
{
	GENERATED_BODY()

public:
	UFinalBattleEffectBonusBreak();

	// 额外削韧数值来源。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalBattleScalarValue Scalar;

	// 生效前要求本次效果链已消耗的状态条件。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalBattleStatusConsumeRequirement ConsumeRequirement;
};
