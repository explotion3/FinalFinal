#include "FirstBattleKernelTestUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

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
