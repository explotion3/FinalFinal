#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FinalBattleStageAnchorActor.generated.h"

class UArrowComponent;
class USceneComponent;
class UTextRenderComponent;

UENUM(BlueprintType)
enum class EFinalBattleStageAnchorSide : uint8
{
	Player,
	Enemy
};

UCLASS()
class FINALAPP_API AFinalBattleStageAnchorActor : public AActor
{
	GENERATED_BODY()

public:
	AFinalBattleStageAnchorActor();

	EFinalBattleStageAnchorSide GetAnchorSide() const { return AnchorSide; }
	int32 GetSlotIndex() const { return SlotIndex; }

#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif

private:
	UPROPERTY(VisibleAnywhere, Category = "Final|Battle|Stage")
	TObjectPtr<USceneComponent> RootSceneComponent;

	UPROPERTY(VisibleAnywhere, Category = "Final|Battle|Stage")
	TObjectPtr<UArrowComponent> ArrowComponent;

	UPROPERTY(VisibleAnywhere, Category = "Final|Battle|Stage")
	TObjectPtr<UTextRenderComponent> LabelComponent;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Stage")
	EFinalBattleStageAnchorSide AnchorSide = EFinalBattleStageAnchorSide::Player;

	UPROPERTY(EditAnywhere, Category = "Final|Battle|Stage", meta = (ClampMin = "0"))
	int32 SlotIndex = 0;
};
