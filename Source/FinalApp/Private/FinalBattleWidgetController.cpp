#include "Controllers/FinalBattleWidgetController.h"

#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalStatusDefinition.h"
#include "Battle/Definitions/FinalUltimateDefinition.h"
#include "Commands/FinalBattleCommand.h"
#include "Facade/FinalBattleSession.h"
#include "Facade/FinalRunSession.h"
#include "GameplayTagContainer.h"
#include "Queries/FinalDataRegistry.h"
#include "Queries/FinalRunSnapshot.h"
#include "Subsystems/FinalBattleFlowSubsystem.h"
#include "Subsystems/FinalGameFlowSubsystem.h"
#include "ViewModels/FinalBattleHUDViewModel.h"

namespace
{
const FName TeamPlayerUnitId(TEXT("team_player"));

FText ResolveStatusDisplayName(const FFinalBattleStatusViewData& StatusView, const UFinalDataRegistry* DataRegistry)
{
	if (DataRegistry && StatusView.StatusId.IsValid())
	{
		if (const UFinalStatusDefinition* StatusDefinition = DataRegistry->FindStatusDefinition(StatusView.StatusId))
		{
			if (!StatusDefinition->DisplayName.IsEmpty())
			{
				return StatusDefinition->DisplayName;
			}
		}
	}

	return !StatusView.DisplayName.IsEmpty()
		? StatusView.DisplayName
		: FText::FromName(StatusView.StatusId.Value);
}

FText FormatStatusText(const FFinalBattleStatusViewData& StatusView, const UFinalDataRegistry* DataRegistry)
{
	const FText StatusName = ResolveStatusDisplayName(StatusView, DataRegistry);

	if (StatusView.RemainingDuration > 0)
	{
		return FText::Format(
			NSLOCTEXT("FinalBattleHUD", "StatusWithDuration", "{0} x{1} ({2})"),
			StatusName,
			FText::AsNumber(StatusView.CurrentStacks),
			FText::AsNumber(StatusView.RemainingDuration));
	}

	return FText::Format(
		NSLOCTEXT("FinalBattleHUD", "StatusStacksOnly", "{0} x{1}"),
		StatusName,
		FText::AsNumber(StatusView.CurrentStacks));
}

FText FormatCardTypeText(const EFinalCardType CardType)
{
	switch (CardType)
	{
	case EFinalCardType::Attack:
		return NSLOCTEXT("FinalBattleHUD", "CardTypeAttack", "攻击");

	case EFinalCardType::Skill:
		return NSLOCTEXT("FinalBattleHUD", "CardTypeSkill", "技能");

	case EFinalCardType::Ability:
		return NSLOCTEXT("FinalBattleHUD", "CardTypeAbility", "战术");

	default:
		return NSLOCTEXT("FinalBattleHUD", "CardTypeUnknown", "未知");
	}
}

FText FormatKeywordText(const FGameplayTagContainer& Keywords)
{
	TArray<FString> KeywordStrings;
	for (const FGameplayTag& Keyword : Keywords)
	{
		KeywordStrings.Add(Keyword.GetTagName().ToString());
	}

	return KeywordStrings.Num() > 0
		? FText::FromString(FString::Join(KeywordStrings, TEXT(" / ")))
		: FText::GetEmpty();
}

FText ResolveTargetText(const FFinalBattleSnapshot& Snapshot)
{
	if (Snapshot.CurrentTargetUnitId.IsNone())
	{
		return NSLOCTEXT("FinalBattleHUD", "NoTargetText", "未锁定目标");
	}

	const FFinalBattleEnemyViewData* TargetEnemy = Snapshot.Enemies.FindByPredicate(
		[&Snapshot](const FFinalBattleEnemyViewData& Candidate)
		{
			return Candidate.RuntimeUnitId == Snapshot.CurrentTargetUnitId;
		});

	if (TargetEnemy == nullptr)
	{
		return FText::Format(
			NSLOCTEXT("FinalBattleHUD", "TargetFallbackText", "目标 {0}"),
			FText::FromName(Snapshot.CurrentTargetUnitId));
	}

	return FText::Format(
		NSLOCTEXT("FinalBattleHUD", "TargetEnemyText", "当前目标: {0}"),
		TargetEnemy->DisplayName);
}

FText ResolveUltimateDisplayName(
	const FFinalBattleUltimateViewData& UltimateView,
	const UFinalDataRegistry* DataRegistry,
	const TMap<FName, FText>& CharacterDisplayNameByRuntimeId)
{
	FText UltimateName = UltimateView.DisplayName;
	if (UltimateName.IsEmpty() && DataRegistry && UltimateView.UltimateId.IsValid())
	{
		if (const UFinalUltimateDefinition* UltimateDefinition = DataRegistry->FindUltimateDefinition(UltimateView.UltimateId))
		{
			UltimateName = UltimateDefinition->DisplayName;
		}
	}

	if (UltimateName.IsEmpty())
	{
		UltimateName = FText::FromName(UltimateView.UltimateId.Value);
	}

	const FText OwnerName = CharacterDisplayNameByRuntimeId.FindRef(UltimateView.OwnerUnitId);
	if (!OwnerName.IsEmpty() && !UltimateName.IsEmpty() && !OwnerName.EqualTo(UltimateName))
	{
		return FText::Format(
			NSLOCTEXT("FinalBattleHUD", "UltimateOwnerNameFormat", "{0} · {1}"),
			OwnerName,
			UltimateName);
	}

	return !UltimateName.IsEmpty() ? UltimateName : OwnerName;
}
}

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
	LastInteractionFeedback = FText::GetEmpty();
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
	if (BattleFlowSubsystem == nullptr || BattleFlowSubsystem->GetActiveBattleSession() == nullptr)
	{
		LastInteractionFeedback = FText::FromString(TEXT("当前没有可操作的战斗。"));
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
		LastInteractionFeedback = FText::FromString(TEXT("当前无法选择该敌人目标。"));
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

	if (!CachedSnapshot.CharacterUltimates.IsValidIndex(CharacterIndex))
	{
		LastInteractionFeedback = FText::FromString(TEXT("奥义索引无效。"));
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
	Presentation.EncounterName = !CachedSnapshot.EncounterDisplayName.IsEmpty()
		? CachedSnapshot.EncounterDisplayName
		: FText::FromString(TEXT("未命名遭遇"));
	Presentation.CurrentRound = CachedSnapshot.CurrentRound;
	Presentation.CurrentAP = CachedSnapshot.CurrentAP;
	Presentation.CurrentEP = CachedSnapshot.CurrentEP;
	Presentation.MaxEP = CachedSnapshot.MaxEP;
	Presentation.TeamCurrentHP = CachedSnapshot.TeamCurrentHP;
	Presentation.TeamMaxHP = CachedSnapshot.TeamMaxHP;
	Presentation.TeamShield = CachedSnapshot.TeamShield;
	Presentation.DrawPileCount = CachedSnapshot.DeckState.DrawPileCount;
	Presentation.HandCount = CachedSnapshot.DeckState.HandCount;
	Presentation.DiscardPileCount = CachedSnapshot.DeckState.DiscardPileCount;
	Presentation.OngoingZoneCount = CachedSnapshot.DeckState.OngoingZoneCount;
	Presentation.ConsumePileCount = CachedSnapshot.DeckState.ConsumePileCount;
	Presentation.FeedbackText = LastInteractionFeedback;
	Presentation.CurrentTargetText = ResolveTargetText(CachedSnapshot);

	UFinalDataRegistry* DataRegistry = nullptr;
	UFinalGameFlowSubsystem* GameFlowSubsystem = nullptr;
	UFinalRunSession* RunSession = nullptr;
	if (BattleFlowSubsystem && BattleFlowSubsystem->GetGameInstance())
	{
		DataRegistry = BattleFlowSubsystem->GetGameInstance()->GetSubsystem<UFinalDataRegistry>();
		GameFlowSubsystem = BattleFlowSubsystem->GetGameInstance()->GetSubsystem<UFinalGameFlowSubsystem>();
		RunSession = GameFlowSubsystem ? GameFlowSubsystem->GetRunSession() : nullptr;
	}

	const FFinalRunSnapshot RunSnapshot = RunSession ? RunSession->GetSnapshot() : FFinalRunSnapshot{};
	Presentation.Gold = RunSnapshot.Gold;
	Presentation.RelicCount = RunSnapshot.RelicCount;
	Presentation.RunDeckCount = RunSnapshot.DeckCount;

	TMap<FName, TArray<FText>> StatusTextsByOwner;
	for (const FFinalBattleStatusViewData& StatusView : CachedSnapshot.Statuses)
	{
		StatusTextsByOwner.FindOrAdd(StatusView.OwnerUnitId).Add(FormatStatusText(StatusView, DataRegistry));
	}

	Presentation.TeamStatusTexts = StatusTextsByOwner.FindRef(TeamPlayerUnitId);

	TMap<FName, FText> CharacterDisplayNameByRuntimeId;
	for (const FFinalBattleCharacterViewData& CharacterView : CachedSnapshot.Characters)
	{
		const FText CharacterName = !CharacterView.DisplayName.IsEmpty()
			? CharacterView.DisplayName
			: FText::FromName(CharacterView.CharacterId.Value);
		CharacterDisplayNameByRuntimeId.Add(CharacterView.RuntimeUnitId, CharacterName);

		FFinalBattleHUDCharacterEntry Entry;
		Entry.RuntimeUnitId = CharacterView.RuntimeUnitId;
		Entry.DisplayName = CharacterName;
		Entry.CurrentStress = CharacterView.CurrentStress;
		Entry.StressCap = CharacterView.StressCap;
		Entry.bCollapsed = CharacterView.bCollapsed;
		Entry.CurrentAwakenCount = CharacterView.CurrentAwakenCount;
		Entry.CurrentAwakenThreshold = CharacterView.CurrentAwakenThreshold;
		Entry.CollapseCount = CharacterView.CollapseCount;
		Entry.VitalShare = CharacterView.VitalShare;
		Entry.StateText = CharacterView.bCollapsed
			? FText::FromString(TEXT("已崩溃"))
			: FText::FromString(TEXT("可行动"));
		Entry.StatusTexts = StatusTextsByOwner.FindRef(CharacterView.RuntimeUnitId);
		Presentation.Characters.Add(Entry);
	}

	for (const FFinalBattleUltimateViewData& UltimateView : CachedSnapshot.CharacterUltimates)
	{
		FFinalBattleHUDUltimateEntry Entry;
		Entry.RuntimeUnitId = UltimateView.OwnerUnitId;
		Entry.DisplayName = ResolveUltimateDisplayName(UltimateView, DataRegistry, CharacterDisplayNameByRuntimeId);
		if (Entry.DisplayName.IsEmpty())
		{
			Entry.DisplayName = FText::FromName(UltimateView.UltimateId.Value);
		}

		Entry.CostEP = UltimateView.CostEP;
		Entry.bEnabled = UltimateView.bCanActivate;
		Entry.bBlockedByCollapse = UltimateView.bBlockedByCollapse;
		Entry.bDefinitionReady = UltimateView.bDefinitionReady;

		if (!UltimateView.bDefinitionReady)
		{
			Entry.StatusText = FText::FromString(TEXT("未绑定定义"));
		}
		else if (UltimateView.bBlockedByCollapse)
		{
			Entry.StatusText = FText::FromString(TEXT("崩溃中，不可释放"));
		}
		else if (UltimateView.bCanActivate)
		{
			Entry.StatusText = FText::Format(
				NSLOCTEXT("FinalBattleHUD", "UltimateReadyState", "可释放 | EP {0}/{1}"),
				FText::AsNumber(CachedSnapshot.CurrentEP),
				FText::AsNumber(UltimateView.CostEP));
		}
		else
		{
			Entry.StatusText = FText::Format(
				NSLOCTEXT("FinalBattleHUD", "UltimateChargeState", "充能中 | EP {0}/{1}"),
				FText::AsNumber(CachedSnapshot.CurrentEP),
				FText::AsNumber(UltimateView.CostEP));
		}

		Presentation.Ultimates.Add(Entry);
	}

	for (const FFinalBattleEnemyViewData& EnemyView : CachedSnapshot.Enemies)
	{
		FFinalBattleHUDEnemyEntry Entry;
		Entry.RuntimeUnitId = EnemyView.RuntimeUnitId;
		Entry.DisplayName = EnemyView.DisplayName;
		Entry.PositionIndex = EnemyView.PositionIndex;
		Entry.CurrentHP = EnemyView.CurrentHP;
		Entry.MaxHP = EnemyView.MaxHP;
		Entry.CurrentShield = EnemyView.CurrentShield;
		Entry.CurrentBreakValue = EnemyView.CurrentBreakValue;
		Entry.MaxBreakValue = EnemyView.MaxBreakValue;
		Entry.CurrentInitiative = EnemyView.CurrentInitiative;
		Entry.IntentText = EnemyView.IntentText;
		Entry.bSelected = EnemyView.RuntimeUnitId == SelectedEnemyUnitId;
		Entry.bActedThisRound = EnemyView.bActedThisRound;
		Entry.StatusTexts = StatusTextsByOwner.FindRef(EnemyView.RuntimeUnitId);
		Presentation.Enemies.Add(Entry);
	}

	for (const FFinalBattleCardViewData& CardView : CachedSnapshot.HandCards)
	{
		FFinalBattleHUDCardEntry Entry;
		Entry.CardInstanceId = CardView.CardInstanceId;
		Entry.DisplayName = CardView.DisplayName;
		Entry.RulesText = FText::GetEmpty();
		if (DataRegistry && CardView.CardId.IsValid())
		{
			if (const UFinalCardDefinition* CardDefinition = DataRegistry->FindCardDefinition(CardView.CardId))
			{
				if (Entry.DisplayName.IsEmpty())
				{
					Entry.DisplayName = CardDefinition->DisplayName;
				}

				Entry.RulesText = CardDefinition->RulesText;
			}
		}

		if (Entry.DisplayName.IsEmpty())
		{
			Entry.DisplayName = FText::FromName(CardView.CardId.Value);
		}

		Entry.OwnerUnitId = CardView.RuntimeOwnerUnitId;
		Entry.OwnerDisplayName = CharacterDisplayNameByRuntimeId.FindRef(CardView.RuntimeOwnerUnitId);
		if (Entry.OwnerDisplayName.IsEmpty() && !CardView.RuntimeOwnerUnitId.IsNone())
		{
			Entry.OwnerDisplayName = FText::FromName(CardView.RuntimeOwnerUnitId);
		}

		Entry.RuntimeCostAP = CardView.RuntimeCostAP;
		Entry.TypeText = FormatCardTypeText(CardView.CardType);
		Entry.KeywordText = FormatKeywordText(CardView.RuntimeKeywords);
		Entry.bRetained = CardView.bRetained;
		Entry.bCollapsedCard = CardView.bCollapsedCard;
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
		Presentation.MissingFieldNotices.Add(FText::FromString(TEXT("缺阶段进度公开查询")));
		Presentation.MissingFieldNotices.Add(FText::FromString(TEXT("缺奥义“本战已释放”公开状态")));
		Presentation.MissingFieldNotices.Add(FText::FromString(TEXT("缺结构化命令拒绝原因")));
	}

	ViewModel->ApplySnapshot(CachedSnapshot);
	ViewModel->ApplyPresentation(Presentation);
}

void UFinalBattleWidgetController::RefreshSelectedEnemyFromSnapshot()
{
	const bool bSnapshotTargetIsAlive = CachedSnapshot.Enemies.ContainsByPredicate(
		[this](const FFinalBattleEnemyViewData& Candidate)
		{
			return Candidate.RuntimeUnitId == CachedSnapshot.CurrentTargetUnitId && Candidate.CurrentHP > 0;
		});

	SelectedEnemyUnitId = bSnapshotTargetIsAlive
		? CachedSnapshot.CurrentTargetUnitId
		: NAME_None;
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
