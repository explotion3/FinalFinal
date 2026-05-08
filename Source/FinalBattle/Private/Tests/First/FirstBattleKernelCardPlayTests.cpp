#include "FirstBattleKernelTestUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

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


#endif
