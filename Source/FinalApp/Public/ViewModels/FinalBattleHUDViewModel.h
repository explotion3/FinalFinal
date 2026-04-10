#pragma once

#include "CoreMinimal.h"
#include "Events/FinalBattleEvent.h"
#include "Queries/FinalBattleSnapshot.h"
#include "UObject/Object.h"
#include "FinalBattleHUDViewModel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFinalPhaseChangedPresentationSignature, const FFinalBattleEvent&, BattleEvent);

UCLASS(BlueprintType)
class FINALAPP_API UFinalBattleHUDViewModel : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void ApplySnapshot(const FFinalBattleSnapshot& InSnapshot);

	UFUNCTION(BlueprintCallable, Category = "Final|UI")
	void ApplyBattleEvent(const FFinalBattleEvent& BattleEvent);

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	FFinalBattleSnapshot GetSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "Final|UI")
	FFinalBattleEvent GetLatestPhaseChangedEvent() const;

	UPROPERTY(BlueprintAssignable, Category = "Final|UI")
	FFinalPhaseChangedPresentationSignature OnPhaseChangedPresentation;

private:
	UPROPERTY(Transient)
	FFinalBattleSnapshot Snapshot;

	UPROPERTY(Transient)
	FFinalBattleEvent LatestPhaseChangedEvent;
};
