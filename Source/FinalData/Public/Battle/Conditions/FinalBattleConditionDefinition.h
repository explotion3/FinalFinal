#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FinalBattleConditionDefinition.generated.h"

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleConditionDefinition : public UObject
{
	GENERATED_BODY()

public:
	// 条件条目的稳定 Id，便于内容定位和调试。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Condition")
	FName ConditionId = NAME_None;

	// 面向编辑器和设计文档的备注文本。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Condition")
	FText Notes;
};
