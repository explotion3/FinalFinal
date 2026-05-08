#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "First/FirstBattleSession.h"

namespace FirstBattleKernelTests
{
	FGuid TestBattleId()
	{
		return FGuid(0x11111111, 0x22222222, 0x33333333, 0x44444444);
	}

	FGuid StrikeCardInstanceId()
	{
		return FGuid(0xaaaaaaaa, 0xbbbbbbbb, 0xcccccccc, 0xdddddddd);
	}

	FGuid SwiftCardInstanceId()
	{
		return FGuid(0xeeeeeeee, 0xffffffff, 0x12121212, 0x34343434);
	}

	FGuid LeftHandCardInstanceId()
	{
		return FGuid(0x10101010, 0x20202020, 0x30303030, 0x40404040);
	}

	FGuid RightHandCardInstanceId()
	{
		return FGuid(0x50505050, 0x60606060, 0x70707070, 0x80808080);
	}

	FGuid DrawCardInstanceId(const uint32 Index)
	{
		return FGuid(0x90000000 + Index, 0x11110000 + Index, 0x22220000 + Index, 0x33330000 + Index);
	}

	FFirstCardInstance MakeCard(const FGuid& CardInstanceId, const FName CardId, const int32 RuntimeCost = 2, const int32 Damage = 7, const EFirstHandRole HandRole = EFirstHandRole::None)
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

	FFirstCardInstance MakeLeftHandCard()
	{
		return MakeCard(LeftHandCardInstanceId(), TEXT("card.test.left_hand"), 1, 1, EFirstHandRole::LeftHandCore);
	}

	FFirstCardInstance MakeRightHandCard()
	{
		return MakeCard(RightHandCardInstanceId(), TEXT("card.test.right_hand"), 1, 1, EFirstHandRole::RightHandCore);
	}

	FFirstCardInstance MakeSwiftCard()
	{
		FFirstCardInstance Card = MakeCard(SwiftCardInstanceId(), TEXT("card.test.swift"), 2, 7);
		Card.Keywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("First.Keyword.Swift")));
		return Card;
	}

	FFirstEnemyPartStartData MakePart(const FName PartId, const int32 PositionIndex, const int32 Initiative = 3, const int32 HP = 10)
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

	FFirstEnemyPartIntentInstance MakeIntent(const FName IntentId, const int32 InitialInitiative, const int32 Damage = 0)
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

	FFirstEnemyPartStartData MakeSequencedPart(const FName PartId, const int32 PositionIndex, const TArray<FFirstEnemyPartIntentInstance>& Intents, const int32 CurrentIntentIndex = 0, const int32 HP = 10)
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

	FFirstBattleStartParams MakeStartParams()
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

	FFirstBattleCommand MakePlayCommand(const FGuid& CardInstanceId, const FName TargetPartId)
	{
		FFirstBattleCommand Command;
		Command.CommandType = EFirstBattleCommandType::PlayCard;
		Command.CardInstanceId = CardInstanceId;
		Command.TargetPartId = TargetPartId;
		return Command;
	}

	FFirstBattleCommand MakeEndTurnCommand()
	{
		FFirstBattleCommand Command;
		Command.CommandType = EFirstBattleCommandType::EndTurn;
		return Command;
	}

	int32 CountEvents(const FFirstBattleSnapshot& Snapshot, const EFirstBattleEventType EventType)
	{
		return Snapshot.RecentEvents.FilterByPredicate(
			[EventType](const FFirstBattleEvent& Event)
			{
				return Event.EventType == EventType;
			}).Num();
	}

	const FFirstBattleEvent* FindEventForPart(const FFirstBattleSnapshot& Snapshot, const EFirstBattleEventType EventType, const FName PartId)
	{
		return Snapshot.RecentEvents.FindByPredicate(
			[EventType, PartId](const FFirstBattleEvent& Event)
			{
				return Event.EventType == EventType && Event.PartId == PartId;
			});
	}

	const FFirstCardViewData* FindCardView(const FFirstBattleSnapshot& Snapshot, const FName CardId)
	{
		return Snapshot.HandCards.FindByPredicate(
			[CardId](const FFirstCardViewData& Card)
			{
				return Card.CardId == CardId;
			});
	}

	FFirstBattleStartParams MakeHandZoneStartParams(const TArray<FFirstCardInstance>& HandCards)
	{
		FFirstBattleStartParams Params;
		Params.BattleId = TestBattleId();
		Params.StartingRound = 1;
		Params.InitialHand = HandCards;
		Params.EnemyParts.Add(MakePart(TEXT("part.head"), 0, 9, 20));
		return Params;
	}

	FFirstBattleStartParams MakeDrawStartParams()
	{
		FFirstBattleStartParams Params;
		Params.BattleId = TestBattleId();
		Params.RandomSeed = 12345;
		Params.StartingRound = 1;
		Params.EnemyParts.Add(MakeSequencedPart(TEXT("part.head"), 0, {MakeIntent(TEXT("intent.test.watch"), 4)}));
		return Params;
	}

	FFirstCardInstance MakeDrawCard(const int32 Index)
	{
		return MakeCard(DrawCardInstanceId(Index), FName(*FString::Printf(TEXT("card.test.draw_%d"), Index)), 1, 1);
	}

	FFirstCardInstance MakeBothRequiredCard(const FGuid& CardInstanceId, const FName CardId, const int32 RuntimeCost = 1, const int32 Damage = 1)
	{
		FFirstCardInstance Card = MakeCard(CardInstanceId, CardId, RuntimeCost, Damage);
		Card.bRequiresHandZoneToPlay = true;
		Card.RequiredHandZone = EFirstHandZone::Both;
		return Card;
	}

	FFirstCardInstance MakeLeftPerfectReleaseSkipCard(const FGuid& CardInstanceId, const FName CardId, const int32 RuntimeCost = 2, const int32 Damage = 1)
	{
		FFirstCardInstance Card = MakeCard(CardInstanceId, CardId, RuntimeCost, Damage);
		Card.bSkipInitiativeReductionOnPerfectReleaseInZone = true;
		Card.PerfectReleaseInitiativeSkipZone = EFirstHandZone::Left;
		return Card;
	}

	FFirstCardInstance MakeMoveHandCard(
		const FGuid& CardInstanceId,
		const FName CardId,
		const EFirstHandMoveTargetPolicy TargetPolicy = EFirstHandMoveTargetPolicy::RandomValidZone,
		const EFirstHandZone TargetZone = EFirstHandZone::None,
		const bool bRequiresSourceZone = false,
		const EFirstHandZone SourceZone = EFirstHandZone::None)
	{
		FFirstCardInstance Card = MakeCard(CardInstanceId, CardId, 1, 0);
		FFirstCardEffectInstance& Effect = Card.Effects.AddDefaulted_GetRef();
		Effect.EffectId = TEXT("effect.move.hand_card");
		Effect.EffectType = EFirstCardEffectType::MoveHandCard;
		Effect.MoveCardCount = 1;
		Effect.bMoveRequiresSourceZone = bRequiresSourceZone;
		Effect.MoveSourceZone = SourceZone;
		Effect.MoveTargetPolicy = TargetPolicy;
		Effect.MoveTargetZone = TargetZone;
		return Card;
	}

	TArray<FName> GetHandCardIds(const FFirstBattleSnapshot& Snapshot)
	{
		TArray<FName> CardIds;
		for (const FFirstCardViewData& Card : Snapshot.HandCards)
		{
			CardIds.Add(Card.CardId);
		}
		return CardIds;
	}

	int32 CountHandCardsInZone(const FFirstBattleSnapshot& Snapshot, const EFirstHandZone Zone)
	{
		return Snapshot.HandCards.FilterByPredicate(
			[Zone](const FFirstCardViewData& Card)
			{
				return Card.HandZone == Zone;
			}).Num();
	}

	const FFirstBattleEvent* FindHandCardMovedEvent(const FFirstBattleSnapshot& Snapshot)
	{
		return Snapshot.RecentEvents.FindByPredicate(
			[](const FFirstBattleEvent& Event)
			{
				return Event.EventType == EFirstBattleEventType::HandCardMoved;
			});
	}

	TArray<FName> GetDrawnCardIdsBySource(const FFirstBattleSnapshot& Snapshot, const EFirstCardDrawSource DrawSource)
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

	int32 CountDrawnCardsBySource(const FFirstBattleSnapshot& Snapshot, const EFirstCardDrawSource DrawSource)
	{
		return GetDrawnCardIdsBySource(Snapshot, DrawSource).Num();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelInitializeBuildsSnapshotTest,
	"Final.Battle.First.Kernel.InitializeBuildsSnapshot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelInitializeBuildsSnapshotTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleSession Session;
	Session.Initialize(MakeStartParams());

	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();
	TestEqual(TEXT("Snapshot should use start BattleId."), Snapshot.BattleId, TestBattleId());
	TestTrue(TEXT("Snapshot should be initialized."), Snapshot.bInitialized);
	TestEqual(TEXT("Snapshot should include default player max HP."), Snapshot.PlayerMaxHP, 30);
	TestEqual(TEXT("Snapshot should include default player current HP."), Snapshot.PlayerCurrentHP, 30);
	TestEqual(TEXT("Snapshot should include initial hand."), Snapshot.HandCards.Num(), 2);
	TestEqual(TEXT("Snapshot should include enemy parts."), Snapshot.EnemyParts.Num(), 2);
	TestEqual(TEXT("Enemy parts should be sorted by position."), Snapshot.EnemyParts[0].PartId, FName(TEXT("part.head")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelSnapshotDoesNotExposeRuntimeStateTest,
	"Final.Battle.First.Kernel.SnapshotDoesNotExposeRuntimeState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelSnapshotDoesNotExposeRuntimeStateTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleSession Session;
	Session.Initialize(MakeStartParams());

	FFirstBattleSnapshot Snapshot = Session.GetSnapshot();
	Snapshot.HandCards.Empty();
	Snapshot.EnemyParts[0].CurrentHP = 0;

	const FFirstBattleSnapshot FreshSnapshot = Session.GetSnapshot();
	TestEqual(TEXT("Runtime hand should be unaffected by snapshot mutation."), FreshSnapshot.HandCards.Num(), 2);
	TestEqual(TEXT("Runtime enemy part HP should be unaffected by snapshot mutation."), FreshSnapshot.EnemyParts[0].CurrentHP, 10);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelHandZonesProjectLeftBothRightTest,
	"Final.Battle.First.Kernel.HandZonesProjectLeftBothRight",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelHandZonesProjectLeftBothRightTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleSession Session;
	Session.Initialize(MakeHandZoneStartParams({
		MakeCard(FGuid(0x10000001, 0, 0, 0), TEXT("card.test.left_zone")),
		MakeLeftHandCard(),
		MakeCard(FGuid(0x10000002, 0, 0, 0), TEXT("card.test.both_zone_a")),
		MakeCard(FGuid(0x10000003, 0, 0, 0), TEXT("card.test.both_zone_b")),
		MakeRightHandCard(),
		MakeCard(FGuid(0x10000004, 0, 0, 0), TEXT("card.test.right_zone"))
	}));

	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Left zone card should project Left."), FindCardView(Snapshot, TEXT("card.test.left_zone"))->HandZone, EFirstHandZone::Left);
	TestEqual(TEXT("Left hand core should have role LeftHandCore."), FindCardView(Snapshot, TEXT("card.test.left_hand"))->HandRole, EFirstHandRole::LeftHandCore);
	TestEqual(TEXT("Left hand core should not be in a zone."), FindCardView(Snapshot, TEXT("card.test.left_hand"))->HandZone, EFirstHandZone::None);
	TestEqual(TEXT("First between-anchor card should project Both."), FindCardView(Snapshot, TEXT("card.test.both_zone_a"))->HandZone, EFirstHandZone::Both);
	TestEqual(TEXT("Second between-anchor card should project Both."), FindCardView(Snapshot, TEXT("card.test.both_zone_b"))->HandZone, EFirstHandZone::Both);
	TestEqual(TEXT("Right hand core should have role RightHandCore."), FindCardView(Snapshot, TEXT("card.test.right_hand"))->HandRole, EFirstHandRole::RightHandCore);
	TestEqual(TEXT("Right hand core should not be in a zone."), FindCardView(Snapshot, TEXT("card.test.right_hand"))->HandZone, EFirstHandZone::None);
	TestEqual(TEXT("Right zone card should project Right."), FindCardView(Snapshot, TEXT("card.test.right_zone"))->HandZone, EFirstHandZone::Right);
	TestEqual(TEXT("Hand index should follow current hand order."), FindCardView(Snapshot, TEXT("card.test.both_zone_b"))->HandIndex, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelHandZonesMissingLeftKeepsRightOnlyTest,
	"Final.Battle.First.Kernel.HandZonesMissingLeftKeepsRightOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelHandZonesMissingLeftKeepsRightOnlyTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleSession Session;
	Session.Initialize(MakeHandZoneStartParams({
		MakeCard(FGuid(0x10000011, 0, 0, 0), TEXT("card.test.before_right")),
		MakeRightHandCard(),
		MakeCard(FGuid(0x10000012, 0, 0, 0), TEXT("card.test.right_zone"))
	}));

	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Card before right anchor should have no Both zone without left anchor."), FindCardView(Snapshot, TEXT("card.test.before_right"))->HandZone, EFirstHandZone::None);
	TestEqual(TEXT("Card after right anchor should project Right."), FindCardView(Snapshot, TEXT("card.test.right_zone"))->HandZone, EFirstHandZone::Right);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelHandZonesMissingRightKeepsLeftOnlyTest,
	"Final.Battle.First.Kernel.HandZonesMissingRightKeepsLeftOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelHandZonesMissingRightKeepsLeftOnlyTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleSession Session;
	Session.Initialize(MakeHandZoneStartParams({
		MakeCard(FGuid(0x10000021, 0, 0, 0), TEXT("card.test.left_zone")),
		MakeLeftHandCard(),
		MakeCard(FGuid(0x10000022, 0, 0, 0), TEXT("card.test.after_left"))
	}));

	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Card before left anchor should project Left."), FindCardView(Snapshot, TEXT("card.test.left_zone"))->HandZone, EFirstHandZone::Left);
	TestEqual(TEXT("Card after left anchor should have no Both zone without right anchor."), FindCardView(Snapshot, TEXT("card.test.after_left"))->HandZone, EFirstHandZone::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelHandZonesReversedAnchorsDoNotCreateBothTest,
	"Final.Battle.First.Kernel.HandZonesReversedAnchorsDoNotCreateBoth",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelHandZonesReversedAnchorsDoNotCreateBothTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleSession Session;
	Session.Initialize(MakeHandZoneStartParams({
		MakeCard(FGuid(0x10000031, 0, 0, 0), TEXT("card.test.left_side")),
		MakeRightHandCard(),
		MakeCard(FGuid(0x10000032, 0, 0, 0), TEXT("card.test.between_reversed")),
		MakeLeftHandCard(),
		MakeCard(FGuid(0x10000033, 0, 0, 0), TEXT("card.test.right_side"))
	}));

	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Card before left anchor should project Left even when anchors are reversed."), FindCardView(Snapshot, TEXT("card.test.left_side"))->HandZone, EFirstHandZone::Left);
	TestEqual(TEXT("Card after right anchor should project Right even when anchors are reversed."), FindCardView(Snapshot, TEXT("card.test.right_side"))->HandZone, EFirstHandZone::Right);
	TestEqual(TEXT("Reversed anchors should not create Both or overlapping zones."), FindCardView(Snapshot, TEXT("card.test.between_reversed"))->HandZone, EFirstHandZone::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelPlayingLeftHandCoreInvalidatesBothZoneTest,
	"Final.Battle.First.Kernel.PlayingLeftHandCoreInvalidatesBothZone",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelPlayingLeftHandCoreInvalidatesBothZoneTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleSession Session;
	Session.Initialize(MakeHandZoneStartParams({
		MakeCard(FGuid(0x10000041, 0, 0, 0), TEXT("card.test.left_zone")),
		MakeLeftHandCard(),
		MakeCard(FGuid(0x10000042, 0, 0, 0), TEXT("card.test.former_both")),
		MakeRightHandCard(),
		MakeCard(FGuid(0x10000043, 0, 0, 0), TEXT("card.test.right_zone"))
	}));

	const FFirstBattleCommandResult Result = Session.SubmitCommand(MakePlayCommand(LeftHandCardInstanceId(), TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestTrue(TEXT("Left hand core should be playable."), Result.IsAccepted());
	TestEqual(TEXT("Played left hand core should enter discard."), Snapshot.DiscardPileCount, 1);
	TestTrue(TEXT("Left hand core should leave hand."), FindCardView(Snapshot, TEXT("card.test.left_hand")) == nullptr);
	TestEqual(TEXT("Former Both card should no longer be Both without left anchor."), FindCardView(Snapshot, TEXT("card.test.former_both"))->HandZone, EFirstHandZone::None);
	TestEqual(TEXT("Right zone should remain projected from right anchor."), FindCardView(Snapshot, TEXT("card.test.right_zone"))->HandZone, EFirstHandZone::Right);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelPlayingRightHandCoreInvalidatesBothZoneTest,
	"Final.Battle.First.Kernel.PlayingRightHandCoreInvalidatesBothZone",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelPlayingRightHandCoreInvalidatesBothZoneTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleSession Session;
	Session.Initialize(MakeHandZoneStartParams({
		MakeCard(FGuid(0x10000051, 0, 0, 0), TEXT("card.test.left_zone")),
		MakeLeftHandCard(),
		MakeCard(FGuid(0x10000052, 0, 0, 0), TEXT("card.test.former_both")),
		MakeRightHandCard(),
		MakeCard(FGuid(0x10000053, 0, 0, 0), TEXT("card.test.right_zone"))
	}));

	const FFirstBattleCommandResult Result = Session.SubmitCommand(MakePlayCommand(RightHandCardInstanceId(), TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestTrue(TEXT("Right hand core should be playable."), Result.IsAccepted());
	TestEqual(TEXT("Played right hand core should enter discard."), Snapshot.DiscardPileCount, 1);
	TestTrue(TEXT("Right hand core should leave hand."), FindCardView(Snapshot, TEXT("card.test.right_hand")) == nullptr);
	TestEqual(TEXT("Former Both card should no longer be Both without right anchor."), FindCardView(Snapshot, TEXT("card.test.former_both"))->HandZone, EFirstHandZone::None);
	TestEqual(TEXT("Left zone should remain projected from left anchor."), FindCardView(Snapshot, TEXT("card.test.left_zone"))->HandZone, EFirstHandZone::Left);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelEndTurnDrawsNextTurnHandTest,
	"Final.Battle.First.Kernel.EndTurnDrawsNextTurnHand",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelEndTurnDrawsNextTurnHandTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params = MakeDrawStartParams();
	Params.InitialDrawPile = {
		MakeLeftHandCard(),
		MakeDrawCard(1),
		MakeDrawCard(2),
		MakeRightHandCard(),
		MakeDrawCard(3),
		MakeDrawCard(4),
		MakeDrawCard(5)
	};

	FFirstBattleSession Session;
	Session.Initialize(Params);

	const FFirstBattleCommandResult Result = Session.SubmitCommand(MakeEndTurnCommand());
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestTrue(TEXT("EndTurn should be accepted."), Result.IsAccepted());
	TestEqual(TEXT("EndTurn should advance round."), Snapshot.CurrentRound, 2);
	TestEqual(TEXT("Player turn start event should be recorded."), CountEvents(Snapshot, EFirstBattleEventType::PlayerTurnStarted), 1);
	TestEqual(TEXT("Player turn start event should record the new round."), Snapshot.RecentEvents.FindByPredicate([](const FFirstBattleEvent& Event) { return Event.EventType == EFirstBattleEventType::PlayerTurnStarted; })->PrimaryValue, 2);
	TestEqual(TEXT("Exactly five cards should be drawn into hand."), Snapshot.HandCards.Num(), 5);
	TestEqual(TEXT("Five drawn card events should be recorded."), CountEvents(Snapshot, EFirstBattleEventType::CardDrawn), 5);
	TestEqual(TEXT("Draw pile should keep cards beyond the five-card draw count."), Snapshot.DrawPileCount, 2);
	TestNotNull(TEXT("Left hand core should be in hand."), FindCardView(Snapshot, TEXT("card.test.left_hand")));
	TestNotNull(TEXT("Right hand core should be in hand."), FindCardView(Snapshot, TEXT("card.test.right_hand")));
	TestEqual(TEXT("Left hand core should retain role."), FindCardView(Snapshot, TEXT("card.test.left_hand"))->HandRole, EFirstHandRole::LeftHandCore);
	TestEqual(TEXT("Right hand core should retain role."), FindCardView(Snapshot, TEXT("card.test.right_hand"))->HandRole, EFirstHandRole::RightHandCore);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelMissingCoreCardsPulledFromDiscardBeforeDrawFillTest,
	"Final.Battle.First.Kernel.MissingCoreCardsPulledFromDiscardBeforeDrawFill",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelMissingCoreCardsPulledFromDiscardBeforeDrawFillTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params = MakeDrawStartParams();
	Params.InitialHand = {
		MakeCard(FGuid(0x91000001, 0, 0, 0), TEXT("card.test.retained"))
	};
	Params.InitialDrawPile = {
		MakeDrawCard(1),
		MakeDrawCard(2),
		MakeDrawCard(3),
		MakeDrawCard(4),
		MakeDrawCard(5)
	};

	FFirstBattleSession Session;
	Session.Initialize(Params);

	Session.SubmitCommand(MakePlayCommand(Params.InitialHand[0].CardInstanceId, TEXT("part.head")));
	FFirstBattleStartParams ParamsWithCores = MakeDrawStartParams();
	ParamsWithCores.InitialDrawPile = Params.InitialDrawPile;
	ParamsWithCores.InitialHand = {MakeLeftHandCard(), MakeRightHandCard()};

	FFirstBattleSession CoreSession;
	CoreSession.Initialize(ParamsWithCores);
	CoreSession.SubmitCommand(MakePlayCommand(LeftHandCardInstanceId(), TEXT("part.head")));
	CoreSession.SubmitCommand(MakePlayCommand(RightHandCardInstanceId(), TEXT("part.head")));
	const FFirstBattleSnapshot BeforeDraw = CoreSession.GetSnapshot();
	TestEqual(TEXT("Both core cards should be in discard before draw."), BeforeDraw.DiscardPileCount, 2);

	CoreSession.SubmitCommand(MakeEndTurnCommand());
	const FFirstBattleSnapshot Snapshot = CoreSession.GetSnapshot();

	TestEqual(TEXT("Missing cores should count toward the five drawn cards."), Snapshot.HandCards.Num(), 5);
	TestEqual(TEXT("Only three normal cards should be drawn after two forced cores."), Snapshot.DrawPileCount, 2);
	TestEqual(TEXT("Discard should be emptied of forced core cards."), Snapshot.DiscardPileCount, 0);
	TestNotNull(TEXT("Left hand core should be pulled from discard."), FindCardView(Snapshot, TEXT("card.test.left_hand")));
	TestNotNull(TEXT("Right hand core should be pulled from discard."), FindCardView(Snapshot, TEXT("card.test.right_hand")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelDrawPileShortageDrawsAvailableCardsOnlyTest,
	"Final.Battle.First.Kernel.DrawPileShortageDrawsAvailableCardsOnly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelDrawPileShortageDrawsAvailableCardsOnlyTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params = MakeDrawStartParams();
	Params.InitialDrawPile = {
		MakeLeftHandCard(),
		MakeDrawCard(1)
	};

	FFirstBattleSession Session;
	Session.Initialize(Params);
	Session.SubmitCommand(MakeEndTurnCommand());
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Only available cards should be drawn."), Snapshot.HandCards.Num(), 2);
	TestEqual(TEXT("Draw pile should be empty."), Snapshot.DrawPileCount, 0);
	TestEqual(TEXT("No discard shuffle should occur."), Snapshot.DiscardPileCount, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelDrawPileShortageShufflesDiscardToContinueDrawingTest,
	"Final.Battle.First.Kernel.DrawPileShortageShufflesDiscardToContinueDrawing",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelDrawPileShortageShufflesDiscardToContinueDrawingTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params = MakeDrawStartParams();
	Params.InitialHand = {
		MakeCard(DrawCardInstanceId(11), TEXT("card.test.discard_1"), 1, 1),
		MakeCard(DrawCardInstanceId(12), TEXT("card.test.discard_2"), 1, 1),
		MakeCard(DrawCardInstanceId(13), TEXT("card.test.discard_3"), 1, 1)
	};
	Params.InitialDrawPile = {
		MakeLeftHandCard(),
		MakeRightHandCard(),
		MakeDrawCard(1)
	};
	Params.EnemyParts[0].MaxHP = 50;
	Params.EnemyParts[0].CurrentHP = 50;

	FFirstBattleSession Session;
	Session.Initialize(Params);
	Session.SubmitCommand(MakePlayCommand(DrawCardInstanceId(11), TEXT("part.head")));
	Session.SubmitCommand(MakePlayCommand(DrawCardInstanceId(12), TEXT("part.head")));
	Session.SubmitCommand(MakePlayCommand(DrawCardInstanceId(13), TEXT("part.head")));

	Session.SubmitCommand(MakeEndTurnCommand());
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Draw should fill from draw pile then shuffled discard."), Snapshot.HandCards.Num(), 5);
	TestEqual(TEXT("Discard should be emptied by shuffle."), Snapshot.DiscardPileCount, 0);
	TestEqual(TEXT("Draw pile should keep leftovers from shuffled discard."), Snapshot.DrawPileCount, 1);
	TestEqual(TEXT("One shuffle event should be recorded."), CountEvents(Snapshot, EFirstBattleEventType::DrawPileShuffled), 1);
	TestEqual(TEXT("Shuffle event should record shuffled card count."), Snapshot.RecentEvents.FindByPredicate([](const FFirstBattleEvent& Event) { return Event.EventType == EFirstBattleEventType::DrawPileShuffled; })->PrimaryValue, 3);
	TestEqual(TEXT("Two core cards should be forced from draw pile."), CountDrawnCardsBySource(Snapshot, EFirstCardDrawSource::ForcedCoreFromDrawPile), 2);
	TestEqual(TEXT("One normal card should be drawn from original draw pile."), CountDrawnCardsBySource(Snapshot, EFirstCardDrawSource::DrawPile), 1);
	TestEqual(TEXT("Two cards should be drawn from shuffled discard."), CountDrawnCardsBySource(Snapshot, EFirstCardDrawSource::ShuffledDiscard), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelEmptyDrawPileShufflesDiscardAndDrawsTest,
	"Final.Battle.First.Kernel.EmptyDrawPileShufflesDiscardAndDraws",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelEmptyDrawPileShufflesDiscardAndDrawsTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params = MakeDrawStartParams();
	Params.InitialHand = {
		MakeCard(DrawCardInstanceId(21), TEXT("card.test.discard_1"), 1, 1),
		MakeCard(DrawCardInstanceId(22), TEXT("card.test.discard_2"), 1, 1)
	};
	Params.EnemyParts[0].MaxHP = 50;
	Params.EnemyParts[0].CurrentHP = 50;

	FFirstBattleSession Session;
	Session.Initialize(Params);
	Session.SubmitCommand(MakePlayCommand(DrawCardInstanceId(21), TEXT("part.head")));
	Session.SubmitCommand(MakePlayCommand(DrawCardInstanceId(22), TEXT("part.head")));

	Session.SubmitCommand(MakeEndTurnCommand());
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Only shuffled discard cards should be drawn."), Snapshot.HandCards.Num(), 2);
	TestEqual(TEXT("Draw pile should be empty after drawing all shuffled cards."), Snapshot.DrawPileCount, 0);
	TestEqual(TEXT("Discard should be empty after shuffle."), Snapshot.DiscardPileCount, 0);
	TestEqual(TEXT("Draw pile shuffle should be recorded."), CountEvents(Snapshot, EFirstBattleEventType::DrawPileShuffled), 1);
	TestEqual(TEXT("All drawn cards should be marked as shuffled discard."), CountDrawnCardsBySource(Snapshot, EFirstCardDrawSource::ShuffledDiscard), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelForcedCoreCardsDoNotEnterDiscardShuffleTest,
	"Final.Battle.First.Kernel.ForcedCoreCardsDoNotEnterDiscardShuffle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelForcedCoreCardsDoNotEnterDiscardShuffleTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params = MakeDrawStartParams();
	Params.InitialHand = {
		MakeLeftHandCard(),
		MakeRightHandCard(),
		MakeCard(DrawCardInstanceId(31), TEXT("card.test.normal"), 1, 1)
	};
	Params.EnemyParts[0].MaxHP = 50;
	Params.EnemyParts[0].CurrentHP = 50;

	FFirstBattleSession Session;
	Session.Initialize(Params);
	Session.SubmitCommand(MakePlayCommand(LeftHandCardInstanceId(), TEXT("part.head")));
	Session.SubmitCommand(MakePlayCommand(RightHandCardInstanceId(), TEXT("part.head")));
	Session.SubmitCommand(MakePlayCommand(DrawCardInstanceId(31), TEXT("part.head")));

	Session.SubmitCommand(MakeEndTurnCommand());
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Both cores should be forced from discard."), CountDrawnCardsBySource(Snapshot, EFirstCardDrawSource::ForcedCoreFromDiscard), 2);
	TestEqual(TEXT("Only the non-core discard should be shuffled."), Snapshot.RecentEvents.FindByPredicate([](const FFirstBattleEvent& Event) { return Event.EventType == EFirstBattleEventType::DrawPileShuffled; })->PrimaryValue, 1);
	TestEqual(TEXT("Normal played card should be drawn from shuffled discard."), CountDrawnCardsBySource(Snapshot, EFirstCardDrawSource::ShuffledDiscard), 1);
	TestNotNull(TEXT("Left hand core should be back in hand."), FindCardView(Snapshot, TEXT("card.test.left_hand")));
	TestNotNull(TEXT("Right hand core should be back in hand."), FindCardView(Snapshot, TEXT("card.test.right_hand")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelSameRandomSeedProducesStableDiscardShuffleTest,
	"Final.Battle.First.Kernel.SameRandomSeedProducesStableDiscardShuffle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelSameRandomSeedProducesStableDiscardShuffleTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	auto BuildSession = []()
	{
		FFirstBattleStartParams Params = MakeDrawStartParams();
		Params.RandomSeed = 2468;
		Params.InitialHand = {
			MakeCard(DrawCardInstanceId(41), TEXT("card.test.discard_1"), 1, 1),
			MakeCard(DrawCardInstanceId(42), TEXT("card.test.discard_2"), 1, 1),
			MakeCard(DrawCardInstanceId(43), TEXT("card.test.discard_3"), 1, 1),
			MakeCard(DrawCardInstanceId(44), TEXT("card.test.discard_4"), 1, 1),
			MakeCard(DrawCardInstanceId(45), TEXT("card.test.discard_5"), 1, 1)
		};
		Params.EnemyParts[0].MaxHP = 100;
		Params.EnemyParts[0].CurrentHP = 100;

		FFirstBattleSession Session;
		Session.Initialize(Params);
		for (const FFirstCardInstance& Card : Params.InitialHand)
		{
			Session.SubmitCommand(MakePlayCommand(Card.CardInstanceId, TEXT("part.head")));
		}
		Session.SubmitCommand(MakeEndTurnCommand());
		return Session;
	};

	FFirstBattleSession SessionA = BuildSession();
	FFirstBattleSession SessionB = BuildSession();

	TestEqual(TEXT("Same seed should produce stable shuffled discard draw order."), GetDrawnCardIdsBySource(SessionA.GetSnapshot(), EFirstCardDrawSource::ShuffledDiscard), GetDrawnCardIdsBySource(SessionB.GetSnapshot(), EFirstCardDrawSource::ShuffledDiscard));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelDifferentRandomSeedsCanProduceDifferentDiscardShuffleTest,
	"Final.Battle.First.Kernel.DifferentRandomSeedsCanProduceDifferentDiscardShuffle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelDifferentRandomSeedsCanProduceDifferentDiscardShuffleTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	auto BuildDrawOrder = [](const int32 RandomSeed)
	{
		FFirstBattleStartParams Params = MakeDrawStartParams();
		Params.RandomSeed = RandomSeed;
		Params.InitialHand = {
			MakeCard(DrawCardInstanceId(51), TEXT("card.test.discard_1"), 1, 1),
			MakeCard(DrawCardInstanceId(52), TEXT("card.test.discard_2"), 1, 1),
			MakeCard(DrawCardInstanceId(53), TEXT("card.test.discard_3"), 1, 1),
			MakeCard(DrawCardInstanceId(54), TEXT("card.test.discard_4"), 1, 1),
			MakeCard(DrawCardInstanceId(55), TEXT("card.test.discard_5"), 1, 1)
		};
		Params.EnemyParts[0].MaxHP = 100;
		Params.EnemyParts[0].CurrentHP = 100;

		FFirstBattleSession Session;
		Session.Initialize(Params);
		for (const FFirstCardInstance& Card : Params.InitialHand)
		{
			Session.SubmitCommand(MakePlayCommand(Card.CardInstanceId, TEXT("part.head")));
		}
		Session.SubmitCommand(MakeEndTurnCommand());
		return GetDrawnCardIdsBySource(Session.GetSnapshot(), EFirstCardDrawSource::ShuffledDiscard);
	};

	TestNotEqual(TEXT("Different seeds should be able to produce different discard shuffle draw order."), BuildDrawOrder(1), BuildDrawOrder(999));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelRetainedHandCanExceedFiveAfterDrawTest,
	"Final.Battle.First.Kernel.RetainedHandCanExceedFiveAfterDraw",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelRetainedHandCanExceedFiveAfterDrawTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params = MakeDrawStartParams();
	Params.InitialHand = {
		MakeCard(DrawCardInstanceId(61), TEXT("card.test.retained_1"), 1, 1),
		MakeCard(DrawCardInstanceId(62), TEXT("card.test.retained_2"), 1, 1),
		MakeCard(DrawCardInstanceId(63), TEXT("card.test.retained_3"), 1, 1)
	};
	Params.InitialDrawPile = {
		MakeLeftHandCard(),
		MakeRightHandCard(),
		MakeDrawCard(1),
		MakeDrawCard(2),
		MakeDrawCard(3)
	};

	FFirstBattleSession Session;
	Session.Initialize(Params);
	Session.SubmitCommand(MakeEndTurnCommand());
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Retained hand cards plus five drawn cards should coexist without hand cap."), Snapshot.HandCards.Num(), 8);
	TestEqual(TEXT("Exactly five draw events should be recorded."), CountEvents(Snapshot, EFirstBattleEventType::CardDrawn), 5);
	TestNotNull(TEXT("Retained card should not be discarded."), FindCardView(Snapshot, TEXT("card.test.retained_1")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelRetainedHandCardsParticipateInRandomZonesTest,
	"Final.Battle.First.Kernel.RetainedHandCardsParticipateInRandomZones",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelRetainedHandCardsParticipateInRandomZonesTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params = MakeDrawStartParams();
	Params.InitialHand = {
		MakeCard(FGuid(0x92000001, 0, 0, 0), TEXT("card.test.retained")),
		MakeLeftHandCard(),
		MakeRightHandCard()
	};
	Params.InitialDrawPile = {
		MakeDrawCard(1),
		MakeDrawCard(2)
	};

	FFirstBattleSession Session;
	Session.Initialize(Params);
	Session.SubmitCommand(MakeEndTurnCommand());
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestNotNull(TEXT("Retained hand card should stay in hand."), FindCardView(Snapshot, TEXT("card.test.retained")));
	TestEqual(TEXT("Retained plus drawn ordinary cards should all receive a valid zone with both anchors."), CountHandCardsInZone(Snapshot, EFirstHandZone::Left) + CountHandCardsInZone(Snapshot, EFirstHandZone::Both) + CountHandCardsInZone(Snapshot, EFirstHandZone::Right), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelSameRandomSeedProducesStableHandOrderTest,
	"Final.Battle.First.Kernel.SameRandomSeedProducesStableHandOrder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelSameRandomSeedProducesStableHandOrderTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params = MakeDrawStartParams();
	Params.RandomSeed = 777;
	Params.InitialDrawPile = {
		MakeLeftHandCard(),
		MakeDrawCard(1),
		MakeDrawCard(2),
		MakeRightHandCard(),
		MakeDrawCard(3),
		MakeDrawCard(4)
	};

	FFirstBattleSession SessionA;
	SessionA.Initialize(Params);
	SessionA.SubmitCommand(MakeEndTurnCommand());

	FFirstBattleSession SessionB;
	SessionB.Initialize(Params);
	SessionB.SubmitCommand(MakeEndTurnCommand());

	TestEqual(TEXT("Same random seed should produce stable hand order."), GetHandCardIds(SessionA.GetSnapshot()), GetHandCardIds(SessionB.GetSnapshot()));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelDifferentRandomSeedsCanProduceDifferentHandOrderTest,
	"Final.Battle.First.Kernel.DifferentRandomSeedsCanProduceDifferentHandOrder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelDifferentRandomSeedsCanProduceDifferentHandOrderTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams ParamsA = MakeDrawStartParams();
	ParamsA.RandomSeed = 1;
	ParamsA.InitialDrawPile = {
		MakeLeftHandCard(),
		MakeDrawCard(1),
		MakeDrawCard(2),
		MakeDrawCard(3),
		MakeRightHandCard(),
		MakeDrawCard(4),
		MakeDrawCard(5)
	};

	FFirstBattleStartParams ParamsB = ParamsA;
	ParamsB.RandomSeed = 999;

	FFirstBattleSession SessionA;
	SessionA.Initialize(ParamsA);
	SessionA.SubmitCommand(MakeEndTurnCommand());

	FFirstBattleSession SessionB;
	SessionB.Initialize(ParamsB);
	SessionB.SubmitCommand(MakeEndTurnCommand());

	TestNotEqual(TEXT("Different random seeds should be able to produce different hand order."), GetHandCardIds(SessionA.GetSnapshot()), GetHandCardIds(SessionB.GetSnapshot()));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelPlayingCoreReturnsItOnNextTurnDrawTest,
	"Final.Battle.First.Kernel.PlayingCoreReturnsItOnNextTurnDraw",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelPlayingCoreReturnsItOnNextTurnDrawTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params = MakeDrawStartParams();
	Params.InitialHand = {
		MakeLeftHandCard(),
		MakeRightHandCard()
	};
	Params.InitialDrawPile = {
		MakeDrawCard(1),
		MakeDrawCard(2),
		MakeDrawCard(3),
		MakeDrawCard(4)
	};

	FFirstBattleSession Session;
	Session.Initialize(Params);
	Session.SubmitCommand(MakePlayCommand(LeftHandCardInstanceId(), TEXT("part.head")));
	TestTrue(TEXT("Left hand should leave hand immediately after play."), FindCardView(Session.GetSnapshot(), TEXT("card.test.left_hand")) == nullptr);

	Session.SubmitCommand(MakeEndTurnCommand());
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestNotNull(TEXT("Played left hand core should return on next turn draw."), FindCardView(Snapshot, TEXT("card.test.left_hand")));
	TestEqual(TEXT("Left hand core should retain role after returning."), FindCardView(Snapshot, TEXT("card.test.left_hand"))->HandRole, EFirstHandRole::LeftHandCore);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelDefeatingEndTurnDoesNotDrawOrAdvanceHandTest,
	"Final.Battle.First.Kernel.DefeatingEndTurnDoesNotDrawOrAdvanceHand",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelDefeatingEndTurnDoesNotDrawOrAdvanceHandTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params;
	Params.BattleId = TestBattleId();
	Params.PlayerMaxHP = 5;
	Params.PlayerCurrentHP = 5;
	Params.InitialDrawPile = {MakeLeftHandCard(), MakeRightHandCard(), MakeDrawCard(1)};
	Params.EnemyParts.Add(MakeSequencedPart(TEXT("part.head"), 0, {MakeIntent(TEXT("intent.test.attack"), 3, 5)}));

	FFirstBattleSession Session;
	Session.Initialize(Params);
	Session.SubmitCommand(MakeEndTurnCommand());
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestTrue(TEXT("Battle should end in defeat."), Snapshot.bBattleEnded);
	TestEqual(TEXT("Defeating EndTurn should not advance round."), Snapshot.CurrentRound, 1);
	TestEqual(TEXT("Defeating EndTurn should not draw cards."), Snapshot.HandCards.Num(), 0);
	TestEqual(TEXT("Draw pile should remain untouched after defeat."), Snapshot.DrawPileCount, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelBothZoneRequirementAllowsBothZoneCardTest,
	"Final.Battle.First.Kernel.BothZoneRequirementAllowsBothZoneCard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelBothZoneRequirementAllowsBothZoneCardTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	const FGuid BothCardId(0x93000001, 0, 0, 0);
	FFirstBattleSession Session;
	Session.Initialize(MakeHandZoneStartParams({
		MakeLeftHandCard(),
		MakeBothRequiredCard(BothCardId, TEXT("card.test.requires_both")),
		MakeRightHandCard()
	}));

	const FFirstBattleCommandResult Result = Session.SubmitCommand(MakePlayCommand(BothCardId, TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestTrue(TEXT("Both-zone card should be playable while in Both."), Result.IsAccepted());
	TestTrue(TEXT("Played Both-zone card should leave hand."), FindCardView(Snapshot, TEXT("card.test.requires_both")) == nullptr);
	TestEqual(TEXT("Played Both-zone card should enter discard."), Snapshot.DiscardPileCount, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelBothZoneRequirementRejectsNonBothZoneCardWithoutMutationTest,
	"Final.Battle.First.Kernel.BothZoneRequirementRejectsNonBothZoneCardWithoutMutation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelBothZoneRequirementRejectsNonBothZoneCardWithoutMutationTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	const FGuid BothCardId(0x93000002, 0, 0, 0);
	FFirstBattleSession Session;
	Session.Initialize(MakeHandZoneStartParams({
		MakeBothRequiredCard(BothCardId, TEXT("card.test.requires_both")),
		MakeLeftHandCard(),
		MakeRightHandCard()
	}));
	const FFirstBattleSnapshot Before = Session.GetSnapshot();

	const FFirstBattleCommandResult Result = Session.SubmitCommand(MakePlayCommand(BothCardId, TEXT("part.head")));
	const FFirstBattleSnapshot After = Session.GetSnapshot();

	TestFalse(TEXT("Both-zone card should be rejected outside Both."), Result.IsAccepted());
	TestEqual(TEXT("Rejection should use stable reason tag."), Result.ReasonTag, FName(TEXT("first.command.rejected.hand_zone_requirement_not_met")));
	TestEqual(TEXT("Rejected card should remain in hand."), After.HandCards.Num(), Before.HandCards.Num());
	TestEqual(TEXT("Rejected card should not enter discard."), After.DiscardPileCount, Before.DiscardPileCount);
	TestEqual(TEXT("Rejected card should not modify target HP."), After.EnemyParts[0].CurrentHP, Before.EnemyParts[0].CurrentHP);
	TestEqual(TEXT("Rejected card should not append events."), After.RecentEvents.Num(), Before.RecentEvents.Num());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelBothZoneRequirementRejectsWhenAnchorMissingTest,
	"Final.Battle.First.Kernel.BothZoneRequirementRejectsWhenAnchorMissing",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelBothZoneRequirementRejectsWhenAnchorMissingTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	const FGuid BothCardId(0x93000003, 0, 0, 0);
	FFirstBattleSession Session;
	Session.Initialize(MakeHandZoneStartParams({
		MakeLeftHandCard(),
		MakeBothRequiredCard(BothCardId, TEXT("card.test.requires_both"))
	}));

	const FFirstBattleCommandResult Result = Session.SubmitCommand(MakePlayCommand(BothCardId, TEXT("part.head")));

	TestFalse(TEXT("Both-zone card should be rejected when right anchor is missing."), Result.IsAccepted());
	TestEqual(TEXT("Missing-anchor rejection should use hand-zone reason."), Result.ReasonTag, FName(TEXT("first.command.rejected.hand_zone_requirement_not_met")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelLeftZonePerfectReleaseSkipsInitiativeReductionTest,
	"Final.Battle.First.Kernel.LeftZonePerfectReleaseSkipsInitiativeReduction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelLeftZonePerfectReleaseSkipsInitiativeReductionTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	const FGuid SkipCardId(0x93000004, 0, 0, 0);
	FFirstBattleStartParams Params = MakeHandZoneStartParams({
		MakeLeftPerfectReleaseSkipCard(SkipCardId, TEXT("card.test.left_skip"), 2, 1),
		MakeLeftHandCard(),
		MakeRightHandCard()
	});
	Params.EnemyParts[0].CurrentInitiative = 2;
	Params.EnemyParts[0].IntentSequence.Empty();
	Params.EnemyParts[0].CurrentIntentId = TEXT("intent.test.wait");
	Params.EnemyParts[0].CurrentIntentDisplayName = NSLOCTEXT("FirstBattleTests", "IntentWait", "Wait");

	FFirstBattleSession Session;
	Session.Initialize(Params);
	Session.SubmitCommand(MakePlayCommand(SkipCardId, TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Perfect release should still trigger."), CountEvents(Snapshot, EFirstBattleEventType::PerfectReleaseTriggered), 1);
	TestEqual(TEXT("Zone perfect release skip should not emit initiative changes."), CountEvents(Snapshot, EFirstBattleEventType::InitiativeChanged), 0);
	TestEqual(TEXT("Enemy initiative should remain unchanged."), Snapshot.EnemyParts[0].CurrentInitiative, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelLeftZoneSkipDoesNotApplyWithoutPerfectReleaseTest,
	"Final.Battle.First.Kernel.LeftZoneSkipDoesNotApplyWithoutPerfectRelease",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelLeftZoneSkipDoesNotApplyWithoutPerfectReleaseTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	const FGuid SkipCardId(0x93000005, 0, 0, 0);
	FFirstBattleStartParams Params = MakeHandZoneStartParams({
		MakeLeftPerfectReleaseSkipCard(SkipCardId, TEXT("card.test.left_skip"), 2, 1),
		MakeLeftHandCard(),
		MakeRightHandCard()
	});
	Params.EnemyParts[0].CurrentInitiative = 5;

	FFirstBattleSession Session;
	Session.Initialize(Params);
	Session.SubmitCommand(MakePlayCommand(SkipCardId, TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("No perfect release should trigger."), CountEvents(Snapshot, EFirstBattleEventType::PerfectReleaseTriggered), 0);
	TestEqual(TEXT("Initiative should still be reduced without perfect release."), CountEvents(Snapshot, EFirstBattleEventType::InitiativeChanged), 1);
	TestEqual(TEXT("Enemy initiative should be reduced by card cost."), Snapshot.EnemyParts[0].CurrentInitiative, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelPerfectReleaseInWrongZoneDoesNotSkipInitiativeTest,
	"Final.Battle.First.Kernel.PerfectReleaseInWrongZoneDoesNotSkipInitiative",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelPerfectReleaseInWrongZoneDoesNotSkipInitiativeTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	const FGuid SkipCardId(0x93000006, 0, 0, 0);
	FFirstBattleStartParams Params = MakeHandZoneStartParams({
		MakeLeftHandCard(),
		MakeLeftPerfectReleaseSkipCard(SkipCardId, TEXT("card.test.left_skip"), 2, 1),
		MakeRightHandCard()
	});
	Params.EnemyParts[0].CurrentInitiative = 2;

	FFirstBattleSession Session;
	Session.Initialize(Params);
	Session.SubmitCommand(MakePlayCommand(SkipCardId, TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Perfect release should trigger in wrong zone."), CountEvents(Snapshot, EFirstBattleEventType::PerfectReleaseTriggered), 1);
	TestEqual(TEXT("Wrong zone should not skip initiative reduction."), CountEvents(Snapshot, EFirstBattleEventType::InitiativeChanged), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelMoveHandCardRandomValidZoneMovesOrdinaryCardTest,
	"Final.Battle.First.Kernel.MoveHandCardRandomValidZoneMovesOrdinaryCard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelMoveHandCardRandomValidZoneMovesOrdinaryCardTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	const FGuid MoveCardId(0x94000001, 0, 0, 0);
	FFirstBattleStartParams Params = MakeHandZoneStartParams({
		MakeMoveHandCard(MoveCardId, TEXT("card.test.move")),
		MakeLeftHandCard(),
		MakeCard(FGuid(0x94000002, 0, 0, 0), TEXT("card.test.ordinary")),
		MakeRightHandCard()
	});
	Params.RandomSeed = 7;

	FFirstBattleSession Session;
	Session.Initialize(Params);
	const FFirstBattleCommandResult Result = Session.SubmitCommand(MakePlayCommand(MoveCardId, TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestTrue(TEXT("Move card should be accepted."), Result.IsAccepted());
	TestEqual(TEXT("Move effect should produce one event."), CountEvents(Snapshot, EFirstBattleEventType::HandCardMoved), 1);
	TestNotNull(TEXT("Moved ordinary card should remain in hand."), FindCardView(Snapshot, TEXT("card.test.ordinary")));
	TestEqual(TEXT("Move effect source card should enter discard."), Snapshot.DiscardPileCount, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelMoveHandCardSourceZoneBothOnlyMovesBothCardTest,
	"Final.Battle.First.Kernel.MoveHandCardSourceZoneBothOnlyMovesBothCard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelMoveHandCardSourceZoneBothOnlyMovesBothCardTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	const FGuid MoveCardId(0x94000003, 0, 0, 0);
	FFirstBattleStartParams Params = MakeHandZoneStartParams({
		MakeMoveHandCard(MoveCardId, TEXT("card.test.move"), EFirstHandMoveTargetPolicy::FixedZone, EFirstHandZone::Right, true, EFirstHandZone::Both),
		MakeCard(FGuid(0x94000004, 0, 0, 0), TEXT("card.test.left_candidate")),
		MakeLeftHandCard(),
		MakeCard(FGuid(0x94000005, 0, 0, 0), TEXT("card.test.both_candidate")),
		MakeRightHandCard(),
		MakeCard(FGuid(0x94000006, 0, 0, 0), TEXT("card.test.right_candidate"))
	});

	FFirstBattleSession Session;
	Session.Initialize(Params);
	Session.SubmitCommand(MakePlayCommand(MoveCardId, TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	const FFirstBattleEvent* MoveEvent = FindHandCardMovedEvent(Snapshot);
	TestNotNull(TEXT("Move event should exist."), MoveEvent);
	if (MoveEvent != nullptr)
	{
		TestEqual(TEXT("Only the Both-zone candidate should move."), MoveEvent->CardInstanceId, FGuid(0x94000005, 0, 0, 0));
		TestEqual(TEXT("Move source zone should be Both."), MoveEvent->PrimaryValue, static_cast<int32>(EFirstHandZone::Both));
		TestEqual(TEXT("Move target zone should be Right."), MoveEvent->SecondaryValue, static_cast<int32>(EFirstHandZone::Right));
	}
	TestEqual(TEXT("Both candidate should now project Right."), FindCardView(Snapshot, TEXT("card.test.both_candidate"))->HandZone, EFirstHandZone::Right);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelMoveHandCardRandomOtherThanSourceDoesNotMoveBackToSourceTest,
	"Final.Battle.First.Kernel.MoveHandCardRandomOtherThanSourceDoesNotMoveBackToSource",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelMoveHandCardRandomOtherThanSourceDoesNotMoveBackToSourceTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	const FGuid MoveCardId(0x94000007, 0, 0, 0);
	FFirstBattleStartParams Params = MakeHandZoneStartParams({
		MakeMoveHandCard(MoveCardId, TEXT("card.test.move"), EFirstHandMoveTargetPolicy::RandomOtherThanSourceZone),
		MakeLeftHandCard(),
		MakeCard(FGuid(0x94000008, 0, 0, 0), TEXT("card.test.both_candidate")),
		MakeRightHandCard(),
		MakeCard(FGuid(0x94000009, 0, 0, 0), TEXT("card.test.right_candidate"))
	});
	Params.RandomSeed = 13;

	FFirstBattleSession Session;
	Session.Initialize(Params);
	Session.SubmitCommand(MakePlayCommand(MoveCardId, TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	const FFirstBattleEvent* MoveEvent = FindHandCardMovedEvent(Snapshot);
	TestNotNull(TEXT("Move event should exist."), MoveEvent);
	if (MoveEvent != nullptr)
	{
		TestNotEqual(TEXT("RandomOtherThanSource should not target the source zone."), MoveEvent->SecondaryValue, MoveEvent->PrimaryValue);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelMoveHandCardFixedZonesInsertIntoRequestedZoneTest,
	"Final.Battle.First.Kernel.MoveHandCardFixedZonesInsertIntoRequestedZone",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelMoveHandCardFixedZonesInsertIntoRequestedZoneTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	const FGuid MoveCardId(0x94000010, 0, 0, 0);
	FFirstBattleStartParams Params = MakeHandZoneStartParams({
		MakeMoveHandCard(MoveCardId, TEXT("card.test.move"), EFirstHandMoveTargetPolicy::FixedZone, EFirstHandZone::Left, true, EFirstHandZone::Both),
		MakeLeftHandCard(),
		MakeCard(FGuid(0x94000011, 0, 0, 0), TEXT("card.test.both_candidate")),
		MakeRightHandCard()
	});

	FFirstBattleSession Session;
	Session.Initialize(Params);
	Session.SubmitCommand(MakePlayCommand(MoveCardId, TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Fixed target Left should project moved card Left."), FindCardView(Snapshot, TEXT("card.test.both_candidate"))->HandZone, EFirstHandZone::Left);
	const FFirstBattleEvent* MoveEvent = FindHandCardMovedEvent(Snapshot);
	TestNotNull(TEXT("Move event should exist."), MoveEvent);
	if (MoveEvent != nullptr)
	{
		TestEqual(TEXT("Event target should be Left."), MoveEvent->SecondaryValue, static_cast<int32>(EFirstHandZone::Left));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelMoveHandCardMissingTargetZoneNoOpsTest,
	"Final.Battle.First.Kernel.MoveHandCardMissingTargetZoneNoOps",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelMoveHandCardMissingTargetZoneNoOpsTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	const FGuid MoveCardId(0x94000012, 0, 0, 0);
	FFirstBattleStartParams Params = MakeHandZoneStartParams({
		MakeMoveHandCard(MoveCardId, TEXT("card.test.move"), EFirstHandMoveTargetPolicy::FixedZone, EFirstHandZone::Both),
		MakeLeftHandCard(),
		MakeCard(FGuid(0x94000013, 0, 0, 0), TEXT("card.test.left_candidate"))
	});

	FFirstBattleSession Session;
	Session.Initialize(Params);
	const TArray<FName> BeforeOrder = GetHandCardIds(Session.GetSnapshot());
	const FFirstBattleCommandResult Result = Session.SubmitCommand(MakePlayCommand(MoveCardId, TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestTrue(TEXT("Move no-op should still accept the play command."), Result.IsAccepted());
	TestEqual(TEXT("No valid Both target should produce no move event."), CountEvents(Snapshot, EFirstBattleEventType::HandCardMoved), 0);
	TArray<FName> ExpectedOrder = BeforeOrder;
	ExpectedOrder.Remove(TEXT("card.test.move"));
	TestEqual(TEXT("No-op move should preserve remaining hand order."), GetHandCardIds(Snapshot), ExpectedOrder);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelMoveHandCardNoOrdinaryCandidatesNoOpsTest,
	"Final.Battle.First.Kernel.MoveHandCardNoOrdinaryCandidatesNoOps",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelMoveHandCardNoOrdinaryCandidatesNoOpsTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	const FGuid MoveCardId(0x94000014, 0, 0, 0);
	FFirstBattleStartParams Params = MakeHandZoneStartParams({
		MakeMoveHandCard(MoveCardId, TEXT("card.test.move")),
		MakeLeftHandCard(),
		MakeRightHandCard()
	});

	FFirstBattleSession Session;
	Session.Initialize(Params);
	const FFirstBattleCommandResult Result = Session.SubmitCommand(MakePlayCommand(MoveCardId, TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestTrue(TEXT("No-candidate move should still accept the play command."), Result.IsAccepted());
	TestEqual(TEXT("No ordinary candidates should produce no move event."), CountEvents(Snapshot, EFirstBattleEventType::HandCardMoved), 0);
	TestEqual(TEXT("Core cards should remain in hand."), Snapshot.HandCards.Num(), 2);
	TestNotNull(TEXT("Left core should not be moved away."), FindCardView(Snapshot, TEXT("card.test.left_hand")));
	TestNotNull(TEXT("Right core should not be moved away."), FindCardView(Snapshot, TEXT("card.test.right_hand")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelPlayCardAppliesDamageAndMovesCardTest,
	"Final.Battle.First.Kernel.PlayCardAppliesDamageAndMovesCard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelPlayCardAppliesDamageAndMovesCardTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleSession Session;
	Session.Initialize(MakeStartParams());

	const FFirstBattleCommandResult Result = Session.SubmitCommand(MakePlayCommand(StrikeCardInstanceId(), TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestTrue(TEXT("PlayCard should be accepted."), Result.IsAccepted());
	TestEqual(TEXT("Played card should leave hand."), Snapshot.HandCards.Num(), 1);
	TestEqual(TEXT("Played card should enter discard pile."), Snapshot.DiscardPileCount, 1);
	TestEqual(TEXT("Damage should reduce target HP."), Snapshot.EnemyParts[0].CurrentHP, 3);
	TestEqual(TEXT("CardPlayed event should be appended."), CountEvents(Snapshot, EFirstBattleEventType::CardPlayed), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelPlayCardReducesAllAlivePartInitiativeTest,
	"Final.Battle.First.Kernel.PlayCardReducesAllAlivePartInitiative",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelPlayCardReducesAllAlivePartInitiativeTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleSession Session;
	Session.Initialize(MakeStartParams());

	Session.SubmitCommand(MakePlayCommand(StrikeCardInstanceId(), TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Head initiative should be reduced by RuntimeCost."), Snapshot.EnemyParts[0].CurrentInitiative, 1);
	TestEqual(TEXT("Tail initiative should be reduced by RuntimeCost."), Snapshot.EnemyParts[1].CurrentInitiative, 2);
	TestEqual(TEXT("InitiativeChanged should be emitted for both alive parts."), CountEvents(Snapshot, EFirstBattleEventType::InitiativeChanged), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelSwiftSkipsInitiativeAndPerfectReleaseTest,
	"Final.Battle.First.Kernel.SwiftSkipsInitiativeAndPerfectRelease",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelSwiftSkipsInitiativeAndPerfectReleaseTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params = MakeStartParams();
	Params.EnemyParts[0].CurrentInitiative = 2;
	Params.EnemyParts[1].CurrentInitiative = 2;

	FFirstBattleSession Session;
	Session.Initialize(Params);

	Session.SubmitCommand(MakePlayCommand(SwiftCardInstanceId(), TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Swift should not reduce head initiative."), Snapshot.EnemyParts[0].CurrentInitiative, 2);
	TestEqual(TEXT("Swift should not reduce tail initiative."), Snapshot.EnemyParts[1].CurrentInitiative, 2);
	TestEqual(TEXT("Swift should not trigger perfect release."), CountEvents(Snapshot, EFirstBattleEventType::PerfectReleaseTriggered), 0);
	TestEqual(TEXT("Swift should not append initiative changed events."), CountEvents(Snapshot, EFirstBattleEventType::InitiativeChanged), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelPerfectReleaseTriggersForMatchingPartsTest,
	"Final.Battle.First.Kernel.PerfectReleaseTriggersForMatchingParts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelPerfectReleaseTriggersForMatchingPartsTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params = MakeStartParams();
	Params.EnemyParts[0].CurrentInitiative = 2;
	Params.EnemyParts[1].CurrentInitiative = 2;

	FFirstBattleSession Session;
	Session.Initialize(Params);

	Session.SubmitCommand(MakePlayCommand(StrikeCardInstanceId(), TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Both matching parts should trigger perfect release."), CountEvents(Snapshot, EFirstBattleEventType::PerfectReleaseTriggered), 2);
	TestNotNull(TEXT("Head perfect release event should exist."), FindEventForPart(Snapshot, EFirstBattleEventType::PerfectReleaseTriggered, TEXT("part.head")));
	TestNotNull(TEXT("Tail perfect release event should exist."), FindEventForPart(Snapshot, EFirstBattleEventType::PerfectReleaseTriggered, TEXT("part.tail")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelInitiativeZeroResolvesActionsInPositionOrderTest,
	"Final.Battle.First.Kernel.InitiativeZeroResolvesActionsInPositionOrder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelInitiativeZeroResolvesActionsInPositionOrderTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params = MakeStartParams();
	Params.EnemyParts[0].CurrentInitiative = 2;
	Params.EnemyParts[1].CurrentInitiative = 2;

	FFirstBattleSession Session;
	Session.Initialize(Params);

	Session.SubmitCommand(MakePlayCommand(StrikeCardInstanceId(), TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Both parts should act."), CountEvents(Snapshot, EFirstBattleEventType::EnemyPartActed), 2);

	TArray<FName> ActedParts;
	for (const FFirstBattleEvent& Event : Snapshot.RecentEvents)
	{
		if (Event.EventType == EFirstBattleEventType::EnemyPartActed)
		{
			ActedParts.Add(Event.PartId);
		}
	}
	TestEqual(TEXT("First acted part should be head by position."), ActedParts[0], FName(TEXT("part.head")));
	TestEqual(TEXT("Second acted part should be tail by position."), ActedParts[1], FName(TEXT("part.tail")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelEndTurnActsInPositionOrderAndAdvancesRoundTest,
	"Final.Battle.First.Kernel.EndTurnActsInPositionOrderAndAdvancesRound",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelEndTurnActsInPositionOrderAndAdvancesRoundTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleSession Session;
	Session.Initialize(MakeStartParams());

	const FFirstBattleCommandResult Result = Session.SubmitCommand(MakeEndTurnCommand());
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestTrue(TEXT("EndTurn should be accepted."), Result.IsAccepted());
	TestEqual(TEXT("EndTurn should advance round."), Snapshot.CurrentRound, 2);
	TestEqual(TEXT("Both parts should act at end turn."), CountEvents(Snapshot, EFirstBattleEventType::EnemyPartActed), 2);

	TArray<FName> ActedParts;
	for (const FFirstBattleEvent& Event : Snapshot.RecentEvents)
	{
		if (Event.EventType == EFirstBattleEventType::EnemyPartActed)
		{
			ActedParts.Add(Event.PartId);
		}
	}
	TestEqual(TEXT("Head should act first by position."), ActedParts[0], FName(TEXT("part.head")));
	TestEqual(TEXT("Tail should act second by position."), ActedParts[1], FName(TEXT("part.tail")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelEndTurnRefreshesIntentAndInitiativeTest,
	"Final.Battle.First.Kernel.EndTurnRefreshesIntentAndInitiative",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelEndTurnRefreshesIntentAndInitiativeTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params;
	Params.BattleId = TestBattleId();
	Params.StartingRound = 1;
	Params.EnemyParts.Add(MakeSequencedPart(
		TEXT("part.head"),
		0,
		{MakeIntent(TEXT("intent.test.attack"), 3), MakeIntent(TEXT("intent.test.bite"), 5)}));

	FFirstBattleSession Session;
	Session.Initialize(Params);

	Session.SubmitCommand(MakeEndTurnCommand());
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Part should refresh to next intent."), Snapshot.EnemyParts[0].CurrentIntentId, FName(TEXT("intent.test.bite")));
	TestEqual(TEXT("Part initiative should reset to next intent initial value."), Snapshot.EnemyParts[0].CurrentInitiative, 5);
	TestEqual(TEXT("Current intent index should advance."), Snapshot.EnemyParts[0].CurrentIntentIndex, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelEndTurnIntentDamageReducesPlayerHPTest,
	"Final.Battle.First.Kernel.EndTurnIntentDamageReducesPlayerHP",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelEndTurnIntentDamageReducesPlayerHPTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params;
	Params.BattleId = TestBattleId();
	Params.PlayerMaxHP = 20;
	Params.PlayerCurrentHP = 20;
	Params.EnemyParts.Add(MakeSequencedPart(
		TEXT("part.head"),
		0,
		{MakeIntent(TEXT("intent.test.attack"), 3, 6), MakeIntent(TEXT("intent.test.bite"), 5)}));

	FFirstBattleSession Session;
	Session.Initialize(Params);

	Session.SubmitCommand(MakeEndTurnCommand());
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Intent damage should reduce player HP."), Snapshot.PlayerCurrentHP, 14);
	TestEqual(TEXT("PlayerDamaged event should be appended."), CountEvents(Snapshot, EFirstBattleEventType::PlayerDamaged), 1);
	TestEqual(TEXT("Battle should continue after nonlethal damage."), Snapshot.bBattleEnded, false);
	TestEqual(TEXT("Nonlethal EndTurn should advance round."), Snapshot.CurrentRound, 2);
	TestEqual(TEXT("Part should still refresh intent after nonlethal damage."), Snapshot.EnemyParts[0].CurrentIntentId, FName(TEXT("intent.test.bite")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelSingleIntentLoopsAndResetsInitiativeTest,
	"Final.Battle.First.Kernel.SingleIntentLoopsAndResetsInitiative",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelSingleIntentLoopsAndResetsInitiativeTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params;
	Params.BattleId = TestBattleId();
	Params.EnemyParts.Add(MakePart(TEXT("part.head"), 0, 4, 10));

	FFirstBattleSession Session;
	Session.Initialize(Params);

	Session.SubmitCommand(MakeEndTurnCommand());
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Single-intent part should keep the same intent."), Snapshot.EnemyParts[0].CurrentIntentId, FName(TEXT("intent.test.attack")));
	TestEqual(TEXT("Single-intent part should reset to its initial initiative."), Snapshot.EnemyParts[0].CurrentInitiative, 4);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelPlayCardInitiativeActionRefreshesIntentTest,
	"Final.Battle.First.Kernel.PlayCardInitiativeActionRefreshesIntent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelPlayCardInitiativeActionRefreshesIntentTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params;
	Params.BattleId = TestBattleId();
	Params.InitialHand.Add(MakeCard(StrikeCardInstanceId(), TEXT("card.test.strike"), 2, 1));
	Params.EnemyParts.Add(MakeSequencedPart(
		TEXT("part.head"),
		0,
		{MakeIntent(TEXT("intent.test.attack"), 2), MakeIntent(TEXT("intent.test.bite"), 5)}));

	FFirstBattleSession Session;
	Session.Initialize(Params);

	Session.SubmitCommand(MakePlayCommand(StrikeCardInstanceId(), TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Initiative action should refresh to next intent."), Snapshot.EnemyParts[0].CurrentIntentId, FName(TEXT("intent.test.bite")));
	TestEqual(TEXT("Initiative action should reset next initiative."), Snapshot.EnemyParts[0].CurrentInitiative, 5);
	TestEqual(TEXT("Part should act once from initiative."), CountEvents(Snapshot, EFirstBattleEventType::EnemyPartActed), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelInitiativeActionExecutesIntentDamageTest,
	"Final.Battle.First.Kernel.InitiativeActionExecutesIntentDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelInitiativeActionExecutesIntentDamageTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params;
	Params.BattleId = TestBattleId();
	Params.PlayerMaxHP = 20;
	Params.PlayerCurrentHP = 20;
	Params.InitialHand.Add(MakeCard(StrikeCardInstanceId(), TEXT("card.test.strike"), 2, 1));
	Params.EnemyParts.Add(MakeSequencedPart(
		TEXT("part.head"),
		0,
		{MakeIntent(TEXT("intent.test.attack"), 2, 4), MakeIntent(TEXT("intent.test.bite"), 5)}));

	FFirstBattleSession Session;
	Session.Initialize(Params);

	Session.SubmitCommand(MakePlayCommand(StrikeCardInstanceId(), TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Initiative-triggered action should damage player."), Snapshot.PlayerCurrentHP, 16);
	TestEqual(TEXT("PlayerDamaged event should be appended."), CountEvents(Snapshot, EFirstBattleEventType::PlayerDamaged), 1);
	TestEqual(TEXT("Initiative action should still refresh intent after nonlethal damage."), Snapshot.EnemyParts[0].CurrentIntentId, FName(TEXT("intent.test.bite")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelPartCanActByInitiativeAndAgainAtEndTurnTest,
	"Final.Battle.First.Kernel.PartCanActByInitiativeAndAgainAtEndTurn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelPartCanActByInitiativeAndAgainAtEndTurnTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params;
	Params.BattleId = TestBattleId();
	Params.StartingRound = 1;
	Params.InitialHand.Add(MakeCard(StrikeCardInstanceId(), TEXT("card.test.strike"), 2, 1));
	Params.EnemyParts.Add(MakeSequencedPart(
		TEXT("part.head"),
		0,
		{MakeIntent(TEXT("intent.test.attack"), 2), MakeIntent(TEXT("intent.test.bite"), 5)}));

	FFirstBattleSession Session;
	Session.Initialize(Params);

	Session.SubmitCommand(MakePlayCommand(StrikeCardInstanceId(), TEXT("part.head")));
	Session.SubmitCommand(MakeEndTurnCommand());
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Part should act once by initiative and once at end turn."), CountEvents(Snapshot, EFirstBattleEventType::EnemyPartActed), 2);
	TestEqual(TEXT("End turn should advance round."), Snapshot.CurrentRound, 2);
	TestEqual(TEXT("Second action should cycle back to first intent."), Snapshot.EnemyParts[0].CurrentIntentId, FName(TEXT("intent.test.attack")));
	TestEqual(TEXT("Second action should reset initiative to first intent initial."), Snapshot.EnemyParts[0].CurrentInitiative, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelDestroyedPartsDoNotActAtEndTurnTest,
	"Final.Battle.First.Kernel.DestroyedPartsDoNotActAtEndTurn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelDestroyedPartsDoNotActAtEndTurnTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params;
	Params.BattleId = TestBattleId();
	Params.EnemyParts.Add(MakePart(TEXT("part.head"), 0, 3, 0));
	Params.EnemyParts.Add(MakePart(TEXT("part.tail"), 1, 4, 10));

	FFirstBattleSession Session;
	Session.Initialize(Params);

	Session.SubmitCommand(MakeEndTurnCommand());
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestTrue(TEXT("Head should start destroyed."), Snapshot.EnemyParts[0].bDestroyed);
	TestEqual(TEXT("Only living tail should act."), CountEvents(Snapshot, EFirstBattleEventType::EnemyPartActed), 1);
	TestNotNull(TEXT("Tail acted event should exist."), FindEventForPart(Snapshot, EFirstBattleEventType::EnemyPartActed, TEXT("part.tail")));
	TestTrue(TEXT("Destroyed head should not act."), FindEventForPart(Snapshot, EFirstBattleEventType::EnemyPartActed, TEXT("part.head")) == nullptr);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelPlayerDefeatStopsEndTurnActionsAndRoundAdvanceTest,
	"Final.Battle.First.Kernel.PlayerDefeatStopsEndTurnActionsAndRoundAdvance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelPlayerDefeatStopsEndTurnActionsAndRoundAdvanceTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params;
	Params.BattleId = TestBattleId();
	Params.PlayerMaxHP = 5;
	Params.PlayerCurrentHP = 5;
	Params.EnemyParts.Add(MakeSequencedPart(TEXT("part.head"), 0, {MakeIntent(TEXT("intent.test.attack"), 3, 5)}));
	Params.EnemyParts.Add(MakeSequencedPart(TEXT("part.tail"), 1, {MakeIntent(TEXT("intent.test.tail"), 3, 5)}));

	FFirstBattleSession Session;
	Session.Initialize(Params);

	Session.SubmitCommand(MakeEndTurnCommand());
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestTrue(TEXT("Battle should end after lethal player damage."), Snapshot.bBattleEnded);
	TestFalse(TEXT("Player should lose after lethal player damage."), Snapshot.bPlayerVictory);
	TestEqual(TEXT("Player HP should be zero."), Snapshot.PlayerCurrentHP, 0);
	TestEqual(TEXT("BattleLost event should be appended."), CountEvents(Snapshot, EFirstBattleEventType::BattleLost), 1);
	TestEqual(TEXT("Only first part should act before defeat stops queue."), CountEvents(Snapshot, EFirstBattleEventType::EnemyPartActed), 1);
	TestNotNull(TEXT("Head acted event should exist."), FindEventForPart(Snapshot, EFirstBattleEventType::EnemyPartActed, TEXT("part.head")));
	TestTrue(TEXT("Tail should not act after player defeat."), FindEventForPart(Snapshot, EFirstBattleEventType::EnemyPartActed, TEXT("part.tail")) == nullptr);
	TestEqual(TEXT("Defeating EndTurn should not advance round."), Snapshot.CurrentRound, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelPlayerDefeatStopsInitiativeActionQueueTest,
	"Final.Battle.First.Kernel.PlayerDefeatStopsInitiativeActionQueue",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelPlayerDefeatStopsInitiativeActionQueueTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params;
	Params.BattleId = TestBattleId();
	Params.PlayerMaxHP = 5;
	Params.PlayerCurrentHP = 5;
	Params.InitialHand.Add(MakeCard(StrikeCardInstanceId(), TEXT("card.test.strike"), 2, 1));
	Params.EnemyParts.Add(MakeSequencedPart(TEXT("part.head"), 0, {MakeIntent(TEXT("intent.test.attack"), 2, 5)}));
	Params.EnemyParts.Add(MakeSequencedPart(TEXT("part.tail"), 1, {MakeIntent(TEXT("intent.test.tail"), 2, 5)}));

	FFirstBattleSession Session;
	Session.Initialize(Params);

	Session.SubmitCommand(MakePlayCommand(StrikeCardInstanceId(), TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestTrue(TEXT("Battle should end after lethal initiative action."), Snapshot.bBattleEnded);
	TestFalse(TEXT("Player should lose after lethal initiative action."), Snapshot.bPlayerVictory);
	TestEqual(TEXT("Only first queued part should act before defeat stops queue."), CountEvents(Snapshot, EFirstBattleEventType::EnemyPartActed), 1);
	TestNotNull(TEXT("Head acted event should exist."), FindEventForPart(Snapshot, EFirstBattleEventType::EnemyPartActed, TEXT("part.head")));
	TestTrue(TEXT("Tail should not act after player defeat."), FindEventForPart(Snapshot, EFirstBattleEventType::EnemyPartActed, TEXT("part.tail")) == nullptr);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelDestroyedPartSkipsInitiativeReductionTest,
	"Final.Battle.First.Kernel.DestroyedPartSkipsInitiativeReduction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelDestroyedPartSkipsInitiativeReductionTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params = MakeStartParams();
	Params.InitialHand[0] = MakeCard(StrikeCardInstanceId(), TEXT("card.test.heavy"), 2, 10);

	FFirstBattleSession Session;
	Session.Initialize(Params);

	Session.SubmitCommand(MakePlayCommand(StrikeCardInstanceId(), TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestTrue(TEXT("Target part should be destroyed."), Snapshot.EnemyParts[0].bDestroyed);
	TestEqual(TEXT("Destroyed target initiative should not be reduced."), Snapshot.EnemyParts[0].CurrentInitiative, 3);
	TestEqual(TEXT("Only surviving tail should receive initiative changed event."), CountEvents(Snapshot, EFirstBattleEventType::InitiativeChanged), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelAllPartsDestroyedWinsBattleTest,
	"Final.Battle.First.Kernel.AllPartsDestroyedWinsBattle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelAllPartsDestroyedWinsBattleTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params;
	Params.BattleId = TestBattleId();
	Params.StartingRound = 1;
	Params.InitialHand.Add(MakeCard(StrikeCardInstanceId(), TEXT("card.test.finish"), 2, 10));
	Params.EnemyParts.Add(MakePart(TEXT("part.head"), 0, 3, 10));

	FFirstBattleSession Session;
	Session.Initialize(Params);

	Session.SubmitCommand(MakePlayCommand(StrikeCardInstanceId(), TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestTrue(TEXT("Battle should end."), Snapshot.bBattleEnded);
	TestTrue(TEXT("Player should win."), Snapshot.bPlayerVictory);
	TestEqual(TEXT("BattleWon event should be appended."), CountEvents(Snapshot, EFirstBattleEventType::BattleWon), 1);
	TestEqual(TEXT("Victory should skip initiative changes."), CountEvents(Snapshot, EFirstBattleEventType::InitiativeChanged), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelInvalidPlayCardRejectedWithoutMutationTest,
	"Final.Battle.First.Kernel.InvalidPlayCardRejectedWithoutMutation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelInvalidPlayCardRejectedWithoutMutationTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleSession Session;
	Session.Initialize(MakeStartParams());
	const FFirstBattleSnapshot Before = Session.GetSnapshot();

	const FFirstBattleCommandResult Result = Session.SubmitCommand(MakePlayCommand(FGuid::NewGuid(), TEXT("part.head")));
	const FFirstBattleSnapshot After = Session.GetSnapshot();

	TestFalse(TEXT("Invalid card should be rejected."), Result.IsAccepted());
	TestEqual(TEXT("Rejected command should not move cards."), After.HandCards.Num(), Before.HandCards.Num());
	TestEqual(TEXT("Rejected command should not discard cards."), After.DiscardPileCount, Before.DiscardPileCount);
	TestEqual(TEXT("Rejected command should not modify HP."), After.EnemyParts[0].CurrentHP, Before.EnemyParts[0].CurrentHP);
	TestEqual(TEXT("Rejected command should not append events."), After.RecentEvents.Num(), Before.RecentEvents.Num());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelDestroyedTargetRejectedWithoutMutationTest,
	"Final.Battle.First.Kernel.DestroyedTargetRejectedWithoutMutation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelDestroyedTargetRejectedWithoutMutationTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params = MakeStartParams();
	Params.InitialHand[0] = MakeCard(StrikeCardInstanceId(), TEXT("card.test.destroy"), 2, 10);

	FFirstBattleSession Session;
	Session.Initialize(Params);
	Session.SubmitCommand(MakePlayCommand(StrikeCardInstanceId(), TEXT("part.head")));
	const FFirstBattleSnapshot Before = Session.GetSnapshot();

	const FFirstBattleCommandResult Result = Session.SubmitCommand(MakePlayCommand(SwiftCardInstanceId(), TEXT("part.head")));
	const FFirstBattleSnapshot After = Session.GetSnapshot();

	TestFalse(TEXT("Destroyed target should be rejected."), Result.IsAccepted());
	TestEqual(TEXT("Rejected command should not move cards."), After.HandCards.Num(), Before.HandCards.Num());
	TestEqual(TEXT("Rejected command should not append events."), After.RecentEvents.Num(), Before.RecentEvents.Num());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelInvalidTargetRejectedWithoutMutationTest,
	"Final.Battle.First.Kernel.InvalidTargetRejectedWithoutMutation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelInvalidTargetRejectedWithoutMutationTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleSession Session;
	Session.Initialize(MakeStartParams());
	const FFirstBattleSnapshot Before = Session.GetSnapshot();

	const FFirstBattleCommandResult Result = Session.SubmitCommand(MakePlayCommand(StrikeCardInstanceId(), TEXT("part.missing")));
	const FFirstBattleSnapshot After = Session.GetSnapshot();

	TestFalse(TEXT("Invalid target should be rejected."), Result.IsAccepted());
	TestEqual(TEXT("Rejected command should not move cards."), After.HandCards.Num(), Before.HandCards.Num());
	TestEqual(TEXT("Rejected command should not discard cards."), After.DiscardPileCount, Before.DiscardPileCount);
	TestEqual(TEXT("Rejected command should not modify HP."), After.EnemyParts[0].CurrentHP, Before.EnemyParts[0].CurrentHP);
	TestEqual(TEXT("Rejected command should not append events."), After.RecentEvents.Num(), Before.RecentEvents.Num());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelBattleEndedPlayCardRejectedWithoutMutationTest,
	"Final.Battle.First.Kernel.BattleEndedPlayCardRejectedWithoutMutation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelBattleEndedPlayCardRejectedWithoutMutationTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params;
	Params.BattleId = TestBattleId();
	Params.InitialHand.Add(MakeCard(StrikeCardInstanceId(), TEXT("card.test.finish"), 2, 10));
	Params.InitialHand.Add(MakeSwiftCard());
	Params.EnemyParts.Add(MakePart(TEXT("part.head"), 0, 3, 10));

	FFirstBattleSession Session;
	Session.Initialize(Params);
	Session.SubmitCommand(MakePlayCommand(StrikeCardInstanceId(), TEXT("part.head")));
	const FFirstBattleSnapshot BeforeRejectedCommand = Session.GetSnapshot();

	const FFirstBattleCommandResult Result = Session.SubmitCommand(MakePlayCommand(SwiftCardInstanceId(), TEXT("part.head")));
	const FFirstBattleSnapshot After = Session.GetSnapshot();

	TestFalse(TEXT("PlayCard after battle end should be rejected."), Result.IsAccepted());
	TestEqual(TEXT("Rejected command should not move cards after battle end."), After.HandCards.Num(), BeforeRejectedCommand.HandCards.Num());
	TestEqual(TEXT("Rejected command should not append events after battle end."), After.RecentEvents.Num(), BeforeRejectedCommand.RecentEvents.Num());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelEndTurnAfterBattleEndRejectedWithoutMutationTest,
	"Final.Battle.First.Kernel.EndTurnAfterBattleEndRejectedWithoutMutation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelEndTurnAfterBattleEndRejectedWithoutMutationTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params;
	Params.BattleId = TestBattleId();
	Params.InitialHand.Add(MakeCard(StrikeCardInstanceId(), TEXT("card.test.finish"), 2, 10));
	Params.EnemyParts.Add(MakePart(TEXT("part.head"), 0, 3, 10));

	FFirstBattleSession Session;
	Session.Initialize(Params);
	Session.SubmitCommand(MakePlayCommand(StrikeCardInstanceId(), TEXT("part.head")));
	const FFirstBattleSnapshot Before = Session.GetSnapshot();

	const FFirstBattleCommandResult Result = Session.SubmitCommand(MakeEndTurnCommand());
	const FFirstBattleSnapshot After = Session.GetSnapshot();

	TestFalse(TEXT("EndTurn after battle end should be rejected."), Result.IsAccepted());
	TestEqual(TEXT("Rejected EndTurn should not advance round."), After.CurrentRound, Before.CurrentRound);
	TestEqual(TEXT("Rejected EndTurn should not append events."), After.RecentEvents.Num(), Before.RecentEvents.Num());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelUninitializedSessionReturnsSafeSnapshotTest,
	"Final.Battle.First.Kernel.UninitializedSessionReturnsSafeSnapshot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelUninitializedSessionReturnsSafeSnapshotTest::RunTest(const FString& Parameters)
{
	FFirstBattleSession Session;
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestFalse(TEXT("Uninitialized snapshot should be marked uninitialized."), Snapshot.bInitialized);
	TestEqual(TEXT("Uninitialized snapshot should have no hand cards."), Snapshot.HandCards.Num(), 0);
	TestEqual(TEXT("Uninitialized snapshot should have no enemy parts."), Snapshot.EnemyParts.Num(), 0);
	TestFalse(TEXT("Uninitialized snapshot should not be ended."), Snapshot.bBattleEnded);
	return true;
}

#endif
