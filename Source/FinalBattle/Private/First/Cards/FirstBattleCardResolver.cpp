#include "First/Cards/FirstBattleCardResolver.h"

#include "First/Enemy/FirstBattleEnemyPartService.h"
#include "First/Events/FirstBattleEventLog.h"
#include "First/Hand/FirstBattleHandService.h"
#include "GameplayTagContainer.h"

void FFirstBattleCardResolver::ExecuteCardEffects(
	FFirstBattleState& State,
	const FFirstCardInstance& Card,
	FFirstEnemyPartState& TargetPart,
	const FFirstBattleEnemyPartService& EnemyPartService,
	const FFirstBattleHandService& HandService,
	const FFirstBattleEventLog& EventLog) const
{
	for (const FFirstCardEffectInstance& Effect : Card.Effects)
	{
		if (Effect.EffectType == EFirstCardEffectType::Damage)
		{
			ApplyCardDamageEffect(State, Card, TargetPart, Effect, EnemyPartService, EventLog);
		}
		else if (Effect.EffectType == EFirstCardEffectType::MoveHandCard)
		{
			ApplyMoveHandCardEffect(State, Card, Effect, HandService, EventLog);
		}
	}
}

bool FFirstBattleCardResolver::ResolvePerfectReleaseEvents(FFirstBattleState& State, const FFirstCardInstance& Card, const TMap<FName, int32>& PrePlayInitiatives, const FFirstBattleEventLog& EventLog) const
{
	if (HasSwiftKeyword(Card))
	{
		return false;
	}

	bool bTriggeredAny = false;
	for (const FFirstEnemyPartState& Part : State.EnemyParts)
	{
		const int32* PreInitiative = PrePlayInitiatives.Find(Part.PartId);
		if (PreInitiative == nullptr || *PreInitiative != Card.RuntimeCost)
		{
			continue;
		}

		FFirstBattleEvent Event;
		Event.EventType = EFirstBattleEventType::PerfectReleaseTriggered;
		Event.RelatedId = Part.PartId;
		Event.CardInstanceId = Card.CardInstanceId;
		Event.PartId = Part.PartId;
		Event.PrimaryValue = Card.RuntimeCost;
		Event.SecondaryValue = *PreInitiative;
		Event.Message = FText::Format(
			NSLOCTEXT("FirstBattle", "FirstPerfectReleaseTriggered", "{0} triggered perfect release."),
			Part.DisplayName.IsEmpty() ? FText::FromName(Part.PartId) : Part.DisplayName);
		EventLog.AppendEvent(State, Event);
		bTriggeredAny = true;
	}

	return bTriggeredAny;
}

void FFirstBattleCardResolver::ResolvePlayedCardDestination(FFirstBattleState& State, const FFirstCardInstance& PlayedCard, const FFirstBattleHandService& HandService, const FFirstBattleEventLog& EventLog) const
{
	if (PlayedCard.PlayDestination == EFirstCardPlayDestination::ReturnToHandRandomZone)
	{
		ReturnPlayedCardToHandRandomZone(State, PlayedCard, HandService, EventLog);
	}
}

bool FFirstBattleCardResolver::ShouldSkipInitiativeReductionForZonePerfectRelease(const FFirstCardInstance& Card, const EFirstHandZone PlayedHandZone, const bool bTriggeredPerfectRelease) const
{
	return Card.bSkipInitiativeReductionOnPerfectReleaseInZone
		&& bTriggeredPerfectRelease
		&& Card.PerfectReleaseInitiativeSkipZone == PlayedHandZone;
}

bool FFirstBattleCardResolver::HasSwiftKeyword(const FFirstCardInstance& Card)
{
	const FGameplayTag SwiftKeyword = FGameplayTag::RequestGameplayTag(TEXT("First.Keyword.Swift"), false);
	return SwiftKeyword.IsValid() && Card.Keywords.HasTagExact(SwiftKeyword);
}

void FFirstBattleCardResolver::ApplyCardDamageEffect(
	FFirstBattleState& State,
	const FFirstCardInstance& Card,
	FFirstEnemyPartState& TargetPart,
	const FFirstCardEffectInstance& Effect,
	const FFirstBattleEnemyPartService& EnemyPartService,
	const FFirstBattleEventLog& EventLog) const
{
	if (Effect.Value <= 0 || !EnemyPartService.IsAlivePart(TargetPart))
	{
		return;
	}

	const int32 PreviousHP = TargetPart.CurrentHP;
	TargetPart.CurrentHP = FMath::Max(0, TargetPart.CurrentHP - Effect.Value);
	if (PreviousHP > 0 && TargetPart.CurrentHP <= 0)
	{
		TargetPart.bDestroyed = true;

		FFirstBattleEvent DestroyedEvent;
		DestroyedEvent.EventType = EFirstBattleEventType::EnemyPartDestroyed;
		DestroyedEvent.RelatedId = TargetPart.PartId;
		DestroyedEvent.CardInstanceId = Card.CardInstanceId;
		DestroyedEvent.PartId = TargetPart.PartId;
		DestroyedEvent.PrimaryValue = PreviousHP;
		DestroyedEvent.SecondaryValue = TargetPart.CurrentHP;
		DestroyedEvent.Message = FText::Format(
			NSLOCTEXT("FirstBattle", "FirstEnemyPartDestroyed", "{0} was destroyed."),
			TargetPart.DisplayName.IsEmpty() ? FText::FromName(TargetPart.PartId) : TargetPart.DisplayName);
		EventLog.AppendEvent(State, DestroyedEvent);
	}
}

void FFirstBattleCardResolver::ApplyMoveHandCardEffect(FFirstBattleState& State, const FFirstCardInstance& Card, const FFirstCardEffectInstance& Effect, const FFirstBattleHandService& HandService, const FFirstBattleEventLog& EventLog) const
{
	const int32 MoveCount = FMath::Max(Effect.MoveCardCount, 0);
	for (int32 MoveIndex = 0; MoveIndex < MoveCount; ++MoveIndex)
	{
		if (!HandService.MoveRandomHandCard(State, Card, Effect, EventLog))
		{
			break;
		}
	}
}

void FFirstBattleCardResolver::ReturnPlayedCardToHandRandomZone(FFirstBattleState& State, const FFirstCardInstance& PlayedCard, const FFirstBattleHandService& HandService, const FFirstBattleEventLog& EventLog) const
{
	const int32 DiscardIndex = State.DiscardPile.IndexOfByPredicate(
		[&PlayedCard](const FFirstCardInstance& Card)
		{
			return Card.CardInstanceId == PlayedCard.CardInstanceId;
		});
	if (DiscardIndex == INDEX_NONE)
	{
		return;
	}

	FFirstCardInstance ReturningCard = State.DiscardPile[DiscardIndex];
	State.DiscardPile.RemoveAt(DiscardIndex);

	EFirstHandZone TargetZone = EFirstHandZone::None;
	const TArray<EFirstHandZone> TargetZones = HandService.ResolveValidHandMoveTargetZones(State);
	if (!TargetZones.IsEmpty())
	{
		TargetZone = TargetZones[State.RandomStream.RandRange(0, TargetZones.Num() - 1)];
		if (!HandService.InsertCardIntoZone(State, FFirstCardInstance(ReturningCard), TargetZone))
		{
			TargetZone = EFirstHandZone::None;
			State.HandCards.Add(ReturningCard);
		}
	}
	else
	{
		State.HandCards.Add(ReturningCard);
	}

	EventLog.AppendCardReturnedToHandEvent(State, ReturningCard, TargetZone);
}
