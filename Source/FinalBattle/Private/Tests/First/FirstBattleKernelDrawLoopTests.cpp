#include "FirstBattleKernelTestUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

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


#endif
