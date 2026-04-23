#pragma once

#include "CoreMinimal.h"
#include "Battle/Conditions/FinalBattleConditionDefinition.h"
#include "Types/FinalCoreTypes.h"
#include "UObject/Object.h"
#include "FinalBattleEffectDefinition.generated.h"

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleEffectDefinition : public UObject
{
	GENERATED_BODY()

public:
	// 该效果条目的稳定 Id。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FName EffectId = NAME_None;

	// 该效果的具体类型。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	EFinalBattleEffectType EffectType = EFinalBattleEffectType::Damage;

	// 该效果使用的目标规则。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	EFinalBattleUnitTargetRule UnitTargetRule = EFinalBattleUnitTargetRule::None;

	// 兼容简单效果的直接数值字段。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	float FlatValue = 0.0f;

	// 生效前必须全部满足的条件；规则判断由 FinalBattle 执行。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Final|Effect")
	TArray<TObjectPtr<UFinalBattleConditionDefinition>> Conditions;

	// 面向编辑器和设计文档的备注文本。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FText Notes;
};
