#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Types/FinalCoreTypes.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "FinalEnemyIntentDefinition.generated.h"

UCLASS(BlueprintType)
class FINALDATA_API UFinalEnemyIntentDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, AssetRegistrySearchable, Category = "Final|Enemy")
	FName IntentId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy")
	EFinalIntentType IntentType = EFinalIntentType::Attack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy")
	FText PreviewText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy", meta = (ClampMin = "0"))
	int32 Weight = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy", meta = (ClampMin = "0"))
	int32 CooldownTurns = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy", meta = (ClampMin = "0"))
	int32 UseLimitPerBattle = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy|Selection", meta = (ClampMin = "1"))
	int32 MinPreviewRound = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy|Selection", meta = (ClampMin = "0"))
	int32 MaxPreviewRound = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy|Selection", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinEnemyHpPercent = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy|Selection", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxEnemyHpPercent = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy|Selection")
	bool bDisallowRepeatLastIntent = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy")
	TArray<FName> PhaseTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Enemy")
	FGameplayTagContainer RequiredEnemyRoleTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Final|Enemy")
	TArray<TObjectPtr<UFinalBattleEffectDefinition>> Effects;
};
