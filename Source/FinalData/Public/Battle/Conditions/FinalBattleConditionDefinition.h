#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FinalBattleConditionDefinition.generated.h"

UENUM(BlueprintType)
enum class EFinalBattleConditionContext : uint8
{
	// 只需要来源单位或当前战斗状态即可判定。
	SourceOnly,

	// 需要读取同一效果链中前置效果产生的执行记录。
	ChainRecord,

	// 必须等具体目标解析完成后才能判定。
	TargetRequired,

	// 需要读取当前 resolved-card 上下文才能判定。
	ResolvedCard
};

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleConditionDefinition : public UObject
{
	GENERATED_BODY()

public:
	virtual EFinalBattleConditionContext GetConditionContext() const { return EFinalBattleConditionContext::SourceOnly; }

	// 条件条目的稳定 Id，便于内容定位和调试。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Condition")
	FName ConditionId = NAME_None;

	// 面向编辑器和设计文档的备注文本。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Condition")
	FText Notes;
};
