#pragma once

#include "CoreMinimal.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "Types/FinalCoreTypes.h"
#include "FinalBattleTargetedEffectDefinition.generated.h"

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleTargetedEffectDefinition : public UFinalBattleEffectDefinition
{
	GENERATED_BODY()

public:
	// 该效果使用的单位目标规则；只有需要单位目标的效果才持有。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	EFinalBattleUnitTargetRule UnitTargetRule = EFinalBattleUnitTargetRule::None;
};
