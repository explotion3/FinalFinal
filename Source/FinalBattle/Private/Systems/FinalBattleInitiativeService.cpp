#include "Systems/FinalBattleInitiativeService.h"

#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "GameplayTagContainer.h"
#include "Runtime/FinalBattleEnemyState.h"
#include "Runtime/FinalBattleState.h"
#include "Systems/FinalBattleEnemyActionService.h"
#include "Systems/FinalBattleEffectExecutionService.h"
#include "Systems/FinalBattleTriggerService.h"
#include "Systems/FinalBattleUnitService.h"
#include "Systems/FinalEnemyIntentService.h"

namespace
{
const FName TeamPlayerUnitId(TEXT("team_player"));
const FName InitiativeCardEventReasonTag(TEXT("battle.initiative.card_resolved"));
const FName InitiativeUltimateEventReasonTag(TEXT("battle.initiative.ultimate_resolved"));
const FName InitiativeReachedZeroReasonTag(TEXT("battle.initiative.reached_zero"));
const FName BreakSkipActionReasonTag(TEXT("battle.enemy_action_override.break"));
const FName EndTurnEnemyActionReasonTag(TEXT("battle.enemy_action.end_turn"));
const FName FastKeywordName(TEXT("Final.Keyword.Fast"));

const FFinalEnemyIntentService& GetEnemyIntentService()
{
	static const FFinalEnemyIntentService IntentService;
	return IntentService;
}

bool IsAlive(const FFinalBattleEnemyState& EnemyState)
{
	return EnemyState.CurrentHP > 0;
}

bool HasSkipActionOverride(const FFinalBattleEnemyState& EnemyState)
{
	return EnemyState.ActionOverrideType == EFinalEnemyActionOverrideType::SkipNextAction;
}

bool CanRespondToInitiative(const FFinalBattleEnemyState& EnemyState)
{
	return IsAlive(EnemyState)
		&& EnemyState.InitiativeState == EFinalEnemyInitiativeState::Counting
		&& !HasSkipActionOverride(EnemyState);
}

bool HasFastKeyword(const FGameplayTagContainer& RuntimeKeywords)
{
	const FGameplayTag FastKeyword = FGameplayTag::RequestGameplayTag(FastKeywordName, false);
	return FastKeyword.IsValid() && RuntimeKeywords.HasTagExact(FastKeyword);
}

FText ResolveEnemyDisplayName(const FFinalBattleEnemyState& EnemyState)
{
	return EnemyState.DisplayName.IsEmpty()
		? FText::FromName(EnemyState.RuntimeUnitId)
		: EnemyState.DisplayName;
}

void MarkActionOpportunitySpent(FFinalBattleEnemyState& EnemyState)
{
	EnemyState.bActedThisRound = true;
	EnemyState.ActionsTakenThisRound = FMath::Max(EnemyState.ActionsTakenThisRound + 1, 0);
	if (EnemyState.ActionsTakenThisRound >= FMath::Max(EnemyState.MaxActionsPerRound, 1))
	{
		EnemyState.InitiativeState = EFinalEnemyInitiativeState::ActionSpent;
		EnemyState.CurrentInitiative = 0;
	}
	else
	{
		EnemyState.InitiativeState = EFinalEnemyInitiativeState::Counting;
		EnemyState.CurrentInitiative = FMath::Max(EnemyState.InitialInitiative, 0);
	}
}

FFinalBattleEvent BuildInitiativeChangedEvent(
	const FFinalBattleEnemyState& EnemyState,
	const int32 PreviousInitiative,
	const FName ReasonTag)
{
	FFinalBattleEvent Event;
	Event.EventType = EFinalBattleEventType::EnemyInitiativeChanged;
	Event.SourceUnitId = EnemyState.RuntimeUnitId;
	Event.ReasonTag = ReasonTag;
	Event.PrimaryValue = PreviousInitiative;
	Event.SecondaryValue = EnemyState.CurrentInitiative;
	Event.Message = FText::Format(
		NSLOCTEXT("FinalBattleInitiativeService", "EnemyInitiativeChanged", "{0} 先机 {1} -> {2}。"),
		ResolveEnemyDisplayName(EnemyState),
		FText::AsNumber(PreviousInitiative),
		FText::AsNumber(EnemyState.CurrentInitiative));
	return Event;
}

FFinalBattleEvent BuildQueuedEvent(const FFinalBattleEnemyState& EnemyState)
{
	FFinalBattleEvent Event;
	Event.EventType = EFinalBattleEventType::EnemyQueuedByInitiative;
	Event.SourceUnitId = EnemyState.RuntimeUnitId;
	Event.ReasonTag = InitiativeReachedZeroReasonTag;
	Event.PrimaryValue = EnemyState.CurrentInitiative;
	Event.Message = FText::Format(
		NSLOCTEXT("FinalBattleInitiativeService", "EnemyQueuedByInitiative", "{0} 先机归零，进入行动队列。"),
		ResolveEnemyDisplayName(EnemyState));
	return Event;
}

FFinalBattleEvent BuildEnemyActedEvent(
	const FFinalBattleEnemyState& EnemyState,
	const FFinalBattleEnemyActionResult& ActionResult)
{
	FFinalBattleEvent Event;
	Event.EventType = EFinalBattleEventType::EnemyActed;
	Event.SourceUnitId = EnemyState.RuntimeUnitId;
	Event.TargetUnitId = TeamPlayerUnitId;
	Event.RelatedTag = EnemyState.CurrentIntentId;
	Event.PrimaryValue = ActionResult.DamageToTeam;
	Event.SecondaryValue = ActionResult.EnemyShieldGained;
	Event.Message = FText::Format(
		NSLOCTEXT("FinalBattleInitiativeService", "EnemyActed", "{0} resolved intent {1}."),
		ResolveEnemyDisplayName(EnemyState),
		EnemyState.CurrentIntentText.IsEmpty() ? FText::FromString(TEXT("Attack")) : EnemyState.CurrentIntentText);
	return Event;
}

FFinalBattleEvent BuildEnemySkippedEvent(const FFinalBattleEnemyState& EnemyState)
{
	FFinalBattleEvent Event;
	Event.EventType = EFinalBattleEventType::EnemyActionSkipped;
	Event.SourceUnitId = EnemyState.RuntimeUnitId;
	Event.ReasonTag = EnemyState.ActionOverrideReasonTag.IsNone() ? BreakSkipActionReasonTag : EnemyState.ActionOverrideReasonTag;
	Event.Message = FText::Format(
		NSLOCTEXT("FinalBattleInitiativeService", "EnemyActionSkipped", "{0} 无法行动。"),
		ResolveEnemyDisplayName(EnemyState));
	return Event;
}

void ClearActionOverride(FFinalBattleEnemyState& EnemyState)
{
	EnemyState.ActionOverrideType = EFinalEnemyActionOverrideType::None;
	EnemyState.ActionOverrideReasonTag = NAME_None;
	EnemyState.ActionOverrideDisplayName = FText::GetEmpty();
	EnemyState.ActionOverridePreviewText = FText::GetEmpty();
}

void RestoreBreakAfterSkippedAction(FFinalBattleEnemyState& EnemyState)
{
	EnemyState.CurrentBreakValue = FMath::Max(EnemyState.MaxBreakValue, 0);
}

FFinalBattleEnemyActionSequenceResult ResolveSingleEnemyAction(
	FFinalBattleState& BattleState,
	FFinalBattleEnemyState& EnemyState,
	const FName ReasonTag,
	const FFinalBattleEnemyActionService& EnemyActionService,
	const FFinalBattleUnitService& UnitService,
	const FFinalBattleTriggerService& TriggerService,
	const FFinalBattleEffectExecutionService& EffectExecutionService)
{
	FFinalBattleEnemyActionSequenceResult Result;
	if (!IsAlive(EnemyState) || EnemyState.InitiativeState == EFinalEnemyInitiativeState::ActionSpent)
	{
		return Result;
	}

	if (HasSkipActionOverride(EnemyState))
	{
		Result.GeneratedEvents.Add(BuildEnemySkippedEvent(EnemyState));
		RestoreBreakAfterSkippedAction(EnemyState);
		ClearActionOverride(EnemyState);
		MarkActionOpportunitySpent(EnemyState);
		return Result;
	}

	const FFinalBattleEnemyActionResult ActionResult = EnemyActionService.ResolveEnemyAction(
		BattleState,
		EnemyState,
		UnitService,
		TriggerService,
		EffectExecutionService);

	Result.TotalDamageToTeam += ActionResult.DamageToTeam;
	Result.TotalEnemyShieldGained += ActionResult.EnemyShieldGained;
	Result.ResolvedEffectCount += ActionResult.ResolvedEffectCount;
	Result.GeneratedEvents.Add(BuildEnemyActedEvent(EnemyState, ActionResult));

	GetEnemyIntentService().CommitCurrentIntentExecution(EnemyState, BattleState.CurrentRound);
	GetEnemyIntentService().RefreshIntent(EnemyState, BattleState.CurrentRound + 1);
	MarkActionOpportunitySpent(EnemyState);
	Result.bBattleLost = BattleState.TeamCurrentHP <= 0;
	return Result;
}

void AppendSequenceResult(FFinalBattleEnemyActionSequenceResult& InOutResult, FFinalBattleEnemyActionSequenceResult&& AddedResult)
{
	InOutResult.TotalDamageToTeam += AddedResult.TotalDamageToTeam;
	InOutResult.TotalEnemyShieldGained += AddedResult.TotalEnemyShieldGained;
	InOutResult.ResolvedEffectCount += AddedResult.ResolvedEffectCount;
	InOutResult.bBattleLost = InOutResult.bBattleLost || AddedResult.bBattleLost;
	InOutResult.GeneratedEvents.Append(MoveTemp(AddedResult.GeneratedEvents));
}

bool IsAlreadyQueued(const FFinalBattleState& BattleState, const FName EnemyUnitId)
{
	return BattleState.PendingEnemyActionQueue.ContainsByPredicate(
		[EnemyUnitId](const FFinalBattlePendingEnemyAction& Entry)
		{
			return Entry.EnemyUnitId == EnemyUnitId;
		});
}

void SortPendingQueue(FFinalBattleState& BattleState)
{
	BattleState.PendingEnemyActionQueue.Sort(
		[&BattleState](const FFinalBattlePendingEnemyAction& Left, const FFinalBattlePendingEnemyAction& Right)
		{
			const FFinalBattleEnemyState* LeftEnemy = BattleState.Enemies.FindByPredicate(
				[&Left](const FFinalBattleEnemyState& EnemyState)
				{
					return EnemyState.RuntimeUnitId == Left.EnemyUnitId;
				});
			const FFinalBattleEnemyState* RightEnemy = BattleState.Enemies.FindByPredicate(
				[&Right](const FFinalBattleEnemyState& EnemyState)
				{
					return EnemyState.RuntimeUnitId == Right.EnemyUnitId;
				});

			const int32 LeftPosition = LeftEnemy != nullptr ? LeftEnemy->PositionIndex : MAX_int32;
			const int32 RightPosition = RightEnemy != nullptr ? RightEnemy->PositionIndex : MAX_int32;
			if (LeftPosition != RightPosition)
			{
				return LeftPosition < RightPosition;
			}

			return Left.EnemyUnitId.LexicalLess(Right.EnemyUnitId);
		});
}

FFinalBattleEnemyActionSequenceResult ResolvePendingQueue(
	FFinalBattleState& BattleState,
	const FFinalBattleEnemyActionService& EnemyActionService,
	const FFinalBattleUnitService& UnitService,
	const FFinalBattleTriggerService& TriggerService,
	const FFinalBattleEffectExecutionService& EffectExecutionService)
{
	FFinalBattleEnemyActionSequenceResult Result;
	SortPendingQueue(BattleState);

	while (!BattleState.PendingEnemyActionQueue.IsEmpty())
	{
		const FFinalBattlePendingEnemyAction QueueEntry = BattleState.PendingEnemyActionQueue[0];
		BattleState.PendingEnemyActionQueue.RemoveAt(0);

		FFinalBattleEnemyState* EnemyState = BattleState.Enemies.FindByPredicate(
			[&QueueEntry](const FFinalBattleEnemyState& Candidate)
			{
				return Candidate.RuntimeUnitId == QueueEntry.EnemyUnitId;
			});

		if (EnemyState == nullptr || !IsAlive(*EnemyState))
		{
			continue;
		}

		AppendSequenceResult(
			Result,
			ResolveSingleEnemyAction(
				BattleState,
				*EnemyState,
				QueueEntry.ReasonTag,
				EnemyActionService,
				UnitService,
				TriggerService,
				EffectExecutionService));

		if (Result.bBattleLost)
		{
			BattleState.PendingEnemyActionQueue.Reset();
			break;
		}
	}

	return Result;
}

void QueueEnemyByInitiative(FFinalBattleState& BattleState, const FFinalBattleEnemyState& EnemyState, TArray<FFinalBattleEvent>& OutEvents)
{
	if (IsAlreadyQueued(BattleState, EnemyState.RuntimeUnitId))
	{
		return;
	}

	FFinalBattlePendingEnemyAction& QueueEntry = BattleState.PendingEnemyActionQueue.AddDefaulted_GetRef();
	QueueEntry.EnemyUnitId = EnemyState.RuntimeUnitId;
	QueueEntry.ReasonTag = InitiativeReachedZeroReasonTag;
	OutEvents.Add(BuildQueuedEvent(EnemyState));
}

int32 ResolveCardInitiativeEventCount(
	const UFinalBattleRuleConfig* RuleConfig,
	const FFinalBattleResolvedCardTriggerContext& CardContext,
	const bool bCollapsedCard)
{
	if (HasFastKeyword(CardContext.RuntimeKeywords))
	{
		return 0;
	}

	if (RuleConfig == nullptr)
	{
		return bCollapsedCard ? 1 : 1;
	}

	return FMath::Max(
		bCollapsedCard ? RuleConfig->CollapsedCardInitiativeEventCount : RuleConfig->NormalCardInitiativeEventCount,
		0);
}

void ResolveOneInitiativeEvent(
	FFinalBattleState& BattleState,
	const FName ReasonTag,
	TArray<FFinalBattleEvent>& OutEvents)
{
	for (FFinalBattleEnemyState& EnemyState : BattleState.Enemies)
	{
		if (!CanRespondToInitiative(EnemyState))
		{
			continue;
		}

		const int32 PreviousInitiative = EnemyState.CurrentInitiative;
		EnemyState.CurrentInitiative = FMath::Max(0, EnemyState.CurrentInitiative - FMath::Max(EnemyState.InitiativeResponse, 0));
		OutEvents.Add(BuildInitiativeChangedEvent(EnemyState, PreviousInitiative, ReasonTag));

		if (EnemyState.CurrentInitiative <= 0)
		{
			EnemyState.InitiativeState = EFinalEnemyInitiativeState::ReadyToAct;
			QueueEnemyByInitiative(BattleState, EnemyState, OutEvents);
		}
	}
}
}

void FFinalBattleInitiativeService::ApplyBreakSkipActionOverride(FFinalBattleEnemyState& EnemyState, const int32 PreviousBreakValue) const
{
	if (PreviousBreakValue <= 0 || EnemyState.CurrentBreakValue > 0 || EnemyState.CurrentHP <= 0)
	{
		return;
	}

	EnemyState.ActionOverrideType = EFinalEnemyActionOverrideType::SkipNextAction;
	EnemyState.ActionOverrideReasonTag = BreakSkipActionReasonTag;
	EnemyState.ActionOverrideDisplayName = NSLOCTEXT("FinalBattleInitiativeService", "BreakSkipActionDisplayName", "无法行动");
	EnemyState.ActionOverridePreviewText = NSLOCTEXT("FinalBattleInitiativeService", "BreakSkipActionPreview", "该敌人处于 Break，本次行动被跳过。");
}

FFinalBattleEnemyActionSequenceResult FFinalBattleInitiativeService::ResolveCardInitiativeEvents(
	FFinalBattleState& BattleState,
	const UFinalBattleRuleConfig* RuleConfig,
	const FFinalBattleResolvedCardTriggerContext& CardContext,
	const bool bCollapsedCard,
	const FFinalBattleEnemyActionService& EnemyActionService,
	const FFinalBattleUnitService& UnitService,
	const FFinalBattleTriggerService& TriggerService,
	const FFinalBattleEffectExecutionService& EffectExecutionService) const
{
	FFinalBattleEnemyActionSequenceResult Result;
	const int32 EventCount = ResolveCardInitiativeEventCount(RuleConfig, CardContext, bCollapsedCard);
	for (int32 EventIndex = 0; EventIndex < EventCount; ++EventIndex)
	{
		ResolveOneInitiativeEvent(BattleState, InitiativeCardEventReasonTag, Result.GeneratedEvents);
	}

	AppendSequenceResult(Result, ResolvePendingQueue(BattleState, EnemyActionService, UnitService, TriggerService, EffectExecutionService));
	return Result;
}

FFinalBattleEnemyActionSequenceResult FFinalBattleInitiativeService::ResolveUltimateInitiativeEvents(
	FFinalBattleState& BattleState,
	const UFinalBattleRuleConfig* RuleConfig,
	const FFinalBattleEnemyActionService& EnemyActionService,
	const FFinalBattleUnitService& UnitService,
	const FFinalBattleTriggerService& TriggerService,
	const FFinalBattleEffectExecutionService& EffectExecutionService) const
{
	FFinalBattleEnemyActionSequenceResult Result;
	if (RuleConfig == nullptr || !RuleConfig->bUltimateTriggersInitiativeEvent)
	{
		return Result;
	}

	ResolveOneInitiativeEvent(BattleState, InitiativeUltimateEventReasonTag, Result.GeneratedEvents);
	AppendSequenceResult(Result, ResolvePendingQueue(BattleState, EnemyActionService, UnitService, TriggerService, EffectExecutionService));
	return Result;
}

FFinalBattleEnemyActionSequenceResult FFinalBattleInitiativeService::ResolveEndTurnEnemyActions(
	FFinalBattleState& BattleState,
	const FFinalBattleEnemyActionService& EnemyActionService,
	const FFinalBattleUnitService& UnitService,
	const FFinalBattleTriggerService& TriggerService,
	const FFinalBattleEffectExecutionService& EffectExecutionService) const
{
	FFinalBattleEnemyActionSequenceResult Result;
	BattleState.PendingEnemyActionQueue.Reset();

	TArray<FFinalBattleEnemyState*> OrderedEnemies;
	for (FFinalBattleEnemyState& EnemyState : BattleState.Enemies)
	{
		if (IsAlive(EnemyState) && EnemyState.InitiativeState != EFinalEnemyInitiativeState::ActionSpent)
		{
			OrderedEnemies.Add(&EnemyState);
		}
	}

	OrderedEnemies.Sort(
		[](const FFinalBattleEnemyState& Left, const FFinalBattleEnemyState& Right)
		{
			if (Left.PositionIndex != Right.PositionIndex)
			{
				return Left.PositionIndex < Right.PositionIndex;
			}

			return Left.RuntimeUnitId.LexicalLess(Right.RuntimeUnitId);
		});

	for (FFinalBattleEnemyState* EnemyState : OrderedEnemies)
	{
		if (EnemyState == nullptr || !IsAlive(*EnemyState) || EnemyState->InitiativeState == EFinalEnemyInitiativeState::ActionSpent)
		{
			continue;
		}

		AppendSequenceResult(
			Result,
			ResolveSingleEnemyAction(
				BattleState,
				*EnemyState,
				EndTurnEnemyActionReasonTag,
				EnemyActionService,
				UnitService,
				TriggerService,
				EffectExecutionService));

		if (Result.bBattleLost)
		{
			break;
		}
	}

	return Result;
}

void FFinalBattleInitiativeService::ResetEnemyInitiativeForNewRound(FFinalBattleState& BattleState) const
{
	BattleState.PendingEnemyActionQueue.Reset();
	for (FFinalBattleEnemyState& EnemyState : BattleState.Enemies)
	{
		if (!IsAlive(EnemyState))
		{
			continue;
		}

		EnemyState.CurrentInitiative = FMath::Max(EnemyState.InitialInitiative, 0);
		EnemyState.InitiativeState = EFinalEnemyInitiativeState::Counting;
		EnemyState.ActionsTakenThisRound = 0;
		EnemyState.bActedThisRound = false;
	}
}
