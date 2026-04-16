#include "World/FinalBattleDirector.h"

#include "BattleBridge/FinalBattleEventPresentationUtils.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/TextRenderActor.h"
#include "Queries/FinalDataRegistry.h"
#include "Subsystems/FinalBattleFlowSubsystem.h"

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

FText GetBattleDirectorRelicEffectTypeText(const EFinalRelicBattleStartEffectType EffectType)
{
	switch (EffectType)
	{
	case EFinalRelicBattleStartEffectType::GainAP:
		return NSLOCTEXT("FinalBattleDirector", "RelicEffectGainAP", "GainAP");

	case EFinalRelicBattleStartEffectType::GainShield:
		return NSLOCTEXT("FinalBattleDirector", "RelicEffectGainShield", "GainShield");

	case EFinalRelicBattleStartEffectType::None:
	default:
		return NSLOCTEXT("FinalBattleDirector", "RelicEffectNone", "None");
	}
}

FText ResolveRelicDisplayName(const FFinalBattleSnapshot& Snapshot, const FFinalRelicId& RelicId)
{
	const FFinalBattleStartRelicInput* RelicInput = Snapshot.ActiveRelics.FindByPredicate(
		[&RelicId](const FFinalBattleStartRelicInput& Candidate)
		{
			return Candidate.RelicId == RelicId;
		});

	if (RelicInput != nullptr)
	{
		if (!RelicInput->DisplayName.IsEmpty())
		{
			return RelicInput->DisplayName;
		}

		if (!RelicInput->DisplayId.IsNone())
		{
			return FText::FromName(RelicInput->DisplayId);
		}
	}

	return RelicId.IsValid()
		? FText::FromName(RelicId.Value)
		: FText::GetEmpty();
}

FText BuildRelicEffectSummaryText(const FFinalBattleSnapshot& Snapshot, const FFinalRelicId& RelicId)
{
	const FFinalBattleStartRelicInput* RelicInput = Snapshot.ActiveRelics.FindByPredicate(
		[&RelicId](const FFinalBattleStartRelicInput& Candidate)
		{
			return Candidate.RelicId == RelicId;
		});

	if (RelicInput == nullptr || RelicInput->BattleStartEffects.IsEmpty())
	{
		return FText::GetEmpty();
	}

	TArray<FString> Segments;
	Segments.Reserve(RelicInput->BattleStartEffects.Num());
	for (const FFinalBattleStartRelicEffectInput& EffectInput : RelicInput->BattleStartEffects)
	{
		const FString EffectTypeString = GetBattleDirectorRelicEffectTypeText(EffectInput.EffectType).ToString();
		Segments.Add(FString::Printf(
			TEXT("%s +%d"),
			*EffectTypeString,
			EffectInput.Value));
	}

	return FText::FromString(FString::Join(Segments, TEXT(" | ")));
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

	return RuntimeUnitId.IsNone()
		? FText::GetEmpty()
		: FText::FromName(RuntimeUnitId);
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
}

void AFinalBattleDirector::BeginPlay()
{
	Super::BeginPlay();

	SummaryTextComponent->SetRelativeLocation(SummaryTextOffset);
	SummaryTextComponent->SetRelativeRotation(PresentationRotation);

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
	SyncPresentationActors();
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

		if (ATextRenderActor* Actor = PresentationActorsByRuntimeId.FindRef(RuntimeUnitId))
		{
			Actor->Destroy();
		}

		PresentationActorsByRuntimeId.Remove(RuntimeUnitId);
	}
}

void AFinalBattleDirector::UpdatePresentationActor(const FName RuntimeUnitId, const FPresentationUnitState& UnitState)
{
	ATextRenderActor* PresentationActor = GetOrSpawnPresentationActor(RuntimeUnitId);
	if (PresentationActor == nullptr)
	{
		return;
	}

	const bool bWasEventSource = LastBattleEvent.SourceUnitId == RuntimeUnitId;
	const bool bWasEventTarget = LastBattleEvent.TargetUnitId == RuntimeUnitId;

	FString HeaderLine;
	if (bWasEventSource)
	{
		HeaderLine += TEXT("[Act] ");
	}
	if (UnitState.bIsTargeted)
	{
		HeaderLine += TEXT("[Target] ");
	}
	if (bWasEventTarget)
	{
		HeaderLine += TEXT("[Impact] ");
	}

	HeaderLine += UnitState.DisplayName.IsEmpty()
		? RuntimeUnitId.ToString()
		: UnitState.DisplayName.ToString();

	const FString DetailText = UnitState.DetailText.IsEmpty()
		? FString(TEXT("无公开表现字段"))
		: UnitState.DetailText.ToString();

	if (UTextRenderComponent* TextRender = PresentationActor->GetTextRender())
	{
		TextRender->SetText(FText::FromString(FString::Printf(TEXT("%s\n%s"), *HeaderLine, *DetailText)));
		TextRender->SetHorizontalAlignment(EHTA_Center);
		TextRender->SetVerticalAlignment(EVRTA_TextCenter);
		TextRender->SetWorldSize(UnitTextWorldSize);
		TextRender->SetTextRenderColor(ResolvePresentationColor(RuntimeUnitId, UnitState).ToFColor(true));
	}

	PresentationActor->SetActorLocation(ResolvePresentationLocation(UnitState));
	PresentationActor->SetActorRotation(GetActorRotation() + PresentationRotation);
}

void AFinalBattleDirector::ClearPresentationActors()
{
	TArray<TObjectPtr<ATextRenderActor>> PresentationActors;
	PresentationActorsByRuntimeId.GenerateValueArray(PresentationActors);
	for (ATextRenderActor* Actor : PresentationActors)
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

ATextRenderActor* AFinalBattleDirector::GetOrSpawnPresentationActor(const FName RuntimeUnitId)
{
	if (ATextRenderActor* ExistingActor = PresentationActorsByRuntimeId.FindRef(RuntimeUnitId))
	{
		return ExistingActor;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATextRenderActor* SpawnedActor = World->SpawnActor<ATextRenderActor>(
		ATextRenderActor::StaticClass(),
		GetActorLocation(),
		GetActorRotation() + PresentationRotation,
		SpawnParameters);
	if (SpawnedActor)
	{
		PresentationActorsByRuntimeId.Add(RuntimeUnitId, SpawnedActor);
	}

	return SpawnedActor;
}

FVector AFinalBattleDirector::ResolvePresentationLocation(const FPresentationUnitState& UnitState) const
{
	const FVector BaseOffset = UnitState.bIsEnemy
		? EnemyPresentationOrigin
		: PlayerPresentationOrigin;
	const float SlotSpacing = UnitState.bIsEnemy ? EnemySlotSpacing : PlayerSlotSpacing;
	const FVector LocalOffset = BaseOffset + FVector(0.0f, SlotSpacing * UnitState.SlotIndex, 0.0f);
	return GetActorTransform().TransformPosition(LocalOffset);
}

FLinearColor AFinalBattleDirector::ResolvePresentationColor(const FName RuntimeUnitId, const FPresentationUnitState& UnitState) const
{
	if (!UnitState.bIsAlive)
	{
		return FLinearColor(0.45f, 0.45f, 0.45f, 1.0f);
	}

	if (UnitState.bIsTargeted)
	{
		return FLinearColor(1.0f, 0.82f, 0.2f, 1.0f);
	}

	if (LastBattleEvent.SourceUnitId == RuntimeUnitId)
	{
		return FLinearColor(0.22f, 0.86f, 0.95f, 1.0f);
	}

	if (LastBattleEvent.TargetUnitId == RuntimeUnitId)
	{
		return FLinearColor(1.0f, 0.52f, 0.24f, 1.0f);
	}

	return UnitState.bIsEnemy
		? FLinearColor(1.0f, 0.40f, 0.40f, 1.0f)
		: FLinearColor(0.38f, 0.78f, 1.0f, 1.0f);
}
