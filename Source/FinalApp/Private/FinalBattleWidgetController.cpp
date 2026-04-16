#include "Controllers/FinalBattleWidgetController.h"

#include "BattleBridge/FinalBattleEventPresentationUtils.h"
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
const FName RejectBattleNotInitializedTag(TEXT("battle.not_initialized"));
const FName RejectNotEnoughEPTag(TEXT("battle.not_enough_ep"));
const FName RejectInvalidTargetTag(TEXT("battle.invalid_target"));
const FName RejectUltimateAlreadyUsedTag(TEXT("battle.ultimate_already_used"));
const FName RejectUltimateBlockedByCollapseTag(TEXT("battle.ultimate_blocked_by_collapse"));
const FName RejectUnsupportedCommandTag(TEXT("battle.unsupported_command"));
const FName RejectNotEnoughAPTag(TEXT("battle.not_enough_ap"));

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

FText FormatEnemyPhaseProgressText(const FFinalBattlePhaseProgressViewData& PhaseProgress)
{
	if (PhaseProgress.TotalPhases <= 0 || PhaseProgress.CurrentPhaseNumber <= 0)
	{
		return FText::GetEmpty();
	}

	return FText::Format(
		NSLOCTEXT("FinalBattleHUD", "EnemyPhaseProgressFormat", "阶段 {0}/{1} | 本阶段 {2}%"),
		FText::AsNumber(PhaseProgress.CurrentPhaseNumber),
		FText::AsNumber(PhaseProgress.TotalPhases),
		FText::AsNumber(FMath::RoundToInt(PhaseProgress.ProgressWithinPhase * 100.0f)));
}

FText FormatRelicEffectTypeText(const EFinalRelicBattleStartEffectType EffectType)
{
	switch (EffectType)
	{
	case EFinalRelicBattleStartEffectType::GainAP:
		return NSLOCTEXT("FinalBattleHUD", "RelicEffectGainAP", "GainAP");

	case EFinalRelicBattleStartEffectType::GainShield:
		return NSLOCTEXT("FinalBattleHUD", "RelicEffectGainShield", "GainShield");

	case EFinalRelicBattleStartEffectType::None:
	default:
		return NSLOCTEXT("FinalBattleHUD", "RelicEffectNone", "None");
	}
}

FText FormatRelicTurnStartEffectTypeText(const EFinalRelicPlayerTurnStartEffectType EffectType)
{
	switch (EffectType)
	{
	case EFinalRelicPlayerTurnStartEffectType::GainAP:
		return NSLOCTEXT("FinalBattleHUD", "RelicTurnStartEffectGainAP", "GainAP");

	case EFinalRelicPlayerTurnStartEffectType::GainShield:
		return NSLOCTEXT("FinalBattleHUD", "RelicTurnStartEffectGainShield", "GainShield");

	case EFinalRelicPlayerTurnStartEffectType::None:
	default:
		return NSLOCTEXT("FinalBattleHUD", "RelicTurnStartEffectNone", "None");
	}
}

FText ResolveRelicDisplayName(const FFinalBattleStartRelicInput& RelicInput)
{
	if (!RelicInput.DisplayName.IsEmpty())
	{
		return RelicInput.DisplayName;
	}

	if (!RelicInput.DisplayId.IsNone())
	{
		return FText::FromName(RelicInput.DisplayId);
	}

	return RelicInput.RelicId.IsValid()
		? FText::FromName(RelicInput.RelicId.Value)
		: FText::GetEmpty();
}

FText BuildRelicEffectSummaryText(const TArray<FFinalBattleStartRelicEffectInput>& EffectInputs)
{
	if (EffectInputs.Num() == 0)
	{
		return NSLOCTEXT("FinalBattleHUD", "RelicNoBattleStartEffect", "No battle-start effects");
	}

	TArray<FString> Segments;
	Segments.Reserve(EffectInputs.Num());
	for (const FFinalBattleStartRelicEffectInput& EffectInput : EffectInputs)
	{
		Segments.Add(FString::Printf(
			TEXT("%s +%d"),
			*FormatRelicEffectTypeText(EffectInput.EffectType).ToString(),
			EffectInput.Value));
	}

	return FText::FromString(FString::Join(Segments, TEXT(" | ")));
}

FText BuildRelicTurnStartEffectSummaryText(const TArray<FFinalBattlePlayerTurnStartRelicEffectInput>& EffectInputs)
{
	if (EffectInputs.Num() == 0)
	{
		return NSLOCTEXT("FinalBattleHUD", "RelicNoTurnStartEffect", "No turn-start effects");
	}

	TArray<FString> Segments;
	Segments.Reserve(EffectInputs.Num());
	for (const FFinalBattlePlayerTurnStartRelicEffectInput& EffectInput : EffectInputs)
	{
		Segments.Add(FString::Printf(
			TEXT("%s +%d"),
			*FormatRelicTurnStartEffectTypeText(EffectInput.EffectType).ToString(),
			EffectInput.Value));
	}

	return FText::FromString(FString::Join(Segments, TEXT(" | ")));
}

FText BuildActiveRelicSummaryText(const FFinalBattleStartRelicInput& RelicInput)
{
	return FText::Format(
		NSLOCTEXT("FinalBattleHUD", "ActiveRelicSummaryFormat", "{0} [Start: {1}] [Turn: {2}]"),
		ResolveRelicDisplayName(RelicInput),
		BuildRelicEffectSummaryText(RelicInput.BattleStartEffects),
		BuildRelicTurnStartEffectSummaryText(RelicInput.PlayerTurnStartEffects));
}

FText ResolveRelicDisplayNameById(const TArray<FFinalBattleStartRelicInput>& ActiveRelics, const FFinalRelicId& RelicId)
{
	if (!RelicId.IsValid())
	{
		return FText::GetEmpty();
	}

	const FFinalBattleStartRelicInput* RelicInput = ActiveRelics.FindByPredicate(
		[&RelicId](const FFinalBattleStartRelicInput& Candidate)
		{
			return Candidate.RelicId == RelicId;
		});
	return RelicInput != nullptr
		? ResolveRelicDisplayName(*RelicInput)
		: FText::FromName(RelicId.Value);
}

FText ResolveRelicEffectSummaryById(const TArray<FFinalBattleStartRelicInput>& ActiveRelics, const FFinalRelicId& RelicId)
{
	const FFinalBattleStartRelicInput* RelicInput = ActiveRelics.FindByPredicate(
		[&RelicId](const FFinalBattleStartRelicInput& Candidate)
		{
			return Candidate.RelicId == RelicId;
		});
	return RelicInput != nullptr
		? FText::Format(
			NSLOCTEXT("FinalBattleHUD", "RelicEffectSummaryCombined", "Start: {0} | Turn: {1}"),
			BuildRelicEffectSummaryText(RelicInput->BattleStartEffects),
			BuildRelicTurnStartEffectSummaryText(RelicInput->PlayerTurnStartEffects))
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

	const FText PhaseProgressText = FormatEnemyPhaseProgressText(TargetEnemy->PhaseProgress);
	if (!PhaseProgressText.IsEmpty())
	{
		return FText::Format(
			NSLOCTEXT("FinalBattleHUD", "TargetEnemyWithPhaseText", "当前目标: {0} | {1}"),
			TargetEnemy->DisplayName,
			PhaseProgressText);
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

FFinalBattleEvent BuildLocalRejectEvent(const FText& Message, const EFinalBattleCommandRejectReason RejectReason, const FName ReasonTag)
{
	FFinalBattleEvent Event;
	Event.EventType = EFinalBattleEventType::CommandRejected;
	Event.RejectReason = RejectReason;
	Event.ReasonTag = ReasonTag;
	Event.Message = Message;
	return Event;
}

FText ResolveRejectReasonLabel(const FFinalBattleEvent& Event)
{
	switch (Event.RejectReason)
	{
	case EFinalBattleCommandRejectReason::BattleNotInitialized:
		return NSLOCTEXT("FinalBattleHUD", "RejectBattleNotInitialized", "战斗未初始化");

	case EFinalBattleCommandRejectReason::InvalidTarget:
		return NSLOCTEXT("FinalBattleHUD", "RejectInvalidTarget", "目标无效");

	case EFinalBattleCommandRejectReason::UltimateAlreadyUsed:
		return NSLOCTEXT("FinalBattleHUD", "RejectUltimateAlreadyUsed", "奥义已释放");

	case EFinalBattleCommandRejectReason::UltimateBlockedByCollapse:
		return NSLOCTEXT("FinalBattleHUD", "RejectUltimateBlockedByCollapse", "角色崩溃");

	case EFinalBattleCommandRejectReason::NotEnoughEP:
		return NSLOCTEXT("FinalBattleHUD", "RejectNotEnoughEP", "EP不足");

	case EFinalBattleCommandRejectReason::NotEnoughAP:
		return NSLOCTEXT("FinalBattleHUD", "RejectNotEnoughAP", "AP不足");

	case EFinalBattleCommandRejectReason::UnsupportedCommand:
		return NSLOCTEXT("FinalBattleHUD", "RejectUnsupportedCommand", "命令不支持");

	case EFinalBattleCommandRejectReason::UltimateDefinitionUnavailable:
		return NSLOCTEXT("FinalBattleHUD", "RejectUltimateDefinitionUnavailable", "奥义定义未就绪");

	default:
		break;
	}

	if (Event.ReasonTag == RejectBattleNotInitializedTag)
	{
		return NSLOCTEXT("FinalBattleHUD", "RejectBattleNotInitializedByTag", "战斗未初始化");
	}

	if (Event.ReasonTag == RejectInvalidTargetTag)
	{
		return NSLOCTEXT("FinalBattleHUD", "RejectInvalidTargetByTag", "目标无效");
	}

	if (Event.ReasonTag == RejectUltimateAlreadyUsedTag)
	{
		return NSLOCTEXT("FinalBattleHUD", "RejectUltimateAlreadyUsedByTag", "奥义已释放");
	}

	if (Event.ReasonTag == RejectUltimateBlockedByCollapseTag)
	{
		return NSLOCTEXT("FinalBattleHUD", "RejectUltimateBlockedByCollapseByTag", "角色崩溃");
	}

	if (Event.ReasonTag == RejectNotEnoughEPTag)
	{
		return NSLOCTEXT("FinalBattleHUD", "RejectNotEnoughEPByTag", "EP不足");
	}

	if (Event.ReasonTag == RejectNotEnoughAPTag)
	{
		return NSLOCTEXT("FinalBattleHUD", "RejectNotEnoughAPByTag", "AP不足");
	}

	if (Event.ReasonTag == RejectUnsupportedCommandTag)
	{
		return NSLOCTEXT("FinalBattleHUD", "RejectUnsupportedCommandByTag", "命令不支持");
	}

	return FText::GetEmpty();
}

FText ResolveFeedbackTitleText(
	const FFinalBattleEvent& Event,
	const FText& FallbackMessage,
	const TArray<FFinalBattleStartRelicInput>& ActiveRelics)
{
	const FText RejectReasonLabel = ResolveRejectReasonLabel(Event);
	if (Event.EventType == EFinalBattleEventType::CommandRejected || Event.RejectReason != EFinalBattleCommandRejectReason::None || !Event.ReasonTag.IsNone())
	{
		if (!RejectReasonLabel.IsEmpty() && !Event.ReasonTag.IsNone())
		{
			return FText::Format(
				NSLOCTEXT("FinalBattleHUD", "RejectFeedbackTitleWithTag", "命令拒绝 · {0} ({1})"),
				RejectReasonLabel,
				FText::FromName(Event.ReasonTag));
		}

		if (!RejectReasonLabel.IsEmpty())
		{
			return FText::Format(
				NSLOCTEXT("FinalBattleHUD", "RejectFeedbackTitle", "命令拒绝 · {0}"),
				RejectReasonLabel);
		}

		return NSLOCTEXT("FinalBattleHUD", "RejectFeedbackTitleFallback", "命令拒绝");
	}

	switch (Event.EventType)
	{
	case EFinalBattleEventType::RelicTriggered:
	{
		const FText RelicName = ResolveRelicDisplayNameById(ActiveRelics, Event.RelicId);
		return !RelicName.IsEmpty()
			? FText::Format(
				NSLOCTEXT("FinalBattleHUD", "FeedbackRelicTriggeredWithName", "遗物触发 · {0}"),
				RelicName)
			: NSLOCTEXT("FinalBattleHUD", "FeedbackRelicTriggered", "遗物触发");
	}

	case EFinalBattleEventType::CommandAccepted:
		return NSLOCTEXT("FinalBattleHUD", "FeedbackCommandAccepted", "命令已接受");

	case EFinalBattleEventType::TargetChanged:
		return NSLOCTEXT("FinalBattleHUD", "FeedbackTargetChanged", "目标切换");

	case EFinalBattleEventType::CardResolved:
		return NSLOCTEXT("FinalBattleHUD", "FeedbackCardResolved", "卡牌结算");

	case EFinalBattleEventType::UltimateResolved:
		return NSLOCTEXT("FinalBattleHUD", "FeedbackUltimateResolved", "奥义结算");

	case EFinalBattleEventType::EnemyActed:
		return NSLOCTEXT("FinalBattleHUD", "FeedbackEnemyActed", "敌方行动");

	case EFinalBattleEventType::TurnTransition:
		return NSLOCTEXT("FinalBattleHUD", "FeedbackTurnTransition", "回合切换");

	case EFinalBattleEventType::PhaseChanged:
		return NSLOCTEXT("FinalBattleHUD", "FeedbackPhaseChanged", "阶段变化");

	case EFinalBattleEventType::BattleResolved:
		return NSLOCTEXT("FinalBattleHUD", "FeedbackBattleResolved", "战斗结算");

	default:
		break;
	}

	return FallbackMessage.IsEmpty()
		? FText::GetEmpty()
		: NSLOCTEXT("FinalBattleHUD", "FeedbackGenericTitle", "交互反馈");
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

bool UFinalBattleWidgetController::SelectEnemyByUnitId(FName RuntimeUnitId)
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

bool UFinalBattleWidgetController::PlayCardByHandIndex(int32 HandIndex)
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

bool UFinalBattleWidgetController::PlayUltimateByCharacterIndex(int32 CharacterIndex)
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
	Presentation.FeedbackRejectReason = LastInteractionEvent.RejectReason;
	Presentation.FeedbackReasonTag = LastInteractionEvent.ReasonTag;

	for (const FFinalBattleStartRelicInput& RelicInput : CachedSnapshot.ActiveRelics)
	{
		Presentation.ActiveRelicTexts.Add(BuildActiveRelicSummaryText(RelicInput));
	}

	const FText EffectiveFeedbackText = !LastInteractionFeedback.IsEmpty()
		? LastInteractionFeedback
		: LastInteractionEvent.Message;
	const bool bHasInteractionEvent = LastInteractionEvent.EventSequence > 0
		|| LastInteractionEvent.EventType != EFinalBattleEventType::Info
		|| LastInteractionEvent.RejectReason != EFinalBattleCommandRejectReason::None
		|| !EffectiveFeedbackText.IsEmpty();
	if (bHasInteractionEvent)
	{
		const FinalBattleEventPresentation::FEventPresentation EventPresentation =
			FinalBattleEventPresentation::BuildPresentation(LastInteractionEvent, CachedSnapshot, DataRegistry);
		Presentation.FeedbackTitleText = EventPresentation.TitleText;
		Presentation.FeedbackText = FinalBattleEventPresentation::BuildCombinedBodyText(EventPresentation);
		if (Presentation.FeedbackText.IsEmpty())
		{
			Presentation.FeedbackText = EffectiveFeedbackText;
		}
	}
	else
	{
		Presentation.FeedbackTitleText = FText::GetEmpty();
		Presentation.FeedbackText = FText::GetEmpty();
	}

	for (const FFinalBattleStatusViewData& TeamStatusView : CachedSnapshot.TeamStatuses)
	{
		Presentation.TeamStatusTexts.Add(FormatStatusText(TeamStatusView, DataRegistry));
	}

	TMap<FName, TArray<FText>> CharacterStatusTextsByOwner;
	for (const FFinalBattleCharacterStatusesViewData& CharacterStatusesView : CachedSnapshot.CharacterStatuses)
	{
		TArray<FText>& CharacterStatusTexts = CharacterStatusTextsByOwner.FindOrAdd(CharacterStatusesView.OwnerUnitId);
		for (const FFinalBattleStatusViewData& StatusView : CharacterStatusesView.StatusEntries)
		{
			CharacterStatusTexts.Add(FormatStatusText(StatusView, DataRegistry));
		}
	}

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
		Entry.StatusTexts = CharacterStatusTextsByOwner.FindRef(CharacterView.RuntimeUnitId);
		Presentation.Characters.Add(Entry);
	}

	TMap<FName, TArray<FText>> EnemyStatusTextsByOwner;
	for (const FFinalBattleStatusViewData& StatusView : CachedSnapshot.Statuses)
	{
		if (StatusView.OwnerUnitId == TeamPlayerUnitId || CharacterDisplayNameByRuntimeId.Contains(StatusView.OwnerUnitId))
		{
			continue;
		}

		EnemyStatusTextsByOwner.FindOrAdd(StatusView.OwnerUnitId).Add(FormatStatusText(StatusView, DataRegistry));
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
		Entry.bEnabled = UltimateView.bCanActivate && !UltimateView.bUsedThisBattle;
		Entry.bBlockedByCollapse = UltimateView.bBlockedByCollapse;
		Entry.bDefinitionReady = UltimateView.bDefinitionReady;
		Entry.bUsedThisBattle = UltimateView.bUsedThisBattle;

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
		else if (UltimateView.bUsedThisBattle)
		{
			Entry.StatusText = NSLOCTEXT("FinalBattleHUD", "UltimateUsedThisBattleState", "本战已释放");
		}
		else if (CachedSnapshot.CurrentEP < UltimateView.CostEP)
		{
			Entry.StatusText = FText::Format(
				NSLOCTEXT("FinalBattleHUD", "UltimateInsufficientEPState", "EP不足 | EP {0}/{1}"),
				FText::AsNumber(CachedSnapshot.CurrentEP),
				FText::AsNumber(UltimateView.CostEP));
		}
		else
		{
			Entry.StatusText = NSLOCTEXT("FinalBattleHUD", "UltimateUnavailableState", "当前不可释放");
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
		Entry.CurrentPhaseNumber = EnemyView.PhaseProgress.CurrentPhaseNumber;
		Entry.TotalPhases = EnemyView.PhaseProgress.TotalPhases;
		Entry.PhaseProgressWithinPhase = EnemyView.PhaseProgress.ProgressWithinPhase;
		Entry.PhaseProgressText = FormatEnemyPhaseProgressText(EnemyView.PhaseProgress);
		Entry.IntentText = EnemyView.IntentText;
		Entry.bSelected = EnemyView.RuntimeUnitId == SelectedEnemyUnitId;
		Entry.bActedThisRound = EnemyView.bActedThisRound;
		Entry.StatusTexts = EnemyStatusTextsByOwner.FindRef(EnemyView.RuntimeUnitId);
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
		const FinalBattleEventPresentation::FEventPresentation EventPresentation =
			FinalBattleEventPresentation::BuildPresentation(Event, CachedSnapshot, DataRegistry);
		FFinalBattleHUDLogEntry Entry;
		Entry.EventSequence = Event.EventSequence;
		Entry.EventType = Event.EventType;
		Entry.Round = Event.Round;
		Entry.TitleText = EventPresentation.TitleText;
		Entry.SummaryText = EventPresentation.SummaryText;
		Entry.DetailText = EventPresentation.DetailText;
		Presentation.LogEntries.Add(Entry);
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
