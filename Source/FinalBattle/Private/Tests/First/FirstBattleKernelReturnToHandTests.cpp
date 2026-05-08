#include "FirstBattleKernelTestUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelReturnToHandRandomZoneDoesNotEnterDiscardTest,
	"Final.Battle.First.Kernel.ReturnToHandRandomZone.DoesNotEnterDiscard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelReturnToHandRandomZoneDoesNotEnterDiscardTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	const FGuid ReturnCardId(0x33330001, 0x44440001, 0x55550001, 0x66660001);
	FFirstBattleStartParams Params = MakeHandZoneStartParams({
		MakeCard(DrawCardInstanceId(1), TEXT("card.test.left_zone"), 1, 1),
		MakeLeftHandCard(),
		MakeReturnToHandCard(ReturnCardId, TEXT("card.test.return"), 1, 7),
		MakeRightHandCard(),
		MakeCard(DrawCardInstanceId(2), TEXT("card.test.right_zone"), 1, 1),
	});
	Params.RandomSeed = 17;

	FFirstBattleSession Session;
	Session.Initialize(Params);

	const FFirstBattleCommandResult Result = Session.SubmitCommand(MakePlayCommand(ReturnCardId, TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestTrue(TEXT("Return destination card should be playable."), Result.IsAccepted());
	TestNotNull(TEXT("Returned card should be back in hand."), FindCardView(Snapshot, TEXT("card.test.return")));
	TestEqual(TEXT("Returned card should not remain in discard."), Snapshot.DiscardPileCount, 0);
	TestEqual(TEXT("CardReturnedToHand should be emitted."), CountEvents(Snapshot, EFirstBattleEventType::CardReturnedToHand), 1);
	TestEqual(TEXT("Card damage should still resolve."), Snapshot.EnemyParts[0].CurrentHP, 13);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelReturnToHandRandomZoneReprojectsHandZoneTest,
	"Final.Battle.First.Kernel.ReturnToHandRandomZone.ReprojectsHandZone",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelReturnToHandRandomZoneReprojectsHandZoneTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	const FGuid ReturnCardId(0x33330002, 0x44440002, 0x55550002, 0x66660002);
	FFirstBattleStartParams Params = MakeHandZoneStartParams({
		MakeLeftHandCard(),
		MakeReturnToHandCard(ReturnCardId, TEXT("card.test.return"), 1, 1),
		MakeRightHandCard(),
	});
	Params.RandomSeed = 23;

	FFirstBattleSession Session;
	Session.Initialize(Params);
	Session.SubmitCommand(MakePlayCommand(ReturnCardId, TEXT("part.head")));

	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();
	const FFirstCardViewData* ReturnedCard = FindCardView(Snapshot, TEXT("card.test.return"));
	if (!TestNotNull(TEXT("Returned card should be in hand."), ReturnedCard))
	{
		return false;
	}

	TestNotEqual(TEXT("Returned card should be projected into an active hand zone when anchors exist."), ReturnedCard->HandZone, EFirstHandZone::None);
	const FFirstBattleEvent* ReturnEvent = Snapshot.RecentEvents.FindByPredicate(
		[ReturnCardId](const FFirstBattleEvent& Event)
		{
			return Event.EventType == EFirstBattleEventType::CardReturnedToHand && Event.CardInstanceId == ReturnCardId;
		});
	TestNotNull(TEXT("Return event should record returned card."), ReturnEvent);
	TestNotEqual(TEXT("Return event should record a valid target zone."), static_cast<EFirstHandZone>(ReturnEvent->PrimaryValue), EFirstHandZone::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelReturnToHandRandomZoneDoesNotReapplyEntryHPTest,
	"Final.Battle.First.Kernel.ReturnToHandRandomZone.DoesNotReapplyEntryHP",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelReturnToHandRandomZoneDoesNotReapplyEntryHPTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	const FGuid ReturnCardId(0x33330003, 0x44440003, 0x55550003, 0x66660003);
	FFirstCardInstance ReturnCard = MakeReturnToHandCard(ReturnCardId, TEXT("card.test.return"), 1, 1);
	ReturnCard.PlayerMaxHPBonusOnEnterBattle = 6;

	FFirstBattleStartParams Params = MakeHandZoneStartParams({
		MakeLeftHandCard(),
		ReturnCard,
		MakeRightHandCard(),
	});
	Params.PlayerMaxHP = 20;
	Params.PlayerCurrentHP = 10;

	FFirstBattleSession Session;
	Session.Initialize(Params);
	FFirstBattleSnapshot Snapshot = Session.GetSnapshot();
	TestEqual(TEXT("Entry HP bonus applies once on initialization."), Snapshot.PlayerMaxHP, 26);
	TestEqual(TEXT("Current HP follows entry bonus once."), Snapshot.PlayerCurrentHP, 16);

	Session.SubmitCommand(MakePlayCommand(ReturnCardId, TEXT("part.head")));
	Snapshot = Session.GetSnapshot();
	TestEqual(TEXT("Returning to hand should not reapply max HP bonus."), Snapshot.PlayerMaxHP, 26);
	TestEqual(TEXT("Returning to hand should not reapply current HP bonus."), Snapshot.PlayerCurrentHP, 16);
	TestEqual(TEXT("Only initial entry HP event should exist."), CountEvents(Snapshot, EFirstBattleEventType::PlayerMaxHPChanged), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleKernelReturnToHandRandomZoneFallbackWithoutValidZoneTest,
	"Final.Battle.First.Kernel.ReturnToHandRandomZone.FallbackWithoutValidZone",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleKernelReturnToHandRandomZoneFallbackWithoutValidZoneTest::RunTest(const FString& Parameters)
{
	using namespace FirstBattleKernelTests;

	const FGuid ReturnCardId(0x33330004, 0x44440004, 0x55550004, 0x66660004);
	FFirstBattleStartParams Params = MakeHandZoneStartParams({
		MakeReturnToHandCard(ReturnCardId, TEXT("card.test.return"), 1, 1),
	});

	FFirstBattleSession Session;
	Session.Initialize(Params);
	const FFirstBattleCommandResult Result = Session.SubmitCommand(MakePlayCommand(ReturnCardId, TEXT("part.head")));
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestTrue(TEXT("Return should still succeed without active hand zones."), Result.IsAccepted());
	const FFirstCardViewData* ReturnedCard = FindCardView(Snapshot, TEXT("card.test.return"));
	if (!TestNotNull(TEXT("Returned card should fallback into hand."), ReturnedCard))
	{
		return false;
	}
	TestEqual(TEXT("Fallback returned card has no active hand zone."), ReturnedCard->HandZone, EFirstHandZone::None);
	TestEqual(TEXT("Returned card should not be discarded."), Snapshot.DiscardPileCount, 0);
	const FFirstBattleEvent* ReturnEvent = Snapshot.RecentEvents.FindByPredicate(
		[](const FFirstBattleEvent& Event)
		{
			return Event.EventType == EFirstBattleEventType::CardReturnedToHand;
		});
	TestNotNull(TEXT("Fallback return should still emit an event."), ReturnEvent);
	TestEqual(TEXT("Fallback event target zone is None."), static_cast<EFirstHandZone>(ReturnEvent->PrimaryValue), EFirstHandZone::None);
	return true;
}

#endif
