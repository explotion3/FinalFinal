#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"
#include "Types/FinalCoreTypes.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "FinalCardDefinition.generated.h"

UENUM(BlueprintType)
enum class EFinalCardTextMode : uint8
{
	ManualRulesText,
	EffectLayout
};

UENUM(BlueprintType)
enum class EFinalCardTextFragmentKind : uint8
{
	FullLine,
	Inline
};

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalCardTextLayoutLine
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Card|Text")
	FString Template;
};

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalCardTextFragmentOverride
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Card|Text")
	FName EffectId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Card|Text")
	EFinalCardTextFragmentKind FragmentKind = EFinalCardTextFragmentKind::FullLine;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Card|Text")
	FText OverrideText;
};

UCLASS(BlueprintType)
class FINALDATA_API UFinalCardDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, AssetRegistrySearchable, Category = "Final|Card")
	FFinalCardId CardId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Card")
	FName OwnerUnitId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Card")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Card")
	EFinalCardType CardType = EFinalCardType::Attack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Card")
	EFinalRarity Rarity = EFinalRarity::Common;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Card")
	int32 BaseCostAP = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Card")
	FGameplayTagContainer Keywords;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Card")
	FText RulesText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Card|Text")
	EFinalCardTextMode TextMode = EFinalCardTextMode::ManualRulesText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Card|Text", meta = (EditCondition = "TextMode == EFinalCardTextMode::EffectLayout"))
	TArray<FFinalCardTextLayoutLine> TextLayoutLines;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Card|Text", meta = (EditCondition = "TextMode == EFinalCardTextMode::EffectLayout"))
	TArray<FFinalCardTextFragmentOverride> TextFragmentOverrides;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Final|Card")
	TArray<TObjectPtr<UFinalBattleEffectDefinition>> Effects;
};
