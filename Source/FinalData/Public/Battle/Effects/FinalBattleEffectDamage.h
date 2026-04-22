#pragma once

#include "CoreMinimal.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "Battle/Effects/FinalBattleGeneratedCardConsumeRequirement.h"
#include "Battle/Effects/FinalBattleTargetStateRequirement.h"
#include "Types/FinalCoreTypes.h"
#include "FinalBattleEffectDamage.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleEffectDamage : public UFinalBattleEffectDefinition
{
	GENERATED_BODY()

public:
	UFinalBattleEffectDamage();

	// 单次伤害的基础数值来源。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalBattleScalarValue Scalar;

	// 本效果会重复命中的次数。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (ClampMin = "1"))
	int32 HitCount = 1;

	// 伤害生效前要求已消耗衍生牌的条件。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalBattleGeneratedCardConsumeRequirement GeneratedCardConsumeRequirement;

	// 目标需要满足的状态条件。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalBattleTargetStateRequirement TargetStateRequirement;
};
