#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Ids/FinalIds.h"
#include "FinalCardEvolutionDefinition.generated.h"

UENUM(BlueprintType)
enum class EFinalCardEvolutionStage : uint8
{
    Base,
    Evolved,
    Mastered
};

UENUM(BlueprintType)
enum class EFinalCardEvolutionType : uint8
{
    ImmediatePower,
    GrowthPotential,
    ArchetypeShift
};

UCLASS(BlueprintType)
class FINALDATA_API UFinalCardEvolutionDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, AssetRegistrySearchable, Category = "Final|Growth")
    FFinalCardEvolutionId EvolutionId;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth")
    FFinalCardId FromCardId;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth")
    FFinalCardId ToCardId;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth")
    EFinalCardEvolutionStage FromStage = EFinalCardEvolutionStage::Base;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth")
    EFinalCardEvolutionStage ToStage = EFinalCardEvolutionStage::Evolved;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth")
    EFinalCardEvolutionType EvolutionType = EFinalCardEvolutionType::ImmediatePower;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth")
    FFinalCharacterId RequiredOwnerCharacterId;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth")
    FGameplayTagContainer RequiredCardTags;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth")
    bool bAllowAsLevelUpCandidate = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth")
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Growth", meta = (MultiLine = "true"))
    FText Description;
};