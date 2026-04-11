#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "Run/Definitions/FinalRelicBattleTypes.h"
#include "FinalBattleRelicPayload.generated.h"

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalBattleStartRelicEffectInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Relic")
	EFinalRelicBattleStartEffectType EffectType = EFinalRelicBattleStartEffectType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Relic")
	int32 Value = 0;
};

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalBattleStartRelicInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Relic")
	FFinalRelicId RelicId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Relic")
	FName DisplayId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Relic")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Relic")
	TArray<FFinalBattleStartRelicEffectInput> BattleStartEffects;
};
