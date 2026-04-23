#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"
#include "FinalBattleGeneratedCardConsumeRequirement.generated.h"

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalBattleGeneratedCardConsumeRequirement
{
	GENERATED_BODY()

	// 是否要求本次效果前已成功消耗过衍生牌。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	bool bRequireConsumedGeneratedCard = false;

	// 要求被消耗衍生牌命中的指定卡牌 Id。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (EditCondition = "bRequireConsumedGeneratedCard"))
	FFinalCardId RequiredCardId;

	// 要求被消耗衍生牌命中的指定关键词。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (EditCondition = "bRequireConsumedGeneratedCard"))
	FGameplayTag RequiredKeyword;

	// 要求至少消耗的衍生牌数量。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (ClampMin = "1", EditCondition = "bRequireConsumedGeneratedCard"))
	int32 MinimumCount = 1;
};
