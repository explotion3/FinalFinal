#pragma once

#include "CoreMinimal.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "Ids/FinalIds.h"
#include "FinalBattleEffectApplyStatus.generated.h"

class UFinalStatusDefinition;

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleEffectApplyStatus : public UFinalBattleEffectDefinition
{
	GENERATED_BODY()

public:
	UFinalBattleEffectApplyStatus();

	// 要施加的状态稳定 Id。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalStatusId StatusId;

	// 编辑器内直接引用的状态定义资产。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	TObjectPtr<UFinalStatusDefinition> StatusDefinition = nullptr;

	// 本次施加的层数。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (ClampMin = "1"))
	int32 Stacks = 1;

	// 覆盖状态默认持续时间；0 表示沿用定义值。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	int32 DurationOverride = 0;
};
