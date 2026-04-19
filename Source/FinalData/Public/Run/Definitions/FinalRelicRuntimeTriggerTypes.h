#pragma once

#include "CoreMinimal.h"
#include "FinalRelicRuntimeTriggerTypes.generated.h"

UENUM(BlueprintType)
enum class EFinalRelicTriggerDomain : uint8
{
	None,
	Battle,
	Run
};

UENUM(BlueprintType)
enum class EFinalRelicTriggerWindow : uint8
{
	None,
	PlayerTeamTookHealthDamage
};

UENUM(BlueprintType)
enum class EFinalRelicTriggerLimit : uint8
{
	None,
	OncePerPlayerTurn,
	OncePerBattle
};

UENUM(BlueprintType)
enum class EFinalRelicTriggerEffectType : uint8
{
	None,
	GainShield
};

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalRelicRuntimeTriggerEffectDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	EFinalRelicTriggerEffectType EffectType = EFinalRelicTriggerEffectType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	int32 Value = 0;
};

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalRelicRuntimeTriggerDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	EFinalRelicTriggerDomain Domain = EFinalRelicTriggerDomain::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	EFinalRelicTriggerWindow Window = EFinalRelicTriggerWindow::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	EFinalRelicTriggerLimit Limit = EFinalRelicTriggerLimit::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	TArray<FFinalRelicRuntimeTriggerEffectDefinition> Effects;
};
