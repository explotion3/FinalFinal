#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FinalBattleTargetInteractorComponent.generated.h"

class AFinalBattlePresentationActor;
class APlayerController;

UCLASS(ClassGroup = (Final), meta = (BlueprintSpawnableComponent))
class FINALAPP_API UFinalBattleTargetInteractorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFinalBattleTargetInteractorComponent();

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|Targeting")
	bool HandlePrimaryClick();

	UFUNCTION(BlueprintCallable, Category = "Final|Battle|Targeting")
	AFinalBattlePresentationActor* TraceBattleTargetUnderCursor() const;

private:
	APlayerController* ResolveOwningPlayerController() const;
	bool SelectPresentationTarget(AFinalBattlePresentationActor* TargetActor) const;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Targeting")
	bool bEnableBattlefieldClickTargeting = true;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Targeting", meta = (ClampMin = "100.0"))
	float TraceDistance = 100000.0f;
};
