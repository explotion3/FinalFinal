#pragma once
#include "Misc/AutomationTest.h"


#include "First/FirstBattleSession.h"

namespace FirstBattleKernelTests
{
	inline FGuid TestBattleId()
	{
		return FGuid(0x11111111, 0x22222222, 0x33333333, 0x44444444);
	}

	inline FGuid StrikeCardInstanceId()
	{
		return FGuid(0xaaaaaaaa, 0xbbbbbbbb, 0xcccccccc, 0xdddddddd);
	}

	inline FGuid SwiftCardInstanceId()
	{
		return FGuid(0xeeeeeeee, 0xffffffff, 0x12121212, 0x34343434);
	}

	inline FGuid LeftHandCardInstanceId()
	{
		return FGuid(0x10101010, 0x20202020, 0x30303030, 0x40404040);
	}

	inline FGuid RightHandCardInstanceId()
	{
		return FGuid(0x50505050, 0x60606060, 0x70707070, 0x80808080);
	}

	inline FGuid DrawCardInstanceId(const uint32 Index)
	{
		return FGuid(0x90000000 + Index, 0x11110000 + Index, 0x22220000 + Index, 0x33330000 + Index);
	}

	inline FFirstCardInstance MakeCard(const FGuid& CardInstanceId, const FName CardId, const int32 RuntimeCost = 2, const int32 Damage = 7, const EFirstHandRole HandRole = EFirstHandRole::None)
	{
		FFirstCardInstance Card;
		Card.CardInstanceId = CardInstanceId;
		Card.CardId = CardId;
		Card.DisplayName = FText::FromName(CardId);
		Card.BaseCost = RuntimeCost;
		Card.RuntimeCost = RuntimeCost;
		Card.HandRole = HandRole;

		FFirstCardEffectInstance& Effect = Card.Effects.AddDefaulted_GetRef();
		Effect.EffectId = TEXT("effect.damage.main");
		Effect.EffectType = EFirstCardEffectType::Damage;
		Effect.Value = Damage;
		return Card;
	}

	inline FFirstCardInstance MakeLeftHandCard()
	{
		return MakeCard(LeftHandCardInstanceId(), TEXT("card.test.left_hand"), 1, 1, EFirstHandRole::LeftHandCore);
	}

	inline FFirstCardInstance MakeRightHandCard()
	{
		return MakeCard(RightHandCardInstanceId(), TEXT("card.test.right_hand"), 1, 1, EFirstHandRole::RightHandCore);
	}

	inline FFirstCardInstance MakeSwiftCard()
	{
		FFirstCardInstance Card = MakeCard(SwiftCardInstanceId(), TEXT("card.test.swift"), 2, 7);
		Card.Keywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("First.Keyword.Swift")));
		return Card;
	}

	inline FFirstEnemyPartStartData MakePart(const FName PartId, const int32 PositionIndex, const int32 Initiative = 3, const int32 HP = 10)
	{
		FFirstEnemyPartStartData Part;
		Part.PartId = PartId;
		Part.DisplayName = FText::FromName(PartId);
		Part.PositionIndex = PositionIndex;
		Part.MaxHP = HP;
		Part.CurrentHP = HP;
		Part.CurrentIntentId = TEXT("intent.test.attack");
		Part.CurrentIntentDisplayName = NSLOCTEXT("FirstBattleTests", "IntentAttack", "Attack");
		Part.CurrentInitiative = Initiative;
		return Part;
	}

	inline FFirstEnemyPartIntentInstance MakeIntent(const FName IntentId, const int32 InitialInitiative, const int32 Damage = 0)
	{
		FFirstEnemyPartIntentInstance Intent;
		Intent.IntentId = IntentId;
		Intent.DisplayName = FText::FromName(IntentId);
		Intent.InitialInitiative = InitialInitiative;
		if (Damage > 0)
		{
			FFirstCardEffectInstance& Effect = Intent.Effects.AddDefaulted_GetRef();
			Effect.EffectId = TEXT("effect.intent.damage");
			Effect.EffectType = EFirstCardEffectType::Damage;
			Effect.Value = Damage;
		}
		return Intent;
	}

	inline FFirstEnemyPartStartData MakeSequencedPart(const FName PartId, const int32 PositionIndex, const TArray<FFirstEnemyPartIntentInstance>& Intents, const int32 CurrentIntentIndex = 0, const int32 HP = 10)
	{
		FFirstEnemyPartStartData Part = MakePart(PartId, PositionIndex, Intents.IsValidIndex(CurrentIntentIndex) ? Intents[CurrentIntentIndex].InitialInitiative : 0, HP);
		Part.IntentSequence = Intents;
		Part.CurrentIntentIndex = CurrentIntentIndex;
		if (Intents.IsValidIndex(CurrentIntentIndex))
		{
			Part.CurrentIntentId = Intents[CurrentIntentIndex].IntentId;
			Part.CurrentIntentDisplayName = Intents[CurrentIntentIndex].DisplayName;
		}
		return Part;
	}

	inline FFirstBattleStartParams MakeStartParams()
	{
		FFirstBattleStartParams Params;
		Params.BattleId = TestBattleId();
		Params.StartingRound = 1;
		Params.InitialHand.Add(MakeCard(StrikeCardInstanceId(), TEXT("card.test.strike"), 2, 7));
		Params.InitialHand.Add(MakeSwiftCard());
		Params.EnemyParts.Add(MakePart(TEXT("part.tail"), 2, 4, 10));
		Params.EnemyParts.Add(MakePart(TEXT("part.head"), 0, 3, 10));
		return Params;
	}

	inline FFirstBattleCommand MakePlayCommand(const FGuid& CardInstanceId, const FName TargetPartId)
	{
		FFirstBattleCommand Command;
		Command.CommandType = EFirstBattleCommandType::PlayCard;
		Command.CardInstanceId = CardInstanceId;
		Command.TargetPartId = TargetPartId;
		return Command;
	}

	inline FFirstBattleCommand MakeEndTurnCommand()
	{
		FFirstBattleCommand Command;
		Command.CommandType = EFirstBattleCommandType::EndTurn;
		return Command;
	}

	inline int32 CountEvents(const FFirstBattleSnapshot& Snapshot, const EFirstBattleEventType EventType)
	{
		return Snapshot.RecentEvents.FilterByPredicate(
			[EventType](const FFirstBattleEvent& Event)
			{
				return Event.EventType == EventType;
			}).Num();
	}

	inline const FFirstBattleEvent* FindEventForPart(const FFirstBattleSnapshot& Snapshot, const EFirstBattleEventType EventType, const FName PartId)
	{
		return Snapshot.RecentEvents.FindByPredicate(
			[EventType, PartId](const FFirstBattleEvent& Event)
			{
				return Event.EventType == EventType && Event.PartId == PartId;
			});
	}

	inline const FFirstCardViewData* FindCardView(const FFirstBattleSnapshot& Snapshot, const FName CardId)
	{
		return Snapshot.HandCards.FindByPredicate(
			[CardId](const FFirstCardViewData& Card)
			{
				return Card.CardId == CardId;
			});
	}

	inline FFirstBattleStartParams MakeHandZoneStartParams(const TArray<FFirstCardInstance>& HandCards)
	{
		FFirstBattleStartParams Params;
		Params.BattleId = TestBattleId();
		Params.StartingRound = 1;
		Params.InitialHand = HandCards;
		Params.EnemyParts.Add(MakePart(TEXT("part.head"), 0, 9, 20));
		return Params;
	}

	inline FFirstBattleStartParams MakeDrawStartParams()
	{
		FFirstBattleStartParams Params;
		Params.BattleId = TestBattleId();
		Params.RandomSeed = 12345;
		Params.StartingRound = 1;
		Params.EnemyParts.Add(MakeSequencedPart(TEXT("part.head"), 0, {MakeIntent(TEXT("intent.test.watch"), 4)}));
		return Params;
	}

	inline FFirstCardInstance MakeDrawCard(const int32 Index)
	{
		return MakeCard(DrawCardInstanceId(Index), FName(*FString::Printf(TEXT("card.test.draw_%d"), Index)), 1, 1);
	}

	inline FFirstCardInstance MakeBothRequiredCard(const FGuid& CardInstanceId, const FName CardId, const int32 RuntimeCost = 1, const int32 Damage = 1)
	{
		FFirstCardInstance Card = MakeCard(CardInstanceId, CardId, RuntimeCost, Damage);
		Card.bRequiresHandZoneToPlay = true;
		Card.RequiredHandZone = EFirstHandZone::Both;
		return Card;
	}

	inline FFirstCardInstance MakeLeftPerfectReleaseSkipCard(const FGuid& CardInstanceId, const FName CardId, const int32 RuntimeCost = 2, const int32 Damage = 1)
	{
		FFirstCardInstance Card = MakeCard(CardInstanceId, CardId, RuntimeCost, Damage);
		Card.bSkipInitiativeReductionOnPerfectReleaseInZone = true;
		Card.PerfectReleaseInitiativeSkipZone = EFirstHandZone::Left;
		return Card;
	}

	inline FFirstCardInstance MakeMoveHandCard(
		const FGuid& CardInstanceId,
		const FName CardId,
		const EFirstHandMoveTargetPolicy TargetPolicy = EFirstHandMoveTargetPolicy::RandomValidZone,
		const EFirstHandZone TargetZone = EFirstHandZone::None,
		const bool bRequiresSourceZone = false,
		const EFirstHandZone SourceZone = EFirstHandZone::None,
		const int32 MoveTargetCostDelta = 0,
		const bool bTransferActualCostReductionToSourceCard = false,
		const int32 RuntimeCost = 1)
	{
		FFirstCardInstance Card = MakeCard(CardInstanceId, CardId, RuntimeCost, 0);
		FFirstCardEffectInstance& Effect = Card.Effects.AddDefaulted_GetRef();
		Effect.EffectId = TEXT("effect.move.hand_card");
		Effect.EffectType = EFirstCardEffectType::MoveHandCard;
		Effect.MoveCardCount = 1;
		Effect.bMoveRequiresSourceZone = bRequiresSourceZone;
		Effect.MoveSourceZone = SourceZone;
		Effect.MoveTargetPolicy = TargetPolicy;
		Effect.MoveTargetZone = TargetZone;
		Effect.MoveTargetCostDelta = MoveTargetCostDelta;
		Effect.bTransferActualCostReductionToSourceCard = bTransferActualCostReductionToSourceCard;
		return Card;
	}

	inline TArray<FName> GetHandCardIds(const FFirstBattleSnapshot& Snapshot)
	{
		TArray<FName> CardIds;
		for (const FFirstCardViewData& Card : Snapshot.HandCards)
		{
			CardIds.Add(Card.CardId);
		}
		return CardIds;
	}

	inline int32 CountHandCardsInZone(const FFirstBattleSnapshot& Snapshot, const EFirstHandZone Zone)
	{
		return Snapshot.HandCards.FilterByPredicate(
			[Zone](const FFirstCardViewData& Card)
			{
				return Card.HandZone == Zone;
			}).Num();
	}

	inline const FFirstBattleEvent* FindHandCardMovedEvent(const FFirstBattleSnapshot& Snapshot)
	{
		return Snapshot.RecentEvents.FindByPredicate(
			[](const FFirstBattleEvent& Event)
			{
				return Event.EventType == EFirstBattleEventType::HandCardMoved;
			});
	}

	inline const FFirstBattleEvent* FindCostChangedEventForCard(const FFirstBattleSnapshot& Snapshot, const FGuid& CardInstanceId)
	{
		return Snapshot.RecentEvents.FindByPredicate(
			[CardInstanceId](const FFirstBattleEvent& Event)
			{
				return Event.EventType == EFirstBattleEventType::CardRuntimeCostChanged && Event.CardInstanceId == CardInstanceId;
			});
	}

	inline TArray<FName> GetDrawnCardIdsBySource(const FFirstBattleSnapshot& Snapshot, const EFirstCardDrawSource DrawSource)
	{
		TArray<FName> CardIds;
		for (const FFirstBattleEvent& Event : Snapshot.RecentEvents)
		{
			if (Event.EventType == EFirstBattleEventType::CardDrawn && Event.PrimaryValue == static_cast<int32>(DrawSource))
			{
				CardIds.Add(Event.RelatedId);
			}
		}
		return CardIds;
	}

	inline int32 CountDrawnCardsBySource(const FFirstBattleSnapshot& Snapshot, const EFirstCardDrawSource DrawSource)
	{
		return GetDrawnCardIdsBySource(Snapshot, DrawSource).Num();
	}

	inline FFirstCardInstance MakeReturnToHandCard(const FGuid& CardInstanceId, const FName CardId, const int32 RuntimeCost = 1, const int32 Damage = 7)
	{
		FFirstCardInstance Card = MakeCard(CardInstanceId, CardId, RuntimeCost, Damage);
		Card.PlayDestination = EFirstCardPlayDestination::ReturnToHandRandomZone;
		return Card;
	}
}
