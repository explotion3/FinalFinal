#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"
#include "Types/FinalCoreTypes.h"
#include "FinalBattleMovedCardRequirement.generated.h"

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalBattleMovedCardRequirement
{
	GENERATED_BODY()

	// 可选：要求已移动卡牌命中的指定卡牌 Id。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalCardId RequiredCardId;

	// 可选：要求已移动卡牌命中的指定关键词。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FGameplayTag RequiredKeyword;

	// 要求至少移动的卡牌数量。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (ClampMin = "1"))
	int32 MinimumCount = 1;

	// 是否只统计战斗中生成的卡牌实例。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	bool bGeneratedOnly = false;

	// 可选：要求已移动卡牌来自指定牌区。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	bool bRequireSourceZone = false;

	// 要求已移动卡牌的来源牌区。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (EditCondition = "bRequireSourceZone"))
	EFinalBattleCardZoneRule SourceZone = EFinalBattleCardZoneRule::Hand;

	// 可选：要求已移动卡牌进入指定牌区。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	bool bRequireDestinationZone = false;

	// 要求已移动卡牌的目标牌区。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (EditCondition = "bRequireDestinationZone"))
	EFinalBattleCardZoneRule DestinationZone = EFinalBattleCardZoneRule::ConsumePile;
};
