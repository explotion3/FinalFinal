#pragma once

#include "CoreMinimal.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"
#include "FinalBattleEffectMoveCards.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleEffectMoveCards : public UFinalBattleEffectDefinition
{
	GENERATED_BODY()

public:
	UFinalBattleEffectMoveCards();

	// 从哪个牌区匹配并移出卡牌实例。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	EFinalBattleCardZoneRule SourceZone = EFinalBattleCardZoneRule::Hand;

	// 匹配到的卡牌实例要移动到哪个牌区。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	EFinalBattleCardZoneRule DestinationZone = EFinalBattleCardZoneRule::ConsumePile;

	// 可选：只匹配指定卡牌 Id。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalCardId RequiredCardId;

	// 可选：只匹配带有该关键词的卡牌。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FGameplayTag RequiredKeyword;

	// 最多移动多少张满足条件的卡牌。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (ClampMin = "1"))
	int32 MoveCount = 1;

	// 是否只匹配战斗中生成的卡牌实例。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	bool bGeneratedOnly = false;

	// 是否把实际移动的衍生牌记录到本条效果链上下文，供后续 ConsumedGeneratedCard 条件读取。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	bool bRecordMovedGeneratedCards = false;
};
