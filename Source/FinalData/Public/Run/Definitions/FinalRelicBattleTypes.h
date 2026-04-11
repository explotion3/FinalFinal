#pragma once

#include "CoreMinimal.h"
#include "FinalRelicBattleTypes.generated.h"

UENUM(BlueprintType)
enum class EFinalRelicBattleStartEffectType : uint8
{
	None,
	GainAP,
	GainShield
};

UENUM(BlueprintType)
enum class EFinalRelicPlayerTurnStartEffectType : uint8
{
	None,
	GainAP,
	GainShield
};

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalRelicBattleStartEffectDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	EFinalRelicBattleStartEffectType EffectType = EFinalRelicBattleStartEffectType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	int32 Value = 0;
};

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalRelicPlayerTurnStartEffectDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	EFinalRelicPlayerTurnStartEffectType EffectType = EFinalRelicPlayerTurnStartEffectType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	int32 Value = 0;
};
