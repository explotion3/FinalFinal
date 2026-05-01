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

	const FName TargetUnitId = ResolveDefaultTargetUnitId();
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

	FFinalBattleCommand Command;
	Command.CommandType = EFinalBattleCommandType::PlayCard;
	Command.CardInstanceId = CachedSnapshot.HandCards[HandIndex].CardInstanceId;
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

UFinalBattleCharacterPanelController* UFinalBattleWidgetController::GetCharacterPanelController() const
{
	return CharacterPanelController;
}

UFinalBattleEnemyPanelController* UFinalBattleWidgetController::GetEnemyPanelController() const
{
	return EnemyPanelController;
}

UFinalBattleHandPanelController* UFinalBattleWidgetController::GetHandPanelController() const
{
	return HandPanelController;
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
		LastInteractionFeedback,
		LastInteractionEvent
	};

	if (TopBarPanelController) { TopBarPanelController->RefreshFromCoordinatorData(CoordinatorData); }
	if (ResourcePanelController) { ResourcePanelController->RefreshFromCoordinatorData(CoordinatorData); }
	if (FeedbackPanelController) { FeedbackPanelController->RefreshFromCoordinatorData(CoordinatorData); }
	if (ContextPanelController) { ContextPanelController->RefreshFromCoordinatorData(CoordinatorData); }
	if (CharacterPanelController) { CharacterPanelController->RefreshFromCoordinatorData(CoordinatorData); }
	if (EnemyPanelController) { EnemyPanelController->RefreshFromCoordinatorData(CoordinatorData); }
	if (HandPanelController) { HandPanelController->RefreshFromCoordinatorData(CoordinatorData); }
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

	if (HandPanelController == nullptr)
	{
		HandPanelController = NewObject<UFinalBattleHandPanelController>(this);
		HandPanelController->InitializeHandPanel(this, ViewModel->GetHandViewModel());
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
