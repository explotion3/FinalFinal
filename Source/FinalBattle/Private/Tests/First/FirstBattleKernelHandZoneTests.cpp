#include "FirstBattleKernelTestUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

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


#endif
