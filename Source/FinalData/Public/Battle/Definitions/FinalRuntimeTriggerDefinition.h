#pragma once

#include "CoreMinimal.h"
#include "Battle/Conditions/FinalBattleConditionDefinition.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "FinalRuntimeTriggerDefinition.generated.h"

UENUM(BlueprintType)
enum class EFinalRuntimeTriggerDomain : uint8
{
	None,
	Battle,
	Run
};

UENUM(BlueprintType)
enum class EFinalRuntimeTriggerWindow : uint8
{
	None,
	OwnerTookHealthDamage,
	PlayerTeamTookHealthDamage,
	PlayerCardResolved
};

UENUM(BlueprintType)
enum class EFinalRuntimeTriggerLimit : uint8
{
	None,
	OncePerPlayerTurn,
	OncePerBattle
};

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalRuntimeTriggerDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Trigger")
	EFinalRuntimeTriggerDomain Domain = EFinalRuntimeTriggerDomain::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Trigger")
	EFinalRuntimeTriggerWindow Window = EFinalRuntimeTriggerWindow::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Trigger")
	EFinalRuntimeTriggerLimit Limit = EFinalRuntimeTriggerLimit::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Final|Trigger")
	TArray<TObjectPtr<UFinalBattleConditionDefinition>> Conditions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Final|Trigger")
	TArray<TObjectPtr<UFinalBattleEffectDefinition>> Effects;
};
