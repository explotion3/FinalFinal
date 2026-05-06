#include "Controllers/Battle/FinalBattleHUDPanelControllers.h"

#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalEnemyDefinition.h"
#include "Battle/Definitions/FinalPassiveDefinition.h"
#include "Battle/Definitions/FinalStatusDefinition.h"
#include "Battle/Definitions/FinalUltimateDefinition.h"
#include "BattleBridge/FinalBattleEventPresentationUtils.h"
#include "Controllers/FinalBattleWidgetController.h"
#include "GameplayTagContainer.h"
#include "Queries/FinalDataRegistry.h"
#include "UI/ViewModels/Battle/FinalBattleHUDPanelViewModels.h"

namespace
{
const FName TeamPlayerUnitId(TEXT("team_player"));
const FName PanelRejectBattleNotInitializedTag(TEXT("battle.not_initialized"));
const FName RejectNotEnoughEPTag(TEXT("battle.not_enough_ep"));
const FName PanelRejectInvalidTargetTag(TEXT("battle.invalid_target"));
const FName RejectUltimateAlreadyUsedTag(TEXT("battle.ultimate_already_used"));
const FName RejectUltimateBlockedByCollapseTag(TEXT("battle.ultimate_blocked_by_collapse"));
const FName PanelRejectUnsupportedCommandTag(TEXT("battle.unsupported_command"));
const FName RejectNotEnoughAPTag(TEXT("battle.not_enough_ap"));
const FText APNotEnoughCardHintText = NSLOCTEXT("FinalBattleHUD", "CardAPNotEnoughHint", "AP不足");

float CalculateClampedPercent(const int32 CurrentValue, const int32 MaxValue)
{
	return MaxValue > 0
		? FMath::Clamp(static_cast<float>(CurrentValue) / static_cast<float>(MaxValue), 0.0f, 1.0f)
		: 0.0f;
}

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

	return !StatusView.DisplayName.IsEmpty() ? StatusView.DisplayName : FText::FromName(StatusView.StatusId.Value);
}

FText ResolveStatusSummaryText(const FFinalBattleStatusViewData& StatusView, const UFinalDataRegistry* DataRegistry)
{
	if (DataRegistry && StatusView.StatusId.IsValid())
	{
		if (const UFinalStatusDefinition* StatusDefinition = DataRegistry->FindStatusDefinition(StatusView.StatusId))
		{
			return StatusDefinition->SummaryText;
		}
	}

	return FText::GetEmpty();
}

FText ResolvePassiveSummaryText(const FFinalBattlePassiveViewData& PassiveView, const UFinalDataRegistry* DataRegistry)
{
	if (DataRegistry && PassiveView.PassiveId.IsValid())
	{
		if (const UFinalPassiveDefinition* PassiveDefinition = DataRegistry->FindPassiveDefinition(PassiveView.PassiveId))
		{
			return PassiveDefinition->SummaryText;
		}
	}

	return FText::GetEmpty();
}

FText BuildGameplayTagSummaryText(const FGameplayTagContainer& Tags)
{
	TArray<FString> TagStrings;
	TArray<FGameplayTag> TagArray;
	Tags.GetGameplayTagArray(TagArray);
	for (const FGameplayTag& Tag : TagArray)
	{
		TagStrings.Add(Tag.GetTagName().ToString());
	}

	return TagStrings.Num() > 0 ? FText::FromString(FString::Join(TagStrings, TEXT(" / "))) : FText::GetEmpty();
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
		return NSLOCTEXT("FinalBattleHUD", "CardTypeAbility", "能力");
	default:
		return NSLOCTEXT("FinalBattleHUD", "CardTypeUnknown", "未知");
	}
}

FText FormatCardZoneText(const EFinalBattleCardZone Zone)
{
	switch (Zone)
	{
	case EFinalBattleCardZone::DrawPile:
		return NSLOCTEXT("FinalBattleHUD", "CardZoneDrawPile", "抽牌堆");
	case EFinalBattleCardZone::Hand:
		return NSLOCTEXT("FinalBattleHUD", "CardZoneHand", "手牌");
	case EFinalBattleCardZone::DiscardPile:
		return NSLOCTEXT("FinalBattleHUD", "CardZoneDiscardPile", "弃牌堆");
	case EFinalBattleCardZone::OngoingZone:
		return NSLOCTEXT("FinalBattleHUD", "CardZoneOngoingZone", "持续区");
	case EFinalBattleCardZone::ConsumePile:
		return NSLOCTEXT("FinalBattleHUD", "CardZoneConsumePile", "消耗区");
	default:
		return NSLOCTEXT("FinalBattleHUD", "CardZoneUnknown", "牌区");
	}
}

FText FormatKeywordText(const FGameplayTagContainer& Keywords)
{
	TArray<FString> KeywordStrings;
	for (const FGameplayTag& Keyword : Keywords)
	{
		KeywordStrings.Add(Keyword.GetTagName().ToString());
	}

	return KeywordStrings.Num() > 0 ? FText::FromString(FString::Join(KeywordStrings, TEXT(" / "))) : FText::GetEmpty();
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

FName ResolveEnemyRankTag(const UFinalEnemyDefinition* EnemyDefinition)
{
	if (EnemyDefinition == nullptr || EnemyDefinition->RoleTags.IsEmpty())
	{
		return NAME_None;
	}

	TArray<FGameplayTag> RoleTags;
	EnemyDefinition->RoleTags.GetGameplayTagArray(RoleTags);
	for (const FGameplayTag& RoleTag : RoleTags)
	{
		const FString TagName = RoleTag.ToString().ToLower();
		if (TagName.Contains(TEXT("rank")) || TagName.Contains(TEXT("elite")) || TagName.Contains(TEXT("boss")))
		{
			return RoleTag.GetTagName();
		}
	}

	return NAME_None;
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

	return RelicInput.RelicId.IsValid() ? FText::FromName(RelicInput.RelicId.Value) : FText::GetEmpty();
}

FText BuildActiveRelicSummaryText(const FFinalBattleStartRelicInput& RelicInput)
{
	return ResolveRelicDisplayName(RelicInput);
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
	return RelicInput != nullptr ? ResolveRelicDisplayName(*RelicInput) : FText::FromName(RelicId.Value);
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

	if (Event.ReasonTag == PanelRejectBattleNotInitializedTag)
	{
		return NSLOCTEXT("FinalBattleHUD", "RejectBattleNotInitializedByTag", "战斗未初始化");
	}

	if (Event.ReasonTag == PanelRejectInvalidTargetTag)
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

	if (Event.ReasonTag == PanelRejectUnsupportedCommandTag)
	{
		return NSLOCTEXT("FinalBattleHUD", "RejectUnsupportedCommandByTag", "命令不支持");
	}

	return FText::GetEmpty();
}

FText ResolveFeedbackTitleTextInternal(
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
			? FText::Format(NSLOCTEXT("FinalBattleHUD", "FeedbackRelicTriggeredWithName", "遗物触发 · {0}"), RelicName)
			: NSLOCTEXT("FinalBattleHUD", "FeedbackRelicTriggered", "遗物触发");
	}
	case EFinalBattleEventType::PassiveApplied:
		return NSLOCTEXT("FinalBattleHUD", "FeedbackPassiveApplied", "被动获得");
	case EFinalBattleEventType::PassiveTriggered:
		return NSLOCTEXT("FinalBattleHUD", "FeedbackPassiveTriggered", "被动触发");
	case EFinalBattleEventType::PassiveRemoved:
		return NSLOCTEXT("FinalBattleHUD", "FeedbackPassiveRemoved", "被动失效");
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
	case EFinalBattleEventType::EnemyInitiativeChanged:
		return NSLOCTEXT("FinalBattleHUD", "FeedbackEnemyInitiativeChanged", "先机变化");
	case EFinalBattleEventType::EnemyQueuedByInitiative:
		return NSLOCTEXT("FinalBattleHUD", "FeedbackEnemyQueuedByInitiative", "先机触发");
	case EFinalBattleEventType::EnemyActionSkipped:
		return NSLOCTEXT("FinalBattleHUD", "FeedbackEnemyActionSkipped", "敌方无法行动");
	case EFinalBattleEventType::TurnTransition:
		return NSLOCTEXT("FinalBattleHUD", "FeedbackTurnTransition", "回合切换");
	case EFinalBattleEventType::PhaseChanged:
		return NSLOCTEXT("FinalBattleHUD", "FeedbackPhaseChanged", "阶段变化");
	case EFinalBattleEventType::BattleResolved:
		return NSLOCTEXT("FinalBattleHUD", "FeedbackBattleResolved", "战斗结算");
	default:
		break;
	}

	return FallbackMessage.IsEmpty() ? FText::GetEmpty() : NSLOCTEXT("FinalBattleHUD", "FeedbackGenericTitle", "交互反馈");
}
}

FText ResolveBattleHUDEventFeedbackTitleText(
	const FFinalBattleEvent& Event,
	const FText& FallbackMessage,
	const TArray<FFinalBattleStartRelicInput>& ActiveRelics)
{
	return ResolveFeedbackTitleTextInternal(Event, FallbackMessage, ActiveRelics);
}

void ApplyBattleHUDCardAPPlayHint(FFinalBattleHUDCardEntry& Entry, const int32 CurrentAP)
{
	if (Entry.RuntimeCostAP > CurrentAP)
	{
		Entry.bCanPlayHint = false;
		Entry.UnplayableHintText = APNotEnoughCardHintText;
		return;
	}

	Entry.bCanPlayHint = true;
	Entry.UnplayableHintText = FText::GetEmpty();
}

void UFinalBattleHUDPanelControllerBase::InitializePanelController(UFinalBattleWidgetController* InCoordinator)
{
	Coordinator = InCoordinator;
}

void UFinalBattleHUDPanelControllerBase::RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData)
{
}

void UFinalBattleTopBarPanelController::InitializeTopBar(UFinalBattleWidgetController* InCoordinator, UFinalBattleTopBarPanelViewModel* InViewModel)
{
	InitializePanelController(InCoordinator);
	ViewModel = InViewModel;
}

void UFinalBattleTopBarPanelController::RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData)
{
	if (ViewModel == nullptr || CoordinatorData.Snapshot == nullptr)
	{
		return;
	}

	FFinalBattleTopBarPanelData Data;
	Data.bHasActiveBattle = CoordinatorData.Snapshot->BattleId.IsValid();
	Data.EncounterName = !CoordinatorData.Snapshot->EncounterDisplayName.IsEmpty()
		? CoordinatorData.Snapshot->EncounterDisplayName
		: FText::FromString(TEXT("未命名遭遇"));
	Data.CurrentRound = CoordinatorData.Snapshot->CurrentRound;
	Data.CurrentAP = CoordinatorData.Snapshot->CurrentAP;
	Data.CurrentEP = CoordinatorData.Snapshot->CurrentEP;
	Data.MaxEP = CoordinatorData.Snapshot->MaxEP;
	Data.TeamCurrentHP = CoordinatorData.Snapshot->TeamCurrentHP;
	Data.TeamMaxHP = CoordinatorData.Snapshot->TeamMaxHP;
	Data.TeamShield = CoordinatorData.Snapshot->TeamShield;
	Data.DrawPileCount = CoordinatorData.Snapshot->DeckState.DrawPileCount;
	Data.HandCount = CoordinatorData.Snapshot->DeckState.HandCount;
	Data.DiscardPileCount = CoordinatorData.Snapshot->DeckState.DiscardPileCount;
	Data.ConsumePileCount = CoordinatorData.Snapshot->DeckState.ConsumePileCount;
	ViewModel->ApplyData(Data);
}

void UFinalBattleResourcePanelController::InitializeResourcePanel(UFinalBattleWidgetController* InCoordinator, UFinalBattleResourcePanelViewModel* InViewModel)
{
	InitializePanelController(InCoordinator);
	ViewModel = InViewModel;
}

void UFinalBattleResourcePanelController::RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData)
{
	if (ViewModel == nullptr || CoordinatorData.Snapshot == nullptr)
	{
		return;
	}

	FFinalBattleResourcePanelData Data;
	Data.bHasActiveBattle = CoordinatorData.Snapshot->BattleId.IsValid();
	Data.CurrentAP = CoordinatorData.Snapshot->CurrentAP;
	Data.CurrentEP = CoordinatorData.Snapshot->CurrentEP;
	Data.MaxEP = CoordinatorData.Snapshot->MaxEP;
	Data.bEPFull = Data.MaxEP > 0 && Data.CurrentEP >= Data.MaxEP;
	Data.MaxQiPipCount = 7;
	Data.ActiveQiPipCount = FMath::Clamp(Data.CurrentEP / 10, 0, Data.MaxQiPipCount);
	ViewModel->ApplyData(Data);
}

void UFinalBattleFeedbackPanelController::InitializeFeedback(UFinalBattleWidgetController* InCoordinator, UFinalBattleFeedbackPanelViewModel* InViewModel)
{
	InitializePanelController(InCoordinator);
	ViewModel = InViewModel;
}

void UFinalBattleFeedbackPanelController::RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData)
{
	if (ViewModel == nullptr || CoordinatorData.Snapshot == nullptr)
	{
		return;
	}

	FFinalBattleFeedbackPanelData Data;
	Data.FeedbackRejectReason = CoordinatorData.LastInteractionEvent.RejectReason;
	Data.FeedbackReasonTag = CoordinatorData.LastInteractionEvent.ReasonTag;

	if (!CoordinatorData.Snapshot->BattleId.IsValid())
	{
		Data.FeedbackText = NSLOCTEXT("FinalBattleHUD", "NoBattleFeedback", "通过控制台命令或地图按钮启动测试战斗后，这里会自动刷新。");
		ViewModel->ApplyData(Data);
		return;
	}

	const FText EffectiveFeedbackText = !CoordinatorData.LastInteractionFeedback.IsEmpty()
		? CoordinatorData.LastInteractionFeedback
		: CoordinatorData.LastInteractionEvent.Message;
	const bool bHasInteractionEvent = CoordinatorData.LastInteractionEvent.EventSequence > 0
		|| CoordinatorData.LastInteractionEvent.EventType != EFinalBattleEventType::Info
		|| CoordinatorData.LastInteractionEvent.RejectReason != EFinalBattleCommandRejectReason::None
		|| !EffectiveFeedbackText.IsEmpty();

	if (bHasInteractionEvent)
	{
		const FinalBattleEventPresentation::FEventPresentation EventPresentation =
			FinalBattleEventPresentation::BuildPresentation(CoordinatorData.LastInteractionEvent, *CoordinatorData.Snapshot, CoordinatorData.DataRegistry);
		Data.FeedbackTitleText = !EventPresentation.TitleText.IsEmpty()
			? EventPresentation.TitleText
			: ResolveBattleHUDEventFeedbackTitleText(CoordinatorData.LastInteractionEvent, EffectiveFeedbackText, CoordinatorData.Snapshot->ActiveRelics);
		Data.FeedbackText = FinalBattleEventPresentation::BuildCombinedBodyText(EventPresentation);
		if (Data.FeedbackText.IsEmpty())
		{
			Data.FeedbackText = EffectiveFeedbackText;
		}
	}

	ViewModel->ApplyData(Data);
}

void UFinalBattleContextPanelController::InitializeContext(UFinalBattleWidgetController* InCoordinator, UFinalBattleContextPanelViewModel* InViewModel)
{
	InitializePanelController(InCoordinator);
	ViewModel = InViewModel;
}

void UFinalBattleContextPanelController::RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData)
{
	if (ViewModel == nullptr || CoordinatorData.Snapshot == nullptr || CoordinatorData.RunSnapshot == nullptr)
	{
		return;
	}

	FFinalBattleContextPanelData Data;
	Data.bHasActiveBattle = CoordinatorData.Snapshot->BattleId.IsValid();
	Data.CurrentTargetText = ResolveTargetText(*CoordinatorData.Snapshot);
	Data.Gold = CoordinatorData.RunSnapshot->Gold;
	Data.RelicCount = CoordinatorData.RunSnapshot->RelicCount;
	Data.RunDeckCount = CoordinatorData.RunSnapshot->DeckCount;
	Data.DrawPileCount = CoordinatorData.Snapshot->DeckState.DrawPileCount;
	Data.HandCount = CoordinatorData.Snapshot->DeckState.HandCount;
	Data.DiscardPileCount = CoordinatorData.Snapshot->DeckState.DiscardPileCount;
	Data.OngoingZoneCount = CoordinatorData.Snapshot->DeckState.OngoingZoneCount;
	Data.ConsumePileCount = CoordinatorData.Snapshot->DeckState.ConsumePileCount;

	for (const FFinalBattleStatusViewData& TeamStatusView : CoordinatorData.Snapshot->TeamStatuses)
	{
		Data.TeamStatusTexts.Add(FormatStatusText(TeamStatusView, CoordinatorData.DataRegistry));
	}

	for (const FFinalBattleStartRelicInput& RelicInput : CoordinatorData.Snapshot->ActiveRelics)
	{
		Data.ActiveRelicTexts.Add(BuildActiveRelicSummaryText(RelicInput));
	}

	ViewModel->ApplyData(Data);
}

void UFinalBattleContextPanelController::InspectCardZone(const EFinalBattleCardZone Zone)
{
	if (Coordinator)
	{
		Coordinator->InspectCardZone(Zone);
	}
}

void UFinalBattleTeamPanelController::InitializeTeamPanel(UFinalBattleWidgetController* InCoordinator, UFinalBattleTeamPanelViewModel* InViewModel)
{
	InitializePanelController(InCoordinator);
	ViewModel = InViewModel;
}

void UFinalBattleTeamPanelController::RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData)
{
	if (ViewModel == nullptr || CoordinatorData.Snapshot == nullptr)
	{
		return;
	}

	TMap<FName, FText> CharacterDisplayNameByRuntimeId;
	for (const FFinalBattleCharacterViewData& CharacterView : CoordinatorData.Snapshot->Characters)
	{
		const FText CharacterName = !CharacterView.DisplayName.IsEmpty() ? CharacterView.DisplayName : FText::FromName(CharacterView.CharacterId.Value);
		CharacterDisplayNameByRuntimeId.Add(CharacterView.RuntimeUnitId, CharacterName);
	}

	TArray<FFinalBattleHUDTeamStatusEntry> AllTeamStatuses;
	auto AppendStatusEntry = [&AllTeamStatuses, &CharacterDisplayNameByRuntimeId, CoordinatorData](const FFinalBattleStatusViewData& StatusView)
	{
		FFinalBattleHUDTeamStatusEntry Entry;
		Entry.StatusInstanceId = StatusView.StatusInstanceId;
		Entry.StatusId = StatusView.StatusId;
		Entry.OwnerUnitId = StatusView.OwnerUnitId;
		Entry.OwnerDisplayName = StatusView.OwnerUnitId == TeamPlayerUnitId
			? NSLOCTEXT("FinalBattleHUD", "TeamStatusOwner", "队伍")
			: CharacterDisplayNameByRuntimeId.FindRef(StatusView.OwnerUnitId);
		Entry.DisplayName = ResolveStatusDisplayName(StatusView, CoordinatorData.DataRegistry);
		Entry.SummaryText = ResolveStatusSummaryText(StatusView, CoordinatorData.DataRegistry);
		Entry.CurrentStacks = StatusView.CurrentStacks;
		Entry.RemainingDuration = StatusView.RemainingDuration;
		AllTeamStatuses.Add(MoveTemp(Entry));
	};

	for (const FFinalBattleStatusViewData& TeamStatusView : CoordinatorData.Snapshot->TeamStatuses)
	{
		AppendStatusEntry(TeamStatusView);
	}

	for (const FFinalBattleCharacterStatusesViewData& CharacterStatusesView : CoordinatorData.Snapshot->CharacterStatuses)
	{
		for (const FFinalBattleStatusViewData& StatusView : CharacterStatusesView.StatusEntries)
		{
			AppendStatusEntry(StatusView);
		}
	}

	FFinalBattleHUDTeamPanelData Data;
	Data.bHasActiveBattle = CoordinatorData.Snapshot->BattleId.IsValid();
	Data.TeamCurrentHP = CoordinatorData.Snapshot->TeamCurrentHP;
	Data.TeamMaxHP = CoordinatorData.Snapshot->TeamMaxHP;
	Data.TeamHealthPercent = CalculateClampedPercent(Data.TeamCurrentHP, Data.TeamMaxHP);
	Data.TeamShield = CoordinatorData.Snapshot->TeamShield;
	Data.TeamShieldFramePercent = CalculateClampedPercent(Data.TeamShield, Data.TeamMaxHP);
	Data.bStatusDetailOpen = CoordinatorData.bTeamStatusDetailOpen;

	for (const FFinalBattleCharacterViewData& CharacterView : CoordinatorData.Snapshot->Characters)
	{
		FFinalBattleHUDTeamCharacterEntry CharacterEntry;
		CharacterEntry.RuntimeUnitId = CharacterView.RuntimeUnitId;
		CharacterEntry.DisplayName = !CharacterView.DisplayName.IsEmpty() ? CharacterView.DisplayName : FText::FromName(CharacterView.CharacterId.Value);
		CharacterEntry.ArtId = CharacterView.CharacterId.Value;
		if (CoordinatorData.RunSnapshot)
		{
			const FFinalRunCharacterViewData* RunCharacterView = CoordinatorData.RunSnapshot->Characters.FindByPredicate(
				[&CharacterView](const FFinalRunCharacterViewData& Candidate)
				{
					return Candidate.CharacterId == CharacterView.CharacterId;
				});
			if (RunCharacterView)
			{
				CharacterEntry.IconId = RunCharacterView->IconId;
				CharacterEntry.BreakthroughValue = RunCharacterView->BreakthroughValue;
				CharacterEntry.BreakthroughRequiredValue = RunCharacterView->BreakthroughRequiredValue;
				CharacterEntry.BreakthroughPercent = CalculateClampedPercent(
					RunCharacterView->BreakthroughValue,
					RunCharacterView->BreakthroughRequiredValue);
				CharacterEntry.bBreakthroughReady =
					RunCharacterView->BreakthroughRequiredValue > 0
					&& RunCharacterView->BreakthroughValue >= RunCharacterView->BreakthroughRequiredValue;
			}
		}
		CharacterEntry.CurrentStress = CharacterView.CurrentStress;
		CharacterEntry.StressCap = CharacterView.StressCap;
		CharacterEntry.StressPercent = CalculateClampedPercent(CharacterView.CurrentStress, CharacterView.StressCap);
		CharacterEntry.bCollapsed = CharacterView.bCollapsed;
		CharacterEntry.bCanActHint = !CharacterView.bCollapsed;
		Data.Characters.Add(MoveTemp(CharacterEntry));
	}

	constexpr int32 MaxPreviewStatusCount = 5;
	const int32 PreviewCount = FMath::Min(MaxPreviewStatusCount, AllTeamStatuses.Num());
	for (int32 Index = 0; Index < PreviewCount; ++Index)
	{
		Data.StatusPreviewEntries.Add(AllTeamStatuses[Index]);
	}
	Data.HiddenStatusCount = FMath::Max(0, AllTeamStatuses.Num() - PreviewCount);

	ViewModel->ApplyData(Data);
}

void UFinalBattleTeamPanelController::OpenTeamStatusDetail()
{
	if (Coordinator)
	{
		Coordinator->OpenTeamStatusDetail();
	}
}

bool UFinalBattleTeamPanelController::InspectCharacterByUnitId(const FName RuntimeUnitId)
{
	return Coordinator ? Coordinator->InspectCharacterByUnitId(RuntimeUnitId) : false;
}

void UFinalBattleTeamStatusDetailPanelController::InitializeTeamStatusDetailPanel(
	UFinalBattleWidgetController* InCoordinator,
	UFinalBattleTeamStatusDetailPanelViewModel* InViewModel)
{
	InitializePanelController(InCoordinator);
	ViewModel = InViewModel;
}

void UFinalBattleTeamStatusDetailPanelController::RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData)
{
	if (ViewModel == nullptr || CoordinatorData.Snapshot == nullptr)
	{
		return;
	}

	FFinalBattleHUDTeamStatusDetailData Data;
	Data.bOpen = CoordinatorData.bTeamStatusDetailOpen;
	if (!Data.bOpen)
	{
		ViewModel->ApplyData(Data);
		return;
	}

	TMap<FName, FText> CharacterDisplayNameByRuntimeId;
	for (const FFinalBattleCharacterViewData& CharacterView : CoordinatorData.Snapshot->Characters)
	{
		CharacterDisplayNameByRuntimeId.Add(
			CharacterView.RuntimeUnitId,
			!CharacterView.DisplayName.IsEmpty() ? CharacterView.DisplayName : FText::FromName(CharacterView.CharacterId.Value));
	}

	auto AppendStatusEntry = [&Data, &CharacterDisplayNameByRuntimeId, CoordinatorData](const FFinalBattleStatusViewData& StatusView)
	{
		FFinalBattleHUDTeamStatusEntry Entry;
		Entry.StatusInstanceId = StatusView.StatusInstanceId;
		Entry.StatusId = StatusView.StatusId;
		Entry.OwnerUnitId = StatusView.OwnerUnitId;
		Entry.OwnerDisplayName = StatusView.OwnerUnitId == TeamPlayerUnitId
			? NSLOCTEXT("FinalBattleHUD", "TeamStatusDetailOwner", "队伍")
			: CharacterDisplayNameByRuntimeId.FindRef(StatusView.OwnerUnitId);
		Entry.DisplayName = ResolveStatusDisplayName(StatusView, CoordinatorData.DataRegistry);
		Entry.SummaryText = ResolveStatusSummaryText(StatusView, CoordinatorData.DataRegistry);
		Entry.CurrentStacks = StatusView.CurrentStacks;
		Entry.RemainingDuration = StatusView.RemainingDuration;
		Data.Statuses.Add(MoveTemp(Entry));
	};

	for (const FFinalBattleStatusViewData& TeamStatusView : CoordinatorData.Snapshot->TeamStatuses)
	{
		AppendStatusEntry(TeamStatusView);
	}

	for (const FFinalBattleCharacterStatusesViewData& CharacterStatusesView : CoordinatorData.Snapshot->CharacterStatuses)
	{
		for (const FFinalBattleStatusViewData& StatusView : CharacterStatusesView.StatusEntries)
		{
			AppendStatusEntry(StatusView);
		}
	}

	ViewModel->ApplyData(Data);
}

void UFinalBattleTeamStatusDetailPanelController::ClearTeamStatusDetail()
{
	if (Coordinator)
	{
		Coordinator->ClearTeamStatusDetail();
	}
}

void UFinalBattleCharacterPanelController::InitializeCharacterPanel(UFinalBattleWidgetController* InCoordinator, UFinalBattleCharacterPanelViewModel* InViewModel)
{
	InitializePanelController(InCoordinator);
	ViewModel = InViewModel;
}

void UFinalBattleCharacterPanelController::RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData)
{
	if (ViewModel == nullptr || CoordinatorData.Snapshot == nullptr)
	{
		return;
	}

	TMap<FName, TArray<FText>> CharacterStatusTextsByOwner;
	for (const FFinalBattleCharacterStatusesViewData& CharacterStatusesView : CoordinatorData.Snapshot->CharacterStatuses)
	{
		TArray<FText>& CharacterStatusTexts = CharacterStatusTextsByOwner.FindOrAdd(CharacterStatusesView.OwnerUnitId);
		for (const FFinalBattleStatusViewData& StatusView : CharacterStatusesView.StatusEntries)
		{
			CharacterStatusTexts.Add(FormatStatusText(StatusView, CoordinatorData.DataRegistry));
		}
	}

	TArray<FFinalBattleHUDCharacterEntry> Entries;
	for (const FFinalBattleCharacterViewData& CharacterView : CoordinatorData.Snapshot->Characters)
	{
		FFinalBattleHUDCharacterEntry Entry;
		Entry.RuntimeUnitId = CharacterView.RuntimeUnitId;
		Entry.DisplayName = !CharacterView.DisplayName.IsEmpty() ? CharacterView.DisplayName : FText::FromName(CharacterView.CharacterId.Value);
		if (CoordinatorData.RunSnapshot)
		{
			const FFinalRunCharacterViewData* RunCharacterView = CoordinatorData.RunSnapshot->Characters.FindByPredicate(
				[&CharacterView](const FFinalRunCharacterViewData& Candidate)
				{
					return Candidate.CharacterId == CharacterView.CharacterId;
				});
			if (RunCharacterView)
			{
				Entry.IconId = RunCharacterView->IconId;
				Entry.Level = RunCharacterView->Level;
				Entry.BreakthroughValue = RunCharacterView->BreakthroughValue;
				Entry.BreakthroughRequiredValue = RunCharacterView->BreakthroughRequiredValue;
				Entry.BreakthroughFillNormalized =
					RunCharacterView->BreakthroughRequiredValue > 0
						? FMath::Clamp(
							static_cast<float>(RunCharacterView->BreakthroughValue) / static_cast<float>(RunCharacterView->BreakthroughRequiredValue),
							0.0f,
							1.0f)
						: 0.0f;
				Entry.bBreakthroughReady =
					RunCharacterView->BreakthroughRequiredValue > 0
					&& RunCharacterView->BreakthroughValue >= RunCharacterView->BreakthroughRequiredValue;
			}
		}
		Entry.ArtId = CharacterView.CharacterId.Value;
		Entry.CurrentStress = CharacterView.CurrentStress;
		Entry.StressCap = CharacterView.StressCap;
		Entry.StressPercent = CharacterView.StressCap > 0
			? FMath::Clamp(static_cast<float>(CharacterView.CurrentStress) / static_cast<float>(CharacterView.StressCap), 0.0f, 1.0f)
			: 0.0f;
		Entry.bCollapsed = CharacterView.bCollapsed;
		Entry.bCanActHint = !CharacterView.bCollapsed;
		Entry.CurrentAwakenCount = CharacterView.CurrentAwakenCount;
		Entry.CurrentAwakenThreshold = CharacterView.CurrentAwakenThreshold;
		Entry.CollapseCount = CharacterView.CollapseCount;
		Entry.VitalShare = CharacterView.VitalShare;
		Entry.StateText = CharacterView.bCollapsed ? FText::FromString(TEXT("已崩溃")) : FText::FromString(TEXT("可行动"));
		Entry.StatusTexts = CharacterStatusTextsByOwner.FindRef(CharacterView.RuntimeUnitId);
		Entries.Add(MoveTemp(Entry));
	}

	ViewModel->ApplyEntries(Entries);
}

bool UFinalBattleCharacterPanelController::InspectCharacterByUnitId(const FName RuntimeUnitId)
{
	return Coordinator ? Coordinator->InspectCharacterByUnitId(RuntimeUnitId) : false;
}

void UFinalBattleEnemyPanelController::InitializeEnemyPanel(UFinalBattleWidgetController* InCoordinator, UFinalBattleEnemyPanelViewModel* InViewModel)
{
	InitializePanelController(InCoordinator);
	ViewModel = InViewModel;
}

void UFinalBattleEnemyPanelController::RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData)
{
	if (ViewModel == nullptr || CoordinatorData.Snapshot == nullptr)
	{
		return;
	}

	TMap<FName, FText> CharacterDisplayNameByRuntimeId;
	for (const FFinalBattleCharacterViewData& CharacterView : CoordinatorData.Snapshot->Characters)
	{
		const FText CharacterName = !CharacterView.DisplayName.IsEmpty() ? CharacterView.DisplayName : FText::FromName(CharacterView.CharacterId.Value);
		CharacterDisplayNameByRuntimeId.Add(CharacterView.RuntimeUnitId, CharacterName);
	}

	TMap<FName, TArray<FText>> EnemyStatusTextsByOwner;
	for (const FFinalBattleStatusViewData& StatusView : CoordinatorData.Snapshot->Statuses)
	{
		if (StatusView.OwnerUnitId == TeamPlayerUnitId || CharacterDisplayNameByRuntimeId.Contains(StatusView.OwnerUnitId))
		{
			continue;
		}

		EnemyStatusTextsByOwner.FindOrAdd(StatusView.OwnerUnitId).Add(FormatStatusText(StatusView, CoordinatorData.DataRegistry));
	}

	TArray<FFinalBattleHUDEnemyEntry> Entries;
	for (const FFinalBattleEnemyViewData& EnemyView : CoordinatorData.Snapshot->Enemies)
	{
		FFinalBattleHUDEnemyEntry Entry;
		Entry.RuntimeUnitId = EnemyView.RuntimeUnitId;
		Entry.DisplayName = EnemyView.DisplayName;
		Entry.ArtId = EnemyView.EnemyId.Value;
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
		Entry.IntentText = !EnemyView.CurrentIntent.PreviewText.IsEmpty()
			? EnemyView.CurrentIntent.PreviewText
			: EnemyView.IntentText;
		Entry.bSelected = EnemyView.RuntimeUnitId == CoordinatorData.SelectedEnemyUnitId;
		Entry.bActedThisRound = EnemyView.bActedThisRound;
		Entry.StatusTexts = EnemyStatusTextsByOwner.FindRef(EnemyView.RuntimeUnitId);
		Entries.Add(MoveTemp(Entry));
	}

	ViewModel->ApplyEntries(Entries);
}

bool UFinalBattleEnemyPanelController::SelectEnemyByUnitId(const FName RuntimeUnitId)
{
	return Coordinator ? Coordinator->SelectEnemyByUnitId(RuntimeUnitId) : false;
}

void UFinalBattleEnemyDetailPanelController::InitializeEnemyDetailPanel(UFinalBattleWidgetController* InCoordinator, UFinalBattleEnemyDetailPanelViewModel* InViewModel)
{
	InitializePanelController(InCoordinator);
	ViewModel = InViewModel;
}

void UFinalBattleEnemyDetailPanelController::RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData)
{
	if (ViewModel == nullptr || CoordinatorData.Snapshot == nullptr || CoordinatorData.InspectedEnemyUnitId.IsNone())
	{
		if (ViewModel)
		{
			ViewModel->ApplyData(FFinalBattleHUDEnemyDetailData{});
		}
		return;
	}

	const FFinalBattleEnemyViewData* EnemyView = CoordinatorData.Snapshot->Enemies.FindByPredicate(
		[&CoordinatorData](const FFinalBattleEnemyViewData& Candidate)
		{
			return Candidate.RuntimeUnitId == CoordinatorData.InspectedEnemyUnitId;
		});

	if (EnemyView == nullptr)
	{
		ViewModel->ApplyData(FFinalBattleHUDEnemyDetailData{});
		return;
	}

	FFinalBattleHUDEnemyDetailData Data;
	Data.bHasEnemy = true;
	Data.RuntimeUnitId = EnemyView->RuntimeUnitId;
	Data.EnemyId = EnemyView->EnemyId;
	Data.DisplayName = !EnemyView->DisplayName.IsEmpty() ? EnemyView->DisplayName : FText::FromName(EnemyView->RuntimeUnitId);
	Data.CurrentHP = EnemyView->CurrentHP;
	Data.MaxHP = EnemyView->MaxHP;
	Data.HealthPercent = CalculateClampedPercent(EnemyView->CurrentHP, EnemyView->MaxHP);
	Data.CurrentShield = EnemyView->CurrentShield;
	Data.ShieldFramePercent = CalculateClampedPercent(EnemyView->CurrentShield, EnemyView->MaxHP);
	Data.CurrentBreakValue = EnemyView->CurrentBreakValue;
	Data.MaxBreakValue = EnemyView->MaxBreakValue;
	Data.BreakPercent = CalculateClampedPercent(EnemyView->CurrentBreakValue, EnemyView->MaxBreakValue);
	Data.CurrentInitiative = EnemyView->CurrentInitiative;
	Data.InitiativeText = FText::AsNumber(EnemyView->CurrentInitiative);
	Data.IntentText = !EnemyView->CurrentIntent.PreviewText.IsEmpty()
		? EnemyView->CurrentIntent.PreviewText
		: EnemyView->IntentText;
	Data.IntentNameText = !EnemyView->CurrentIntent.DisplayName.IsEmpty()
		? EnemyView->CurrentIntent.DisplayName
		: Data.IntentText;
	Data.IntentType = EnemyView->CurrentIntent.IntentType;
	Data.IntentIconId = EnemyView->CurrentIntent.IconId.IsNone()
		? EnemyView->CurrentIntentId
		: EnemyView->CurrentIntent.IconId;
	Data.PhaseProgressText = FormatEnemyPhaseProgressText(EnemyView->PhaseProgress);
	Data.bIsCurrentBattleTarget = EnemyView->RuntimeUnitId == CoordinatorData.SelectedEnemyUnitId;
	Data.bIsInspected = true;
	Data.bIsAlive = EnemyView->CurrentHP > 0;

	const UFinalEnemyDefinition* EnemyDefinition =
		CoordinatorData.DataRegistry != nullptr && EnemyView->EnemyId.IsValid()
			? CoordinatorData.DataRegistry->FindEnemyDefinition(EnemyView->EnemyId)
			: nullptr;
	Data.EnemyRankTag = ResolveEnemyRankTag(EnemyDefinition);

	for (const FFinalBattleStatusViewData& StatusView : CoordinatorData.Snapshot->Statuses)
	{
		if (StatusView.OwnerUnitId != EnemyView->RuntimeUnitId)
		{
			continue;
		}

		FFinalBattleHUDEnemyDetailStatusEntry StatusEntry;
		StatusEntry.StatusId = StatusView.StatusId;
		StatusEntry.DisplayName = ResolveStatusDisplayName(StatusView, CoordinatorData.DataRegistry);
		StatusEntry.SummaryText = ResolveStatusSummaryText(StatusView, CoordinatorData.DataRegistry);
		StatusEntry.CurrentStacks = StatusView.CurrentStacks;
		StatusEntry.RemainingDuration = StatusView.RemainingDuration;
		Data.Statuses.Add(MoveTemp(StatusEntry));
	}

	ViewModel->ApplyData(Data);
}

void UFinalBattleEnemyDetailPanelController::ClearInspectedEnemy()
{
	if (Coordinator)
	{
		Coordinator->ClearInspectedEnemy();
	}
}

void UFinalBattleCharacterDetailPanelController::InitializeCharacterDetailPanel(UFinalBattleWidgetController* InCoordinator, UFinalBattleCharacterDetailPanelViewModel* InViewModel)
{
	InitializePanelController(InCoordinator);
	ViewModel = InViewModel;
}

void UFinalBattleCharacterDetailPanelController::RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData)
{
	if (ViewModel == nullptr || CoordinatorData.Snapshot == nullptr || CoordinatorData.InspectedCharacterUnitId.IsNone())
	{
		if (ViewModel)
		{
			ViewModel->ApplyData(FFinalBattleHUDCharacterDetailData{});
		}
		return;
	}

	const FFinalBattleCharacterViewData* CharacterView = CoordinatorData.Snapshot->Characters.FindByPredicate(
		[&CoordinatorData](const FFinalBattleCharacterViewData& Candidate)
		{
			return Candidate.RuntimeUnitId == CoordinatorData.InspectedCharacterUnitId;
		});

	if (CharacterView == nullptr)
	{
		ViewModel->ApplyData(FFinalBattleHUDCharacterDetailData{});
		return;
	}

	FFinalBattleHUDCharacterDetailData Data;
	Data.bHasCharacter = true;
	Data.RuntimeUnitId = CharacterView->RuntimeUnitId;
	Data.CharacterId = CharacterView->CharacterId;
	Data.DisplayName = !CharacterView->DisplayName.IsEmpty() ? CharacterView->DisplayName : FText::FromName(CharacterView->CharacterId.Value);
	Data.ArtId = CharacterView->CharacterId.Value;
	Data.CurrentStress = CharacterView->CurrentStress;
	Data.StressCap = CharacterView->StressCap;
	Data.StressPercent = CalculateClampedPercent(CharacterView->CurrentStress, CharacterView->StressCap);
	Data.VitalShare = CharacterView->VitalShare;
	Data.CurrentAwakenCount = CharacterView->CurrentAwakenCount;
	Data.CurrentAwakenThreshold = CharacterView->CurrentAwakenThreshold;
	Data.CollapseCount = CharacterView->CollapseCount;
	Data.bCollapsed = CharacterView->bCollapsed;
	Data.bIsInspected = true;
	Data.RuntimeAttack = CharacterView->RuntimeAttack;
	Data.RuntimeDefense = CharacterView->RuntimeDefense;
	Data.RuntimeBreakRate = CharacterView->RuntimeBreakRate;
	Data.RuntimeCritChance = CharacterView->RuntimeCritChance;
	Data.RuntimeCritDamage = CharacterView->RuntimeCritDamage;

	if (CoordinatorData.RunSnapshot)
	{
		const FFinalRunCharacterViewData* RunCharacterView = CoordinatorData.RunSnapshot->Characters.FindByPredicate(
			[&CharacterView](const FFinalRunCharacterViewData& Candidate)
			{
				return Candidate.CharacterId == CharacterView->CharacterId;
			});
		if (RunCharacterView)
		{
			Data.IconId = RunCharacterView->IconId;
			Data.Level = RunCharacterView->Level;
			Data.BreakthroughValue = RunCharacterView->BreakthroughValue;
			Data.BreakthroughRequiredValue = RunCharacterView->BreakthroughRequiredValue;
			Data.BreakthroughFillNormalized = CalculateClampedPercent(
				RunCharacterView->BreakthroughValue,
				RunCharacterView->BreakthroughRequiredValue);
			Data.bBreakthroughReady =
				RunCharacterView->BreakthroughRequiredValue > 0
				&& RunCharacterView->BreakthroughValue >= RunCharacterView->BreakthroughRequiredValue;
			Data.RootBone = RunCharacterView->RootBone;
			Data.Insight = RunCharacterView->Insight;
			Data.KillingIntent = RunCharacterView->KillingIntent;
		}
	}

	const UFinalCharacterDefinition* CharacterDefinition =
		CoordinatorData.DataRegistry != nullptr && CharacterView->CharacterId.IsValid()
			? CoordinatorData.DataRegistry->FindCharacterDefinition(CharacterView->CharacterId)
			: nullptr;
	if (CharacterDefinition)
	{
		Data.RoleText = BuildGameplayTagSummaryText(CharacterDefinition->RoleTags);
	}

	for (const FFinalBattleCharacterStatusesViewData& CharacterStatusesView : CoordinatorData.Snapshot->CharacterStatuses)
	{
		if (CharacterStatusesView.OwnerUnitId != CharacterView->RuntimeUnitId)
		{
			continue;
		}

		for (const FFinalBattleStatusViewData& StatusView : CharacterStatusesView.StatusEntries)
		{
			FFinalBattleHUDCharacterDetailStatusEntry StatusEntry;
			StatusEntry.StatusId = StatusView.StatusId;
			StatusEntry.DisplayName = ResolveStatusDisplayName(StatusView, CoordinatorData.DataRegistry);
			StatusEntry.SummaryText = ResolveStatusSummaryText(StatusView, CoordinatorData.DataRegistry);
			StatusEntry.CurrentStacks = StatusView.CurrentStacks;
			StatusEntry.RemainingDuration = StatusView.RemainingDuration;
			Data.Statuses.Add(MoveTemp(StatusEntry));
		}
	}

	for (const FFinalBattlePassiveViewData& PassiveView : CoordinatorData.Snapshot->Passives)
	{
		if (PassiveView.OwnerUnitId != CharacterView->RuntimeUnitId)
		{
			continue;
		}

		FFinalBattleHUDCharacterDetailPassiveEntry PassiveEntry;
		PassiveEntry.PassiveId = PassiveView.PassiveId;
		PassiveEntry.DisplayName = !PassiveView.DisplayName.IsEmpty()
			? PassiveView.DisplayName
			: FText::FromName(PassiveView.PassiveId.Value);
		PassiveEntry.SummaryText = ResolvePassiveSummaryText(PassiveView, CoordinatorData.DataRegistry);
		PassiveEntry.CurrentStacks = PassiveView.CurrentStacks;
		PassiveEntry.RemainingDuration = PassiveView.RemainingDuration;
		Data.Passives.Add(MoveTemp(PassiveEntry));
	}

	const FFinalBattleUltimateViewData* UltimateView = CoordinatorData.Snapshot->CharacterUltimates.FindByPredicate(
		[&CharacterView](const FFinalBattleUltimateViewData& Candidate)
		{
			return Candidate.OwnerUnitId == CharacterView->RuntimeUnitId;
		});
	if (UltimateView)
	{
		Data.UltimateNameText = !UltimateView->DisplayName.IsEmpty()
			? UltimateView->DisplayName
			: FText::FromName(UltimateView->UltimateId.Value);
		Data.UltimateCostEP = UltimateView->CostEP;
		Data.bUltimateDefinitionReady = UltimateView->bDefinitionReady;
		Data.bUltimateCanActivate = UltimateView->bCanActivate;
		Data.bUltimateBlockedByCollapse = UltimateView->bBlockedByCollapse;
		Data.bUltimateUsedThisBattle = UltimateView->bUsedThisBattle;
		if (CoordinatorData.DataRegistry && UltimateView->UltimateId.IsValid())
		{
			if (const UFinalUltimateDefinition* UltimateDefinition = CoordinatorData.DataRegistry->FindUltimateDefinition(UltimateView->UltimateId))
			{
				if (!UltimateDefinition->DisplayName.IsEmpty())
				{
					Data.UltimateNameText = UltimateDefinition->DisplayName;
				}
				Data.UltimateRulesText = UltimateDefinition->RulesText;
			}
		}
	}

	ViewModel->ApplyData(Data);
}

void UFinalBattleCharacterDetailPanelController::ClearInspectedCharacter()
{
	if (Coordinator)
	{
		Coordinator->ClearInspectedCharacter();
	}
}

void UFinalBattleHandPanelController::InitializeHandPanel(UFinalBattleWidgetController* InCoordinator, UFinalBattleHandPanelViewModel* InViewModel)
{
	InitializePanelController(InCoordinator);
	ViewModel = InViewModel;
}

void UFinalBattleHandPanelController::RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData)
{
	if (ViewModel == nullptr || CoordinatorData.Snapshot == nullptr)
	{
		return;
	}

	TMap<FName, FText> CharacterDisplayNameByRuntimeId;
	for (const FFinalBattleCharacterViewData& CharacterView : CoordinatorData.Snapshot->Characters)
	{
		const FText CharacterName = !CharacterView.DisplayName.IsEmpty() ? CharacterView.DisplayName : FText::FromName(CharacterView.CharacterId.Value);
		CharacterDisplayNameByRuntimeId.Add(CharacterView.RuntimeUnitId, CharacterName);
	}

	TArray<FFinalBattleHUDCardEntry> Entries;
	for (const FFinalBattleCardViewData& CardView : CoordinatorData.Snapshot->HandCards)
	{
		FFinalBattleHUDCardEntry Entry;
		Entry.CardInstanceId = CardView.CardInstanceId;
		Entry.DisplayName = CardView.DisplayName;
		Entry.ArtId = CardView.CardId.Value;
		Entry.RulesText = FText::GetEmpty();
		if (CoordinatorData.DataRegistry && CardView.CardId.IsValid())
		{
			if (const UFinalCardDefinition* CardDefinition = CoordinatorData.DataRegistry->FindCardDefinition(CardView.CardId))
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

		Entry.BaseCostAP = CardView.BaseCostAP;
		Entry.RuntimeCostAP = CardView.RuntimeCostAP;
		Entry.CardType = CardView.CardType;
		Entry.TargetRequirement = CardView.TargetRequirement;
		Entry.TypeText = FormatCardTypeText(CardView.CardType);
		Entry.KeywordText = FormatKeywordText(CardView.RuntimeKeywords);
		Entry.ResolvedRulesText = CardView.ResolvedRulesText;
		ApplyBattleHUDCardAPPlayHint(Entry, CoordinatorData.Snapshot->CurrentAP);
		Entry.bRetained = CardView.bRetained;
		Entry.bCollapsedCard = CardView.bCollapsedCard;
		Entries.Add(MoveTemp(Entry));
	}

	ViewModel->ApplyEntries(Entries);
}

bool UFinalBattleHandPanelController::PlayCardByHandIndex(const int32 HandIndex)
{
	return Coordinator ? Coordinator->RequestPlayCardByHandIndex(HandIndex) : false;
}

bool UFinalBattleHandPanelController::PlayCardByHandIndexWithTarget(const int32 HandIndex, const FName TargetUnitId)
{
	return Coordinator ? Coordinator->RequestPlayCardByHandIndexWithTarget(HandIndex, TargetUnitId) : false;
}

void UFinalBattleCardZoneDetailPanelController::InitializeCardZoneDetailPanel(UFinalBattleWidgetController* InCoordinator, UFinalBattleCardZoneDetailPanelViewModel* InViewModel)
{
	InitializePanelController(InCoordinator);
	ViewModel = InViewModel;
}

void UFinalBattleCardZoneDetailPanelController::RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData)
{
	if (ViewModel == nullptr || CoordinatorData.Snapshot == nullptr)
	{
		return;
	}

	FFinalBattleHUDCardZoneDetailData Data;
	Data.bIsOpen = CoordinatorData.bCardZoneDetailOpen;
	Data.bHasActiveBattle = CoordinatorData.Snapshot->BattleId.IsValid();
	Data.SelectedZone = CoordinatorData.SelectedCardZone;
	Data.DrawPileCount = CoordinatorData.Snapshot->DeckState.DrawPileCount;
	Data.HandCount = CoordinatorData.Snapshot->DeckState.HandCount;
	Data.DiscardPileCount = CoordinatorData.Snapshot->DeckState.DiscardPileCount;
	Data.OngoingZoneCount = CoordinatorData.Snapshot->DeckState.OngoingZoneCount;
	Data.ConsumePileCount = CoordinatorData.Snapshot->DeckState.ConsumePileCount;

	if (!Data.bIsOpen || !Data.bHasActiveBattle)
	{
		ViewModel->ApplyData(Data);
		return;
	}

	TMap<FName, FText> CharacterDisplayNameByRuntimeId;
	for (const FFinalBattleCharacterViewData& CharacterView : CoordinatorData.Snapshot->Characters)
	{
		CharacterDisplayNameByRuntimeId.Add(
			CharacterView.RuntimeUnitId,
			!CharacterView.DisplayName.IsEmpty() ? CharacterView.DisplayName : FText::FromName(CharacterView.CharacterId.Value));
	}

	const FFinalBattleCardZoneViewData* ZoneView = CoordinatorData.Snapshot->CardZones.FindByPredicate(
		[&Data](const FFinalBattleCardZoneViewData& Candidate)
		{
			return Candidate.Zone == Data.SelectedZone;
		});

	if (ZoneView == nullptr)
	{
		Data.TitleText = FormatCardZoneText(Data.SelectedZone);
		ViewModel->ApplyData(Data);
		return;
	}

	Data.TitleText = FText::Format(
		NSLOCTEXT("FinalBattleHUD", "CardZoneDetailTitleFormat", "{0} {1}"),
		!ZoneView->DisplayName.IsEmpty() ? ZoneView->DisplayName : FormatCardZoneText(ZoneView->Zone),
		FText::AsNumber(ZoneView->Count));
	Data.Count = ZoneView->Count;
	Data.Entries.Reserve(ZoneView->Cards.Num());

	for (const FFinalBattleCardZoneEntryViewData& CardView : ZoneView->Cards)
	{
		FFinalBattleHUDCardZoneEntry Entry;
		Entry.CardInstanceId = CardView.CardInstanceId;
		Entry.CardId = CardView.CardId;
		Entry.OwnerUnitId = CardView.RuntimeOwnerUnitId;
		Entry.OwnerDisplayName = CharacterDisplayNameByRuntimeId.FindRef(CardView.RuntimeOwnerUnitId);
		if (Entry.OwnerDisplayName.IsEmpty() && !CardView.RuntimeOwnerUnitId.IsNone())
		{
			Entry.OwnerDisplayName = FText::FromName(CardView.RuntimeOwnerUnitId);
		}
		Entry.DisplayName = !CardView.DisplayName.IsEmpty() ? CardView.DisplayName : FText::FromName(CardView.CardId.Value);
		Entry.CardType = CardView.CardType;
		Entry.TypeText = FormatCardTypeText(CardView.CardType);
		Entry.BaseCostAP = CardView.BaseCostAP;
		Entry.RuntimeCostAP = CardView.RuntimeCostAP;
		Entry.KeywordText = FormatKeywordText(CardView.RuntimeKeywords);
		Entry.RulesText = CardView.ResolvedRulesText;
		Entry.bRetained = CardView.bRetained;
		Entry.bConsumeOnPlay = CardView.bConsumeOnPlay;
		Entry.bOngoingCard = CardView.bOngoingCard;
		Entry.bGeneratedCard = CardView.bGeneratedCard;
		Entry.bTemporaryCard = CardView.bTemporaryCard;
		Data.Entries.Add(MoveTemp(Entry));
	}

	ViewModel->ApplyData(Data);
}

void UFinalBattleCardZoneDetailPanelController::SetSelectedCardZone(const EFinalBattleCardZone Zone)
{
	if (Coordinator)
	{
		Coordinator->SetSelectedCardZone(Zone);
	}
}

void UFinalBattleCardZoneDetailPanelController::ClearCardZoneDetail()
{
	if (Coordinator)
	{
		Coordinator->ClearCardZoneDetail();
	}
}

void UFinalBattleUltimatePanelController::InitializeUltimatePanel(UFinalBattleWidgetController* InCoordinator, UFinalBattleUltimatePanelViewModel* InViewModel)
{
	InitializePanelController(InCoordinator);
	ViewModel = InViewModel;
}

void UFinalBattleUltimatePanelController::RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData)
{
	if (ViewModel == nullptr || CoordinatorData.Snapshot == nullptr)
	{
		return;
	}

	TMap<FName, FText> CharacterDisplayNameByRuntimeId;
	for (const FFinalBattleCharacterViewData& CharacterView : CoordinatorData.Snapshot->Characters)
	{
		const FText CharacterName = !CharacterView.DisplayName.IsEmpty() ? CharacterView.DisplayName : FText::FromName(CharacterView.CharacterId.Value);
		CharacterDisplayNameByRuntimeId.Add(CharacterView.RuntimeUnitId, CharacterName);
	}

	TArray<FFinalBattleHUDUltimateEntry> Entries;
	for (const FFinalBattleUltimateViewData& UltimateView : CoordinatorData.Snapshot->CharacterUltimates)
	{
		FFinalBattleHUDUltimateEntry Entry;
		Entry.RuntimeUnitId = UltimateView.OwnerUnitId;
		Entry.DisplayName = ResolveUltimateDisplayName(UltimateView, CoordinatorData.DataRegistry, CharacterDisplayNameByRuntimeId);
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
				FText::AsNumber(CoordinatorData.Snapshot->CurrentEP),
				FText::AsNumber(UltimateView.CostEP));
		}
		else if (UltimateView.bUsedThisBattle)
		{
			Entry.StatusText = NSLOCTEXT("FinalBattleHUD", "UltimateUsedThisBattleState", "本战已释放");
		}
		else if (CoordinatorData.Snapshot->CurrentEP < UltimateView.CostEP)
		{
			Entry.StatusText = FText::Format(
				NSLOCTEXT("FinalBattleHUD", "UltimateInsufficientEPState", "EP不足 | EP {0}/{1}"),
				FText::AsNumber(CoordinatorData.Snapshot->CurrentEP),
				FText::AsNumber(UltimateView.CostEP));
		}
		else
		{
			Entry.StatusText = NSLOCTEXT("FinalBattleHUD", "UltimateUnavailableState", "当前不可释放");
		}

		Entries.Add(MoveTemp(Entry));
	}

	ViewModel->ApplyEntries(Entries);
}

bool UFinalBattleUltimatePanelController::PlayUltimateByCharacterIndex(const int32 CharacterIndex)
{
	return Coordinator ? Coordinator->PlayUltimateByCharacterIndex(CharacterIndex) : false;
}

void UFinalBattleRecentEventPanelController::InitializeRecentEventPanel(UFinalBattleWidgetController* InCoordinator, UFinalBattleRecentEventPanelViewModel* InViewModel)
{
	InitializePanelController(InCoordinator);
	ViewModel = InViewModel;
}

void UFinalBattleRecentEventPanelController::RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData)
{
	if (ViewModel == nullptr || CoordinatorData.Snapshot == nullptr || CoordinatorData.BattleEvents == nullptr)
	{
		return;
	}

	constexpr int32 MaxLogEntries = 2;
	const int32 StartIndex = FMath::Max(CoordinatorData.BattleEvents->Num() - MaxLogEntries, 0);

	TArray<FFinalBattleHUDLogEntry> Entries;
	for (int32 Index = StartIndex; Index < CoordinatorData.BattleEvents->Num(); ++Index)
	{
		const FFinalBattleEvent& Event = (*CoordinatorData.BattleEvents)[Index];
		const FinalBattleEventPresentation::FEventPresentation EventPresentation =
			FinalBattleEventPresentation::BuildPresentation(Event, *CoordinatorData.Snapshot, CoordinatorData.DataRegistry);

		FFinalBattleHUDLogEntry Entry;
		Entry.EventSequence = Event.EventSequence;
		Entry.EventType = Event.EventType;
		Entry.Round = Event.Round;
		Entry.TitleText = EventPresentation.TitleText;
		Entry.SummaryText = EventPresentation.SummaryText;
		Entry.DetailText = EventPresentation.DetailText;
		Entries.Add(MoveTemp(Entry));
	}

	ViewModel->ApplyEntries(Entries);
}

void UFinalBattleActionPanelController::InitializeActionPanel(UFinalBattleWidgetController* InCoordinator, UFinalBattleActionPanelViewModel* InViewModel)
{
	InitializePanelController(InCoordinator);
	ViewModel = InViewModel;
}

void UFinalBattleActionPanelController::RefreshFromCoordinatorData(const FFinalBattleHUDCoordinatorData& CoordinatorData)
{
	if (ViewModel == nullptr || CoordinatorData.Snapshot == nullptr)
	{
		return;
	}

	FFinalBattleActionPanelData Data;
	Data.bHasActiveBattle = CoordinatorData.Snapshot->BattleId.IsValid();
	Data.DiscardPileCount = CoordinatorData.Snapshot->DeckState.DiscardPileCount;
	Data.ConsumePileCount = CoordinatorData.Snapshot->DeckState.ConsumePileCount;
	ViewModel->ApplyData(Data);
}

bool UFinalBattleActionPanelController::EndTurn()
{
	return Coordinator ? Coordinator->EndTurn() : false;
}

void UFinalBattleActionPanelController::OpenDebugOverlay()
{
	if (Coordinator)
	{
		Coordinator->OpenDebugOverlay();
	}
}

void UFinalBattleActionPanelController::OpenEventLedgerOverlay()
{
	if (Coordinator)
	{
		Coordinator->OpenEventLedgerOverlay();
	}
}
