#include "Controllers/FinalBattleWidgetController.h"

#include "Commands/FinalBattleCommand.h"
#include "Engine/GameInstance.h"
#include "Facade/FinalBattleSession.h"
#include "Facade/FinalRunSession.h"
#include "Queries/FinalDataRegistry.h"
#include "Queries/FinalRunSnapshot.h"
#include "Subsystems/FinalBattleFlowSubsystem.h"
#include "Subsystems/FinalGameFlowSubsystem.h"
#include "Subsystems/UI/FinalUISubsystem.h"

namespace
{
const FName RejectBattleNotInitializedTag(TEXT("battle.not_initialized"));
const FName RejectInvalidTargetTag(TEXT("battle.invalid_target"));
const FName RejectUnsupportedCommandTag(TEXT("battle.unsupported_command"));

FFinalBattleEvent BuildLocalRejectEvent(const FText& Message, const EFinalBattleCommandRejectReason RejectReason, const FName ReasonTag)
{
	FFinalBattleEvent Event;
	Event.EventType = EFinalBattleEventType::CommandRejected;
	Event.RejectReason = RejectReason;
	Event.ReasonTag = ReasonTag;
	Event.Message = Message;
	return Event;
}
}

void UFinalBattleWidgetController::Initialize(UFinalBattleHUDViewModel* InViewModel)
{
	ViewModel = InViewModel;
	CachedSnapshot = FFinalBattleSnapshot{};
	CachedBattleEvents.Reset();
	LastInteractionFeedback = FText::GetEmpty();
	LastInteractionEvent = FFinalBattleEvent{};
	SelectedEnemyUnitId = NAME_None;
	InspectedEnemyUnitId = NAME_None;
	InspectedCharacterUnitId = NAME_None;
	bCardZoneDetailOpen = false;
	SelectedCardZone = EFinalBattleCardZone::DrawPile;

	if (ViewModel)
	{
		ViewModel->EnsurePanelViewModels();
	}

	EnsurePanelControllers();
	RebuildPresentation();
}

void UFinalBattleWidgetController::BindToBattleFlow(UFinalBattleFlowSubsystem* InBattleFlowSubsystem)
{
	if (BattleFlowSubsystem == InBattleFlowSubsystem)
	{
		return;
	}

	UnbindFromBattleFlow();
	BattleFlowSubsystem = InBattleFlowSubsystem;
	CachedBattleEvents.Reset();
	CachedSnapshot = FFinalBattleSnapshot{};
	LastInteractionFeedback = FText::GetEmpty();
	LastInteractionEvent = FFinalBattleEvent{};
	SelectedEnemyUnitId = NAME_None;
	InspectedEnemyUnitId = NAME_None;
	InspectedCharacterUnitId = NAME_None;
	bCardZoneDetailOpen = false;
	SelectedCardZone = EFinalBattleCardZone::DrawPile;

	if (BattleFlowSubsystem == nullptr)
	{
		RebuildPresentation();
		return;
	}

	BattleFlowSubsystem->OnBattleSnapshotChanged.AddDynamic(this, &UFinalBattleWidgetController::HandleBattleSnapshotChanged);
	BattleFlowSubsystem->OnBattleEventBroadcast.AddDynamic(this, &UFinalBattleWidgetController::HandleBattleEventBroadcast);

	HandleBattleSnapshotChanged(BattleFlowSubsystem->GetCurrentSnapshot());
	for (const FFinalBattleEvent& BattleEvent : BattleFlowSubsystem->GetBattleLogEntries())
	{
		HandleBattleEventBroadcast(BattleEvent);
	}
}

void UFinalBattleWidgetController::UnbindFromBattleFlow()
{
	if (BattleFlowSubsystem)
	{
		BattleFlowSubsystem->OnBattleSnapshotChanged.RemoveDynamic(this, &UFinalBattleWidgetController::HandleBattleSnapshotChanged);
		BattleFlowSubsystem->OnBattleEventBroadcast.RemoveDynamic(this, &UFinalBattleWidgetController::HandleBattleEventBroadcast);
	}

	BattleFlowSubsystem = nullptr;
	CachedSnapshot = FFinalBattleSnapshot{};
	CachedBattleEvents.Reset();
	LastInteractionFeedback = FText::GetEmpty();
	LastInteractionEvent = FFinalBattleEvent{};
	SelectedEnemyUnitId = NAME_None;
	InspectedEnemyUnitId = NAME_None;
	InspectedCharacterUnitId = NAME_None;
	bCardZoneDetailOpen = false;
	SelectedCardZone = EFinalBattleCardZone::DrawPile;
}

void UFinalBattleWidgetController::RefreshFromSession(UFinalBattleSession* Session)
{
	if (!ViewModel || !Session)
	{
		return;
	}

	CachedSnapshot = Session->GetSnapshot();
	CachedBattleEvents = Session->GetBattleLogEntries();
	LastInteractionFeedback = CachedBattleEvents.Num() > 0 ? CachedBattleEvents.Last().Message : FText::GetEmpty();
	LastInteractionEvent = CachedBattleEvents.Num() > 0 ? CachedBattleEvents.Last() : FFinalBattleEvent{};
	RefreshSelectedEnemyFromSnapshot();
	RefreshInspectedEnemyFromSnapshot();
	RefreshInspectedCharacterFromSnapshot();
	RefreshInspectedCardZoneFromSnapshot();
	ViewModel->ApplySnapshot(CachedSnapshot);
	for (const FFinalBattleEvent& BattleEvent : CachedBattleEvents)
	{
		ViewModel->ApplyBattleEvent(BattleEvent);
	}

	RebuildPresentation();
}

UFinalBattleHUDViewModel* UFinalBattleWidgetController::GetViewModel() const
{
	return ViewModel;
}

bool UFinalBattleWidgetController::SelectEnemyByUnitId(const FName RuntimeUnitId)
{
	if (BattleFlowSubsystem == nullptr || BattleFlowSubsystem->GetActiveBattleSession() == nullptr)
	{
		LastInteractionEvent = BuildLocalRejectEvent(
			FText::FromString(TEXT("当前没有可操作的战斗。")),
			EFinalBattleCommandRejectReason::BattleNotInitialized,
			RejectBattleNotInitializedTag);
		LastInteractionFeedback = LastInteractionEvent.Message;
		RebuildPresentation();
		return false;
	}

	const bool bExists = CachedSnapshot.Enemies.ContainsByPredicate(
		[&RuntimeUnitId](const FFinalBattleEnemyViewData& Candidate)
		{
			return Candidate.RuntimeUnitId == RuntimeUnitId && Candidate.CurrentHP > 0;
		});

	if (!bExists)
	{
		LastInteractionEvent = BuildLocalRejectEvent(
			FText::FromString(TEXT("当前无法选择该敌人目标。")),
			EFinalBattleCommandRejectReason::InvalidTarget,
			RejectInvalidTargetTag);
		LastInteractionFeedback = LastInteractionEvent.Message;
		RebuildPresentation();
		return false;
	}

	if (CachedSnapshot.CurrentTargetUnitId == RuntimeUnitId)
	{
		return true;
	}

	FFinalBattleCommand Command;
	Command.CommandType = EFinalBattleCommandType::SelectTarget;
	Command.TargetUnitId = RuntimeUnitId;
	return SubmitBattleCommandWithFeedback(Command);
}

bool UFinalBattleWidgetController::RequestPlayCardByHandIndex(const int32 HandIndex)
{
	if (BattleFlowSubsystem == nullptr || BattleFlowSubsystem->GetActiveBattleSession() == nullptr)
	{
		LastInteractionEvent = BuildLocalRejectEvent(
			FText::FromString(TEXT("当前没有可操作的战斗。")),
			EFinalBattleCommandRejectReason::BattleNotInitialized,
			RejectBattleNotInitializedTag);
		LastInteractionFeedback = LastInteractionEvent.Message;
		RebuildPresentation();
		return false;
	}

	if (!CachedSnapshot.HandCards.IsValidIndex(HandIndex))
	{
		LastInteractionEvent = BuildLocalRejectEvent(
			FText::FromString(TEXT("手牌索引无效。")),
			EFinalBattleCommandRejectReason::UnsupportedCommand,
			RejectUnsupportedCommandTag);
		LastInteractionFeedback = LastInteractionEvent.Message;
		RebuildPresentation();
		return false;
	}

	const FFinalBattleCardViewData& CardView = CachedSnapshot.HandCards[HandIndex];
	FName TargetUnitId = NAME_None;
	if (CardView.TargetRequirement == EFinalBattleCardTargetRequirement::Enemy)
	{
		TargetUnitId = ResolveDefaultTargetUnitId();
		if (TargetUnitId.IsNone())
		{
			LastInteractionEvent = BuildLocalRejectEvent(
				FText::FromString(TEXT("当前没有可选中的敌人目标。")),
				EFinalBattleCommandRejectReason::InvalidTarget,
				RejectInvalidTargetTag);
			LastInteractionFeedback = LastInteractionEvent.Message;
			RebuildPresentation();
			return false;
		}
	}

	FFinalBattleCommand Command;
	Command.CommandType = EFinalBattleCommandType::PlayCard;
	Command.CardInstanceId = CardView.CardInstanceId;
	Command.TargetUnitId = TargetUnitId;
	return SubmitBattleCommandWithFeedback(Command);
}

bool UFinalBattleWidgetController::PlayCardByHandIndex(const int32 HandIndex)
{
	return RequestPlayCardByHandIndex(HandIndex);
}

bool UFinalBattleWidgetController::PlayUltimateByCharacterIndex(const int32 CharacterIndex)
{
	if (BattleFlowSubsystem == nullptr || BattleFlowSubsystem->GetActiveBattleSession() == nullptr)
	{
		LastInteractionEvent = BuildLocalRejectEvent(
			FText::FromString(TEXT("当前没有可操作的战斗。")),
			EFinalBattleCommandRejectReason::BattleNotInitialized,
			RejectBattleNotInitializedTag);
		LastInteractionFeedback = LastInteractionEvent.Message;
		RebuildPresentation();
		return false;
	}

	if (!CachedSnapshot.CharacterUltimates.IsValidIndex(CharacterIndex))
	{
		LastInteractionEvent = BuildLocalRejectEvent(
			FText::FromString(TEXT("奥义索引无效。")),
			EFinalBattleCommandRejectReason::UnsupportedCommand,
			RejectUnsupportedCommandTag);
		LastInteractionFeedback = LastInteractionEvent.Message;
		RebuildPresentation();
		return false;
	}

	FFinalBattleCommand Command;
	Command.CommandType = EFinalBattleCommandType::PlayUltimate;
	Command.UltimateOwnerUnitId = CachedSnapshot.CharacterUltimates[CharacterIndex].OwnerUnitId;
	return SubmitBattleCommandWithFeedback(Command);
}

bool UFinalBattleWidgetController::EndTurn()
{
	FFinalBattleCommand Command;
	Command.CommandType = EFinalBattleCommandType::EndTurn;
	return SubmitBattleCommandWithFeedback(Command);
}

FName UFinalBattleWidgetController::GetSelectedEnemyUnitId() const
{
	return SelectedEnemyUnitId;
}

bool UFinalBattleWidgetController::InspectEnemyByUnitId(const FName RuntimeUnitId)
{
	const bool bExists = CachedSnapshot.Enemies.ContainsByPredicate(
		[&RuntimeUnitId](const FFinalBattleEnemyViewData& Candidate)
		{
			return Candidate.RuntimeUnitId == RuntimeUnitId;
		});

	if (!bExists)
	{
		InspectedEnemyUnitId = NAME_None;
		RebuildPresentation();
		return false;
	}

	InspectedEnemyUnitId = RuntimeUnitId;
	InspectedCharacterUnitId = NAME_None;
	RebuildPresentation();
	return true;
}

void UFinalBattleWidgetController::ClearInspectedEnemy()
{
	if (InspectedEnemyUnitId.IsNone())
	{
		return;
	}

	InspectedEnemyUnitId = NAME_None;
	RebuildPresentation();
}

FName UFinalBattleWidgetController::GetInspectedEnemyUnitId() const
{
	return InspectedEnemyUnitId;
}

bool UFinalBattleWidgetController::InspectCharacterByUnitId(const FName RuntimeUnitId)
{
	const bool bExists = CachedSnapshot.Characters.ContainsByPredicate(
		[&RuntimeUnitId](const FFinalBattleCharacterViewData& Candidate)
		{
			return Candidate.RuntimeUnitId == RuntimeUnitId;
		});

	if (!bExists)
	{
		InspectedCharacterUnitId = NAME_None;
		RebuildPresentation();
		return false;
	}

	InspectedCharacterUnitId = RuntimeUnitId;
	InspectedEnemyUnitId = NAME_None;
	RebuildPresentation();
	return true;
}

void UFinalBattleWidgetController::ClearInspectedCharacter()
{
	if (InspectedCharacterUnitId.IsNone())
	{
		return;
	}

	InspectedCharacterUnitId = NAME_None;
	RebuildPresentation();
}

FName UFinalBattleWidgetController::GetInspectedCharacterUnitId() const
{
	return InspectedCharacterUnitId;
}

void UFinalBattleWidgetController::InspectCardZone(const EFinalBattleCardZone Zone)
{
	bCardZoneDetailOpen = true;
	SelectedCardZone = Zone;
	RebuildPresentation();
}

void UFinalBattleWidgetController::SetSelectedCardZone(const EFinalBattleCardZone Zone)
{
	if (!bCardZoneDetailOpen)
	{
		bCardZoneDetailOpen = true;
	}

	SelectedCardZone = Zone;
	RebuildPresentation();
}

void UFinalBattleWidgetController::ClearCardZoneDetail()
{
	if (!bCardZoneDetailOpen)
	{
		return;
	}

	bCardZoneDetailOpen = false;
	RebuildPresentation();
}

bool UFinalBattleWidgetController::IsCardZoneDetailOpen() const
{
	return bCardZoneDetailOpen;
}

EFinalBattleCardZone UFinalBattleWidgetController::GetSelectedCardZone() const
{
	return SelectedCardZone;
}

void UFinalBattleWidgetController::OpenTeamStatusDetail()
{
	if (bTeamStatusDetailOpen)
	{
		return;
	}

	bTeamStatusDetailOpen = true;
	RebuildPresentation();
}

void UFinalBattleWidgetController::ClearTeamStatusDetail()
{
	if (!bTeamStatusDetailOpen)
	{
		return;
	}

	bTeamStatusDetailOpen = false;
	RebuildPresentation();
}

bool UFinalBattleWidgetController::IsTeamStatusDetailOpen() const
{
	return bTeamStatusDetailOpen;
}

FText UFinalBattleWidgetController::GetLastInteractionFeedback() const
{
	return LastInteractionFeedback;
}

void UFinalBattleWidgetController::OpenDebugOverlay()
{
	if (UGameInstance* GameInstance = GetTypedOuter<UGameInstance>())
	{
		if (UFinalUISubsystem* UISubsystem = GameInstance->GetSubsystem<UFinalUISubsystem>())
		{
			UISubsystem->OpenPrototypeRunDebugOverlay();
		}
	}
}

void UFinalBattleWidgetController::OpenEventLedgerOverlay()
{
	if (UGameInstance* GameInstance = GetTypedOuter<UGameInstance>())
	{
		if (UFinalUISubsystem* UISubsystem = GameInstance->GetSubsystem<UFinalUISubsystem>())
		{
			UISubsystem->OpenBattleEventOverlay();
		}
	}
}

UFinalBattleTopBarPanelController* UFinalBattleWidgetController::GetTopBarPanelController() const
{
	return TopBarPanelController;
}

UFinalBattleResourcePanelController* UFinalBattleWidgetController::GetResourcePanelController() const
{
	return ResourcePanelController;
}

UFinalBattleFeedbackPanelController* UFinalBattleWidgetController::GetFeedbackPanelController() const
{
	return FeedbackPanelController;
}

UFinalBattleContextPanelController* UFinalBattleWidgetController::GetContextPanelController() const
{
	return ContextPanelController;
}

UFinalBattleTeamPanelController* UFinalBattleWidgetController::GetTeamPanelController() const
{
	return TeamPanelController;
}

UFinalBattleTeamStatusDetailPanelController* UFinalBattleWidgetController::GetTeamStatusDetailPanelController() const
{
	return TeamStatusDetailPanelController;
}

UFinalBattleCharacterPanelController* UFinalBattleWidgetController::GetCharacterPanelController() const
{
	return CharacterPanelController;
}

UFinalBattleEnemyPanelController* UFinalBattleWidgetController::GetEnemyPanelController() const
{
	return EnemyPanelController;
}

UFinalBattleEnemyDetailPanelController* UFinalBattleWidgetController::GetEnemyDetailPanelController() const
{
	return EnemyDetailPanelController;
}

UFinalBattleCharacterDetailPanelController* UFinalBattleWidgetController::GetCharacterDetailPanelController() const
{
	return CharacterDetailPanelController;
}

UFinalBattleHandPanelController* UFinalBattleWidgetController::GetHandPanelController() const
{
	return HandPanelController;
}

UFinalBattleCardZoneDetailPanelController* UFinalBattleWidgetController::GetCardZoneDetailPanelController() const
{
	return CardZoneDetailPanelController;
}

UFinalBattleUltimatePanelController* UFinalBattleWidgetController::GetUltimatePanelController() const
{
	return UltimatePanelController;
}

UFinalBattleRecentEventPanelController* UFinalBattleWidgetController::GetRecentEventPanelController() const
{
	return RecentEventPanelController;
}

UFinalBattleActionPanelController* UFinalBattleWidgetController::GetActionPanelController() const
{
	return ActionPanelController;
}

void UFinalBattleWidgetController::ShutdownController()
{
	UnbindFromBattleFlow();
}

void UFinalBattleWidgetController::HandleBattleSnapshotChanged(const FFinalBattleSnapshot& Snapshot)
{
	if (ViewModel == nullptr)
	{
		return;
	}

	CachedSnapshot = Snapshot;
	if (BattleFlowSubsystem && BattleFlowSubsystem->GetActiveBattleSession() == nullptr)
	{
		CachedBattleEvents.Reset();
		LastInteractionFeedback = FText::GetEmpty();
		LastInteractionEvent = FFinalBattleEvent{};
	}

	RefreshSelectedEnemyFromSnapshot();
	RefreshInspectedEnemyFromSnapshot();
	RefreshInspectedCharacterFromSnapshot();
	RefreshInspectedCardZoneFromSnapshot();
	ViewModel->ApplySnapshot(CachedSnapshot);
	RebuildPresentation();
}

void UFinalBattleWidgetController::HandleBattleEventBroadcast(const FFinalBattleEvent& BattleEvent)
{
	if (ViewModel == nullptr)
	{
		return;
	}

	CachedBattleEvents.Add(BattleEvent);
	LastInteractionEvent = BattleEvent;
	LastInteractionFeedback = BattleEvent.Message;
	ViewModel->ApplyBattleEvent(BattleEvent);
	RebuildPresentation();
}

void UFinalBattleWidgetController::RebuildPresentation()
{
	if (ViewModel == nullptr)
	{
		return;
	}

	EnsurePanelControllers();
	ViewModel->EnsurePanelViewModels();
	ViewModel->ApplySnapshot(CachedSnapshot);

	UFinalDataRegistry* DataRegistry = nullptr;
	UFinalGameFlowSubsystem* GameFlowSubsystem = nullptr;
	UFinalRunSession* RunSession = nullptr;
	if (UGameInstance* GameInstance = GetTypedOuter<UGameInstance>())
	{
		DataRegistry = GameInstance->GetSubsystem<UFinalDataRegistry>();
		GameFlowSubsystem = GameInstance->GetSubsystem<UFinalGameFlowSubsystem>();
		RunSession = GameFlowSubsystem ? GameFlowSubsystem->GetRunSession() : nullptr;
	}

	const FFinalRunSnapshot RunSnapshot = RunSession ? RunSession->GetSnapshot() : FFinalRunSnapshot{};
	const FFinalBattleHUDCoordinatorData CoordinatorData{
		&CachedSnapshot,
		&RunSnapshot,
		&CachedBattleEvents,
		DataRegistry,
		SelectedEnemyUnitId,
		InspectedEnemyUnitId,
		InspectedCharacterUnitId,
		bCardZoneDetailOpen,
		SelectedCardZone,
		bTeamStatusDetailOpen,
		LastInteractionFeedback,
		LastInteractionEvent
	};

	if (TopBarPanelController) { TopBarPanelController->RefreshFromCoordinatorData(CoordinatorData); }
	if (ResourcePanelController) { ResourcePanelController->RefreshFromCoordinatorData(CoordinatorData); }
	if (FeedbackPanelController) { FeedbackPanelController->RefreshFromCoordinatorData(CoordinatorData); }
	if (ContextPanelController) { ContextPanelController->RefreshFromCoordinatorData(CoordinatorData); }
	if (TeamPanelController) { TeamPanelController->RefreshFromCoordinatorData(CoordinatorData); }
	if (TeamStatusDetailPanelController) { TeamStatusDetailPanelController->RefreshFromCoordinatorData(CoordinatorData); }
	if (CharacterPanelController) { CharacterPanelController->RefreshFromCoordinatorData(CoordinatorData); }
	if (EnemyPanelController) { EnemyPanelController->RefreshFromCoordinatorData(CoordinatorData); }
	if (EnemyDetailPanelController) { EnemyDetailPanelController->RefreshFromCoordinatorData(CoordinatorData); }
	if (CharacterDetailPanelController) { CharacterDetailPanelController->RefreshFromCoordinatorData(CoordinatorData); }
	if (HandPanelController) { HandPanelController->RefreshFromCoordinatorData(CoordinatorData); }
	if (CardZoneDetailPanelController) { CardZoneDetailPanelController->RefreshFromCoordinatorData(CoordinatorData); }
	if (UltimatePanelController) { UltimatePanelController->RefreshFromCoordinatorData(CoordinatorData); }
	if (RecentEventPanelController) { RecentEventPanelController->RefreshFromCoordinatorData(CoordinatorData); }
	if (ActionPanelController) { ActionPanelController->RefreshFromCoordinatorData(CoordinatorData); }
}

void UFinalBattleWidgetController::EnsurePanelControllers()
{
	if (ViewModel == nullptr)
	{
		return;
	}

	ViewModel->EnsurePanelViewModels();

	if (TopBarPanelController == nullptr)
	{
		TopBarPanelController = NewObject<UFinalBattleTopBarPanelController>(this);
		TopBarPanelController->InitializeTopBar(this, ViewModel->GetTopBarViewModel());
	}

	if (ResourcePanelController == nullptr)
	{
		ResourcePanelController = NewObject<UFinalBattleResourcePanelController>(this);
		ResourcePanelController->InitializeResourcePanel(this, ViewModel->GetResourceViewModel());
	}

	if (FeedbackPanelController == nullptr)
	{
		FeedbackPanelController = NewObject<UFinalBattleFeedbackPanelController>(this);
		FeedbackPanelController->InitializeFeedback(this, ViewModel->GetFeedbackViewModel());
	}

	if (ContextPanelController == nullptr)
	{
		ContextPanelController = NewObject<UFinalBattleContextPanelController>(this);
		ContextPanelController->InitializeContext(this, ViewModel->GetContextViewModel());
	}

	if (TeamPanelController == nullptr)
	{
		TeamPanelController = NewObject<UFinalBattleTeamPanelController>(this);
		TeamPanelController->InitializeTeamPanel(this, ViewModel->GetTeamViewModel());
	}

	if (TeamStatusDetailPanelController == nullptr)
	{
		TeamStatusDetailPanelController = NewObject<UFinalBattleTeamStatusDetailPanelController>(this);
		TeamStatusDetailPanelController->InitializeTeamStatusDetailPanel(this, ViewModel->GetTeamStatusDetailViewModel());
	}

	if (CharacterPanelController == nullptr)
	{
		CharacterPanelController = NewObject<UFinalBattleCharacterPanelController>(this);
		CharacterPanelController->InitializeCharacterPanel(this, ViewModel->GetCharacterViewModel());
	}

	if (EnemyPanelController == nullptr)
	{
		EnemyPanelController = NewObject<UFinalBattleEnemyPanelController>(this);
		EnemyPanelController->InitializeEnemyPanel(this, ViewModel->GetEnemyViewModel());
	}

	if (EnemyDetailPanelController == nullptr)
	{
		EnemyDetailPanelController = NewObject<UFinalBattleEnemyDetailPanelController>(this);
		EnemyDetailPanelController->InitializeEnemyDetailPanel(this, ViewModel->GetEnemyDetailViewModel());
	}

	if (CharacterDetailPanelController == nullptr)
	{
		CharacterDetailPanelController = NewObject<UFinalBattleCharacterDetailPanelController>(this);
		CharacterDetailPanelController->InitializeCharacterDetailPanel(this, ViewModel->GetCharacterDetailViewModel());
	}

	if (HandPanelController == nullptr)
	{
		HandPanelController = NewObject<UFinalBattleHandPanelController>(this);
		HandPanelController->InitializeHandPanel(this, ViewModel->GetHandViewModel());
	}

	if (CardZoneDetailPanelController == nullptr)
	{
		CardZoneDetailPanelController = NewObject<UFinalBattleCardZoneDetailPanelController>(this);
		CardZoneDetailPanelController->InitializeCardZoneDetailPanel(this, ViewModel->GetCardZoneDetailViewModel());
	}

	if (UltimatePanelController == nullptr)
	{
		UltimatePanelController = NewObject<UFinalBattleUltimatePanelController>(this);
		UltimatePanelController->InitializeUltimatePanel(this, ViewModel->GetUltimateViewModel());
	}

	if (RecentEventPanelController == nullptr)
	{
		RecentEventPanelController = NewObject<UFinalBattleRecentEventPanelController>(this);
		RecentEventPanelController->InitializeRecentEventPanel(this, ViewModel->GetRecentEventViewModel());
	}

	if (ActionPanelController == nullptr)
	{
		ActionPanelController = NewObject<UFinalBattleActionPanelController>(this);
		ActionPanelController->InitializeActionPanel(this, ViewModel->GetActionViewModel());
	}
}

void UFinalBattleWidgetController::RefreshSelectedEnemyFromSnapshot()
{
	const bool bSnapshotTargetIsAlive = CachedSnapshot.Enemies.ContainsByPredicate(
		[this](const FFinalBattleEnemyViewData& Candidate)
		{
			return Candidate.RuntimeUnitId == CachedSnapshot.CurrentTargetUnitId && Candidate.CurrentHP > 0;
		});

	SelectedEnemyUnitId = bSnapshotTargetIsAlive ? CachedSnapshot.CurrentTargetUnitId : NAME_None;
}

void UFinalBattleWidgetController::RefreshInspectedEnemyFromSnapshot()
{
	if (InspectedEnemyUnitId.IsNone())
	{
		return;
	}

	const bool bInspectedEnemyStillExists = CachedSnapshot.Enemies.ContainsByPredicate(
		[this](const FFinalBattleEnemyViewData& Candidate)
		{
			return Candidate.RuntimeUnitId == InspectedEnemyUnitId;
		});

	if (!bInspectedEnemyStillExists)
	{
		InspectedEnemyUnitId = NAME_None;
	}
}

void UFinalBattleWidgetController::RefreshInspectedCharacterFromSnapshot()
{
	if (InspectedCharacterUnitId.IsNone())
	{
		return;
	}

	const bool bInspectedCharacterStillExists = CachedSnapshot.Characters.ContainsByPredicate(
		[this](const FFinalBattleCharacterViewData& Candidate)
		{
			return Candidate.RuntimeUnitId == InspectedCharacterUnitId;
		});

	if (!bInspectedCharacterStillExists)
	{
		InspectedCharacterUnitId = NAME_None;
	}
}

void UFinalBattleWidgetController::RefreshInspectedCardZoneFromSnapshot()
{
	if (!bCardZoneDetailOpen)
	{
		return;
	}

	const bool bSelectedZoneExists = CachedSnapshot.CardZones.ContainsByPredicate(
		[this](const FFinalBattleCardZoneViewData& Candidate)
		{
			return Candidate.Zone == SelectedCardZone;
		});

	if (!bSelectedZoneExists)
	{
		bCardZoneDetailOpen = false;
		SelectedCardZone = EFinalBattleCardZone::DrawPile;
	}
}

FName UFinalBattleWidgetController::ResolveDefaultTargetUnitId() const
{
	if (SelectedEnemyUnitId != NAME_None)
	{
		return SelectedEnemyUnitId;
	}

	const FFinalBattleEnemyViewData* AliveEnemy = CachedSnapshot.Enemies.FindByPredicate(
		[](const FFinalBattleEnemyViewData& Candidate)
		{
			return Candidate.CurrentHP > 0;
		});

	return AliveEnemy ? AliveEnemy->RuntimeUnitId : NAME_None;
}

bool UFinalBattleWidgetController::SubmitBattleCommandWithFeedback(const FFinalBattleCommand& Command)
{
	if (BattleFlowSubsystem == nullptr)
	{
		LastInteractionEvent = BuildLocalRejectEvent(
			FText::FromString(TEXT("BattleFlowSubsystem 不可用。")),
			EFinalBattleCommandRejectReason::BattleNotInitialized,
			RejectBattleNotInitializedTag);
		LastInteractionFeedback = LastInteractionEvent.Message;
		RebuildPresentation();
		return false;
	}

	const bool bAccepted = BattleFlowSubsystem->SubmitBattleCommand(Command);
	const FFinalBattleEvent Event = BattleFlowSubsystem->GetLastCommandEvent();
	LastInteractionEvent = Event;
	LastInteractionFeedback = Event.Message.IsEmpty() ? BattleFlowSubsystem->GetLastFailureReason() : Event.Message;

	if (!bAccepted)
	{
		RebuildPresentation();
	}

	return bAccepted;
}
