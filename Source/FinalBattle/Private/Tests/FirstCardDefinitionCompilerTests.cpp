#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "First/FirstCardDefinition.h"
#include "First/FirstCardDefinitionCompiler.h"
#include "First/FirstBattleSession.h"
#include "Engine/GameInstance.h"
#include "Queries/FinalDataRegistry.h"

namespace FirstCardDefinitionCompilerTests
{
	FGuid TestCardInstanceId()
	{
		return FGuid(0x12345678, 0x22222222, 0x33333333, 0x44444444);
	}

	UFirstCardDefinition* MakeFirstCardDefinition(const FName CardId = TEXT("first.card.test"))
	{
		UFirstCardDefinition* Definition = NewObject<UFirstCardDefinition>(GetTransientPackage());
		Definition->CardId = CardId;
		Definition->DisplayName = FText::FromString(TEXT("First Test Card"));
		Definition->BaseCost = 2;
		return Definition;
	}

	UFinalDataRegistry* MakeRegistry(UGameInstance*& OutGameInstance)
	{
		OutGameInstance = NewObject<UGameInstance>(GetTransientPackage());
		return NewObject<UFinalDataRegistry>(OutGameInstance);
	}

	FFirstCardInstance MakeRuntimeCard(const FName CardId)
	{
		FFirstCardInstance Card;
		Card.CardInstanceId = FGuid::NewGuid();
		Card.CardId = CardId;
		Card.DisplayName = FText::FromName(CardId);
		Card.BaseCost = 1;
		Card.RuntimeCost = 1;
		return Card;
	}

	int32 CountEvents(const FFirstBattleSnapshot& Snapshot, const EFirstBattleEventType EventType)
	{
		return Snapshot.RecentEvents.FilterByPredicate(
			[EventType](const FFirstBattleEvent& Event)
			{
				return Event.EventType == EventType;
			}).Num();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstCardDefinitionRegistryTest,
	"Final.Data.First.CardDefinition.RegisterAndFind",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCardDefinitionRegistryTest::RunTest(const FString& Parameters)
{
	using namespace FirstCardDefinitionCompilerTests;

	UGameInstance* GameInstance = nullptr;
	UFinalDataRegistry* Registry = MakeRegistry(GameInstance);
	UFirstCardDefinition* Definition = MakeFirstCardDefinition(TEXT("first.card.registry"));

	Registry->RegisterFirstCardDefinition(Definition);

	TestTrue(TEXT("FindFirstCardDefinition returns registered definition"), Registry->FindFirstCardDefinition(TEXT("first.card.registry")) == Definition);
	TestNull(TEXT("FindFirstCardDefinition returns null for unknown id"), Registry->FindFirstCardDefinition(TEXT("first.card.unknown")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleInitializeFromDefinitionsCompilesHandAndDrawPileTest,
	"Final.Battle.First.Session.InitializeFromDefinitions.CompilesHandAndDrawPile",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleInitializeFromDefinitionsCompilesHandAndDrawPileTest::RunTest(const FString& Parameters)
{
	using namespace FirstCardDefinitionCompilerTests;

	UGameInstance* GameInstance = nullptr;
	UFinalDataRegistry* Registry = MakeRegistry(GameInstance);
	Registry->RegisterFirstCardDefinition(MakeFirstCardDefinition(TEXT("first.card.hand")));
	Registry->RegisterFirstCardDefinition(MakeFirstCardDefinition(TEXT("first.card.draw")));

	FFirstBattleStartParams Params;
	Params.BattleId = FGuid(0x10101010, 0x20202020, 0x30303030, 0x40404040);
	Params.InitialHandCardDefinitions.Add({TEXT("first.card.hand"), 2});
	Params.InitialDrawPileCardDefinitions.Add({TEXT("first.card.draw"), 1});

	FFirstBattleSession Session;
	const FFirstBattleInitializeResult Result = Session.InitializeFromDefinitions(Params, *Registry);
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestTrue(TEXT("InitializeFromDefinitions succeeds"), Result.bSuccess);
	TestTrue(TEXT("Snapshot is initialized"), Snapshot.bInitialized);
	TestEqual(TEXT("Hand count comes from CardId + Count"), Snapshot.HandCards.Num(), 2);
	TestEqual(TEXT("Draw pile count comes from CardId + Count"), Snapshot.DrawPileCount, 1);
	TestEqual(TEXT("First hand CardId is compiled"), Snapshot.HandCards[0].CardId, FName(TEXT("first.card.hand")));
	TestNotEqual(TEXT("Compiled hand card instances are unique"), Snapshot.HandCards[0].CardInstanceId, Snapshot.HandCards[1].CardInstanceId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleInitializeFromDefinitionsAppliesEntryHPBonusesTest,
	"Final.Battle.First.Session.InitializeFromDefinitions.AppliesEntryHPBonuses",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleInitializeFromDefinitionsAppliesEntryHPBonusesTest::RunTest(const FString& Parameters)
{
	using namespace FirstCardDefinitionCompilerTests;

	UGameInstance* GameInstance = nullptr;
	UFinalDataRegistry* Registry = MakeRegistry(GameInstance);
	UFirstCardDefinition* HandDefinition = MakeFirstCardDefinition(TEXT("first.card.hand_bonus"));
	HandDefinition->PlayerMaxHPBonusOnEnterBattle = 2;
	UFirstCardDefinition* DrawDefinition = MakeFirstCardDefinition(TEXT("first.card.draw_bonus"));
	DrawDefinition->PlayerMaxHPBonusOnEnterBattle = 3;
	Registry->RegisterFirstCardDefinition(HandDefinition);
	Registry->RegisterFirstCardDefinition(DrawDefinition);

	FFirstBattleStartParams Params;
	Params.PlayerMaxHP = 20;
	Params.PlayerCurrentHP = 12;
	Params.InitialHandCardDefinitions.Add({TEXT("first.card.hand_bonus"), 1});
	Params.InitialDrawPileCardDefinitions.Add({TEXT("first.card.draw_bonus"), 1});

	FFirstBattleSession Session;
	const FFirstBattleInitializeResult Result = Session.InitializeFromDefinitions(Params, *Registry);
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestTrue(TEXT("InitializeFromDefinitions succeeds"), Result.bSuccess);
	TestEqual(TEXT("Entry HP bonuses should increase max HP."), Snapshot.PlayerMaxHP, 25);
	TestEqual(TEXT("Entry HP bonuses should increase current HP by the same total."), Snapshot.PlayerCurrentHP, 17);
	TestEqual(TEXT("One hand card should be visible."), Snapshot.HandCards.Num(), 1);
	TestEqual(TEXT("Card view preserves entry HP bonus."), Snapshot.HandCards[0].PlayerMaxHPBonusOnEnterBattle, 2);
	TestEqual(TEXT("Entry HP changes are event-visible per card."), CountEvents(Snapshot, EFirstBattleEventType::PlayerMaxHPChanged), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleInitializeFromDefinitionsPreservesRuntimeCardsTest,
	"Final.Battle.First.Session.InitializeFromDefinitions.PreservesRuntimeCards",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleInitializeFromDefinitionsPreservesRuntimeCardsTest::RunTest(const FString& Parameters)
{
	using namespace FirstCardDefinitionCompilerTests;

	UGameInstance* GameInstance = nullptr;
	UFinalDataRegistry* Registry = MakeRegistry(GameInstance);
	Registry->RegisterFirstCardDefinition(MakeFirstCardDefinition(TEXT("first.card.compiled")));

	FFirstBattleStartParams Params;
	FFirstCardInstance RuntimeHandCard = MakeRuntimeCard(TEXT("first.card.runtime.hand"));
	RuntimeHandCard.PlayerMaxHPBonusOnEnterBattle = 4;
	FFirstCardInstance RuntimeDrawCard = MakeRuntimeCard(TEXT("first.card.runtime.draw"));
	RuntimeDrawCard.PlayerMaxHPBonusOnEnterBattle = 2;
	Params.InitialHand.Add(RuntimeHandCard);
	Params.InitialDrawPile.Add(RuntimeDrawCard);
	Params.InitialHandCardDefinitions.Add({TEXT("first.card.compiled"), 1});
	Params.InitialDrawPileCardDefinitions.Add({TEXT("first.card.compiled"), 1});

	FFirstBattleSession Session;
	const FFirstBattleInitializeResult Result = Session.InitializeFromDefinitions(Params, *Registry);
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestTrue(TEXT("InitializeFromDefinitions succeeds"), Result.bSuccess);
	TestEqual(TEXT("Runtime hand cards are preserved and compiled hand cards appended"), Snapshot.HandCards.Num(), 2);
	TestEqual(TEXT("Runtime hand card remains first"), Snapshot.HandCards[0].CardId, FName(TEXT("first.card.runtime.hand")));
	TestEqual(TEXT("Runtime hand entry HP bonus is preserved."), Snapshot.HandCards[0].PlayerMaxHPBonusOnEnterBattle, 4);
	TestEqual(TEXT("Compiled hand card is appended"), Snapshot.HandCards[1].CardId, FName(TEXT("first.card.compiled")));
	TestEqual(TEXT("Runtime draw pile and compiled draw pile coexist"), Snapshot.DrawPileCount, 2);
	TestEqual(TEXT("Runtime hand/draw entry HP bonuses should apply."), Snapshot.PlayerMaxHP, 36);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleInitializeFromDefinitionsMissingCardFailsWithoutOverwriteTest,
	"Final.Battle.First.Session.InitializeFromDefinitions.MissingCardFailsWithoutOverwrite",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleInitializeFromDefinitionsMissingCardFailsWithoutOverwriteTest::RunTest(const FString& Parameters)
{
	using namespace FirstCardDefinitionCompilerTests;

	UGameInstance* GameInstance = nullptr;
	UFinalDataRegistry* Registry = MakeRegistry(GameInstance);

	FFirstBattleSession Session;
	FFirstBattleStartParams InitialParams;
	InitialParams.InitialHand.Add(MakeRuntimeCard(TEXT("first.card.existing")));
	Session.Initialize(InitialParams);

	FFirstBattleStartParams FailingParams;
	FailingParams.InitialHandCardDefinitions.Add({TEXT("first.card.missing"), 1});
	const FFirstBattleInitializeResult Result = Session.InitializeFromDefinitions(FailingParams, *Registry);
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestFalse(TEXT("Missing card fails"), Result.bSuccess);
	TestEqual(TEXT("Missing card id is reported"), Result.MissingCardIds.Num(), 1);
	TestEqual(TEXT("Existing initialized state is not overwritten"), Snapshot.HandCards.Num(), 1);
	TestEqual(TEXT("Existing card remains"), Snapshot.HandCards[0].CardId, FName(TEXT("first.card.existing")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleInitializeFromDefinitionsInvalidCountFailsTest,
	"Final.Battle.First.Session.InitializeFromDefinitions.InvalidCountFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleInitializeFromDefinitionsInvalidCountFailsTest::RunTest(const FString& Parameters)
{
	using namespace FirstCardDefinitionCompilerTests;

	UGameInstance* GameInstance = nullptr;
	UFinalDataRegistry* Registry = MakeRegistry(GameInstance);
	Registry->RegisterFirstCardDefinition(MakeFirstCardDefinition(TEXT("first.card.invalid_count")));

	FFirstBattleStartParams Params;
	Params.InitialHandCardDefinitions.Add({TEXT("first.card.invalid_count"), 0});

	FFirstBattleSession Session;
	const FFirstBattleInitializeResult Result = Session.InitializeFromDefinitions(Params, *Registry);
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestFalse(TEXT("Invalid count fails"), Result.bSuccess);
	TestEqual(TEXT("Invalid card id is reported"), Result.InvalidCardIds.Num(), 1);
	TestFalse(TEXT("Session remains uninitialized"), Snapshot.bInitialized);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstBattleInitializeFromDefinitionsPreservesCompiledFieldsTest,
	"Final.Battle.First.Session.InitializeFromDefinitions.PreservesCompiledFields",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstBattleInitializeFromDefinitionsPreservesCompiledFieldsTest::RunTest(const FString& Parameters)
{
	using namespace FirstCardDefinitionCompilerTests;

	UFirstCardDefinition* Definition = MakeFirstCardDefinition(TEXT("first.card.full"));
	Definition->PlayerMaxHPBonusOnEnterBattle = 5;
	Definition->PlayDestination = EFirstCardDefinitionPlayDestination::ReturnToHandRandomZone;
	Definition->Keywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("First.Keyword.Swift")));
	Definition->HandRole = EFirstCardDefinitionHandRole::RightHandCore;
	Definition->bRequiresHandZoneToPlay = true;
	Definition->RequiredHandZone = EFirstCardDefinitionHandZone::Right;
	Definition->bSkipInitiativeReductionOnPerfectReleaseInZone = true;
	Definition->PerfectReleaseInitiativeSkipZone = EFirstCardDefinitionHandZone::Both;

	FFirstCardDefinitionEffect& MoveEffect = Definition->Effects.AddDefaulted_GetRef();
	MoveEffect.EffectType = EFirstCardDefinitionEffectType::MoveHandCard;
	MoveEffect.MoveTargetPolicy = EFirstCardDefinitionHandMoveTargetPolicy::FixedZone;
	MoveEffect.MoveTargetZone = EFirstCardDefinitionHandZone::Left;
	MoveEffect.MoveTargetCostDelta = -1;
	MoveEffect.bTransferActualCostReductionToSourceCard = true;

	UGameInstance* GameInstance = nullptr;
	UFinalDataRegistry* Registry = MakeRegistry(GameInstance);
	Registry->RegisterFirstCardDefinition(Definition);

	FFirstBattleStartParams Params;
	Params.InitialHandCardDefinitions.Add({TEXT("first.card.full"), 1});

	FFirstBattleSession Session;
	const FFirstBattleInitializeResult Result = Session.InitializeFromDefinitions(Params, *Registry);
	const FFirstBattleSnapshot Snapshot = Session.GetSnapshot();

	TestTrue(TEXT("InitializeFromDefinitions succeeds"), Result.bSuccess);
	TestEqual(TEXT("One compiled hand card"), Snapshot.HandCards.Num(), 1);
	const FFirstCardViewData& Card = Snapshot.HandCards[0];
	TestEqual(TEXT("HandRole preserved"), Card.HandRole, EFirstHandRole::RightHandCore);
	TestEqual(TEXT("RuntimeCost starts from BaseCost"), Card.RuntimeCost, 2);
	TestEqual(TEXT("Entry HP bonus preserved"), Card.PlayerMaxHPBonusOnEnterBattle, 5);
	TestEqual(TEXT("Play destination preserved"), Card.PlayDestination, EFirstCardPlayDestination::ReturnToHandRandomZone);
	TestTrue(TEXT("Keyword preserved"), Card.Keywords.HasTagExact(FGameplayTag::RequestGameplayTag(TEXT("First.Keyword.Swift"))));
	TestEqual(TEXT("Move effect preserved"), Card.Effects.Num(), 1);
	TestEqual(TEXT("Move target zone preserved"), Card.Effects[0].MoveTargetZone, EFirstHandZone::Left);
	TestTrue(TEXT("Cost transfer flag preserved"), Card.Effects[0].bTransferActualCostReductionToSourceCard);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstCardDefinitionCompilerBasicTest,
	"Final.Battle.First.CardDefinition.Compiler.BasicFields",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCardDefinitionCompilerBasicTest::RunTest(const FString& Parameters)
{
	using namespace FirstCardDefinitionCompilerTests;

	UFirstCardDefinition* Definition = MakeFirstCardDefinition(TEXT("first.card.basic"));
	Definition->PlayerMaxHPBonusOnEnterBattle = 6;
	Definition->PlayDestination = EFirstCardDefinitionPlayDestination::ReturnToHandRandomZone;
	Definition->Keywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("First.Keyword.Swift")));
	Definition->HandRole = EFirstCardDefinitionHandRole::LeftHandCore;
	Definition->bRequiresHandZoneToPlay = true;
	Definition->RequiredHandZone = EFirstCardDefinitionHandZone::Both;
	Definition->bSkipInitiativeReductionOnPerfectReleaseInZone = true;
	Definition->PerfectReleaseInitiativeSkipZone = EFirstCardDefinitionHandZone::Left;

	const FFirstCardInstance RuntimeCard = FFirstCardDefinitionCompiler::CompileCardDefinition(Definition, TestCardInstanceId());

	TestEqual(TEXT("CardInstanceId is assigned"), RuntimeCard.CardInstanceId, TestCardInstanceId());
	TestEqual(TEXT("CardId is copied"), RuntimeCard.CardId, FName(TEXT("first.card.basic")));
	TestEqual(TEXT("BaseCost is copied"), RuntimeCard.BaseCost, 2);
	TestEqual(TEXT("RuntimeCost starts from BaseCost"), RuntimeCard.RuntimeCost, 2);
	TestEqual(TEXT("Entry HP bonus is copied"), RuntimeCard.PlayerMaxHPBonusOnEnterBattle, 6);
	TestEqual(TEXT("PlayDestination is compiled"), RuntimeCard.PlayDestination, EFirstCardPlayDestination::ReturnToHandRandomZone);
	TestEqual(TEXT("HandRole is compiled"), RuntimeCard.HandRole, EFirstHandRole::LeftHandCore);
	TestTrue(TEXT("Swift keyword is copied"), RuntimeCard.Keywords.HasTagExact(FGameplayTag::RequestGameplayTag(TEXT("First.Keyword.Swift"))));
	TestTrue(TEXT("Zone requirement flag is copied"), RuntimeCard.bRequiresHandZoneToPlay);
	TestEqual(TEXT("RequiredHandZone is compiled"), RuntimeCard.RequiredHandZone, EFirstHandZone::Both);
	TestTrue(TEXT("Perfect release skip flag is copied"), RuntimeCard.bSkipInitiativeReductionOnPerfectReleaseInZone);
	TestEqual(TEXT("Perfect release skip zone is compiled"), RuntimeCard.PerfectReleaseInitiativeSkipZone, EFirstHandZone::Left);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstCardDefinitionCompilerEffectsTest,
	"Final.Battle.First.CardDefinition.Compiler.Effects",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCardDefinitionCompilerEffectsTest::RunTest(const FString& Parameters)
{
	using namespace FirstCardDefinitionCompilerTests;

	UFirstCardDefinition* Definition = MakeFirstCardDefinition(TEXT("first.card.effects"));

	FFirstCardDefinitionEffect& DamageEffect = Definition->Effects.AddDefaulted_GetRef();
	DamageEffect.EffectId = TEXT("effect.damage.main");
	DamageEffect.EffectType = EFirstCardDefinitionEffectType::Damage;
	DamageEffect.Value = 8;

	FFirstCardDefinitionEffect& MoveEffect = Definition->Effects.AddDefaulted_GetRef();
	MoveEffect.EffectId = TEXT("effect.move.cost_transfer");
	MoveEffect.EffectType = EFirstCardDefinitionEffectType::MoveHandCard;
	MoveEffect.MoveCardCount = 1;
	MoveEffect.bMoveRequiresSourceZone = true;
	MoveEffect.MoveSourceZone = EFirstCardDefinitionHandZone::Both;
	MoveEffect.MoveTargetPolicy = EFirstCardDefinitionHandMoveTargetPolicy::FixedZone;
	MoveEffect.MoveTargetZone = EFirstCardDefinitionHandZone::Left;
	MoveEffect.MoveTargetCostDelta = -1;
	MoveEffect.bTransferActualCostReductionToSourceCard = true;

	const FFirstCardInstance RuntimeCard = FFirstCardDefinitionCompiler::CompileCardDefinition(Definition, TestCardInstanceId());

	TestEqual(TEXT("Effects are compiled"), RuntimeCard.Effects.Num(), 2);
	TestEqual(TEXT("Damage effect type is compiled"), RuntimeCard.Effects[0].EffectType, EFirstCardEffectType::Damage);
	TestEqual(TEXT("Damage value is copied"), RuntimeCard.Effects[0].Value, 8);
	TestEqual(TEXT("Move effect type is compiled"), RuntimeCard.Effects[1].EffectType, EFirstCardEffectType::MoveHandCard);
	TestEqual(TEXT("Move source zone is compiled"), RuntimeCard.Effects[1].MoveSourceZone, EFirstHandZone::Both);
	TestEqual(TEXT("Move target policy is compiled"), RuntimeCard.Effects[1].MoveTargetPolicy, EFirstHandMoveTargetPolicy::FixedZone);
	TestEqual(TEXT("Move target zone is compiled"), RuntimeCard.Effects[1].MoveTargetZone, EFirstHandZone::Left);
	TestEqual(TEXT("Move target cost delta is copied"), RuntimeCard.Effects[1].MoveTargetCostDelta, -1);
	TestTrue(TEXT("Cost transfer flag is copied"), RuntimeCard.Effects[1].bTransferActualCostReductionToSourceCard);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
