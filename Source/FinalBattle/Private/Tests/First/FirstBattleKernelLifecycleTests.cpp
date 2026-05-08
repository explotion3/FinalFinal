#include "FirstBattleKernelTestUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

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
	FFirstBattleKernelInitializeAppliesEntryHPBonusesTest,
	"Final.Battle.First.Kernel.InitializeAppliesEntryHPBonuses",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelInitializeAppliesEntryHPBonusesTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstCardInstance HandBonusCard = MakeCard(FGuid(0x70000001, 0, 0, 0), TEXT("card.test.hand_hp_bonus"), 1, 1);
	HandBonusCard.PlayerMaxHPBonusOnEnterBattle = 2;
	FFirstCardInstance DrawBonusCard = MakeCard(FGuid(0x70000002, 0, 0, 0), TEXT("card.test.draw_hp_bonus"), 1, 1);
	DrawBonusCard.PlayerMaxHPBonusOnEnterBattle = 3;

	FFirstBattleStartParams Params;
	Params.BattleId = TestBattleId();
	Params.PlayerMaxHP = 20;
	Params.PlayerCurrentHP = 11;
	Params.InitialHand.Add(HandBonusCard);
	Params.InitialDrawPile.Add(DrawBonusCard);
	Params.EnemyParts.Add(MakePart(TEXT("part.head"), 0, 9, 20));

	FFirstBattleSession Session;
	Session.Initialize(Params);
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestEqual(TEXT("Entry HP bonuses should increase player max HP."), Snapshot.PlayerMaxHP, 25);
	TestEqual(TEXT("Entry HP bonuses should increase current HP by the same total."), Snapshot.PlayerCurrentHP, 16);
	TestEqual(TEXT("Card view should expose entry HP bonus."), Snapshot.HandCards[0].PlayerMaxHPBonusOnEnterBattle, 2);
	TestEqual(TEXT("Entry HP bonus events should be emitted per card."), CountEvents(Snapshot, EFirstBattleEventType::PlayerMaxHPChanged), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelEntryHPBonusDoesNotRepeatOnDrawTest,
	"Final.Battle.First.Kernel.EntryHPBonusDoesNotRepeatOnDraw",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelEntryHPBonusDoesNotRepeatOnDrawTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	FFirstBattleStartParams Params = MakeDrawStartParams();
	FFirstCardInstance DrawBonusCard = MakeDrawCard(1);
	DrawBonusCard.PlayerMaxHPBonusOnEnterBattle = 3;
	Params.InitialDrawPile.Add(DrawBonusCard);

	FFirstBattleSession Session;
	Session.Initialize(Params);
	FFirstBattleSnapshot Snapshot = Session.GetSnapshot();
	TestEqual(TEXT("Entry HP bonus applies at initialization."), Snapshot.PlayerMaxHP, 33);
	TestEqual(TEXT("Entry HP bonus event applies once at initialization."), CountEvents(Snapshot, EFirstBattleEventType::PlayerMaxHPChanged), 1);

	Session.SubmitCommand(MakeEndTurnCommand());
	Snapshot = Session.GetSnapshot();
	TestEqual(TEXT("Drawing the same card should not apply entry HP again."), Snapshot.PlayerMaxHP, 33);
	TestEqual(TEXT("No additional entry HP event should be emitted on draw."), CountEvents(Snapshot, EFirstBattleEventType::PlayerMaxHPChanged), 1);
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


#endif
