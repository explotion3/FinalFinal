#include "FirstBattleKernelTestUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

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


#endif
