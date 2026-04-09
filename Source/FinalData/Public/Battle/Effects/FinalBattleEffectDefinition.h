#pragma once

#include "CoreMinimal.h"
#include "Types/FinalCoreTypes.h"
#include "UObject/Object.h"
#include "FinalBattleEffectDefinition.generated.h"

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleEffectDefinition : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FName EffectId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	EFinalBattleEffectType EffectType = EFinalBattleEffectType::Damage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	EFinalBattleUnitTargetRule UnitTargetRule = EFinalBattleUnitTargetRule::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	float FlatValue = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FText Notes;
};
