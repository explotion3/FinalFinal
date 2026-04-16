#pragma once

#include "CoreMinimal.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalStatusDefinition.h"
#include "Battle/Definitions/FinalUltimateDefinition.h"
#include "Events/FinalBattleEvent.h"
#include "Queries/FinalBattleSnapshot.h"
#include "Queries/FinalDataRegistry.h"

namespace FinalBattleEventPresentation
{
struct FEventPresentation
{
	FText EventTypeText;
	FText TitleText;
	FText SummaryText;
	FText DetailText;
	FText ShortWorldText;
	FText LedgerText;
};

inline FString JoinLines(const TArray<FText>& Lines, const TCHAR* Separator)
{
	TArray<FString> Segments;
	for (const FText& Line : Lines)
	{
		if (!Line.IsEmpty())
		{
			Segments.Add(Line.ToString());
		}
	}

	return FString::Join(Segments, Separator);
}

inline FText ResolveUnitDisplayName(const FFinalBattleSnapshot& Snapshot, const FName RuntimeUnitId)
{
	if (RuntimeUnitId.IsNone())
	{
		return FText::GetEmpty();
	}

	if (const FFinalBattleCharacterViewData* CharacterView = Snapshot.Characters.FindByPredicate(
			[RuntimeUnitId](const FFinalBattleCharacterViewData& Candidate)
			{
				return Candidate.RuntimeUnitId == RuntimeUnitId;
			}))
	{
		return !CharacterView->DisplayName.IsEmpty()
			? CharacterView->DisplayName
			: FText::FromName(RuntimeUnitId);
	}

	if (const FFinalBattleEnemyViewData* EnemyView = Snapshot.Enemies.FindByPredicate(
			[RuntimeUnitId](const FFinalBattleEnemyViewData& Candidate)
			{
				return Candidate.RuntimeUnitId == RuntimeUnitId;
			}))
	{
		return !EnemyView->DisplayName.IsEmpty()
			? EnemyView->DisplayName
			: FText::FromName(RuntimeUnitId);
	}

	return FText::FromName(RuntimeUnitId);
}

inline FText ResolveCardDisplayName(const FFinalCardId& CardId, const UFinalDataRegistry* DataRegistry)
{
	if (!CardId.IsValid())
	{
		return FText::GetEmpty();
	}

	if (DataRegistry != nullptr)
	{
		if (const UFinalCardDefinition* CardDefinition = DataRegistry->FindCardDefinition(CardId))
		{
			if (!CardDefinition->DisplayName.IsEmpty())
			{
				return CardDefinition->DisplayName;
			}
		}
	}

	return FText::FromName(CardId.Value);
}

inline FText ResolveUltimateDisplayName(const FFinalUltimateId& UltimateId, const UFinalDataRegistry* DataRegistry)
{
	if (!UltimateId.IsValid())
	{
		return FText::GetEmpty();
	}

	if (DataRegistry != nullptr)
	{
		if (const UFinalUltimateDefinition* UltimateDefinition = DataRegistry->FindUltimateDefinition(UltimateId))
		{
			if (!UltimateDefinition->DisplayName.IsEmpty())
			{
				return UltimateDefinition->DisplayName;
			}
		}
	}

	return FText::FromName(UltimateId.Value);
}

inline FText ResolveStatusDisplayName(const FFinalStatusId& StatusId, const UFinalDataRegistry* DataRegistry)
{
	if (!StatusId.IsValid())
	{
		return FText::GetEmpty();
	}

	if (DataRegistry != nullptr)
	{
		if (const UFinalStatusDefinition* StatusDefinition = DataRegistry->FindStatusDefinition(StatusId))
		{
			if (!StatusDefinition->DisplayName.IsEmpty())
			{
				return StatusDefinition->DisplayName;
			}
		}
	}

	return FText::FromName(StatusId.Value);
}

inline FText ResolveRelicDisplayName(const FFinalBattleSnapshot& Snapshot, const FFinalRelicId& RelicId)
{
	if (!RelicId.IsValid())
	{
		return FText::GetEmpty();
	}

	if (const FFinalBattleStartRelicInput* RelicInput = Snapshot.ActiveRelics.FindByPredicate(
			[&RelicId](const FFinalBattleStartRelicInput& Candidate)
			{
				return Candidate.RelicId == RelicId;
			}))
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

	return FText::FromName(RelicId.Value);
}

inline FText GetEventTypeText(const EFinalBattleEventType EventType)
{
	switch (EventType)
	{
	case EFinalBattleEventType::SessionStarted:
		return NSLOCTEXT("FinalBattleEventPresentation", "EventTypeSessionStarted", "SessionStarted");
	case EFinalBattleEventType::RelicTriggered:
		return NSLOCTEXT("FinalBattleEventPresentation", "EventTypeRelicTriggered", "RelicTriggered");
	case EFinalBattleEventType::CommandAccepted:
		return NSLOCTEXT("FinalBattleEventPresentation", "EventTypeCommandAccepted", "CommandAccepted");
	case EFinalBattleEventType::CommandRejected:
		return NSLOCTEXT("FinalBattleEventPresentation", "EventTypeCommandRejected", "CommandRejected");
	case EFinalBattleEventType::StateChanged:
		return NSLOCTEXT("FinalBattleEventPresentation", "EventTypeStateChanged", "StateChanged");
	case EFinalBattleEventType::CardResolved:
		return NSLOCTEXT("FinalBattleEventPresentation", "EventTypeCardResolved", "CardResolved");
	case EFinalBattleEventType::UltimateResolved:
		return NSLOCTEXT("FinalBattleEventPresentation", "EventTypeUltimateResolved", "UltimateResolved");
	case EFinalBattleEventType::TargetChanged:
		return NSLOCTEXT("FinalBattleEventPresentation", "EventTypeTargetChanged", "TargetChanged");
	case EFinalBattleEventType::EnemyActed:
		return NSLOCTEXT("FinalBattleEventPresentation", "EventTypeEnemyActed", "EnemyActed");
	case EFinalBattleEventType::TurnTransition:
		return NSLOCTEXT("FinalBattleEventPresentation", "EventTypeTurnTransition", "TurnTransition");
	case EFinalBattleEventType::PhaseChanged:
		return NSLOCTEXT("FinalBattleEventPresentation", "EventTypePhaseChanged", "PhaseChanged");
	case EFinalBattleEventType::BattleResolved:
		return NSLOCTEXT("FinalBattleEventPresentation", "EventTypeBattleResolved", "BattleResolved");
	case EFinalBattleEventType::Info:
	default:
		return NSLOCTEXT("FinalBattleEventPresentation", "EventTypeInfo", "Info");
	}
}

inline FText ResolveRejectReasonText(const FFinalBattleEvent& Event)
{
	switch (Event.RejectReason)
	{
	case EFinalBattleCommandRejectReason::BattleAlreadyResolved:
		return NSLOCTEXT("FinalBattleEventPresentation", "RejectBattleAlreadyResolved", "战斗已结束");
	case EFinalBattleCommandRejectReason::BattleNotInitialized:
		return NSLOCTEXT("FinalBattleEventPresentation", "RejectBattleNotInitialized", "战斗未初始化");
	case EFinalBattleCommandRejectReason::CardInstanceNotFound:
		return NSLOCTEXT("FinalBattleEventPresentation", "RejectCardInstanceNotFound", "卡牌实例不存在");
	case EFinalBattleCommandRejectReason::CardDefinitionMissing:
		return NSLOCTEXT("FinalBattleEventPresentation", "RejectCardDefinitionMissing", "卡牌定义缺失");
	case EFinalBattleCommandRejectReason::CardNotInHand:
		return NSLOCTEXT("FinalBattleEventPresentation", "RejectCardNotInHand", "卡牌不在手牌中");
	case EFinalBattleCommandRejectReason::NotEnoughAP:
		return NSLOCTEXT("FinalBattleEventPresentation", "RejectNotEnoughAP", "AP不足");
	case EFinalBattleCommandRejectReason::UnsupportedCardEffects:
		return NSLOCTEXT("FinalBattleEventPresentation", "RejectUnsupportedCardEffects", "卡牌效果未支持");
	case EFinalBattleCommandRejectReason::UltimateOwnerNotFound:
		return NSLOCTEXT("FinalBattleEventPresentation", "RejectUltimateOwnerNotFound", "奥义角色不存在");
	case EFinalBattleCommandRejectReason::UltimateBlockedByCollapse:
		return NSLOCTEXT("FinalBattleEventPresentation", "RejectUltimateBlockedByCollapse", "角色崩溃");
	case EFinalBattleCommandRejectReason::UltimateAlreadyUsed:
		return NSLOCTEXT("FinalBattleEventPresentation", "RejectUltimateAlreadyUsed", "奥义已释放");
	case EFinalBattleCommandRejectReason::UltimateDefinitionUnavailable:
		return NSLOCTEXT("FinalBattleEventPresentation", "RejectUltimateDefinitionUnavailable", "奥义定义未就绪");
	case EFinalBattleCommandRejectReason::NotEnoughEP:
		return NSLOCTEXT("FinalBattleEventPresentation", "RejectNotEnoughEP", "EP不足");
	case EFinalBattleCommandRejectReason::InvalidTarget:
		return NSLOCTEXT("FinalBattleEventPresentation", "RejectInvalidTarget", "目标无效");
	case EFinalBattleCommandRejectReason::UnsupportedCommand:
		return NSLOCTEXT("FinalBattleEventPresentation", "RejectUnsupportedCommand", "命令不支持");
	case EFinalBattleCommandRejectReason::None:
	default:
		return FText::GetEmpty();
	}
}

inline FText BuildValueText(const FFinalBattleEvent& Event)
{
	if (Event.PrimaryValue == 0 && Event.SecondaryValue == 0)
	{
		return FText::GetEmpty();
	}

	if (Event.PrimaryValue != 0 && Event.SecondaryValue != 0)
	{
		return FText::Format(
			NSLOCTEXT("FinalBattleEventPresentation", "PrimarySecondaryValues", "Primary {0} | Secondary {1}"),
			FText::AsNumber(Event.PrimaryValue),
			FText::AsNumber(Event.SecondaryValue));
	}

	return FText::Format(
		NSLOCTEXT("FinalBattleEventPresentation", "PrimaryOnlyValue", "Primary {0}"),
		FText::AsNumber(Event.PrimaryValue != 0 ? Event.PrimaryValue : Event.SecondaryValue));
}

inline FText BuildTitleText(
	const FFinalBattleEvent& Event,
	const FFinalBattleSnapshot& Snapshot,
	const UFinalDataRegistry* DataRegistry)
{
	switch (Event.EventType)
	{
	case EFinalBattleEventType::RelicTriggered:
	{
		const FText RelicName = ResolveRelicDisplayName(Snapshot, Event.RelicId);
		return !RelicName.IsEmpty()
			? FText::Format(NSLOCTEXT("FinalBattleEventPresentation", "TitleRelicTriggered", "遗物触发 · {0}"), RelicName)
			: NSLOCTEXT("FinalBattleEventPresentation", "TitleRelicTriggeredFallback", "遗物触发");
	}

	case EFinalBattleEventType::CommandRejected:
	{
		const FText RejectReason = ResolveRejectReasonText(Event);
		return !RejectReason.IsEmpty()
			? FText::Format(NSLOCTEXT("FinalBattleEventPresentation", "TitleCommandRejected", "命令拒绝 · {0}"), RejectReason)
			: NSLOCTEXT("FinalBattleEventPresentation", "TitleCommandRejectedFallback", "命令拒绝");
	}

	case EFinalBattleEventType::CardResolved:
	{
		const FText CardName = ResolveCardDisplayName(Event.CardId, DataRegistry);
		return !CardName.IsEmpty()
			? FText::Format(NSLOCTEXT("FinalBattleEventPresentation", "TitleCardResolved", "卡牌结算 · {0}"), CardName)
			: NSLOCTEXT("FinalBattleEventPresentation", "TitleCardResolvedFallback", "卡牌结算");
	}

	case EFinalBattleEventType::UltimateResolved:
	{
		const FText UltimateName = ResolveUltimateDisplayName(Event.UltimateId, DataRegistry);
		return !UltimateName.IsEmpty()
			? FText::Format(NSLOCTEXT("FinalBattleEventPresentation", "TitleUltimateResolved", "奥义结算 · {0}"), UltimateName)
			: NSLOCTEXT("FinalBattleEventPresentation", "TitleUltimateResolvedFallback", "奥义结算");
	}

	case EFinalBattleEventType::TargetChanged:
		return NSLOCTEXT("FinalBattleEventPresentation", "TitleTargetChanged", "目标切换");
	case EFinalBattleEventType::EnemyActed:
		return NSLOCTEXT("FinalBattleEventPresentation", "TitleEnemyActed", "敌方行动");
	case EFinalBattleEventType::TurnTransition:
		return NSLOCTEXT("FinalBattleEventPresentation", "TitleTurnTransition", "回合切换");
	case EFinalBattleEventType::PhaseChanged:
		return NSLOCTEXT("FinalBattleEventPresentation", "TitlePhaseChanged", "阶段变化");
	case EFinalBattleEventType::BattleResolved:
		return Event.bPlayerVictory
			? NSLOCTEXT("FinalBattleEventPresentation", "TitleBattleResolvedVictory", "战斗结算 · 胜利")
			: NSLOCTEXT("FinalBattleEventPresentation", "TitleBattleResolvedDefeat", "战斗结算 · 失败");
	case EFinalBattleEventType::SessionStarted:
		return NSLOCTEXT("FinalBattleEventPresentation", "TitleSessionStarted", "战斗开始");
	case EFinalBattleEventType::CommandAccepted:
		return NSLOCTEXT("FinalBattleEventPresentation", "TitleCommandAccepted", "命令已接受");
	case EFinalBattleEventType::StateChanged:
	{
		const FText StatusName = ResolveStatusDisplayName(Event.StatusId, DataRegistry);
		return !StatusName.IsEmpty()
			? FText::Format(NSLOCTEXT("FinalBattleEventPresentation", "TitleStateChanged", "状态变化 · {0}"), StatusName)
			: NSLOCTEXT("FinalBattleEventPresentation", "TitleStateChangedFallback", "状态变化");
	}
	case EFinalBattleEventType::Info:
	default:
		return GetEventTypeText(Event.EventType);
	}
}

inline FText BuildSummaryText(
	const FFinalBattleEvent& Event,
	const FFinalBattleSnapshot& Snapshot,
	const UFinalDataRegistry* DataRegistry)
{
	if (!Event.Message.IsEmpty())
	{
		return Event.Message;
	}

	const FText SourceName = ResolveUnitDisplayName(Snapshot, Event.SourceUnitId);
	const FText TargetName = ResolveUnitDisplayName(Snapshot, Event.TargetUnitId);

	switch (Event.EventType)
	{
	case EFinalBattleEventType::CardResolved:
	{
		const FText CardName = ResolveCardDisplayName(Event.CardId, DataRegistry);
		if (!SourceName.IsEmpty() && !TargetName.IsEmpty() && !CardName.IsEmpty())
		{
			return FText::Format(
				NSLOCTEXT("FinalBattleEventPresentation", "SummaryCardResolved", "{0} 对 {1} 结算了 {2}"),
				SourceName,
				TargetName,
				CardName);
		}
		break;
	}

	case EFinalBattleEventType::UltimateResolved:
	{
		const FText UltimateName = ResolveUltimateDisplayName(Event.UltimateId, DataRegistry);
		if (!SourceName.IsEmpty() && !UltimateName.IsEmpty())
		{
			return FText::Format(
				NSLOCTEXT("FinalBattleEventPresentation", "SummaryUltimateResolved", "{0} 释放了 {1}"),
				SourceName,
				UltimateName);
		}
		break;
	}

	case EFinalBattleEventType::TargetChanged:
		if (!TargetName.IsEmpty())
		{
			return FText::Format(NSLOCTEXT("FinalBattleEventPresentation", "SummaryTargetChanged", "当前目标切换为 {0}"), TargetName);
		}
		break;

	case EFinalBattleEventType::EnemyActed:
		if (!SourceName.IsEmpty())
		{
			return FText::Format(NSLOCTEXT("FinalBattleEventPresentation", "SummaryEnemyActed", "{0} 已执行当前行动"), SourceName);
		}
		break;

	case EFinalBattleEventType::TurnTransition:
		return FText::Format(NSLOCTEXT("FinalBattleEventPresentation", "SummaryTurnTransition", "进入第 {0} 回合"), FText::AsNumber(Event.Round));

	case EFinalBattleEventType::PhaseChanged:
		if (!SourceName.IsEmpty())
		{
			return FText::Format(NSLOCTEXT("FinalBattleEventPresentation", "SummaryPhaseChanged", "{0} 进入新的阶段"), SourceName);
		}
		break;

	case EFinalBattleEventType::RelicTriggered:
	{
		const FText RelicName = ResolveRelicDisplayName(Snapshot, Event.RelicId);
		if (!RelicName.IsEmpty())
		{
			return FText::Format(NSLOCTEXT("FinalBattleEventPresentation", "SummaryRelicTriggered", "{0} 在当前战斗窗口触发"), RelicName);
		}
		break;
	}

	case EFinalBattleEventType::SessionStarted:
		return NSLOCTEXT("FinalBattleEventPresentation", "SummarySessionStarted", "战斗初始化已完成。");

	case EFinalBattleEventType::CommandAccepted:
		return NSLOCTEXT("FinalBattleEventPresentation", "SummaryCommandAccepted", "命令已经进入战斗结算链。");

	case EFinalBattleEventType::BattleResolved:
		return Event.bPlayerVictory
			? NSLOCTEXT("FinalBattleEventPresentation", "SummaryBattleResolvedVictory", "本场战斗已结束，结果为胜利。")
			: NSLOCTEXT("FinalBattleEventPresentation", "SummaryBattleResolvedDefeat", "本场战斗已结束，结果为失败。");

	default:
		break;
	}

	return FText::GetEmpty();
}

inline FText BuildDetailText(
	const FFinalBattleEvent& Event,
	const FFinalBattleSnapshot& Snapshot,
	const UFinalDataRegistry* DataRegistry)
{
	TArray<FText> Segments;
	Segments.Add(FText::Format(NSLOCTEXT("FinalBattleEventPresentation", "DetailSequence", "Seq {0}"), FText::AsNumber(Event.EventSequence)));

	if (Event.Round > 0)
	{
		Segments.Add(FText::Format(NSLOCTEXT("FinalBattleEventPresentation", "DetailRound", "Round {0}"), FText::AsNumber(Event.Round)));
	}

	const FText SourceName = ResolveUnitDisplayName(Snapshot, Event.SourceUnitId);
	if (!SourceName.IsEmpty())
	{
		Segments.Add(FText::Format(NSLOCTEXT("FinalBattleEventPresentation", "DetailSource", "源 {0}"), SourceName));
	}

	const FText TargetName = ResolveUnitDisplayName(Snapshot, Event.TargetUnitId);
	if (!TargetName.IsEmpty())
	{
		Segments.Add(FText::Format(NSLOCTEXT("FinalBattleEventPresentation", "DetailTarget", "目标 {0}"), TargetName));
	}

	if (!Event.RelatedTag.IsNone())
	{
		Segments.Add(FText::Format(NSLOCTEXT("FinalBattleEventPresentation", "DetailTag", "Tag {0}"), FText::FromName(Event.RelatedTag)));
	}

	if (!Event.ReasonTag.IsNone())
	{
		Segments.Add(FText::Format(NSLOCTEXT("FinalBattleEventPresentation", "DetailReasonTag", "Reason {0}"), FText::FromName(Event.ReasonTag)));
	}

	const FText RejectReason = ResolveRejectReasonText(Event);
	if (!RejectReason.IsEmpty())
	{
		Segments.Add(FText::Format(NSLOCTEXT("FinalBattleEventPresentation", "DetailRejectReason", "Reject {0}"), RejectReason));
	}

	const FText ValueText = BuildValueText(Event);
	if (!ValueText.IsEmpty())
	{
		Segments.Add(ValueText);
	}

	if (Event.EventType == EFinalBattleEventType::CardResolved)
	{
		const FText CardName = ResolveCardDisplayName(Event.CardId, DataRegistry);
		if (!CardName.IsEmpty())
		{
			Segments.Add(FText::Format(NSLOCTEXT("FinalBattleEventPresentation", "DetailCard", "Card {0}"), CardName));
		}
	}
	else if (Event.EventType == EFinalBattleEventType::UltimateResolved)
	{
		const FText UltimateName = ResolveUltimateDisplayName(Event.UltimateId, DataRegistry);
		if (!UltimateName.IsEmpty())
		{
			Segments.Add(FText::Format(NSLOCTEXT("FinalBattleEventPresentation", "DetailUltimate", "Ultimate {0}"), UltimateName));
		}
	}
	else if (Event.EventType == EFinalBattleEventType::StateChanged)
	{
		const FText StatusName = ResolveStatusDisplayName(Event.StatusId, DataRegistry);
		if (!StatusName.IsEmpty())
		{
			Segments.Add(FText::Format(NSLOCTEXT("FinalBattleEventPresentation", "DetailStatus", "Status {0}"), StatusName));
		}
	}
	else if (Event.EventType == EFinalBattleEventType::RelicTriggered)
	{
		const FText RelicName = ResolveRelicDisplayName(Snapshot, Event.RelicId);
		if (!RelicName.IsEmpty())
		{
			Segments.Add(FText::Format(NSLOCTEXT("FinalBattleEventPresentation", "DetailRelic", "Relic {0}"), RelicName));
		}
	}

	return FText::FromString(JoinLines(Segments, TEXT(" | ")));
}

inline FText BuildShortWorldText(const FEventPresentation& Presentation)
{
	if (!Presentation.TitleText.IsEmpty() && !Presentation.SummaryText.IsEmpty())
	{
		return FText::Format(
			NSLOCTEXT("FinalBattleEventPresentation", "ShortWorldTitleSummary", "{0}\n{1}"),
			Presentation.TitleText,
			Presentation.SummaryText);
	}

	if (!Presentation.TitleText.IsEmpty())
	{
		return Presentation.TitleText;
	}

	return Presentation.SummaryText;
}

inline FText BuildCombinedBodyText(const FEventPresentation& Presentation)
{
	TArray<FText> Lines;
	if (!Presentation.SummaryText.IsEmpty())
	{
		Lines.Add(Presentation.SummaryText);
	}
	if (!Presentation.DetailText.IsEmpty())
	{
		Lines.Add(Presentation.DetailText);
	}
	return FText::FromString(JoinLines(Lines, TEXT("\n")));
}

inline FEventPresentation BuildPresentation(
	const FFinalBattleEvent& Event,
	const FFinalBattleSnapshot& Snapshot,
	const UFinalDataRegistry* DataRegistry)
{
	FEventPresentation Presentation;
	Presentation.EventTypeText = GetEventTypeText(Event.EventType);
	Presentation.TitleText = BuildTitleText(Event, Snapshot, DataRegistry);
	Presentation.SummaryText = BuildSummaryText(Event, Snapshot, DataRegistry);
	Presentation.DetailText = BuildDetailText(Event, Snapshot, DataRegistry);
	Presentation.ShortWorldText = BuildShortWorldText(Presentation);

	TArray<FText> LedgerLines;
	LedgerLines.Add(FText::Format(
		NSLOCTEXT("FinalBattleEventPresentation", "LedgerHeader", "[{0}] {1}"),
		FText::AsNumber(Event.EventSequence),
		!Presentation.TitleText.IsEmpty() ? Presentation.TitleText : Presentation.EventTypeText));

	if (!Presentation.SummaryText.IsEmpty())
	{
		LedgerLines.Add(Presentation.SummaryText);
	}

	if (!Presentation.DetailText.IsEmpty())
	{
		LedgerLines.Add(Presentation.DetailText);
	}

	Presentation.LedgerText = FText::FromString(JoinLines(LedgerLines, TEXT("\n")));
	return Presentation;
}
}
