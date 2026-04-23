#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "FinalBattleStatusChangeRequirement.generated.h"

UENUM(BlueprintType)
enum class EFinalBattleStatusChangeKind : uint8
{
	Removed
};

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalBattleStatusChangeRequirement
{
	GENERATED_BODY()

	// 要求本次效果链中出现的状态变化类型。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	EFinalBattleStatusChangeKind ChangeKind = EFinalBattleStatusChangeKind::Removed;

	// 要求发生变化的状态 Id。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalStatusId RequiredStatusId;

	// 要求至少变化的状态层数。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (ClampMin = "1"))
	int32 MinimumStacks = 1;
};
