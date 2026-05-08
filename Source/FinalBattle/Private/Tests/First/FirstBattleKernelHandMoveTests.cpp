#include "FirstBattleKernelTestUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

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
	FFirstBattleKernelMoveHandCardCostTransferReducesTargetAndTransfersToSourceTest,
	"Final.Battle.First.Kernel.MoveHandCardCostTransferReducesTargetAndTransfersToSource",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelMoveHandCardCostTransferReducesTargetAndTransfersToSourceTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	const FGuid SourceCardId(0x95000001, 0, 0, 0);
	const FGuid TargetCardId(0x95000002, 0, 0, 0);
	FFirstBattleStartParams Params = MakeHandZoneStartParams({
		MakeMoveHandCard(SourceCardId, TEXT("card.test.cost_transfer"), EFirstHandMoveTargetPolicy::FixedZone, EFirstHandZone::Right, true, EFirstHandZone::Both, -1, true, 1),
		MakeLeftHandCard(),
		MakeCard(TargetCardId, TEXT("card.test.target"), 3, 0),
		MakeRightHandCard()
	});

	FFirstBattleSession Session;
	Session.Initialize(Params);
	Session.SubmitCommand(MakePlayCommand(SourceCardId, TEXT("part.head")));
	const FFirstBattleSnapshot AfterPlay = Session.GetSnapshot();

	const FFirstCardViewData* TargetCard = FindCardView(AfterPlay, TEXT("card.test.target"));
	TestNotNull(TEXT("Moved target should remain in hand."), TargetCard);
	if (TargetCard != nullptr)
	{
		TestEqual(TEXT("Moved target should be reduced by actual cost delta."), TargetCard->RuntimeCost, 2);
	}

	const FFirstBattleEvent* TargetCostEvent = FindCostChangedEventForCard(AfterPlay, TargetCardId);
	TestNotNull(TEXT("Target cost change should be visible."), TargetCostEvent);
	if (TargetCostEvent != nullptr)
	{
		TestEqual(TEXT("Target cost event should record previous cost."), TargetCostEvent->PrimaryValue, 3);
		TestEqual(TEXT("Target cost event should record new cost."), TargetCostEvent->SecondaryValue, 2);
	}

	const FFirstBattleEvent* SourceCostEvent = FindCostChangedEventForCard(AfterPlay, SourceCardId);
	TestNotNull(TEXT("Source cost transfer should be visible."), SourceCostEvent);
	if (SourceCostEvent != nullptr)
	{
		TestEqual(TEXT("Source cost event should record previous cost."), SourceCostEvent->PrimaryValue, 1);
		TestEqual(TEXT("Source cost event should record transferred cost."), SourceCostEvent->SecondaryValue, 2);
	}

	Session.SubmitCommand(MakeEndTurnCommand());
	const FFirstBattleSnapshot AfterDraw = Session.GetSnapshot();
	const FFirstCardViewData* SourceCard = FindCardView(AfterDraw, TEXT("card.test.cost_transfer"));
	TestNotNull(TEXT("Source card should return from discard through draw loop."), SourceCard);
	if (SourceCard != nullptr)
	{
		TestEqual(TEXT("Source card should preserve transferred runtime cost after returning to hand."), SourceCard->RuntimeCost, 2);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelMoveHandCardCostTransferUsesActualReductionTest,
	"Final.Battle.First.Kernel.MoveHandCardCostTransferUsesActualReduction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelMoveHandCardCostTransferUsesActualReductionTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	const FGuid SourceCardId(0x95000003, 0, 0, 0);
	const FGuid TargetCardId(0x95000004, 0, 0, 0);
	FFirstBattleStartParams Params = MakeHandZoneStartParams({
		MakeMoveHandCard(SourceCardId, TEXT("card.test.cost_transfer"), EFirstHandMoveTargetPolicy::FixedZone, EFirstHandZone::Right, true, EFirstHandZone::Both, -1, true, 1),
		MakeLeftHandCard(),
		MakeCard(TargetCardId, TEXT("card.test.zero_cost_target"), 0, 0),
		MakeRightHandCard()
	});

	FFirstBattleSession Session;
	Session.Initialize(Params);
	Session.SubmitCommand(MakePlayCommand(SourceCardId, TEXT("part.head")));
	const FFirstBattleSnapshot AfterPlay = Session.GetSnapshot();

	const FFirstCardViewData* TargetCard = FindCardView(AfterPlay, TEXT("card.test.zero_cost_target"));
	TestNotNull(TEXT("Zero-cost target should remain in hand."), TargetCard);
	if (TargetCard != nullptr)
	{
		TestEqual(TEXT("Zero-cost target should not go below zero."), TargetCard->RuntimeCost, 0);
	}
	TestEqual(TEXT("No actual reduction should emit no cost events."), CountEvents(AfterPlay, EFirstBattleEventType::CardRuntimeCostChanged), 0);

	Session.SubmitCommand(MakeEndTurnCommand());
	const FFirstBattleSnapshot AfterDraw = Session.GetSnapshot();
	const FFirstCardViewData* SourceCard = FindCardView(AfterDraw, TEXT("card.test.cost_transfer"));
	TestNotNull(TEXT("Source card should return to hand."), SourceCard);
	if (SourceCard != nullptr)
	{
		TestEqual(TEXT("Source card should not gain cost when target did not actually reduce."), SourceCard->RuntimeCost, 1);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelMoveHandCardCostTransferNoOpsWithoutMoveTest,
	"Final.Battle.First.Kernel.MoveHandCardCostTransferNoOpsWithoutMove",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelMoveHandCardCostTransferNoOpsWithoutMoveTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	const FGuid SourceCardId(0x95000005, 0, 0, 0);
	FFirstBattleStartParams Params = MakeHandZoneStartParams({
		MakeMoveHandCard(SourceCardId, TEXT("card.test.cost_transfer"), EFirstHandMoveTargetPolicy::FixedZone, EFirstHandZone::Both, false, EFirstHandZone::None, -1, true, 1),
		MakeLeftHandCard(),
		MakeCard(FGuid(0x95000006, 0, 0, 0), TEXT("card.test.left_candidate"), 3, 0)
	});

	FFirstBattleSession Session;
	Session.Initialize(Params);
	Session.SubmitCommand(MakePlayCommand(SourceCardId, TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Missing target zone should produce no move event."), CountEvents(Snapshot, EFirstBattleEventType::HandCardMoved), 0);
	TestEqual(TEXT("Missing target zone should produce no cost event."), CountEvents(Snapshot, EFirstBattleEventType::CardRuntimeCostChanged), 0);
	const FFirstCardViewData* Candidate = FindCardView(Snapshot, TEXT("card.test.left_candidate"));
	TestNotNull(TEXT("Candidate should remain in hand."), Candidate);
	if (Candidate != nullptr)
	{
		TestEqual(TEXT("Candidate cost should remain unchanged."), Candidate->RuntimeCost, 3);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelMoveHandCardCostTransferDoesNotAffectCurrentPlayCostTest,
	"Final.Battle.First.Kernel.MoveHandCardCostTransferDoesNotAffectCurrentPlayCost",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelMoveHandCardCostTransferDoesNotAffectCurrentPlayCostTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	const FGuid SourceCardId(0x95000007, 0, 0, 0);
	FFirstBattleStartParams Params = MakeHandZoneStartParams({
		MakeMoveHandCard(SourceCardId, TEXT("card.test.cost_transfer"), EFirstHandMoveTargetPolicy::FixedZone, EFirstHandZone::Right, true, EFirstHandZone::Both, -1, true, 1),
		MakeLeftHandCard(),
		MakeCard(FGuid(0x95000008, 0, 0, 0), TEXT("card.test.target"), 3, 0),
		MakeRightHandCard()
	});
	Params.EnemyParts[0].CurrentInitiative = 5;

	FFirstBattleSession Session;
	Session.Initialize(Params);
	Session.SubmitCommand(MakePlayCommand(SourceCardId, TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Current play should reduce initiative by source cost before transfer."), Snapshot.EnemyParts[0].CurrentInitiative, 4);
	const FFirstBattleEvent* PlayedEvent = Snapshot.RecentEvents.FindByPredicate(
		[SourceCardId](const FFirstBattleEvent& Event)
		{
			return Event.EventType == EFirstBattleEventType::CardPlayed && Event.CardInstanceId == SourceCardId;
		});
	TestNotNull(TEXT("CardPlayed event should exist."), PlayedEvent);
	if (PlayedEvent != nullptr)
	{
		TestEqual(TEXT("CardPlayed should record pre-transfer runtime cost."), PlayedEvent->PrimaryValue, 1);
	}
	return true;
}


#endif
