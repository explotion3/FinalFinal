#pragma once

#include "CoreMinimal.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "FinalBattleTriggerDefinition.generated.h"

UENUM(BlueprintType)
enum class EFinalBattleTriggerWindow : uint8
{
	None,
	OwnerTookHealthDamage
};

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalBattleTriggerDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|BattleTrigger")
	EFinalBattleTriggerWindow TriggerWindow = EFinalBattleTriggerWindow::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Final|BattleTrigger")
	TArray<TObjectPtr<UFinalBattleEffectDefinition>> Effects;
};
