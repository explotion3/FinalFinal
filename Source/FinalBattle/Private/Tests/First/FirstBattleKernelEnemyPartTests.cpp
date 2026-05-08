#include "FirstBattleKernelTestUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

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


#endif
