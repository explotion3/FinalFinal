#pragma once

#include "CoreMinimal.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "Ids/FinalIds.h"
#include "FinalBattleEffectGenerateCard.generated.h"

class UFinalCardDefinition;

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleEffectGenerateCard : public UFinalBattleEffectDefinition
{
	GENERATED_BODY()

public:
	UFinalBattleEffectGenerateCard();

	// 要生成的固定卡牌 Id。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalCardId GeneratedCardId;

	// 编辑器内直接引用的生成卡牌定义。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	TObjectPtr<UFinalCardDefinition> GeneratedCardDefinition = nullptr;

	// 可用于随机生成的候选卡牌定义列表。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	TArray<TObjectPtr<UFinalCardDefinition>> CandidateCardDefinitions;

	// 本次生成到手牌的数量。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (ClampMin = "1"))
	int32 GenerateCount = 1;

	// 是否从候选列表中随机选择卡牌生成。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	bool bChooseRandomCandidate = false;

	// 生成的牌是否标记为运行时衍生牌。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	bool bGeneratedCard = true;

	// 生成的牌是否仅在当前战斗内临时存在。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	bool bTemporaryCard = true;
};
