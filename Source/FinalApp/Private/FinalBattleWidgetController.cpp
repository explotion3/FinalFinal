#include "Controllers/FinalBattleWidgetController.h"

#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Facade/FinalBattleSession.h"
#include "Facade/FinalRunSession.h"
#include "Queries/FinalDataRegistry.h"
#include "Runtime/FinalRunState.h"
#include "Subsystems/FinalBattleFlowSubsystem.h"
#include "Subsystems/FinalGameFlowSubsystem.h"
#include "ViewModels/FinalBattleHUDViewModel.h"

void UFinalBattleWidgetController::Initialize(UFinalBattleHUDViewModel* InViewModel)
{
	ViewModel = InViewModel;
	CachedSnapshot = FFinalBattleSnapshot{};
	CachedBattleEvents.Reset();
	LastInteractionFeedback = FText::GetEmpty();
	SelectedEnemyUnitId = NAME_None;
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

bool UFinalBattleWidgetController::SelectEnemyByUnitId(FName RuntimeUnitId)
{
	const bool bExists = CachedSnapshot.Enemies.ContainsByPredicate(
		[&RuntimeUnitId](const FFinalBattleEnemyViewData& Candidate)
		{
			return Candidate.RuntimeUnitId == RuntimeUnitId && Candidate.CurrentHP > 0;
		});

	if (!bExists)
	{
		LastInteractionFeedback = FText::FromString(TEXT("当前无法选择该敌人目标。"));
		RebuildPresentation();
		return false;
	}

	SelectedEnemyUnitId = RuntimeUnitId;
	RebuildPresentation();
	return true;
}

bool UFinalBattleWidgetController::PlayCardByHandIndex(int32 HandIndex)
{
	if (BattleFlowSubsystem == nullptr || BattleFlowSubsystem->GetActiveBattleSession() == nullptr)
	{
		LastInteractionFeedback = FText::FromString(TEXT("当前没有可操作的战斗。"));
		RebuildPresentation();
		return false;
	}

	if (!CachedSnapshot.HandCards.IsValidIndex(HandIndex))
	{
		LastInteractionFeedback = FText::FromString(TEXT("手牌索引无效。"));
		RebuildPresentation();
		return false;
	}

	const FName TargetUnitId = ResolveDefaultTargetUnitId();
	if (TargetUnitId.IsNone())
	{
		LastInteractionFeedback = FText::FromString(TEXT("当前没有可选中的敌人目标。"));
		RebuildPresentation();
		return false;
	}

	FFinalBattleCommand Command;
	Command.CommandType = EFinalBattleCommandType::PlayCard;
	Command.CardInstanceId = CachedSnapshot.HandCards[HandIndex].CardInstanceId;
	Command.TargetUnitId = TargetUnitId;
	return SubmitBattleCommandWithFeedback(Command);
}

bool UFinalBattleWidgetController::PlayUltimateByCharacterIndex(int32 CharacterIndex)
{
	if (BattleFlowSubsystem == nullptr || BattleFlowSubsystem->GetActiveBattleSession() == nullptr)
	{
		LastInteractionFeedback = FText::FromString(TEXT("当前没有可操作的战斗。"));
		RebuildPresentation();
		return false;
	}

	if (!CachedSnapshot.Characters.IsValidIndex(CharacterIndex))
	{
		LastInteractionFeedback = FText::FromString(TEXT("角色索引无效。"));
		RebuildPresentation();
		return false;
	}

	FFinalBattleCommand Command;
	Command.CommandType = EFinalBattleCommandType::PlayUltimate;
	Command.UltimateOwnerUnitId = CachedSnapshot.Characters[CharacterIndex].RuntimeUnitId;
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

	FFinalBattleHUDPresentationData Presentation;
	Presentation.bHasActiveBattle = BattleFlowSubsystem != nullptr && BattleFlowSubsystem->GetActiveBattleSession() != nullptr;
	Presentation.CurrentRound = CachedSnapshot.CurrentRound;
	Presentation.CurrentAP = CachedSnapshot.CurrentAP;
	Presentation.CurrentEP = CachedSnapshot.CurrentEP;
	Presentation.TeamCurrentHP = CachedSnapshot.TeamCurrentHP;
	Presentation.TeamMaxHP = CachedSnapshot.TeamMaxHP;
	Presentation.TeamShield = CachedSnapshot.TeamShield;
	Presentation.FeedbackText = LastInteractionFeedback;

	UFinalDataRegistry* DataRegistry = nullptr;
	UFinalGameFlowSubsystem* GameFlowSubsystem = nullptr;
	UFinalRunSession* RunSession = nullptr;
	if (BattleFlowSubsystem && BattleFlowSubsystem->GetGameInstance())
	{
		DataRegistry = BattleFlowSubsystem->GetGameInstance()->GetSubsystem<UFinalDataRegistry>();
		GameFlowSubsystem = BattleFlowSubsystem->GetGameInstance()->GetSubsystem<UFinalGameFlowSubsystem>();
		RunSession = GameFlowSubsystem ? GameFlowSubsystem->GetRunSession() : nullptr;
	}

	FFinalRunState RunState;
	if (RunSession)
	{
		RunState = RunSession->GetRunState();
		Presentation.Gold = RunState.Gold;

		if (DataRegistry)
		{
			if (UFinalBattleEncounterDefinition* EncounterDefinition = DataRegistry->FindEncounterDefinition(RunState.CurrentEncounterId))
			{
				Presentation.EncounterName = EncounterDefinition->DisplayName;
			}

			if (UFinalBattleRuleConfig* RuleConfig = DataRegistry->FindRuleConfig(RunState.CurrentRuleConfigId))
			{
				Presentation.MaxEP = RuleConfig->MaxEP;
			}
		}
	}

	if (Presentation.EncounterName.IsEmpty())
	{
		Presentation.EncounterName = FText::FromString(TEXT("未命名遭遇"));
	}

	for (const FFinalBattleCharacterViewData& CharacterView : CachedSnapshot.Characters)
	{
		FFinalBattleHUDCharacterEntry Entry;
		Entry.RuntimeUnitId = CharacterView.RuntimeUnitId;
		Entry.CurrentStress = CharacterView.CurrentStress;
		Entry.bCollapsed = CharacterView.bCollapsed;
		Entry.StateText = CharacterView.bCollapsed
			? FText::FromString(TEXT("已崩溃"))
			: FText::FromString(TEXT("状态稳定"));

		if (DataRegistry)
		{
			if (UFinalCharacterDefinition* CharacterDefinition = DataRegistry->FindCharacterDefinition(CharacterView.CharacterId))
			{
				Entry.DisplayName = CharacterDefinition->DisplayName;
				Entry.StressCap = CharacterDefinition->BaseStressCap;
			}
		}

		if (Entry.DisplayName.IsEmpty())
		{
			Entry.DisplayName = FText::FromName(CharacterView.CharacterId.Value);
		}

		Presentation.Characters.Add(Entry);

		FFinalBattleHUDUltimateEntry UltimateEntry;
		UltimateEntry.RuntimeUnitId = CharacterView.RuntimeUnitId;
		UltimateEntry.DisplayName = FText::Format(NSLOCTEXT("FinalBattleHUD", "UltimateNameFormat", "{0} 奥义"), Entry.DisplayName);
		UltimateEntry.StatusText = FText::FromString(TEXT("待 Battle/Run 公开奥义可用态与消耗字段"));
		UltimateEntry.bEnabled = false;
		Presentation.Ultimates.Add(UltimateEntry);
	}

	for (const FFinalBattleEnemyViewData& EnemyView : CachedSnapshot.Enemies)
	{
		FFinalBattleHUDEnemyEntry Entry;
		Entry.RuntimeUnitId = EnemyView.RuntimeUnitId;
		Entry.DisplayName = EnemyView.DisplayName;
		Entry.CurrentHP = EnemyView.CurrentHP;
		Entry.CurrentShield = EnemyView.CurrentShield;
		Entry.CurrentBreakValue = EnemyView.CurrentBreakValue;
		Entry.CurrentInitiative = EnemyView.CurrentInitiative;
		Entry.IntentText = EnemyView.IntentText;
		Entry.bSelected = EnemyView.RuntimeUnitId == SelectedEnemyUnitId;
		Presentation.Enemies.Add(Entry);
	}

	for (const FFinalBattleCardViewData& CardView : CachedSnapshot.HandCards)
	{
		FFinalBattleHUDCardEntry Entry;
		Entry.CardInstanceId = CardView.CardInstanceId;
		Entry.RuntimeCostAP = CardView.RuntimeCostAP;

		if (DataRegistry)
		{
			if (UFinalCardDefinition* CardDefinition = DataRegistry->FindCardDefinition(CardView.CardId))
			{
				Entry.DisplayName = CardDefinition->DisplayName;
				Entry.RulesText = CardDefinition->RulesText;
			}
		}

		if (Entry.DisplayName.IsEmpty())
		{
			Entry.DisplayName = FText::FromName(CardView.CardId.Value);
		}

		Presentation.HandCards.Add(Entry);
	}

	const int32 MaxLogEntries = 6;
	const int32 StartIndex = FMath::Max(CachedBattleEvents.Num() - MaxLogEntries, 0);
	for (int32 Index = StartIndex; Index < CachedBattleEvents.Num(); ++Index)
	{
		const FFinalBattleEvent& Event = CachedBattleEvents[Index];
		FFinalBattleHUDLogEntry Entry;
		Entry.EventType = Event.EventType;
		Entry.Round = Event.Round;
		Entry.Message = Event.Message;
		Presentation.LogEntries.Add(Entry);
	}

	if (Presentation.bHasActiveBattle)
	{
		Presentation.MissingFieldNotices.Add(FText::FromString(TEXT("缺 Team/Character Status 公开查询")));
		Presentation.MissingFieldNotices.Add(FText::FromString(TEXT("缺 Draw/Discard/Exhaust 计数")));
		Presentation.MissingFieldNotices.Add(FText::FromString(TEXT("缺奥义消耗/可用态/已释放信息")));
		Presentation.MissingFieldNotices.Add(FText::FromString(TEXT("缺苏醒进度与崩溃计数战斗内显示字段")));
	}

	ViewModel->ApplySnapshot(CachedSnapshot);
	ViewModel->ApplyPresentation(Presentation);
}

void UFinalBattleWidgetController::RefreshSelectedEnemyFromSnapshot()
{
	const bool bKeepSelection = CachedSnapshot.Enemies.ContainsByPredicate(
		[this](const FFinalBattleEnemyViewData& Candidate)
		{
			return Candidate.RuntimeUnitId == SelectedEnemyUnitId && Candidate.CurrentHP > 0;
		});

	if (bKeepSelection)
	{
		return;
	}

	SelectedEnemyUnitId = ResolveDefaultTargetUnitId();
}

FName UFinalBattleWidgetController::ResolveDefaultTargetUnitId() const
{
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
		LastInteractionFeedback = FText::FromString(TEXT("BattleFlowSubsystem 不可用。"));
		RebuildPresentation();
		return false;
	}

	const bool bAccepted = BattleFlowSubsystem->SubmitBattleCommand(Command);
	const FFinalBattleEvent Event = BattleFlowSubsystem->GetLastCommandEvent();
	LastInteractionFeedback = Event.Message.IsEmpty() ? BattleFlowSubsystem->GetLastFailureReason() : Event.Message;

	if (!bAccepted)
	{
		RebuildPresentation();
	}

	return bAccepted;
}
