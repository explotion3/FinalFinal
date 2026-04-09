#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/FinalCoreTypes.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "FinalEnemyIntentDefinition.generated.h"

UCLASS(BlueprintType)
class FINALDATA_API UFinalEnemyIntentDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy")
	FName IntentId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy")
	EFinalIntentType IntentType = EFinalIntentType::Attack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy")
	FText PreviewText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Final|Enemy")
	TArray<TObjectPtr<UFinalBattleEffectDefinition>> Effects;
};
