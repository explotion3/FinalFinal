#include "World/FinalBattleTargetInteractorComponent.h"

#include "Controllers/FinalBattleWidgetController.h"
#include "DrawDebugHelpers.h"
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

	return TraceBattleTargetAtScreenPosition(FVector2D(ScreenX, ScreenY));
}

AFinalBattlePresentationActor* UFinalBattleTargetInteractorComponent::TraceEnemyTargetUnderCursor() const
{
	return TraceEnemyTargetAtScreenPosition(FVector2D::ZeroVector);
}

AFinalBattlePresentationActor* UFinalBattleTargetInteractorComponent::TraceBattleTargetAtScreenPosition(const FVector2D ViewportScreenPosition) const
{
	APlayerController* PlayerController = ResolveOwningPlayerController();
	if (PlayerController == nullptr)
	{
		return nullptr;
	}

	FVector2D EffectiveScreenPosition = ViewportScreenPosition;
	if (EffectiveScreenPosition.IsNearlyZero())
	{
		float ScreenX = 0.0f;
		float ScreenY = 0.0f;
		if (!PlayerController->GetMousePosition(ScreenX, ScreenY))
		{
			return nullptr;
		}
		EffectiveScreenPosition = FVector2D(ScreenX, ScreenY);
	}

	FVector WorldOrigin = FVector::ZeroVector;
	FVector WorldDirection = FVector::ForwardVector;
	if (!PlayerController->DeprojectScreenPositionToWorld(EffectiveScreenPosition.X, EffectiveScreenPosition.Y, WorldOrigin, WorldDirection))
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

	AFinalBattlePresentationActor* FallbackPresentationActor = nullptr;
	FVector DebugHitLocation = TraceEnd;
	for (const FHitResult& Hit : Hits)
	{
		AFinalBattlePresentationActor* PresentationActor = Cast<AFinalBattlePresentationActor>(Hit.GetActor());
		if (PresentationActor == nullptr)
		{
			continue;
		}

		if (PresentationActor->IsTargetHitComponent(Hit.GetComponent()))
		{
			DebugHitLocation = Hit.ImpactPoint;
			if (bDrawDebugTargetTrace)
			{
				DrawDebugLine(World, WorldOrigin, TraceEnd, FColor::Green, false, DebugTraceDrawDuration, 0, 1.5f);
				DrawDebugSphere(World, DebugHitLocation, 10.0f, 8, FColor::Green, false, DebugTraceDrawDuration);
			}
			return PresentationActor;
		}

		if (FallbackPresentationActor == nullptr)
		{
			FallbackPresentationActor = PresentationActor;
			DebugHitLocation = Hit.ImpactPoint;
		}
	}

	if (bDrawDebugTargetTrace)
	{
		const FColor DebugColor = FallbackPresentationActor ? FColor::Yellow : FColor::Red;
		DrawDebugLine(World, WorldOrigin, TraceEnd, DebugColor, false, DebugTraceDrawDuration, 0, 1.5f);
		if (FallbackPresentationActor)
		{
			DrawDebugSphere(World, DebugHitLocation, 10.0f, 8, DebugColor, false, DebugTraceDrawDuration);
		}
	}

	return FallbackPresentationActor;
}

AFinalBattlePresentationActor* UFinalBattleTargetInteractorComponent::TraceEnemyTargetAtScreenPosition(const FVector2D ViewportScreenPosition) const
{
	AFinalBattlePresentationActor* TargetActor = TraceBattleTargetAtScreenPosition(ViewportScreenPosition);
	if (TargetActor == nullptr
		|| TargetActor->GetPresentationTeam() != EFinalBattlePresentationTeam::Enemy
		|| TargetActor->GetRuntimeUnitId().IsNone()
		|| !TargetActor->IsPresentationAlive())
	{
		return nullptr;
	}

	return TargetActor;
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
