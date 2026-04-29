#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "Run/Definitions/FinalGrowthChoiceDefinition.h"
#include "FinalRunGrowthChoice.generated.h"

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunGrowthChoiceInstance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Growth")
    FName ChoiceInstanceId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Growth")
    EFinalGrowthChoiceType ChoiceType = EFinalGrowthChoiceType::AttributeGrowth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Growth")
    FFinalCharacterId CharacterId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Growth")
    EFinalGrowthAttributeType AttributeType = EFinalGrowthAttributeType::RootBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Growth")
    int32 AttributeDelta = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Growth")
    FName TargetRunCardInstanceId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Growth")
    FFinalCardEvolutionId CardEvolutionId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Growth")
    FFinalCardId FromCardId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Growth")
    FFinalCardId ToCardId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Growth")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Growth", meta = (MultiLine = "true"))
    FText Description;

    bool IsValid() const
    {
        return ChoiceInstanceId != NAME_None && CharacterId.IsValid();
    }
};

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunPendingGrowthChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Growth")
    bool bIsValid = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Growth")
    FFinalCharacterId CharacterId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run|Growth")
    TArray<FFinalRunGrowthChoiceInstance> Choices;

    void Reset()
    {
        bIsValid = false;
        CharacterId = FFinalCharacterId{};
        Choices.Reset();
    }
};