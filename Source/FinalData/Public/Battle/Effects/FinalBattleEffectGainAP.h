#pragma once

#include "CoreMinimal.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "Battle/Effects/FinalBattleStatusConsumeRequirement.h"
#include "FinalBattleEffectGainAP.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleEffectGainAP : public UFinalBattleEffectDefinition
{
	GENERATED_BODY()

public:
	UFinalBattleEffectGainAP();

	// 本次增加的 AP 数值。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (ClampMin = "0"))
	int32 GainValue = 0;

	// 生效前要求本次效果链已消耗的状态条件。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalBattleStatusConsumeRequirement ConsumeRequirement;
};
