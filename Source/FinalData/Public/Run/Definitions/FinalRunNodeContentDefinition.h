#pragma once

#include "CoreMinimal.h"
#include "Run/Rewards/FinalRunRewardTypes.h"
#include "FinalRunNodeContentDefinition.generated.h"

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalRunRewardNodeContentDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText Summary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunRewardEntry> RewardEntries;
};

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalRunEventOptionDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName OptionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText DisplayText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText OutcomeSummary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bStartsDisabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText DisabledReason;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunRewardEntry> RewardEntries;
};

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalRunEventNodeContentDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText Summary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunEventOptionDefinition> Options;
};

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalRunShopOfferDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName OfferId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName DisplayId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 Price = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bStartsUnavailable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText UnavailableReason;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunRewardEntry> RewardEntries;
};

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalRunShopNodeContentDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText Summary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunShopOfferDefinition> Offers;
};
