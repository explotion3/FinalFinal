#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "First/FirstBattleCommand.h"
#include "First/FirstBattleSession.h"
#include "First/FirstCardDefinitionCompiler.h"
#include "First/FirstCardDefinition.h"
#include "Misc/PackageName.h"
#include "Queries/FinalDataRegistry.h"
#include "UObject/StrongObjectPtr.h"

namespace FirstCardContentBootstrapTests
{
	const FString CardsPath(TEXT("/Game/Prototype/FirstProject/Cards"));

	const FString LeftHandCardPath = CardsPath / TEXT("DA_FirstCard_Core_LeftHand");
	const FString RightHandCardPath = CardsPath / TEXT("DA_FirstCard_Core_RightHand");
	const FString ChaoGuangMuDieCardPath = CardsPath / TEXT("DA_FirstCard_Insect_ChaoGuangMuDie");
	const FString ChiFuGongYiCardPath = CardsPath / TEXT("DA_FirstCard_Insect_ChiFuGongYi");
	const FString ShuoGuangDieCardPath = CardsPath / TEXT("DA_FirstCard_Insect_ShuoGuangDie");

	const FName LeftHandCardId(TEXT("first.card.core.left_hand"));
	const FName RightHandCardId(TEXT("first.card.core.right_hand"));
	const FName ChaoGuangMuDieCardId(TEXT("first.card.insect.chao_guang_mu_die"));
	const FName ChiFuGongYiCardId(TEXT("first.card.insect.chi_fu_gong_yi"));
	const FName ShuoGuangDieCardId(TEXT("first.card.insect.shuo_guang_die"));

	UFirstCardDefinition* LoadFirstCardDefinition(const FString& PackagePath)
	{
		const FString AssetName = FPackageName::GetLongPackageAssetName(PackagePath);
		const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackagePath, *AssetName);
		return LoadObject<UFirstCardDefinition>(nullptr, *ObjectPath);
	}

	FFirstEnemyPartStartData MakeTargetPart()
	{
		FFirstEnemyPartStartData Part;
		Part.PartId = TEXT("first.enemy_part.test.head");
		Part.DisplayName = FText::FromString(TEXT("Test Head"));
		Part.PositionIndex = 0;
		Part.MaxHP = 20;
		Part.CurrentHP = 20;
		Part.CurrentIntentId = TEXT("first.intent.test.idle");
		Part.CurrentIntentDisplayName = FText::FromString(TEXT("Idle"));
		Part.CurrentInitiative = 9;
		Part.IntentSequence.Add({Part.CurrentIntentId, Part.CurrentIntentDisplayName, Part.CurrentInitiative, {}});
		return Part;
	}

	FFirstBattleCommand MakePlayCommand(const FGuid& CardInstanceId)
	{
		FFirstBattleCommand Command;
		Command.CommandType = EFirstBattleCommandType::PlayCard;
		Command.CardInstanceId = CardInstanceId;
		Command.TargetPartId = TEXT("first.enemy_part.test.head");
		return Command;
	}

	struct FAutomationContext
	{
		~FAutomationContext()
		{
			Shutdown();
		}

		bool Initialize(FAutomationTestBase& Test, const TCHAR* ContextName)
		{
			if (GameInstance.IsValid())
			{
				return true;
			}

			if (GEngine == nullptr)
			{
				Test.AddError(TEXT("GEngine is unavailable; cannot create a standalone automation GameInstance."));
				return false;
			}

			UGameInstance* RawGameInstance = NewObject<UGameInstance>(GEngine, UGameInstance::StaticClass());
			if (RawGameInstance == nullptr)
			{
				Test.AddError(TEXT("Failed to allocate a transient UGameInstance."));
				return false;
			}

			GameInstance.Reset(RawGameInstance);
			RawGameInstance->InitializeStandalone(FName(*FString::Printf(TEXT("/Temp/%s"), ContextName)));

			DataRegistry = RawGameInstance->GetSubsystem<UFinalDataRegistry>();
			return Test.TestNotNull(TEXT("FinalDataRegistry should initialize."), DataRegistry);
		}

		void Shutdown()
		{
			if (!GameInstance.IsValid())
			{
				return;
			}

			if (UGameInstance* RawGameInstance = GameInstance.Get())
			{
				UWorld* World = RawGameInstance->GetWorld();
				RawGameInstance->Shutdown();

				if (World != nullptr)
				{
					if (GEngine != nullptr)
					{
						GEngine->DestroyWorldContext(World);
					}
					World->DestroyWorld(false);
				}
			}

			DataRegistry = nullptr;
			GameInstance.Reset();
			CollectGarbage(RF_NoFlags);
		}

		TStrongObjectPtr<UGameInstance> GameInstance;
		UFinalDataRegistry* DataRegistry = nullptr;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstCardContentBootstrapAssetsLoadTest,
	"Final.Editor.FirstContentBootstrap.AssetsLoad",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCardContentBootstrapAssetsLoadTest::RunTest(const FString& Parameters)
{
	using namespace FirstCardContentBootstrapTests;

	TestNotNull(TEXT("Left hand First card asset should load."), LoadFirstCardDefinition(LeftHandCardPath));
	TestNotNull(TEXT("Right hand First card asset should load."), LoadFirstCardDefinition(RightHandCardPath));
	TestNotNull(TEXT("Chao Guang Mu Die First card asset should load."), LoadFirstCardDefinition(ChaoGuangMuDieCardPath));
	TestNotNull(TEXT("Chi Fu Gong Yi First card asset should load."), LoadFirstCardDefinition(ChiFuGongYiCardPath));
	TestNotNull(TEXT("Shuo Guang Die First card asset should load."), LoadFirstCardDefinition(ShuoGuangDieCardPath));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstCardContentBootstrapRegistryDiscoveryTest,
	"Final.Editor.FirstContentBootstrap.RegistryDiscovery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCardContentBootstrapRegistryDiscoveryTest::RunTest(const FString& Parameters)
{
	using namespace FirstCardContentBootstrapTests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FirstCardContentBootstrapRegistryDiscovery")))
	{
		return false;
	}

	TestNotNull(TEXT("Left hand should be discoverable by First CardId."), Context.DataRegistry->FindFirstCardDefinition(LeftHandCardId));
	TestNotNull(TEXT("Right hand should be discoverable by First CardId."), Context.DataRegistry->FindFirstCardDefinition(RightHandCardId));
	TestNotNull(TEXT("Chao Guang Mu Die should be discoverable by First CardId."), Context.DataRegistry->FindFirstCardDefinition(ChaoGuangMuDieCardId));
	TestNotNull(TEXT("Chi Fu Gong Yi should be discoverable by First CardId."), Context.DataRegistry->FindFirstCardDefinition(ChiFuGongYiCardId));
	TestNotNull(TEXT("Shuo Guang Die should be discoverable by First CardId."), Context.DataRegistry->FindFirstCardDefinition(ShuoGuangDieCardId));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstCardContentBootstrapCompiledFieldsTest,
	"Final.Editor.FirstContentBootstrap.CompiledFields",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCardContentBootstrapCompiledFieldsTest::RunTest(const FString& Parameters)
{
	using namespace FirstCardContentBootstrapTests;

	const UFirstCardDefinition* RightHandDefinition = LoadFirstCardDefinition(RightHandCardPath);
	const UFirstCardDefinition* ChaoGuangMuDieDefinition = LoadFirstCardDefinition(ChaoGuangMuDieCardPath);
	const UFirstCardDefinition* ChiFuGongYiDefinition = LoadFirstCardDefinition(ChiFuGongYiCardPath);
	const UFirstCardDefinition* ShuoGuangDieDefinition = LoadFirstCardDefinition(ShuoGuangDieCardPath);

	if (!TestNotNull(TEXT("Right hand definition should load."), RightHandDefinition)
		|| !TestNotNull(TEXT("Chao Guang Mu Die definition should load."), ChaoGuangMuDieDefinition)
		|| !TestNotNull(TEXT("Chi Fu Gong Yi definition should load."), ChiFuGongYiDefinition)
		|| !TestNotNull(TEXT("Shuo Guang Die definition should load."), ShuoGuangDieDefinition))
	{
		return false;
	}

	const FFirstCardInstance RightHand = FFirstCardDefinitionCompiler::CompileCardDefinition(RightHandDefinition);
	TestEqual(TEXT("Right hand keeps right core role."), RightHand.HandRole, EFirstHandRole::RightHandCore);
	TestEqual(TEXT("Right hand has one effect."), RightHand.Effects.Num(), 1);
	TestEqual(TEXT("Right hand effect is damage."), RightHand.Effects[0].EffectType, EFirstCardEffectType::Damage);
	TestEqual(TEXT("Right hand damage is 8."), RightHand.Effects[0].Value, 8);

	const FFirstCardInstance ChaoGuangMuDie = FFirstCardDefinitionCompiler::CompileCardDefinition(ChaoGuangMuDieDefinition);
	TestEqual(TEXT("Chao Guang Mu Die grants +1 entry max HP."), ChaoGuangMuDie.PlayerMaxHPBonusOnEnterBattle, 1);
	TestEqual(TEXT("Chao Guang Mu Die has one effect."), ChaoGuangMuDie.Effects.Num(), 1);
	TestEqual(TEXT("Chao Guang Mu Die effect is hand move."), ChaoGuangMuDie.Effects[0].EffectType, EFirstCardEffectType::MoveHandCard);
	TestEqual(TEXT("Chao Guang Mu Die target cost delta is -1."), ChaoGuangMuDie.Effects[0].MoveTargetCostDelta, -1);
	TestTrue(TEXT("Chao Guang Mu Die transfers actual reduction to source."), ChaoGuangMuDie.Effects[0].bTransferActualCostReductionToSourceCard);

	const FFirstCardInstance ChiFuGongYi = FFirstCardDefinitionCompiler::CompileCardDefinition(ChiFuGongYiDefinition);
	TestEqual(TEXT("Chi Fu Gong Yi grants +1 entry max HP."), ChiFuGongYi.PlayerMaxHPBonusOnEnterBattle, 1);
	TestTrue(TEXT("Chi Fu Gong Yi requires a hand zone."), ChiFuGongYi.bRequiresHandZoneToPlay);
	TestEqual(TEXT("Chi Fu Gong Yi requires Both."), ChiFuGongYi.RequiredHandZone, EFirstHandZone::Both);
	TestEqual(TEXT("Chi Fu Gong Yi move source is Both."), ChiFuGongYi.Effects[0].MoveSourceZone, EFirstHandZone::Both);

	const FFirstCardInstance ShuoGuangDie = FFirstCardDefinitionCompiler::CompileCardDefinition(ShuoGuangDieDefinition);
	TestEqual(TEXT("Shuo Guang Die grants +6 entry max HP."), ShuoGuangDie.PlayerMaxHPBonusOnEnterBattle, 6);
	TestEqual(TEXT("Shuo Guang Die returns to hand after play."), ShuoGuangDie.PlayDestination, EFirstCardPlayDestination::ReturnToHandRandomZone);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstCardContentBootstrapInitializeAndPlayTest,
	"Final.Editor.FirstContentBootstrap.InitializeFromDefinitionsAndPlay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCardContentBootstrapInitializeAndPlayTest::RunTest(const FString& Parameters)
{
	using namespace FirstCardContentBootstrapTests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FirstCardContentBootstrapInitializeAndPlay")))
	{
		return false;
	}

	FFirstBattleStartParams Params;
	Params.BattleId = FGuid(0x11111111, 0x22222222, 0x33333333, 0x44444444);
	Params.InitialHandCardDefinitions.Add({RightHandCardId, 1});
	Params.InitialDrawPileCardDefinitions.Add({ChaoGuangMuDieCardId, 1});
	Params.EnemyParts.Add(MakeTargetPart());

	FFirstBattleSession Session;
	const FFirstBattleInitializeResult InitResult = Session.InitializeFromDefinitions(Params, *Context.DataRegistry);
	if (!TestTrue(TEXT("InitializeFromDefinitions should succeed with bootstrap cards."), InitResult.bSuccess))
	{
		return false;
	}

	FFirstBattleSnapshot Snapshot = Session.GetSnapshot();
	TestEqual(TEXT("One hand card should be initialized."), Snapshot.HandCards.Num(), 1);
	TestEqual(TEXT("One draw pile card should be initialized."), Snapshot.DrawPileCount, 1);
	TestEqual(TEXT("Hand card should be right hand."), Snapshot.HandCards[0].CardId, RightHandCardId);
	TestEqual(TEXT("Draw pile entry HP bonus should apply on initialization."), Snapshot.PlayerMaxHP, 31);
	TestEqual(TEXT("Draw pile entry HP bonus should increase current HP."), Snapshot.PlayerCurrentHP, 31);

	const FFirstBattleCommandResult PlayResult = Session.SubmitCommand(MakePlayCommand(Snapshot.HandCards[0].CardInstanceId));
	TestTrue(TEXT("Playing right hand should be accepted."), PlayResult.IsAccepted());

	Snapshot = Session.GetSnapshot();
	TestEqual(TEXT("Right hand should deal 8 damage to the target part."), Snapshot.EnemyParts[0].CurrentHP, 12);
	TestEqual(TEXT("Played card should move to discard."), Snapshot.DiscardPileCount, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFirstCardContentBootstrapShuoGuangDieReturnsToHandTest,
	"Final.Editor.FirstContentBootstrap.ShuoGuangDieReturnsToHand",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFirstCardContentBootstrapShuoGuangDieReturnsToHandTest::RunTest(const FString& Parameters)
{
	using namespace FirstCardContentBootstrapTests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FirstCardContentBootstrapShuoGuangDieReturnsToHand")))
	{
		return false;
	}

	FFirstBattleStartParams Params;
	Params.BattleId = FGuid(0x22222222, 0x33333333, 0x44444444, 0x55555555);
	Params.RandomSeed = 19;
	Params.InitialHandCardDefinitions.Add({LeftHandCardId, 1});
	Params.InitialHandCardDefinitions.Add({ShuoGuangDieCardId, 1});
	Params.InitialHandCardDefinitions.Add({RightHandCardId, 1});
	Params.EnemyParts.Add(MakeTargetPart());

	FFirstBattleSession Session;
	const FFirstBattleInitializeResult InitResult = Session.InitializeFromDefinitions(Params, *Context.DataRegistry);
	if (!TestTrue(TEXT("InitializeFromDefinitions should succeed with Shuo Guang Die."), InitResult.bSuccess))
	{
		return false;
	}

	FFirstBattleSnapshot Snapshot = Session.GetSnapshot();
	const FFirstCardViewData* ShuoGuangDie = Snapshot.HandCards.FindByPredicate(
		[](const FFirstCardViewData& Card)
		{
			return Card.CardId == ShuoGuangDieCardId;
		});
	if (!TestNotNull(TEXT("Shuo Guang Die should start in hand."), ShuoGuangDie))
	{
		return false;
	}

	const FFirstBattleCommandResult PlayResult = Session.SubmitCommand(MakePlayCommand(ShuoGuangDie->CardInstanceId));
	TestTrue(TEXT("Playing Shuo Guang Die should be accepted."), PlayResult.IsAccepted());

	Snapshot = Session.GetSnapshot();
	TestEqual(TEXT("Shuo Guang Die should deal 7 damage to the target part."), Snapshot.EnemyParts[0].CurrentHP, 13);
	TestEqual(TEXT("Shuo Guang Die should not enter discard."), Snapshot.DiscardPileCount, 0);
	TestNotNull(TEXT("Shuo Guang Die should return to hand."), Snapshot.HandCards.FindByPredicate(
		[](const FFirstCardViewData& Card)
		{
			return Card.CardId == ShuoGuangDieCardId;
		}));
	TestEqual(TEXT("CardReturnedToHand event should be emitted."), Snapshot.RecentEvents.FilterByPredicate(
		[](const FFirstBattleEvent& Event)
		{
			return Event.EventType == EFirstBattleEventType::CardReturnedToHand;
		}).Num(), 1);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
