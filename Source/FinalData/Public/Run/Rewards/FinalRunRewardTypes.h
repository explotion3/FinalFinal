#pragma once

#include "CoreMinimal.h"
#include "FinalRunRewardTypes.generated.h"

UENUM(BlueprintType)
enum class EFinalRunRewardType : uint8
{
	None,
	Gold,
	CardGrant,
	RelicGrant,
	RemoveCard,
	UpgradeCard,
	Growth
};

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalRunRewardEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName RewardId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalRunRewardType RewardType = EFinalRunRewardType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	int32 Value = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName DisplayId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bCanClaim = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	bool bClaimed = false;

	bool IsClaimable() const
	{
		return bCanClaim && !bClaimed;
	}
};
