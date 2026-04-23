#include "Systems/FinalBattleConditionService.h"

#include "Battle/Conditions/FinalBattleConditionDefinition.h"
#include "Battle/Conditions/FinalBattleConditionHandCard.h"
#include "Battle/Conditions/FinalBattleConditionMovedCards.h"
#include "Battle/Conditions/FinalBattleConditionStatusChanged.h"
#include "Battle/Conditions/FinalBattleConditionTargetState.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "Runtime/FinalBattleCardInstance.h"
#include "Runtime/FinalBattleEnemyState.h"
#include "Runtime/FinalBattleState.h"
#include "Systems/FinalBattleCardService.h"
#include "Systems/FinalBattleRelicService.h"

namespace
{
enum class EFinalBattleConditionEvaluationPass : uint8
{
	SourceAndChain,
	TargetOnly,
	All
};

bool HasTargetStateRequirement(const FFinalBattleTargetStateRequirement& Requirement)
{
	return Requirement.bRequireEnemyTarget
		|| Requirement.bRequireTargetBroken
		|| Requirement.bRequireTargetAlive;
}

bool IsEnemyBroken(const FFinalBattleEnemyState& EnemyState)
{
	return EnemyState.CurrentBreakValue <= 0;
}

bool SatisfiesTargetStateRequirement(
	const FFinalBattleTargetStateRequirement& Requirement,
	const FFinalBattleEnemyState* TargetEnemyState)
{
	if (!HasTargetStateRequirement(Requirement))
	{
		return true;
	}

	if (Requirement.bRequireEnemyTarget && TargetEnemyState == nullptr)
	{
		return false;
	}

	if (TargetEnemyState == nullptr)
	{
		return false;
	}

	if (Requirement.bRequireTargetAlive && TargetEnemyState->CurrentHP <= 0)
	{
		return false;
	}

	if (Requirement.bRequireTargetBroken && !IsEnemyBroken(*TargetEnemyState))
	{
		return false;
	}

	return true;
}

int32 ResolveStatusChangedStacks(
	const FFinalBattleEffectChainRecordContext& ChainRecords,
	const FName OwnerUnitId,
	const FFinalStatusId& StatusId,
	const EFinalBattleStatusChangeKind ChangeKind)
{
	if (const FFinalBattleStatusChangeRecord* ExistingRecord = ChainRecords.StatusChangeRecords.FindByPredicate(
		[&OwnerUnitId, &StatusId, ChangeKind](const FFinalBattleStatusChangeRecord& Candidate)
		{
			return Candidate.OwnerUnitId == OwnerUnitId
				&& Candidate.StatusId == StatusId
				&& Candidate.ChangeKind == ChangeKind;
		}))
	{
		return ExistingRecord->ChangedStacks;
	}

	return 0;
}

int32 ResolveMovedCardCount(
	const FFinalBattleEffectChainRecordContext& ChainRecords,
	const FName RuntimeOwnerUnitId,
	const FFinalBattleMovedCardRequirement& Requirement)
{
	int32 TotalMovedCount = 0;

	for (const FFinalBattleMovedCardRecord& Record : ChainRecords.MovedCardRecords)
	{
		if (Record.RuntimeOwnerUnitId != RuntimeOwnerUnitId)
		{
			continue;
		}

		if (Requirement.RequiredCardId.IsValid() && Record.CardId != Requirement.RequiredCardId)
		{
			continue;
		}

		if (Requirement.RequiredKeyword.IsValid() && !Record.RuntimeKeywords.HasTagExact(Requirement.RequiredKeyword))
		{
			continue;
		}

		if (Requirement.bGeneratedOnly && !Record.bGeneratedCard)
		{
			continue;
		}

		if (Requirement.bRequireSourceZone && Record.SourceZone != Requirement.SourceZone)
		{
			continue;
		}

		if (Requirement.bRequireDestinationZone && Record.DestinationZone != Requirement.DestinationZone)
		{
			continue;
		}

		TotalMovedCount += Record.MovedCount;
	}

	return TotalMovedCount;
}

bool SatisfiesStatusChangeRequirement(
	const FFinalBattleStatusChangeRequirement& Requirement,
	const FFinalBattleConditionEvaluationContext& Context)
{
	return Context.ChainRecords != nullptr
		&& Requirement.RequiredStatusId.IsValid()
		&& !Context.SourceOwnerUnitId.IsNone()
		&& ResolveStatusChangedStacks(
			*Context.ChainRecords,
			Context.SourceOwnerUnitId,
			Requirement.RequiredStatusId,
			Requirement.ChangeKind) >= FMath::Max(Requirement.MinimumStacks, 1);
}

bool SatisfiesMovedCardRequirement(
	const FFinalBattleMovedCardRequirement& Requirement,
	const FFinalBattleConditionEvaluationContext& Context)
{
	return Context.ChainRecords != nullptr
		&& !Context.SourceOwnerUnitId.IsNone()
		&& ResolveMovedCardCount(
			*Context.ChainRecords,
			Context.SourceOwnerUnitId,
			Requirement) >= FMath::Max(Requirement.MinimumCount, 1);
}

bool SatisfiesHandCardRequirement(
	const FFinalBattleHandCardRequirement& Requirement,
	const FFinalBattleConditionEvaluationContext& Context)
{
	if (!Requirement.bRequireInHand)
	{
		return true;
	}

	return Context.BattleState != nullptr
		&& Context.CardService != nullptr
		&& !Context.SourceOwnerUnitId.IsNone()
		&& Context.CardService->SatisfiesHandCardRequirement(*Context.BattleState, Context.SourceOwnerUnitId, Requirement);
}

bool ShouldEvaluateConditionInPass(
	const EFinalBattleConditionContext ConditionContext,
	const EFinalBattleConditionEvaluationPass Pass)
{
	switch (Pass)
	{
	case EFinalBattleConditionEvaluationPass::SourceAndChain:
		return ConditionContext != EFinalBattleConditionContext::TargetRequired;

	case EFinalBattleConditionEvaluationPass::TargetOnly:
		return ConditionContext == EFinalBattleConditionContext::TargetRequired;

	case EFinalBattleConditionEvaluationPass::All:
	default:
		return true;
	}
}

}

bool FFinalBattleConditionService::SatisfiesSourceAndChainConditions(
	const UFinalBattleEffectDefinition* EffectDefinition,
	const FFinalBattleConditionEvaluationContext& Context) const
{
	if (EffectDefinition == nullptr)
	{
		return false;
	}

	for (const UFinalBattleConditionDefinition* Condition : EffectDefinition->Conditions)
	{
		if (Condition == nullptr)
		{
			return false;
		}

		if (!ShouldEvaluateConditionInPass(Condition->GetConditionContext(), EFinalBattleConditionEvaluationPass::SourceAndChain))
		{
			continue;
		}

		if (!EvaluateCondition(Condition, Context))
		{
			return false;
		}
	}

	return true;
}

bool FFinalBattleConditionService::SatisfiesTargetConditions(
	const UFinalBattleEffectDefinition* EffectDefinition,
	const FFinalBattleConditionEvaluationContext& Context) const
{
	if (EffectDefinition == nullptr)
	{
		return false;
	}

	for (const UFinalBattleConditionDefinition* Condition : EffectDefinition->Conditions)
	{
		if (Condition == nullptr)
		{
			return false;
		}

		if (!ShouldEvaluateConditionInPass(Condition->GetConditionContext(), EFinalBattleConditionEvaluationPass::TargetOnly))
		{
			continue;
		}

		if (!EvaluateCondition(Condition, Context))
		{
			return false;
		}
	}

	return true;
}

bool FFinalBattleConditionService::SatisfiesAllEffectConditions(
	const UFinalBattleEffectDefinition* EffectDefinition,
	const FFinalBattleConditionEvaluationContext& Context) const
{
	if (EffectDefinition == nullptr)
	{
		return false;
	}

	for (const UFinalBattleConditionDefinition* Condition : EffectDefinition->Conditions)
	{
		if (Condition == nullptr)
		{
			return false;
		}

		if (!ShouldEvaluateConditionInPass(Condition->GetConditionContext(), EFinalBattleConditionEvaluationPass::All))
		{
			continue;
		}

		if (!EvaluateCondition(Condition, Context))
		{
			return false;
		}
	}

	return true;
}

bool FFinalBattleConditionService::SatisfiesResolvedCardCondition(
	const FFinalRelicRuntimeCardConditionDefinition& CardCondition,
	const FFinalBattleResolvedCardTriggerContext& CardContext) const
{
	if (CardCondition.bRequireCardCostAP && CardContext.RuntimeCostAP != CardCondition.RequiredCardCostAP)
	{
		return false;
	}

	if (CardCondition.bRequireCardType && CardContext.CardType != CardCondition.RequiredCardType)
	{
		return false;
	}

	if (CardCondition.RequiredKeyword.IsValid() && !CardContext.RuntimeKeywords.HasTagExact(CardCondition.RequiredKeyword))
	{
		return false;
	}

	return true;
}

void FFinalBattleConditionService::RecordStatusChange(
	FFinalBattleEffectChainRecordContext& ChainRecords,
	const FName OwnerUnitId,
	const FFinalStatusId& StatusId,
	const EFinalBattleStatusChangeKind ChangeKind,
	const int32 ChangedStacks) const
{
	if (OwnerUnitId.IsNone() || !StatusId.IsValid() || ChangedStacks <= 0)
	{
		return;
	}

	if (FFinalBattleStatusChangeRecord* ExistingRecord = ChainRecords.StatusChangeRecords.FindByPredicate(
		[&OwnerUnitId, &StatusId, ChangeKind](const FFinalBattleStatusChangeRecord& Candidate)
		{
			return Candidate.OwnerUnitId == OwnerUnitId
				&& Candidate.StatusId == StatusId
				&& Candidate.ChangeKind == ChangeKind;
		}))
	{
		ExistingRecord->ChangedStacks += ChangedStacks;
		return;
	}

	FFinalBattleStatusChangeRecord& NewRecord = ChainRecords.StatusChangeRecords.AddDefaulted_GetRef();
	NewRecord.OwnerUnitId = OwnerUnitId;
	NewRecord.StatusId = StatusId;
	NewRecord.ChangeKind = ChangeKind;
	NewRecord.ChangedStacks = ChangedStacks;
}

void FFinalBattleConditionService::RecordMovedCard(
	FFinalBattleEffectChainRecordContext& ChainRecords,
	const FName RuntimeOwnerUnitId,
	const FFinalBattleCardInstance& CardInstance,
	const EFinalBattleCardZoneRule SourceZone,
	const EFinalBattleCardZoneRule DestinationZone,
	const int32 MovedCount) const
{
	if (RuntimeOwnerUnitId.IsNone() || !CardInstance.CardId.IsValid() || MovedCount <= 0)
	{
		return;
	}

	if (FFinalBattleMovedCardRecord* ExistingRecord = ChainRecords.MovedCardRecords.FindByPredicate(
		[&RuntimeOwnerUnitId, &CardInstance, SourceZone, DestinationZone](const FFinalBattleMovedCardRecord& Candidate)
		{
			return Candidate.RuntimeOwnerUnitId == RuntimeOwnerUnitId
				&& Candidate.CardId == CardInstance.CardId
				&& Candidate.bGeneratedCard == CardInstance.bGeneratedCard
				&& Candidate.SourceZone == SourceZone
				&& Candidate.DestinationZone == DestinationZone;
		}))
	{
		ExistingRecord->MovedCount += MovedCount;
		ExistingRecord->RuntimeKeywords.AppendTags(CardInstance.RuntimeKeywords);
		return;
	}

	FFinalBattleMovedCardRecord& NewRecord = ChainRecords.MovedCardRecords.AddDefaulted_GetRef();
	NewRecord.RuntimeOwnerUnitId = RuntimeOwnerUnitId;
	NewRecord.CardId = CardInstance.CardId;
	NewRecord.MovedCount = MovedCount;
	NewRecord.bGeneratedCard = CardInstance.bGeneratedCard;
	NewRecord.RuntimeKeywords = CardInstance.RuntimeKeywords;
	NewRecord.SourceZone = SourceZone;
	NewRecord.DestinationZone = DestinationZone;
}

bool FFinalBattleConditionService::EvaluateCondition(
	const UFinalBattleConditionDefinition* Condition,
	const FFinalBattleConditionEvaluationContext& Context) const
{
	if (Condition == nullptr)
	{
		return false;
	}

	switch (Condition->GetConditionContext())
	{
	case EFinalBattleConditionContext::SourceOnly:
		if (const UFinalBattleConditionHandCard* HandCardCondition = Cast<UFinalBattleConditionHandCard>(Condition))
		{
			return SatisfiesHandCardRequirement(HandCardCondition->Requirement, Context);
		}
		break;

	case EFinalBattleConditionContext::ChainRecord:
		if (const UFinalBattleConditionStatusChanged* StatusChangedCondition = Cast<UFinalBattleConditionStatusChanged>(Condition))
		{
			return SatisfiesStatusChangeRequirement(StatusChangedCondition->Requirement, Context);
		}

		if (const UFinalBattleConditionMovedCards* MovedCardsCondition = Cast<UFinalBattleConditionMovedCards>(Condition))
		{
			return SatisfiesMovedCardRequirement(MovedCardsCondition->Requirement, Context);
		}
		break;

	case EFinalBattleConditionContext::TargetRequired:
		if (const UFinalBattleConditionTargetState* TargetStateCondition = Cast<UFinalBattleConditionTargetState>(Condition))
		{
			return SatisfiesTargetStateRequirement(TargetStateCondition->Requirement, Context.TargetEnemyState);
		}
		break;

	default:
		break;
	}

	return false;
}
