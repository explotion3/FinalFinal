#include "First/Deck/FirstBattleDeckService.h"

#include "First/Events/FirstBattleEventLog.h"
#include "First/Hand/FirstBattleHandService.h"

void FFirstBattleDeckService::ApplyEntryStatBonuses(FFirstBattleState& State, const FFirstBattleEventLog& EventLog) const
{
	for (const FFirstCardInstance& Card : State.HandCards)
	{
		ApplyCardEntryStatBonus(State, Card, EventLog);
	}

	for (const FFirstCardInstance& Card : State.DrawPile)
	{
		ApplyCardEntryStatBonus(State, Card, EventLog);
	}
}

void FFirstBattleDeckService::BeginPlayerTurnDraw(FFirstBattleState& State, const FFirstBattleEventLog& EventLog, const FFirstBattleHandService& HandService) const
{
	constexpr int32 DrawTargetCount = 5;

	TArray<FFirstCardInstance> DrawnCards;
	PullMissingCoreCards(State, DrawnCards, DrawTargetCount, EventLog);
	DrawCardsFromDrawPile(State, DrawnCards, DrawTargetCount, EventLog);

	if (DrawnCards.IsEmpty())
	{
		HandService.RebuildHandWithRandomZones(State, State.HandCards);
		return;
	}

	State.HandCards.Append(DrawnCards);
	HandService.RebuildHandWithRandomZones(State, State.HandCards);
}

void FFirstBattleDeckService::ApplyCardEntryStatBonus(FFirstBattleState& State, const FFirstCardInstance& Card, const FFirstBattleEventLog& EventLog) const
{
	const int32 Bonus = FMath::Max(0, Card.PlayerMaxHPBonusOnEnterBattle);
	if (Bonus <= 0)
	{
		return;
	}

	const int32 PreviousMaxHP = State.PlayerMaxHP;
	State.PlayerMaxHP += Bonus;
	State.PlayerCurrentHP = FMath::Clamp(State.PlayerCurrentHP + Bonus, 0, State.PlayerMaxHP);
	EventLog.AppendPlayerMaxHPChangedEvent(State, Card, PreviousMaxHP, State.PlayerMaxHP);
}

void FFirstBattleDeckService::PullMissingCoreCards(FFirstBattleState& State, TArray<FFirstCardInstance>& DrawnCards, const int32 DrawTargetCount, const FFirstBattleEventLog& EventLog) const
{
	if (DrawnCards.Num() >= DrawTargetCount)
	{
		return;
	}

	PullMissingCoreCard(State, EFirstHandRole::LeftHandCore, DrawnCards, EventLog);
	if (DrawnCards.Num() >= DrawTargetCount)
	{
		return;
	}

	PullMissingCoreCard(State, EFirstHandRole::RightHandCore, DrawnCards, EventLog);
}

bool FFirstBattleDeckService::PullMissingCoreCard(FFirstBattleState& State, const EFirstHandRole HandRole, TArray<FFirstCardInstance>& DrawnCards, const FFirstBattleEventLog& EventLog) const
{
	const bool bAlreadyInHand = State.HandCards.ContainsByPredicate(
		[HandRole](const FFirstCardInstance& Card)
		{
			return Card.HandRole == HandRole;
		});
	if (bAlreadyInHand)
	{
		return false;
	}

	FFirstCardInstance CoreCard;
	if (RemoveFirstCardWithRole(State.DrawPile, HandRole, CoreCard))
	{
		DrawnCards.Add(CoreCard);
		EventLog.AppendCardDrawnEvent(State, CoreCard, EFirstCardDrawSource::ForcedCoreFromDrawPile);
		return true;
	}

	if (RemoveFirstCardWithRole(State.DiscardPile, HandRole, CoreCard))
	{
		DrawnCards.Add(CoreCard);
		EventLog.AppendCardDrawnEvent(State, CoreCard, EFirstCardDrawSource::ForcedCoreFromDiscard);
		return true;
	}

	return false;
}

bool FFirstBattleDeckService::RemoveFirstCardWithRole(TArray<FFirstCardInstance>& Cards, const EFirstHandRole HandRole, FFirstCardInstance& OutCard) const
{
	const int32 CardIndex = Cards.IndexOfByPredicate(
		[HandRole](const FFirstCardInstance& Card)
		{
			return Card.HandRole == HandRole;
		});
	if (CardIndex == INDEX_NONE)
	{
		return false;
	}

	OutCard = Cards[CardIndex];
	Cards.RemoveAt(CardIndex);
	return true;
}

void FFirstBattleDeckService::DrawCardsFromDrawPile(FFirstBattleState& State, TArray<FFirstCardInstance>& DrawnCards, const int32 DrawTargetCount, const FFirstBattleEventLog& EventLog) const
{
	bool bDrawingFromShuffledDiscard = false;
	while (DrawnCards.Num() < DrawTargetCount)
	{
		if (State.DrawPile.IsEmpty())
		{
			if (!ShuffleDiscardIntoDrawPile(State, EventLog))
			{
				break;
			}

			bDrawingFromShuffledDiscard = true;
		}

		FFirstCardInstance DrawnCard = State.DrawPile[0];
		State.DrawPile.RemoveAt(0);
		DrawnCards.Add(DrawnCard);
		EventLog.AppendCardDrawnEvent(State, DrawnCard, bDrawingFromShuffledDiscard ? EFirstCardDrawSource::ShuffledDiscard : EFirstCardDrawSource::DrawPile);
	}
}

bool FFirstBattleDeckService::ShuffleDiscardIntoDrawPile(FFirstBattleState& State, const FFirstBattleEventLog& EventLog) const
{
	if (State.DiscardPile.IsEmpty())
	{
		return false;
	}

	TArray<FFirstCardInstance> ShuffledCards = State.DiscardPile;
	State.DiscardPile.Reset();
	ShuffleCards(State, ShuffledCards);
	EventLog.AppendDrawPileShuffledEvent(State, ShuffledCards.Num());
	State.DrawPile.Append(ShuffledCards);
	return true;
}

void FFirstBattleDeckService::ShuffleCards(FFirstBattleState& State, TArray<FFirstCardInstance>& Cards) const
{
	for (int32 Index = Cards.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = State.RandomStream.RandRange(0, Index);
		if (SwapIndex != Index)
		{
			Cards.Swap(Index, SwapIndex);
		}
	}
}
