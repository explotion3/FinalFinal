#include "First/Hand/FirstBattleHandService.h"

#include "First/Events/FirstBattleEventLog.h"

FFirstCardInstance* FFirstBattleHandService::FindHandCard(FFirstBattleState& State, const FGuid& CardInstanceId) const
{
	return State.HandCards.FindByPredicate(
		[&CardInstanceId](const FFirstCardInstance& Card)
		{
			return Card.CardInstanceId == CardInstanceId;
		});
}

EFirstHandZone FFirstBattleHandService::ResolveCurrentHandZoneForCardIndex(const FFirstBattleState& State, const int32 CardIndex) const
{
	int32 LeftHandIndex = INDEX_NONE;
	int32 RightHandIndex = INDEX_NONE;
	ResolveHandAnchorIndices(State.HandCards, LeftHandIndex, RightHandIndex);
	return ResolveHandZoneForCard(State.HandCards, CardIndex, LeftHandIndex, RightHandIndex);
}

bool FFirstBattleHandService::MoveRandomHandCard(FFirstBattleState& State, const FFirstCardInstance& SourceCard, const FFirstCardEffectInstance& Effect, const FFirstBattleEventLog& EventLog) const
{
	const TArray<int32> CandidateIndices = CollectMoveCandidateIndices(State, Effect);
	if (CandidateIndices.IsEmpty())
	{
		return false;
	}

	const int32 CandidateListIndex = State.RandomStream.RandRange(0, CandidateIndices.Num() - 1);
	const int32 CardIndex = CandidateIndices[CandidateListIndex];
	if (!State.HandCards.IsValidIndex(CardIndex))
	{
		return false;
	}

	const EFirstHandZone SourceZone = ResolveCurrentHandZoneForCardIndex(State, CardIndex);
	const TArray<EFirstHandZone> TargetZones = ResolveMoveTargetZones(State, Effect, SourceZone);
	if (TargetZones.IsEmpty())
	{
		return false;
	}

	const EFirstHandZone TargetZone = TargetZones[State.RandomStream.RandRange(0, TargetZones.Num() - 1)];
	FFirstCardInstance MovedCard = State.HandCards[CardIndex];
	State.HandCards.RemoveAt(CardIndex);
	const int32 PreviousTargetCost = MovedCard.RuntimeCost;
	const int32 ActualTargetCostDelta = ApplyRuntimeCostDelta(MovedCard, Effect.MoveTargetCostDelta);

	if (!InsertCardIntoZone(State, FFirstCardInstance(MovedCard), TargetZone))
	{
		MovedCard.RuntimeCost = PreviousTargetCost;
		State.HandCards.Insert(MovedCard, FMath::Clamp(CardIndex, 0, State.HandCards.Num()));
		return false;
	}

	EventLog.AppendHandCardMovedEvent(State, SourceCard, MovedCard, SourceZone, TargetZone);
	if (ActualTargetCostDelta != 0)
	{
		EventLog.AppendCardRuntimeCostChangedEvent(State, MovedCard, PreviousTargetCost, MovedCard.RuntimeCost);
	}

	if (Effect.bTransferActualCostReductionToSourceCard && ActualTargetCostDelta < 0)
	{
		ApplyTransferredCostReductionToSourceCard(State, SourceCard, -ActualTargetCostDelta, EventLog);
	}
	return true;
}

TArray<EFirstHandZone> FFirstBattleHandService::ResolveValidHandMoveTargetZones(const FFirstBattleState& State) const
{
	TArray<EFirstHandZone> TargetZones;
	for (const EFirstHandZone Zone : {EFirstHandZone::Left, EFirstHandZone::Both, EFirstHandZone::Right})
	{
		if (IsValidHandMoveTargetZone(State.HandCards, Zone))
		{
			TargetZones.Add(Zone);
		}
	}
	return TargetZones;
}

bool FFirstBattleHandService::InsertCardIntoZone(FFirstBattleState& State, FFirstCardInstance&& Card, const EFirstHandZone TargetZone) const
{
	int32 LeftHandIndex = INDEX_NONE;
	int32 RightHandIndex = INDEX_NONE;
	ResolveHandAnchorIndices(State.HandCards, LeftHandIndex, RightHandIndex);

	if (TargetZone == EFirstHandZone::Left && LeftHandIndex != INDEX_NONE)
	{
		const int32 InsertIndex = State.RandomStream.RandRange(0, LeftHandIndex);
		State.HandCards.Insert(MoveTemp(Card), InsertIndex);
		return true;
	}

	if (TargetZone == EFirstHandZone::Both && LeftHandIndex != INDEX_NONE && RightHandIndex != INDEX_NONE && LeftHandIndex < RightHandIndex)
	{
		const int32 InsertIndex = State.RandomStream.RandRange(LeftHandIndex + 1, RightHandIndex);
		State.HandCards.Insert(MoveTemp(Card), InsertIndex);
		return true;
	}

	if (TargetZone == EFirstHandZone::Right && RightHandIndex != INDEX_NONE)
	{
		const int32 InsertIndex = State.RandomStream.RandRange(RightHandIndex + 1, State.HandCards.Num());
		State.HandCards.Insert(MoveTemp(Card), InsertIndex);
		return true;
	}

	return false;
}

void FFirstBattleHandService::RebuildHandWithRandomZones(FFirstBattleState& State, TArray<FFirstCardInstance>& Cards) const
{
	TArray<FFirstCardInstance> LeftCoreCards;
	TArray<FFirstCardInstance> RightCoreCards;
	TArray<FFirstCardInstance> OrdinaryCards;

	for (const FFirstCardInstance& Card : Cards)
	{
		if (Card.HandRole == EFirstHandRole::LeftHandCore)
		{
			LeftCoreCards.Add(Card);
		}
		else if (Card.HandRole == EFirstHandRole::RightHandCore)
		{
			RightCoreCards.Add(Card);
		}
		else
		{
			OrdinaryCards.Add(Card);
		}
	}

	const bool bHasLeftCore = !LeftCoreCards.IsEmpty();
	const bool bHasRightCore = !RightCoreCards.IsEmpty();
	TArray<FFirstCardInstance> LeftZoneCards;
	TArray<FFirstCardInstance> BothZoneCards;
	TArray<FFirstCardInstance> RightZoneCards;
	TArray<FFirstCardInstance> UnzonedCards;

	for (const FFirstCardInstance& Card : OrdinaryCards)
	{
		switch (RollZoneBucket(State, bHasLeftCore, bHasRightCore))
		{
		case 0:
			LeftZoneCards.Add(Card);
			break;
		case 1:
			BothZoneCards.Add(Card);
			break;
		case 2:
			RightZoneCards.Add(Card);
			break;
		default:
			UnzonedCards.Add(Card);
			break;
		}
	}

	Cards.Reset();
	Cards.Append(LeftZoneCards);
	Cards.Append(LeftCoreCards);
	Cards.Append(BothZoneCards);
	Cards.Append(RightCoreCards);
	Cards.Append(RightZoneCards);
	Cards.Append(UnzonedCards);
}

void FFirstBattleHandService::ResolveHandAnchorIndices(const TArray<FFirstCardInstance>& HandCards, int32& OutLeftHandIndex, int32& OutRightHandIndex)
{
	OutLeftHandIndex = HandCards.IndexOfByPredicate(
		[](const FFirstCardInstance& Card)
		{
			return Card.HandRole == EFirstHandRole::LeftHandCore;
		});
	OutRightHandIndex = HandCards.IndexOfByPredicate(
		[](const FFirstCardInstance& Card)
		{
			return Card.HandRole == EFirstHandRole::RightHandCore;
		});
}

bool FFirstBattleHandService::IsValidHandMoveTargetZone(const TArray<FFirstCardInstance>& HandCards, const EFirstHandZone Zone)
{
	int32 LeftHandIndex = INDEX_NONE;
	int32 RightHandIndex = INDEX_NONE;
	ResolveHandAnchorIndices(HandCards, LeftHandIndex, RightHandIndex);

	if (Zone == EFirstHandZone::Left)
	{
		return LeftHandIndex != INDEX_NONE;
	}
	if (Zone == EFirstHandZone::Both)
	{
		return LeftHandIndex != INDEX_NONE && RightHandIndex != INDEX_NONE && LeftHandIndex < RightHandIndex;
	}
	if (Zone == EFirstHandZone::Right)
	{
		return RightHandIndex != INDEX_NONE;
	}
	return false;
}

EFirstHandZone FFirstBattleHandService::ResolveHandZoneForCard(const TArray<FFirstCardInstance>& HandCards, const int32 CardIndex, const int32 LeftHandIndex, const int32 RightHandIndex)
{
	if (!HandCards.IsValidIndex(CardIndex) || HandCards[CardIndex].HandRole != EFirstHandRole::None)
	{
		return EFirstHandZone::None;
	}

	if (LeftHandIndex != INDEX_NONE && RightHandIndex != INDEX_NONE)
	{
		if (LeftHandIndex < RightHandIndex)
		{
			if (CardIndex < LeftHandIndex)
			{
				return EFirstHandZone::Left;
			}
			if (CardIndex > RightHandIndex)
			{
				return EFirstHandZone::Right;
			}
			if (CardIndex > LeftHandIndex && CardIndex < RightHandIndex)
			{
				return EFirstHandZone::Both;
			}
		}
		else
		{
			if (CardIndex < RightHandIndex)
			{
				return EFirstHandZone::Left;
			}
			if (CardIndex > LeftHandIndex)
			{
				return EFirstHandZone::Right;
			}
		}

		return EFirstHandZone::None;
	}

	if (LeftHandIndex != INDEX_NONE && CardIndex < LeftHandIndex)
	{
		return EFirstHandZone::Left;
	}

	if (RightHandIndex != INDEX_NONE && CardIndex > RightHandIndex)
	{
		return EFirstHandZone::Right;
	}

	return EFirstHandZone::None;
}

TArray<int32> FFirstBattleHandService::CollectMoveCandidateIndices(const FFirstBattleState& State, const FFirstCardEffectInstance& Effect) const
{
	TArray<int32> CandidateIndices;
	for (int32 CardIndex = 0; CardIndex < State.HandCards.Num(); ++CardIndex)
	{
		const FFirstCardInstance& Card = State.HandCards[CardIndex];
		if (Card.HandRole != EFirstHandRole::None)
		{
			continue;
		}

		const EFirstHandZone CardZone = ResolveCurrentHandZoneForCardIndex(State, CardIndex);
		if (Effect.bMoveRequiresSourceZone && CardZone != Effect.MoveSourceZone)
		{
			continue;
		}

		CandidateIndices.Add(CardIndex);
	}
	return CandidateIndices;
}

TArray<EFirstHandZone> FFirstBattleHandService::ResolveMoveTargetZones(const FFirstBattleState& State, const FFirstCardEffectInstance& Effect, const EFirstHandZone SourceZone) const
{
	TArray<EFirstHandZone> TargetZones;
	if (Effect.MoveTargetPolicy == EFirstHandMoveTargetPolicy::FixedZone)
	{
		if (IsValidHandMoveTargetZone(State.HandCards, Effect.MoveTargetZone))
		{
			TargetZones.Add(Effect.MoveTargetZone);
		}
		return TargetZones;
	}

	for (const EFirstHandZone Zone : {EFirstHandZone::Left, EFirstHandZone::Both, EFirstHandZone::Right})
	{
		if (!IsValidHandMoveTargetZone(State.HandCards, Zone))
		{
			continue;
		}
		if (Effect.MoveTargetPolicy == EFirstHandMoveTargetPolicy::RandomOtherThanSourceZone && Zone == SourceZone)
		{
			continue;
		}
		TargetZones.Add(Zone);
	}
	return TargetZones;
}

int32 FFirstBattleHandService::ApplyRuntimeCostDelta(FFirstCardInstance& Card, const int32 CostDelta) const
{
	if (CostDelta == 0)
	{
		return 0;
	}

	const int32 PreviousCost = Card.RuntimeCost;
	Card.RuntimeCost = FMath::Max(0, Card.RuntimeCost + CostDelta);
	return Card.RuntimeCost - PreviousCost;
}

void FFirstBattleHandService::ApplyTransferredCostReductionToSourceCard(FFirstBattleState& State, const FFirstCardInstance& SourceCard, const int32 ActualCostReduction, const FFirstBattleEventLog& EventLog) const
{
	if (ActualCostReduction <= 0)
	{
		return;
	}

	FFirstCardInstance* DiscardedSourceCard = State.DiscardPile.FindByPredicate(
		[&SourceCard](const FFirstCardInstance& Card)
		{
			return Card.CardInstanceId == SourceCard.CardInstanceId;
		});
	if (DiscardedSourceCard == nullptr)
	{
		return;
	}

	const int32 PreviousCost = DiscardedSourceCard->RuntimeCost;
	DiscardedSourceCard->RuntimeCost += ActualCostReduction;
	EventLog.AppendCardRuntimeCostChangedEvent(State, *DiscardedSourceCard, PreviousCost, DiscardedSourceCard->RuntimeCost);
}

int32 FFirstBattleHandService::RollZoneBucket(FFirstBattleState& State, const bool bHasLeftCore, const bool bHasRightCore) const
{
	if (bHasLeftCore && bHasRightCore)
	{
		return State.RandomStream.RandRange(0, 2);
	}

	if (bHasLeftCore)
	{
		return 0;
	}

	if (bHasRightCore)
	{
		return 2;
	}

	return INDEX_NONE;
}
