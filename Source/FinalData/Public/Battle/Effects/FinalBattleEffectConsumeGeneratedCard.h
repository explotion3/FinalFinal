#pragma once

#include "CoreMinimal.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"
#include "FinalBattleEffectConsumeGeneratedCard.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleEffectConsumeGeneratedCard : public UFinalBattleEffectDefinition
{
	GENERATED_BODY()

public:
	UFinalBattleEffectConsumeGeneratedCard();

	// 要消耗的指定衍生牌 Id。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalCardId RequiredCardId;

	// 要消耗的衍生牌关键词条件。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FGameplayTag RequiredKeyword;

	// 需要消耗的衍生牌数量。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (ClampMin = "1"))
	int32 ConsumeCount = 1;

	// 是否只允许消耗运行时生成的牌。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	bool bGeneratedOnly = true;
};
