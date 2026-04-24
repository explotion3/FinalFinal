#include "World/FinalBattleStageAnchorActor.h"

#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"

AFinalBattleStageAnchorActor::AFinalBattleStageAnchorActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootSceneComponent);

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	ArrowComponent->SetupAttachment(RootSceneComponent);
	ArrowComponent->ArrowSize = 1.75f;
	ArrowComponent->SetArrowColor(FLinearColor(0.25f, 0.85f, 1.0f).ToFColor(true));

	LabelComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
	LabelComponent->SetupAttachment(RootSceneComponent);
	LabelComponent->SetHorizontalAlignment(EHTA_Center);
	LabelComponent->SetVerticalAlignment(EVRTA_TextCenter);
	LabelComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));
	LabelComponent->SetWorldSize(28.0f);
}

#if WITH_EDITOR
void AFinalBattleStageAnchorActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (ArrowComponent)
	{
		ArrowComponent->SetArrowColor(
			AnchorSide == EFinalBattleStageAnchorSide::Enemy
				? FColor(255, 110, 110)
				: FColor(95, 210, 255));
	}

	if (LabelComponent)
	{
		const TCHAR* SideLabel = AnchorSide == EFinalBattleStageAnchorSide::Enemy ? TEXT("Enemy") : TEXT("Player");
		LabelComponent->SetText(FText::FromString(FString::Printf(TEXT("%s Slot %d"), SideLabel, SlotIndex)));
	}
}
#endif
