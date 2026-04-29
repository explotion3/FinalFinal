#pragma once

#include "CoreMinimal.h"
#include "Battle/Conditions/FinalBattleConditionDefinition.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "Types/FinalCoreTypes.h"
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

UENUM(BlueprintType)
enum class EFinalTriggeredCardModifierTargetSource : uint8
{
	None,
	DrawnCardsFromExecutedEffects
};

UENUM(BlueprintType)
enum class EFinalTriggeredCardModifierDurationPolicy : uint8
{
	UntilPlayed,
	EndOfTurn,
	EndOfRound,
	EndOfBattle,
	ManualClear
};

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalTriggeredCardModifierDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Trigger")
	EFinalTriggeredCardModifierTargetSource TargetSource = EFinalTriggeredCardModifierTargetSource::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Trigger")
	bool bRequireCardType = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Trigger", meta = (EditCondition = "bRequireCardType"))
	EFinalCardType RequiredCardType = EFinalCardType::Attack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Trigger")
	int32 CostDeltaAP = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Trigger")
	int32 OutgoingDamagePercentDelta = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Trigger")
	EFinalTriggeredCardModifierDurationPolicy DurationPolicy = EFinalTriggeredCardModifierDurationPolicy::UntilPlayed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Trigger")
	bool bExpireAtPlayerTurnEnd = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Trigger")
	bool bApplyToAllSameSourceRunCardInstances = false;
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Trigger")
	TArray<FFinalTriggeredCardModifierDefinition> TriggeredCardModifiers;
};
