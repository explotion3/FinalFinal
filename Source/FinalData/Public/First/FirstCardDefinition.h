#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "FirstCardDefinition.generated.h"

UENUM(BlueprintType)
enum class EFirstCardDefinitionEffectType : uint8
{
	None,
	Damage,
	MoveHandCard
};

UENUM(BlueprintType)
enum class EFirstCardDefinitionHandRole : uint8
{
	None,
	LeftHandCore,
	RightHandCore
};

UENUM(BlueprintType)
enum class EFirstCardDefinitionHandZone : uint8
{
	None,
	Left,
	Both,
	Right
};

UENUM(BlueprintType)
enum class EFirstCardDefinitionHandMoveTargetPolicy : uint8
{
	RandomValidZone,
	RandomOtherThanSourceZone,
	FixedZone
};

UENUM(BlueprintType)
enum class EFirstCardDefinitionPlayDestination : uint8
{
	DiscardPile,
	ReturnToHandRandomZone
};

USTRUCT(BlueprintType)
struct FINALDATA_API FFirstCardDefinitionEffect
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First|Card|Effect")
	EFirstCardDefinitionEffectType EffectType = EFirstCardDefinitionEffectType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First|Card|Effect")
	FName EffectId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First|Card|Effect")
	int32 Value = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First|Card|Move")
	int32 MoveCardCount = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First|Card|Move")
	bool bMoveRequiresSourceZone = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First|Card|Move", meta = (EditCondition = "bMoveRequiresSourceZone"))
	EFirstCardDefinitionHandZone MoveSourceZone = EFirstCardDefinitionHandZone::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First|Card|Move")
	EFirstCardDefinitionHandMoveTargetPolicy MoveTargetPolicy = EFirstCardDefinitionHandMoveTargetPolicy::RandomValidZone;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First|Card|Move", meta = (EditCondition = "MoveTargetPolicy == EFirstCardDefinitionHandMoveTargetPolicy::FixedZone"))
	EFirstCardDefinitionHandZone MoveTargetZone = EFirstCardDefinitionHandZone::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First|Card|Move")
	int32 MoveTargetCostDelta = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First|Card|Move")
	bool bTransferActualCostReductionToSourceCard = false;
};

UCLASS(BlueprintType)
class FINALDATA_API UFirstCardDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, AssetRegistrySearchable, Category = "First|Card")
	FName CardId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First|Card")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First|Card")
	int32 BaseCost = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First|Card|Entry Stats")
	int32 PlayerMaxHPBonusOnEnterBattle = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First|Card")
	EFirstCardDefinitionPlayDestination PlayDestination = EFirstCardDefinitionPlayDestination::DiscardPile;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First|Card")
	FGameplayTagContainer Keywords;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First|Card|Hand")
	EFirstCardDefinitionHandRole HandRole = EFirstCardDefinitionHandRole::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First|Card|Hand")
	bool bRequiresHandZoneToPlay = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First|Card|Hand", meta = (EditCondition = "bRequiresHandZoneToPlay"))
	EFirstCardDefinitionHandZone RequiredHandZone = EFirstCardDefinitionHandZone::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First|Card|PerfectRelease")
	bool bSkipInitiativeReductionOnPerfectReleaseInZone = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First|Card|PerfectRelease", meta = (EditCondition = "bSkipInitiativeReductionOnPerfectReleaseInZone"))
	EFirstCardDefinitionHandZone PerfectReleaseInitiativeSkipZone = EFirstCardDefinitionHandZone::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "First|Card|Effects")
	TArray<FFirstCardDefinitionEffect> Effects;
};
