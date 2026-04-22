#pragma once

#include "CoreMinimal.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "Battle/Effects/FinalBattleGeneratedCardConsumeRequirement.h"
#include "Battle/Effects/FinalBattleHandCardRequirement.h"
#include "Battle/Effects/FinalBattleStatusConsumeRequirement.h"
#include "FinalBattleEffectDrawCards.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleEffectDrawCards : public UFinalBattleEffectDefinition
{
	GENERATED_BODY()

public:
	UFinalBattleEffectDrawCards();

	// 本次要抽取的牌数。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (ClampMin = "1"))
	int32 DrawCount = 1;

	// 生效前要求本次效果链已消耗的状态条件。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalBattleStatusConsumeRequirement ConsumeRequirement;

	// 生效前要求当前手牌满足的条件。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalBattleHandCardRequirement HandCardRequirement;

	// 生效前要求已消耗衍生牌的条件。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalBattleGeneratedCardConsumeRequirement GeneratedCardConsumeRequirement;
};
