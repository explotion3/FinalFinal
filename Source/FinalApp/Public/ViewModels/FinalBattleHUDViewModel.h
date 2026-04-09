#pragma once

#include "CoreMinimal.h"
#include "Queries/FinalBattleSnapshot.h"
#include "UObject/Object.h"
#include "FinalBattleHUDViewModel.generated.h"

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleHUDViewModel : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void ApplySnapshot(const FFinalBattleSnapshot& InSnapshot);

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	FFinalBattleSnapshot GetSnapshot() const;

private:
	UPROPERTY(Transient)
	FFinalBattleSnapshot Snapshot;
};
