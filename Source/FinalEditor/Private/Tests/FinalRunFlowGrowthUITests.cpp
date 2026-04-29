#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "Facade/FinalRunSession.h"
#include "Queries/FinalDataRegistry.h"
#include "Requests/FinalBattleResult.h"
#include "Run/Definitions/FinalCharacterGrowthConfig.h"
#include "Run/Definitions/FinalPrototypeBootstrapDefinition.h"
#include "Subsystems/FinalBattleFlowSubsystem.h"
#include "Subsystems/FinalGameFlowSubsystem.h"
#include "Subsystems/FinalRunFlowSubsystem.h"
#include "UI/Screens/Flow/FinalRunGrowthChoiceOverlayScreen.h"
#include "UObject/StrongObjectPtr.h"

namespace FinalRunFlowGrowthUITests
{
	const FName StarterBootstrapId(TEXT("prototype.bootstrap.starter.chapter1"));

	FFinalRunPersistentCharacterState BuildInitialRunCharacterState(
		const FFinalPrototypeBootstrapCharacterState& BootstrapCharacterState,
		const UFinalCharacterGrowthConfig* GrowthConfig)
	{
		FFinalRunPersistentCharacterState CharacterState;
		CharacterState.CharacterId = BootstrapCharacterState.CharacterId;
		CharacterState.Level = FMath::Max(BootstrapCharacterState.Level, 1);
		CharacterState.BreakthroughValue = FMath::Max(BootstrapCharacterState.BreakthroughValue, 0);
		CharacterState.BreakthroughRequiredValue = BootstrapCharacterState.BreakthroughRequiredValue > 0
			? BootstrapCharacterState.BreakthroughRequiredValue
			: (GrowthConfig != nullptr && GrowthConfig->BaseBreakthroughRequiredValue > 0 ? GrowthConfig->BaseBreakthroughRequiredValue : 100);
		CharacterState.RootBone = FMath::Max(BootstrapCharacterState.RootBone, 0);
		CharacterState.Insight = FMath::Max(BootstrapCharacterState.Insight, 0);
		CharacterState.KillingIntent = FMath::Max(BootstrapCharacterState.KillingIntent, 0);
		CharacterState.CurrentStress = BootstrapCharacterState.CurrentStress;
		CharacterState.bCollapsed = BootstrapCharacterState.bCollapsed;
		CharacterState.CurrentAwakenCount = BootstrapCharacterState.CurrentAwakenCount;
		CharacterState.CollapseCount = BootstrapCharacterState.CollapseCount;
		return CharacterState;
	}

	bool ResolvePendingGrowthChoiceIfPresent(UFinalRunSession& RunSession)
	{
		const FFinalRunPendingGrowthChoice& PendingGrowthChoice = RunSession.GetPendingGrowthChoice();
		if (!PendingGrowthChoice.bIsValid || PendingGrowthChoice.Choices.IsEmpty())
		{
			return true;
		}

		FFinalRunCommand Command;
		Command.CommandType = EFinalRunCommandType::SelectGrowthChoice;
		Command.PayloadId = PendingGrowthChoice.Choices[0].ChoiceInstanceId;
		return RunSession.SubmitRunCommand(Command);
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

			const FName WorldName(*FString::Printf(TEXT("/Temp/%s"), ContextName));
			UGameInstance* RawGameInstance = NewObject<UGameInstance>(GEngine, UGameInstance::StaticClass());
			if (RawGameInstance == nullptr)
			{
				Test.AddError(TEXT("Failed to allocate a transient UGameInstance for run flow growth UI automation."));
				return false;
			}

			GameInstance.Reset(RawGameInstance);
			RawGameInstance->InitializeStandalone(WorldName);

			DataRegistry = RawGameInstance->GetSubsystem<UFinalDataRegistry>();
			GameFlowSubsystem = RawGameInstance->GetSubsystem<UFinalGameFlowSubsystem>();
			BattleFlowSubsystem = RawGameInstance->GetSubsystem<UFinalBattleFlowSubsystem>();
			RunFlowSubsystem = RawGameInstance->GetSubsystem<UFinalRunFlowSubsystem>();

			return Test.TestNotNull(TEXT("FinalDataRegistry should initialize in standalone automation context."), DataRegistry)
				&& Test.TestNotNull(TEXT("FinalGameFlowSubsystem should initialize in standalone automation context."), GameFlowSubsystem)
				&& Test.TestNotNull(TEXT("FinalBattleFlowSubsystem should initialize in standalone automation context."), BattleFlowSubsystem)
				&& Test.TestNotNull(TEXT("FinalRunFlowSubsystem should initialize in standalone automation context."), RunFlowSubsystem);
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

			RunFlowSubsystem = nullptr;
			BattleFlowSubsystem = nullptr;
			GameFlowSubsystem = nullptr;
			DataRegistry = nullptr;
			GameInstance.Reset();
			CollectGarbage(RF_NoFlags);
		}

		UFinalCardDefinition* RegisterCardDefinition(
			const FFinalCardId& CardId,
			const FFinalCharacterId& OwnerCharacterId,
			const FString& DisplayName) const
		{
			UFinalCardDefinition* CardDefinition = NewObject<UFinalCardDefinition>(GameInstance.Get());
			CardDefinition->CardId = CardId;
			CardDefinition->OwnerUnitId = OwnerCharacterId.Value;
			CardDefinition->DisplayName = FText::FromString(DisplayName);
			DataRegistry->RegisterCardDefinition(CardDefinition);
			return CardDefinition;
		}

		UFinalRunSession* CreateRunSessionWithSingleCharacter(
			const FFinalCharacterId& CharacterId,
			const FFinalCardId& StarterCardId) const
		{
			UFinalRunSession* RunSession = GameFlowSubsystem->BootstrapNewRun();
			if (RunSession == nullptr)
			{
				return nullptr;
			}

			FFinalRunPersistentCharacterState CharacterState;
			CharacterState.CharacterId = CharacterId;
			CharacterState.Level = 1;
			CharacterState.BreakthroughValue = 0;
			CharacterState.BreakthroughRequiredValue = 100;

			TArray<FFinalRunPersistentCharacterState> PartyStates;
			PartyStates.Add(CharacterState);

			TArray<FFinalCardId> StarterDeck;
			StarterDeck.Add(StarterCardId);

			RunSession->ConfigureBattleStartState(
				FFinalEncounterId(FName(TEXT("encounter.test"))),
				FFinalRuleConfigId(FName(TEXT("rules.test"))),
				PartyStates,
				StarterDeck,
				20);

			return RunSession;
		}

		UFinalRunSession* BootstrapStarterRun(FAutomationTestBase& Test, FFinalCharacterId& OutCharacterId) const
		{
			UFinalPrototypeBootstrapDefinition* BootstrapDefinition = DataRegistry->FindPrototypeBootstrapDefinition(StarterBootstrapId);
			if (!Test.TestNotNull(TEXT("Starter bootstrap definition must be discoverable through FinalDataRegistry."), BootstrapDefinition))
			{
				return nullptr;
			}

			UFinalRunSession* RunSession = GameFlowSubsystem->BootstrapNewRun();
			if (!Test.TestNotNull(TEXT("Game flow should create a starter RunSession."), RunSession))
			{
				return nullptr;
			}

			TArray<FFinalRunPersistentCharacterState> PartyStates;
			PartyStates.Reserve(BootstrapDefinition->InitialCharacterStates.Num());
			for (const FFinalPrototypeBootstrapCharacterState& BootstrapCharacterState : BootstrapDefinition->InitialCharacterStates)
			{
				const UFinalCharacterDefinition* CharacterDefinition = DataRegistry->FindCharacterDefinition(BootstrapCharacterState.CharacterId);
				const UFinalCharacterGrowthConfig* GrowthConfig =
					(CharacterDefinition != nullptr && CharacterDefinition->GrowthConfigId.IsValid())
						? DataRegistry->FindCharacterGrowthConfig(CharacterDefinition->GrowthConfigId)
						: nullptr;
				PartyStates.Add(BuildInitialRunCharacterState(BootstrapCharacterState, GrowthConfig));
			}

			RunSession->ConfigureBattleStartState(
				BootstrapDefinition->EncounterId,
				BootstrapDefinition->RuleConfigId,
				PartyStates,
				BootstrapDefinition->StarterDeckCardIds,
				BootstrapDefinition->InitialTeamCurrentHP);
			if (!Test.TestTrue(TEXT("Starter RunSession should expose a valid battle start state after bootstrap configuration."), RunSession->HasValidBattleStartState()))
			{
				return nullptr;
			}

			if (!Test.TestTrue(TEXT("Starter RunSession should resolve the configured route."), RunSession->ConfigureRunRouteById(BootstrapDefinition->RunRouteId)))
			{
				return nullptr;
			}

			OutCharacterId = BootstrapDefinition->InitialCharacterStates.Num() > 0
				? BootstrapDefinition->InitialCharacterStates[0].CharacterId
				: FFinalCharacterId{};
			return RunSession;
		}

		FFinalBattleResult BuildSyntheticVictoryResult(const UFinalRunSession& RunSession) const
		{
			const FFinalRunState RunState = RunSession.GetRunState();
			const FFinalBattleSnapshot BattleSnapshot = BattleFlowSubsystem->GetCurrentSnapshot();

			FFinalBattleResult Result;
			Result.EncounterId = RunState.CurrentEncounterId;
			Result.Outcome = EFinalBattleOutcome::Victory;
			Result.TeamCurrentHP = BattleSnapshot.TeamCurrentHP > 0 ? BattleSnapshot.TeamCurrentHP : 1;
			Result.RewardGold = 15;
			Result.UpdatedCharacterStates = RunState.Characters;
			return Result;
		}

		TStrongObjectPtr<UGameInstance> GameInstance;
		UFinalDataRegistry* DataRegistry = nullptr;
		UFinalGameFlowSubsystem* GameFlowSubsystem = nullptr;
		UFinalBattleFlowSubsystem* BattleFlowSubsystem = nullptr;
		UFinalRunFlowSubsystem* RunFlowSubsystem = nullptr;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalStarterBootstrapInitialGrowthOverlayTest,
	"Final.Editor.RunFlow.GrowthStarterBootstrapShowsInitialOverlay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalStarterBootstrapInitialGrowthOverlayTest::RunTest(const FString& Parameters)
{
	using namespace FinalRunFlowGrowthUITests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalStarterBootstrapInitialGrowthOverlayTest")))
	{
		return false;
	}

	FFinalCharacterId CharacterId;
	UFinalRunSession* RunSession = Context.BootstrapStarterRun(*this, CharacterId);
	if (RunSession == nullptr)
	{
		return false;
	}

	Context.RunFlowSubsystem->HandleRunSessionChanged();
	Context.RunFlowSubsystem->RefreshRunFlow(true);

	const FFinalRunSnapshot Snapshot = Context.RunFlowSubsystem->GetCurrentRunSnapshot();
	TestTrue(TEXT("Starter bootstrap should now create an initial pending growth choice."), Snapshot.PendingGrowthChoice.bHasPendingChoice);
	TestEqual(TEXT("Starter bootstrap should present GrowthChoice before the prepared battle auto-starts."), Context.RunFlowSubsystem->GetPresentedOverlay(), EFinalRunPresentedOverlay::GrowthChoice);
	TestEqual(TEXT("Starter bootstrap initial growth choice should target the first configured starter character."), Snapshot.PendingGrowthChoice.CharacterId, CharacterId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalRunFlowGrowthOverlayPriorityTest,
	"Final.Editor.RunFlow.GrowthOverlayTakesPriorityOverPendingBattleReward",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalRunFlowGrowthOverlayPriorityTest::RunTest(const FString& Parameters)
{
	using namespace FinalRunFlowGrowthUITests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalRunFlowGrowthOverlayPriorityTest")))
	{
		return false;
	}

	FFinalCharacterId CharacterId;
	UFinalRunSession* RunSession = Context.BootstrapStarterRun(*this, CharacterId);
	if (RunSession == nullptr || !TestTrue(TEXT("Starter bootstrap should provide a valid first character id."), CharacterId.IsValid()))
	{
		return false;
	}

	TestTrue(TEXT("Starter bootstrap initial pending growth choice should be resolvable before starting battle."), ResolvePendingGrowthChoiceIfPresent(*RunSession));

	if (!TestNotNull(TEXT("Starter run should start a battle from the configured route entry."), Context.GameFlowSubsystem->StartBattleFromRunSession()))
	{
		return false;
	}

	TestTrue(TEXT("Starter battle result should apply back to the run."), Context.GameFlowSubsystem->CompleteBattleAndApplyResult(Context.BuildSyntheticVictoryResult(*RunSession)));

	const FFinalRunSnapshot PostBattleSnapshot = RunSession->GetSnapshot();
	if (!TestTrue(TEXT("This priority test expects the starter opening battle to surface a pending reward."), PostBattleSnapshot.PendingBattleReward.bHasPendingReward))
	{
		return false;
	}

	TestTrue(TEXT("Breakthrough gain should still create a pending growth choice while a battle reward is waiting."), RunSession->AddBreakthroughValue(CharacterId, 100));

	Context.RunFlowSubsystem->HandleRunSessionChanged();
	Context.RunFlowSubsystem->RefreshRunFlow(true);

	const FFinalRunSnapshot Snapshot = Context.RunFlowSubsystem->GetCurrentRunSnapshot();
	TestTrue(TEXT("Snapshot should expose the pending growth choice alongside the pending reward."), Snapshot.PendingGrowthChoice.bHasPendingChoice);
	TestTrue(TEXT("Snapshot should still retain the pending battle reward."), Snapshot.PendingBattleReward.bHasPendingReward);
	TestEqual(TEXT("Growth overlay should take priority over the normal run flow overlay when both are pending."), Context.RunFlowSubsystem->GetPresentedOverlay(), EFinalRunPresentedOverlay::GrowthChoice);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalRunFlowGrowthCommandPresentationTest,
	"Final.Editor.RunFlow.GrowthSelectionUpdatesPresentation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalRunFlowGrowthCommandPresentationTest::RunTest(const FString& Parameters)
{
	using namespace FinalRunFlowGrowthUITests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalRunFlowGrowthCommandPresentationTest")))
	{
		return false;
	}

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.huo")));
	const FFinalCardId StarterCardId(FName(TEXT("card.test.base")));
	Context.RegisterCardDefinition(StarterCardId, CharacterId, TEXT("Starter Card"));

	UFinalRunSession* RunSession = Context.CreateRunSessionWithSingleCharacter(CharacterId, StarterCardId);
	if (!TestNotNull(TEXT("RunSession should be created for run flow growth presentation test."), RunSession))
	{
		return false;
	}

	TestTrue(TEXT("Breakthrough gain should create a pending growth choice."), RunSession->AddBreakthroughValue(CharacterId, 100));

	Context.RunFlowSubsystem->HandleRunSessionChanged();
	Context.RunFlowSubsystem->RefreshRunFlow(true);

	const FFinalRunSnapshot PendingSnapshot = Context.RunFlowSubsystem->GetCurrentRunSnapshot();
	TestTrue(TEXT("Pending snapshot should expose a growth choice."), PendingSnapshot.PendingGrowthChoice.bHasPendingChoice);
	TestEqual(TEXT("Run flow should present the dedicated growth overlay while a choice is pending."), Context.RunFlowSubsystem->GetPresentedOverlay(), EFinalRunPresentedOverlay::GrowthChoice);
	TestTrue(TEXT("Pending-growth flow message should mention growth selection."), Context.RunFlowSubsystem->GetLastFlowMessage().ToString().Contains(TEXT("成长")));

	const FName ChoiceInstanceId = PendingSnapshot.PendingGrowthChoice.Choices[0].ChoiceInstanceId;
	TestFalse(TEXT("A valid pending growth choice should expose a concrete choice id."), ChoiceInstanceId.IsNone());
	TestTrue(TEXT("Selecting the pending growth choice through RunFlowSubsystem should succeed."), Context.RunFlowSubsystem->SelectGrowthChoice(ChoiceInstanceId));

	const FFinalRunSnapshot ResolvedSnapshot = Context.RunFlowSubsystem->GetCurrentRunSnapshot();
	TestFalse(TEXT("Resolved snapshot should clear the pending growth choice."), ResolvedSnapshot.PendingGrowthChoice.bHasPendingChoice);
	TestFalse(TEXT("Presented overlay should move away from GrowthChoice after a selection is applied."), Context.RunFlowSubsystem->GetPresentedOverlay() == EFinalRunPresentedOverlay::GrowthChoice);
	TestEqual(TEXT("Last processed event should be GrowthChoiceApplied after a successful selection."), Context.RunFlowSubsystem->GetLastProcessedRunEvent().EventType, EFinalRunEventType::GrowthChoiceApplied);
	TestEqual(TEXT("Last processed event should report the selected choice id."), Context.RunFlowSubsystem->GetLastProcessedRunEvent().PayloadId, ChoiceInstanceId);
	TestTrue(TEXT("Post-apply flow message should remain populated for UI feedback."), !Context.RunFlowSubsystem->GetLastFlowMessage().IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalRunGrowthOverlayWidgetSelectionTest,
	"Final.Editor.RunFlow.GrowthOverlayWidgetDefaultsAndSubmitsSelection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalRunGrowthOverlayWidgetSelectionTest::RunTest(const FString& Parameters)
{
	using namespace FinalRunFlowGrowthUITests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalRunGrowthOverlayWidgetSelectionTest")))
	{
		return false;
	}

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.huo")));
	const FFinalCardId StarterCardId(FName(TEXT("card.test.base")));
	Context.RegisterCardDefinition(StarterCardId, CharacterId, TEXT("Starter Card"));

	UFinalRunSession* RunSession = Context.CreateRunSessionWithSingleCharacter(CharacterId, StarterCardId);
	if (!TestNotNull(TEXT("RunSession should be created for growth overlay widget test."), RunSession))
	{
		return false;
	}

	TestTrue(TEXT("Breakthrough gain should create a pending growth choice for the growth overlay widget test."), RunSession->AddBreakthroughValue(CharacterId, 100));

	Context.RunFlowSubsystem->HandleRunSessionChanged();
	Context.RunFlowSubsystem->RefreshRunFlow(true);

	const FFinalRunSnapshot PendingSnapshot = Context.RunFlowSubsystem->GetCurrentRunSnapshot();
	if (!TestTrue(TEXT("Pending snapshot should expose at least three growth choices."), PendingSnapshot.PendingGrowthChoice.Choices.Num() >= 3))
	{
		return false;
	}

	UFinalRunGrowthChoiceOverlayScreen* GrowthScreen = CreateWidget<UFinalRunGrowthChoiceOverlayScreen>(Context.GameInstance.Get(), UFinalRunGrowthChoiceOverlayScreen::StaticClass());
	if (!TestNotNull(TEXT("Growth overlay screen should be constructible in automation context."), GrowthScreen))
	{
		return false;
	}

	GrowthScreen->ConfigureFromRunSnapshot(PendingSnapshot);
	TestEqual(TEXT("Growth overlay should default to the first growth choice."), GrowthScreen->GetSelectedChoiceIndex(), 0);
	TestEqual(TEXT("Growth overlay should default to the first choice instance id."), GrowthScreen->GetSelectedChoiceInstanceId(), PendingSnapshot.PendingGrowthChoice.Choices[0].ChoiceInstanceId);

	TestTrue(TEXT("Selecting the second growth option through the widget should succeed."), GrowthScreen->SelectChoiceByIndex(1));
	TestEqual(TEXT("Widget selection should update to the requested choice index."), GrowthScreen->GetSelectedChoiceIndex(), 1);
	TestEqual(TEXT("Widget selection should expose the requested choice instance id."), GrowthScreen->GetSelectedChoiceInstanceId(), PendingSnapshot.PendingGrowthChoice.Choices[1].ChoiceInstanceId);

	TestTrue(TEXT("Confirming the current growth choice through the widget should submit the RunCommand."), GrowthScreen->ConfirmCurrentChoice());
	TestFalse(TEXT("RunSession pending growth choice should be cleared after widget-driven confirmation."), RunSession->GetSnapshot().PendingGrowthChoice.bHasPendingChoice);
	TestEqual(TEXT("Widget-driven confirmation should still route through GrowthChoiceApplied."), Context.RunFlowSubsystem->GetLastProcessedRunEvent().EventType, EFinalRunEventType::GrowthChoiceApplied);
	return true;
}

#endif
