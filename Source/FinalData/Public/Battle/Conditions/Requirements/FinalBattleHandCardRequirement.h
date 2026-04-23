#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"
#include "FinalBattleHandCardRequirement.generated.h"

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalBattleHandCardRequirement
{
	GENERATED_BODY()

	// 是否要求当前手牌满足指定条件。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	bool bRequireInHand = false;

	// 要求手牌中存在的指定卡牌 Id。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (EditCondition = "bRequireInHand"))
	FFinalCardId RequiredCardId;

	// 要求手牌中存在的指定关键词。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (EditCondition = "bRequireInHand"))
	FGameplayTag RequiredKeyword;

	// 要求手牌中至少满足条件的数量。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (ClampMin = "1", EditCondition = "bRequireInHand"))
	int32 MinimumCount = 1;

	// 是否只统计运行时生成的手牌。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (EditCondition = "bRequireInHand"))
	bool bGeneratedOnly = false;
};
