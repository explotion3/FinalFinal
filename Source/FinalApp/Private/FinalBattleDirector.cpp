#include "World/FinalBattleDirector.h"

#include "BattleBridge/FinalBattleEventPresentationUtils.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "EngineUtils.h"
#include "Queries/FinalDataRegistry.h"
#include "Subsystems/FinalBattleFlowSubsystem.h"
#include "World/FinalBattlePresentationActor.h"
#include "World/FinalBattleStageAnchorActor.h"

namespace
{
FText BuildCharacterDetailText(const FFinalBattleCharacterViewData& CharacterView)
{
	const FText CollapseState = CharacterView.bCollapsed
		? NSLOCTEXT("FinalBattleDirector", "CharacterCollapsed", "状态: 崩溃中")
		: NSLOCTEXT("FinalBattleDirector", "CharacterReady", "状态: 可行动");

	return FText::Format(
		NSLOCTEXT("FinalBattleDirector", "CharacterDetailFormat", "压力 {0}/{1} | 生命份额 {2}\n苏醒 {3}/{4} | 崩溃 {5}\n{6}"),
		FText::AsNumber(CharacterView.CurrentStress),
		FText::AsNumber(CharacterView.StressCap),
		FText::AsNumber(CharacterView.VitalShare),
		FText::AsNumber(CharacterView.CurrentAwakenCount),
		FText::AsNumber(CharacterView.CurrentAwakenThreshold),
		FText::AsNumber(CharacterView.CollapseCount),
		CollapseState);
}

FText BuildEnemyDetailText(const FFinalBattleEnemyViewData& EnemyView)
{
	if (EnemyView.CurrentHP <= 0)
	{
		return FText::Format(
			NSLOCTEXT("FinalBattleDirector", "EnemyDefeatedFormat", "站位 {0}\n已击败"),
			FText::AsNumber(EnemyView.PositionIndex + 1));
	}

	const FText IntentText = !EnemyView.IntentText.IsEmpty()
		? EnemyView.IntentText
		: NSLOCTEXT("FinalBattleDirector", "EnemyNoIntent", "当前无公开意图");

	return FText::Format(
		NSLOCTEXT("FinalBattleDirector", "EnemyDetailFormat", "站位 {0} | HP {1}/{2} | 护盾 {3}\nBreak {4}/{5} | Init {6}\n{7}"),
		FText::AsNumber(EnemyView.PositionIndex + 1),
		FText::AsNumber(EnemyView.CurrentHP),
		FText::AsNumber(EnemyView.MaxHP),
		FText::AsNumber(EnemyView.CurrentShield),
		FText::AsNumber(EnemyView.CurrentBreakValue),
		FText::AsNumber(EnemyView.MaxBreakValue),
		FText::AsNumber(EnemyView.CurrentInitiative),
		IntentText);
}

FText ResolveUnitDisplayName(const FFinalBattleSnapshot& Snapshot, const FName RuntimeUnitId)
{
	if (const FFinalBattleCharacterViewData* CharacterView = Snapshot.Characters.FindByPredicate(
			[RuntimeUnitId](const FFinalBattleCharacterViewData& Candidate)
			{
				return Candidate.RuntimeUnitId == RuntimeUnitId;
			}))
	{
		return CharacterView->DisplayName;
	}

	if (const FFinalBattleEnemyViewData* EnemyView = Snapshot.Enemies.FindByPredicate(
			[RuntimeUnitId](const FFinalBattleEnemyViewData& Candidate)
			{
				return Candidate.RuntimeUnitId == RuntimeUnitId;
			}))
	{
		return EnemyView->DisplayName;
	}

	return RuntimeUnitId.IsNone() ? FText::GetEmpty() : FText::FromName(RuntimeUnitId);
}
}

AFinalBattleDirector::AFinalBattleDirector()
{
	PrimaryActorTick.bCanEverTick = false;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootSceneComponent);

	SummaryTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("SummaryText"));
	SummaryTextComponent->SetupAttachment(RootSceneComponent);
	SummaryTextComponent->SetHorizontalAlignment(EHTA_Center);
	SummaryTextComponent->SetVerticalAlignment(EVRTA_TextCenter);
	SummaryTextComponent->SetTextRenderColor(FColor(225, 236, 255));
	SummaryTextComponent->SetWorldSize(34.0f);
	SummaryTextComponent->SetRelativeLocation(SummaryTextOffset);
	SummaryTextComponent->SetRelativeRotation(PresentationRotation);
	SummaryTextComponent->SetText(NSLOCTEXT("FinalBattleDirector", "WaitingForBattle", "BattleDirector\n等待战斗数据"));

	PresentationActorClass = AFinalBattlePresentationActor::StaticClass();
	DefaultPlayerPresentationClass = PresentationActorClass;
	DefaultEnemyPresentationClass = PresentationActorClass;
}

void AFinalBattleDirector::BeginPlay()
{
	Super::BeginPlay();

	SummaryTextComponent->SetRelativeLocation(SummaryTextOffset);
	SummaryTextComponent->SetRelativeRotation(PresentationRotation);
	RefreshStageAnchors();

	CachedBattleFlowSubsystem = ResolveBattleFlowSubsystem();
	if (CachedBattleFlowSubsystem)
	{
		CachedBattleFlowSubsystem->OnBattleSnapshotChanged.AddDynamic(this, &AFinalBattleDirector::HandleBattleSnapshotChanged);
		CachedBattleFlowSubsystem->OnBattleEventBroadcast.AddDynamic(this, &AFinalBattleDirector::HandleBattleEventBroadcast);
		HandleBattleSnapshotChanged(CachedBattleFlowSubsystem->GetCurrentSnapshot());
	}
	else
	{
		UpdateSummaryText();
	}
}

void AFinalBattleDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CachedBattleFlowSubsystem)
	{
		CachedBattleFlowSubsystem->OnBattleSnapshotChanged.RemoveDynamic(this, &AFinalBattleDirector::HandleBattleSnapshotChanged);
		CachedBattleFlowSubsystem->OnBattleEventBroadcast.RemoveDynamic(this, &AFinalBattleDirector::HandleBattleEventBroadcast);
	}

	ClearPresentationActors();
	PlayerStageAnchorsByIndex.Reset();
	EnemyStageAnchorsByIndex.Reset();
	Super::EndPlay(EndPlayReason);
}

void AFinalBattleDirector::HandleBattleSnapshotChanged(const FFinalBattleSnapshot& Snapshot)
{
	CachedSnapshot = Snapshot;
	RefreshPresentationFromSnapshot(Snapshot);
}

void AFinalBattleDirector::HandleBattleEventBroadcast(const FFinalBattleEvent& BattleEvent)
{
	LastBattleEvent = BattleEvent;
	UpdateSummaryText();
	ApplyEventPresentation(BattleEvent);
}

UFinalBattleFlowSubsystem* AFinalBattleDirector::ResolveBattleFlowSubsystem() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalBattleFlowSubsystem>() : nullptr;
}

void AFinalBattleDirector::RefreshPresentationFromSnapshot(const FFinalBattleSnapshot& Snapshot)
{
	PresentationUnitsByRuntimeId.Reset();

	for (int32 CharacterIndex = 0; CharacterIndex < Snapshot.Characters.Num(); ++CharacterIndex)
	{
		const FFinalBattleCharacterViewData& CharacterView = Snapshot.Characters[CharacterIndex];
		if (CharacterView.RuntimeUnitId.IsNone())
		{
			continue;
		}

		FPresentationUnitState& UnitState = PresentationUnitsByRuntimeId.FindOrAdd(CharacterView.RuntimeUnitId);
		UnitState.bIsEnemy = false;
		UnitState.SlotIndex = CharacterIndex;
		UnitState.UnitDefinitionId = CharacterView.CharacterId.Value;
		UnitState.DisplayName = !CharacterView.DisplayName.IsEmpty()
			? CharacterView.DisplayName
			: FText::FromName(CharacterView.RuntimeUnitId);
		UnitState.DetailText = BuildCharacterDetailText(CharacterView);
		UnitState.bIsAlive = true;
		UnitState.bIsTargeted = Snapshot.CurrentTargetUnitId == CharacterView.RuntimeUnitId;
	}

	for (const FFinalBattleEnemyViewData& EnemyView : Snapshot.Enemies)
	{
		if (EnemyView.RuntimeUnitId.IsNone())
		{
			continue;
		}

		FPresentationUnitState& UnitState = PresentationUnitsByRuntimeId.FindOrAdd(EnemyView.RuntimeUnitId);
		UnitState.bIsEnemy = true;
		UnitState.SlotIndex = EnemyView.PositionIndex;
		UnitState.UnitDefinitionId = EnemyView.EnemyId.Value;
		UnitState.DisplayName = !EnemyView.DisplayName.IsEmpty()
			? EnemyView.DisplayName
			: FText::FromName(EnemyView.RuntimeUnitId);
		UnitState.DetailText = BuildEnemyDetailText(EnemyView);
		UnitState.bIsAlive = EnemyView.CurrentHP > 0;
		UnitState.bIsTargeted = Snapshot.CurrentTargetUnitId == EnemyView.RuntimeUnitId;
	}

	UpdateSummaryText();
	SyncPresentationActors();
}

void AFinalBattleDirector::SyncPresentationActors()
{
	TSet<FName> ActiveRuntimeUnitIds;

	for (const TPair<FName, FPresentationUnitState>& Pair : PresentationUnitsByRuntimeId)
	{
		ActiveRuntimeUnitIds.Add(Pair.Key);
		UpdatePresentationActor(Pair.Key, Pair.Value);
	}

	TArray<FName> StaleRuntimeUnitIds;
	PresentationActorsByRuntimeId.GenerateKeyArray(StaleRuntimeUnitIds);
	for (const FName RuntimeUnitId : StaleRuntimeUnitIds)
	{
		if (ActiveRuntimeUnitIds.Contains(RuntimeUnitId))
		{
			continue;
		}

		if (AFinalBattlePresentationActor* Actor = PresentationActorsByRuntimeId.FindRef(RuntimeUnitId))
		{
			Actor->Destroy();
		}

		PresentationActorsByRuntimeId.Remove(RuntimeUnitId);
	}
}

void AFinalBattleDirector::RefreshStageAnchors()
{
	PlayerStageAnchorsByIndex.Reset();
	EnemyStageAnchorsByIndex.Reset();

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	for (TActorIterator<AFinalBattleStageAnchorActor> It(World); It; ++It)
	{
		AFinalBattleStageAnchorActor* AnchorActor = *It;
		if (AnchorActor == nullptr)
		{
			continue;
		}

		TMap<int32, TObjectPtr<AFinalBattleStageAnchorActor>>& TargetMap =
			AnchorActor->GetAnchorSide() == EFinalBattleStageAnchorSide::Enemy
				? EnemyStageAnchorsByIndex
				: PlayerStageAnchorsByIndex;

		TargetMap.Add(AnchorActor->GetSlotIndex(), AnchorActor);
	}
}

void AFinalBattleDirector::UpdatePresentationActor(const FName RuntimeUnitId, const FPresentationUnitState& UnitState)
{
	AFinalBattlePresentationActor* PresentationActor = GetOrSpawnPresentationActor(RuntimeUnitId, UnitState);
	if (PresentationActor == nullptr)
	{
		return;
	}

	PresentationActor->ApplySnapshotView(
		UnitState.DisplayName,
		UnitState.DetailText,
		UnitState.bIsAlive,
		UnitState.bIsTargeted);

	PresentationActor->SetActorTransform(ResolvePresentationTransform(UnitState));
}

void AFinalBattleDirector::ApplyEventPresentation(const FFinalBattleEvent& BattleEvent)
{
	AFinalBattlePresentationActor* SourceActor = PresentationActorsByRuntimeId.FindRef(BattleEvent.SourceUnitId);
	AFinalBattlePresentationActor* TargetActor = PresentationActorsByRuntimeId.FindRef(BattleEvent.TargetUnitId);

	switch (BattleEvent.EventType)
	{
	case EFinalBattleEventType::CardResolved:
	case EFinalBattleEventType::UltimateResolved:
	case EFinalBattleEventType::EnemyActed:
		if (SourceActor)
		{
			SourceActor->PlayAttackPresentation();
		}

		if (TargetActor && BattleEvent.TargetUnitId != BattleEvent.SourceUnitId)
		{
			TargetActor->PlayHitPresentation();
		}
		break;

	default:
		break;
	}
}

void AFinalBattleDirector::ClearPresentationActors()
{
	TArray<TObjectPtr<AFinalBattlePresentationActor>> PresentationActors;
	PresentationActorsByRuntimeId.GenerateValueArray(PresentationActors);
	for (AFinalBattlePresentationActor* Actor : PresentationActors)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}

	PresentationActorsByRuntimeId.Reset();
	PresentationUnitsByRuntimeId.Reset();
}

void AFinalBattleDirector::UpdateSummaryText()
{
	if (SummaryTextComponent == nullptr)
	{
		return;
	}

	if (CachedSnapshot.BattleId.IsValid() == false)
	{
		SummaryTextComponent->SetText(NSLOCTEXT("FinalBattleDirector", "SummaryNoBattle", "BattleDirector\n当前没有活动战斗"));
		return;
	}

	const FText CurrentTargetName = ResolveUnitDisplayName(CachedSnapshot, CachedSnapshot.CurrentTargetUnitId);
	const UFinalDataRegistry* DataRegistry = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalDataRegistry>() : nullptr;
	const FinalBattleEventPresentation::FEventPresentation EventPresentation =
		FinalBattleEventPresentation::BuildPresentation(LastBattleEvent, CachedSnapshot, DataRegistry);
	const FText EventMessage = !EventPresentation.ShortWorldText.IsEmpty()
		? EventPresentation.ShortWorldText
		: NSLOCTEXT("FinalBattleDirector", "NoBattleEventYet", "尚无事件反馈");

	SummaryTextComponent->SetText(FText::Format(
		NSLOCTEXT("FinalBattleDirector", "SummaryFormat", "{0}\nRound {1} | AP {2} | EP {3}\nTarget: {4}\nLast: {5}"),
		!CachedSnapshot.EncounterDisplayName.IsEmpty()
			? CachedSnapshot.EncounterDisplayName
			: NSLOCTEXT("FinalBattleDirector", "UnnamedEncounter", "未命名遭遇"),
		FText::AsNumber(CachedSnapshot.CurrentRound),
		FText::AsNumber(CachedSnapshot.CurrentAP),
		FText::AsNumber(CachedSnapshot.CurrentEP),
		!CurrentTargetName.IsEmpty()
			? CurrentTargetName
			: NSLOCTEXT("FinalBattleDirector", "NoCurrentTarget", "无当前目标"),
		EventMessage));
}

AFinalBattlePresentationActor* AFinalBattleDirector::GetOrSpawnPresentationActor(
	const FName RuntimeUnitId,
	const FPresentationUnitState& UnitState)
{
	const TSubclassOf<AFinalBattlePresentationActor> DesiredPresentationClass = ResolvePresentationActorClass(UnitState);

	if (AFinalBattlePresentationActor* ExistingActor = PresentationActorsByRuntimeId.FindRef(RuntimeUnitId))
	{
		if (DesiredPresentationClass == nullptr || ExistingActor->IsA(DesiredPresentationClass))
		{
			return ExistingActor;
		}

		ExistingActor->Destroy();
		PresentationActorsByRuntimeId.Remove(RuntimeUnitId);
	}

	UWorld* World = GetWorld();
	if (World == nullptr || DesiredPresentationClass == nullptr)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AFinalBattlePresentationActor* SpawnedActor = World->SpawnActor<AFinalBattlePresentationActor>(
		DesiredPresentationClass,
		ResolvePresentationTransform(UnitState),
		SpawnParameters);
	if (SpawnedActor)
	{
		SpawnedActor->InitializePresentationActor(
			RuntimeUnitId,
			UnitState.bIsEnemy ? EFinalBattlePresentationTeam::Enemy : EFinalBattlePresentationTeam::Player);
		PresentationActorsByRuntimeId.Add(RuntimeUnitId, SpawnedActor);
	}

	return SpawnedActor;
}

FTransform AFinalBattleDirector::ResolvePresentationTransform(const FPresentationUnitState& UnitState) const
{
	if (const AFinalBattleStageAnchorActor* StageAnchor = ResolveStageAnchor(UnitState))
	{
		return StageAnchor->GetActorTransform();
	}

	const FVector BaseOffset = UnitState.bIsEnemy
		? EnemyPresentationOrigin
		: PlayerPresentationOrigin;
	const float SlotSpacing = UnitState.bIsEnemy ? EnemySlotSpacing : PlayerSlotSpacing;
	const FVector LocalOffset = BaseOffset + FVector(0.0f, SlotSpacing * UnitState.SlotIndex, 0.0f);
	return FTransform(GetActorRotation() + PresentationRotation, GetActorTransform().TransformPosition(LocalOffset));
}

AFinalBattleStageAnchorActor* AFinalBattleDirector::ResolveStageAnchor(const FPresentationUnitState& UnitState) const
{
	const TMap<int32, TObjectPtr<AFinalBattleStageAnchorActor>>& SourceMap =
		UnitState.bIsEnemy ? EnemyStageAnchorsByIndex : PlayerStageAnchorsByIndex;

	if (const TObjectPtr<AFinalBattleStageAnchorActor>* AnchorPtr = SourceMap.Find(UnitState.SlotIndex))
	{
		return AnchorPtr->Get();
	}

	return nullptr;
}

TSubclassOf<AFinalBattlePresentationActor> AFinalBattleDirector::ResolvePresentationActorClass(
	const FPresentationUnitState& UnitState) const
{
	const TArray<FFinalBattlePresentationClassMapping>& SourceMappings =
		UnitState.bIsEnemy ? EnemyPresentationClassMappings : PlayerPresentationClassMappings;

	if (!UnitState.UnitDefinitionId.IsNone())
	{
		if (const FFinalBattlePresentationClassMapping* Mapping = SourceMappings.FindByPredicate(
				[&UnitState](const FFinalBattlePresentationClassMapping& Candidate)
				{
					return Candidate.UnitDefinitionId == UnitState.UnitDefinitionId && Candidate.PresentationClass != nullptr;
				}))
		{
			return Mapping->PresentationClass;
		}
	}

	if (UnitState.bIsEnemy && DefaultEnemyPresentationClass != nullptr)
	{
		return DefaultEnemyPresentationClass;
	}

	if (!UnitState.bIsEnemy && DefaultPlayerPresentationClass != nullptr)
	{
		return DefaultPlayerPresentationClass;
	}

	return PresentationActorClass;
}
