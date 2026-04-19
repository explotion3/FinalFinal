#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Types/FinalCoreTypes.h"
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
	PlayerTeamTookHealthDamage,
	PlayerCardResolved
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
	GainShield,
	DrawCards
};

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalRelicRuntimeCardConditionDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	bool bRequireCardCostAP = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic", meta = (EditCondition = "bRequireCardCostAP"))
	int32 RequiredCardCostAP = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	bool bRequireCardType = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic", meta = (EditCondition = "bRequireCardType"))
	EFinalCardType RequiredCardType = EFinalCardType::Attack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	FGameplayTag RequiredKeyword;
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
	FFinalRelicRuntimeCardConditionDefinition CardCondition;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Relic")
	TArray<FFinalRelicRuntimeTriggerEffectDefinition> Effects;
};
