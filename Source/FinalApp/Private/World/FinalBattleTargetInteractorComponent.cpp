#include "World/FinalBattleTargetInteractorComponent.h"

#include "Controllers/FinalBattleWidgetController.h"
#include "GameFramework/PlayerController.h"
#include "Subsystems/UI/FinalUISubsystem.h"
#include "World/FinalBattlePresentationActor.h"

UFinalBattleTargetInteractorComponent::UFinalBattleTargetInteractorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UFinalBattleTargetInteractorComponent::HandlePrimaryClick()
{
	if (!bEnableBattlefieldClickTargeting)
	{
		return false;
	}

	return SelectPresentationTarget(TraceBattleTargetUnderCursor());
}

AFinalBattlePresentationActor* UFinalBattleTargetInteractorComponent::TraceBattleTargetUnderCursor() const
{
	APlayerController* PlayerController = ResolveOwningPlayerController();
	if (PlayerController == nullptr)
	{
		return nullptr;
	}

	float ScreenX = 0.0f;
	float ScreenY = 0.0f;
	if (!PlayerController->GetMousePosition(ScreenX, ScreenY))
	{
		return nullptr;
	}

	FVector WorldOrigin = FVector::ZeroVector;
	FVector WorldDirection = FVector::ForwardVector;
	if (!PlayerController->DeprojectScreenPositionToWorld(ScreenX, ScreenY, WorldOrigin, WorldDirection))
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return nullptr;
	}

	TArray<FHitResult> Hits;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(FinalBattleTargetTrace), true);
	QueryParams.AddIgnoredActor(PlayerController->GetPawn());

	const FVector TraceEnd = WorldOrigin + WorldDirection * TraceDistance;
	World->LineTraceMultiByChannel(Hits, WorldOrigin, TraceEnd, ECC_Visibility, QueryParams);

	for (const FHitResult& Hit : Hits)
	{
		AFinalBattlePresentationActor* PresentationActor = Cast<AFinalBattlePresentationActor>(Hit.GetActor());
		if (PresentationActor == nullptr || !PresentationActor->IsTargetHitComponent(Hit.GetComponent()))
		{
			continue;
		}

		return PresentationActor;
	}

	return nullptr;
}

APlayerController* UFinalBattleTargetInteractorComponent::ResolveOwningPlayerController() const
{
	return Cast<APlayerController>(GetOwner());
}

bool UFinalBattleTargetInteractorComponent::SelectPresentationTarget(AFinalBattlePresentationActor* TargetActor) const
{
	if (TargetActor == nullptr
		|| TargetActor->GetPresentationTeam() != EFinalBattlePresentationTeam::Enemy
		|| TargetActor->GetRuntimeUnitId().IsNone()
		|| !TargetActor->IsPresentationAlive())
	{
		return false;
	}

	const APlayerController* PlayerController = ResolveOwningPlayerController();
	const UGameInstance* GameInstance = PlayerController ? PlayerController->GetGameInstance() : nullptr;
	const UFinalUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UFinalUISubsystem>() : nullptr;
	UFinalBattleWidgetController* BattleWidgetController = UISubsystem ? UISubsystem->GetBattleWidgetController() : nullptr;
	return BattleWidgetController ? BattleWidgetController->SelectEnemyByUnitId(TargetActor->GetRuntimeUnitId()) : false;
}
