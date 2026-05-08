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

	FFirstCardInstance MakeCard(const FGuid& CardInstanceId, const FName CardId, const int32 RuntimeCost = 2, const int32 Damage = 7)
	{
		FFirstCardInstance Card;
		Card.CardInstanceId = CardInstanceId;
		Card.CardId = CardId;
		Card.DisplayName = FText::FromName(CardId);
		Card.BaseCost = RuntimeCost;
		Card.RuntimeCost = RuntimeCost;

		FFirstCardEffectInstance& Effect = Card.Effects.AddDefaulted_GetRef();
		Effect.EffectId = TEXT("effect.damage.main");
		Effect.EffectType = EFirstCardEffectType::Damage;
		Effect.Value = Damage;
		return Card;
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

	FFirstEnemyPartIntentInstance MakeIntent(const FName IntentId, const int32 InitialInitiative)
	{
		FFirstEnemyPartIntentInstance Intent;
		Intent.IntentId = IntentId;
		Intent.DisplayName = FText::FromName(IntentId);
		Intent.InitialInitiative = InitialInitiative;
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
