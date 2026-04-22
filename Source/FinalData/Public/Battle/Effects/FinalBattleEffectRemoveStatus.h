#pragma once

#include "CoreMinimal.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "Ids/FinalIds.h"
#include "FinalBattleEffectRemoveStatus.generated.h"

class UFinalStatusDefinition;

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleEffectRemoveStatus : public UFinalBattleEffectDefinition
{
	GENERATED_BODY()

public:
	UFinalBattleEffectRemoveStatus();

	// 要移除的状态稳定 Id。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalStatusId StatusId;

	// 编辑器内直接引用的状态定义资产。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	TObjectPtr<UFinalStatusDefinition> StatusDefinition = nullptr;

	// 本次要移除的层数。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (ClampMin = "1"))
	int32 Stacks = 1;
};
