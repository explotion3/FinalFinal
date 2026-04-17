#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Commands/FinalBattleCommand.h"
#include "Engine/GameInstance.h"
#include "Facade/FinalRunSession.h"
#include "Queries/FinalDataRegistry.h"
#include "Run/Definitions/FinalPrototypeBootstrapDefinition.h"
#include "Subsystems/FinalBattleFlowSubsystem.h"
#include "Subsystems/FinalGameFlowSubsystem.h"
#include "UObject/StrongObjectPtr.h"

namespace FinalPrototypeSmokeTests
{
	const FName PrototypeBootstrapId(TEXT("prototype.bootstrap.test"));
	const FName StarterBootstrapId(TEXT("prototype.bootstrap.starter.chapter1"));
	const int32 SyntheticVictoryRewardGold = 15;
	const int32 BattleCommandSafetyLimit = 64;

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

			const FName WorldName(*FString::Printf(TEXT("/Temp/%s"), ContextName));
			if (GEngine == nullptr)
			{
				Test.AddError(TEXT("GEngine is unavailable; cannot create a standalone automation GameInstance."));
				return false;
			}

			UGameInstance* RawGameInstance = NewObject<UGameInstance>(GEngine, UGameInstance::StaticClass());
			if (RawGameInstance == nullptr)
			{
				Test.AddError(TEXT("Failed to allocate a transient UGameInstance for automation."));
				return false;
			}

			GameInstance.Reset(RawGameInstance);
			RawGameInstance->InitializeStandalone(WorldName);

			DataRegistry = RawGameInstance->GetSubsystem<UFinalDataRegistry>();
			GameFlowSubsystem = RawGameInstance->GetSubsystem<UFinalGameFlowSubsystem>();
			BattleFlowSubsystem = RawGameInstance->GetSubsystem<UFinalBattleFlowSubsystem>();

			const bool bReady =
				Test.TestNotNull(TEXT("FinalDataRegistry should initialize in standalone automation context."), DataRegistry)
				&& Test.TestNotNull(TEXT("FinalGameFlowSubsystem should initialize in standalone automation context."), GameFlowSubsystem)
				&& Test.TestNotNull(TEXT("FinalBattleFlowSubsystem should initialize in standalone automation context."), BattleFlowSubsystem);

			if (!bReady)
			{
				Test.AddError(TEXT("Failed to initialize the prototype smoke test automation context."));
			}

			return bReady;
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

			BattleFlowSubsystem = nullptr;
			GameFlowSubsystem = nullptr;
			DataRegistry = nullptr;
			GameInstance.Reset();
			CollectGarbage(RF_NoFlags);
		}

		UFinalPrototypeBootstrapDefinition* ResolvePrototypeBootstrapDefinition(FAutomationTestBase& Test) const
		{
			if (!Test.TestNotNull(TEXT("DataRegistry must be available before resolving prototype bootstrap content."), DataRegistry))
			{
				return nullptr;
			}

			UFinalPrototypeBootstrapDefinition* BootstrapDefinition = DataRegistry->FindPrototypeBootstrapDefinition(PrototypeBootstrapId);
			if (!Test.TestNotNull(TEXT("Prototype bootstrap definition must be discoverable through FinalDataRegistry."), BootstrapDefinition))
			{
				return nullptr;
			}

			Test.TestTrue(TEXT("Prototype bootstrap definition should satisfy its own structural validity check."), BootstrapDefinition->IsValidDefinition());
			return BootstrapDefinition;
		}

		UFinalRunSession* BootstrapRunFromDefinition(FAutomationTestBase& Test, const UFinalPrototypeBootstrapDefinition& BootstrapDefinition) const
		{
			if (!Test.TestNotNull(TEXT("FinalGameFlowSubsystem must be available before bootstrapping a run."), GameFlowSubsystem))
			{
				return nullptr;
			}

			UFinalRunSession* RunSession = GameFlowSubsystem->BootstrapNewRun();
			if (!Test.TestNotNull(TEXT("Game flow should create a RunSession."), RunSession))
			{
				return nullptr;
			}

			TArray<FFinalRunPersistentCharacterState> PartyStates;
			PartyStates.Reserve(BootstrapDefinition.InitialCharacterStates.Num());
			for (const FFinalPrototypeBootstrapCharacterState& BootstrapCharacterState : BootstrapDefinition.InitialCharacterStates)
			{
				FFinalRunPersistentCharacterState CharacterState;
				CharacterState.CharacterId = BootstrapCharacterState.CharacterId;
				CharacterState.CurrentStress = BootstrapCharacterState.CurrentStress;
				CharacterState.bCollapsed = BootstrapCharacterState.bCollapsed;
				CharacterState.CurrentAwakenCount = BootstrapCharacterState.CurrentAwakenCount;
				CharacterState.CollapseCount = BootstrapCharacterState.CollapseCount;
				PartyStates.Add(CharacterState);
			}

			RunSession->ConfigureBattleStartState(
				BootstrapDefinition.EncounterId,
				BootstrapDefinition.RuleConfigId,
				PartyStates,
				BootstrapDefinition.StarterDeckCardIds,
				BootstrapDefinition.InitialTeamCurrentHP);

			Test.TestTrue(TEXT("RunSession should expose a valid battle start state after prototype bootstrap configuration."), RunSession->HasValidBattleStartState());
			Test.TestTrue(TEXT("RunSession should resolve the bootstrap route through FinalDataRegistry."), RunSession->ConfigureRunRouteById(BootstrapDefinition.RunRouteId));
			return RunSession;
		}

		UFinalBattleSession* StartBattleFromRun(FAutomationTestBase& Test) const
		{
			if (!Test.TestNotNull(TEXT("FinalGameFlowSubsystem must be available before starting a battle from run."), GameFlowSubsystem))
			{
				return nullptr;
			}

			UFinalBattleSession* BattleSession = GameFlowSubsystem->StartBattleFromRunSession();
			if (BattleSession == nullptr)
			{
				Test.AddError(FString::Printf(TEXT("Failed to start battle from run: %s"), *GameFlowSubsystem->GetLastBattleFailureReason().ToString()));
				return nullptr;
			}

			Test.TestNotNull(TEXT("Active battle session should be set after starting a battle from run."), GameFlowSubsystem->GetActiveBattleSession());
			return BattleSession;
		}

		TStrongObjectPtr<UGameInstance> GameInstance;
		UFinalDataRegistry* DataRegistry = nullptr;
		UFinalGameFlowSubsystem* GameFlowSubsystem = nullptr;
		UFinalBattleFlowSubsystem* BattleFlowSubsystem = nullptr;
	};

	void AddMissingReferenceError(FAutomationTestBase& Test, const FString& FieldName, const FString& StableId)
	{
		Test.AddError(FString::Printf(TEXT("%s could not be resolved through FinalDataRegistry: %s"), *FieldName, *StableId));
	}

	bool ValidateBootstrapRegistryReferences(
		FAutomationTestBase& Test,
		const FAutomationContext& Context,
		const UFinalPrototypeBootstrapDefinition& BootstrapDefinition)
	{
		bool bValid = true;

		bValid &= Test.TestNotNull(TEXT("Bootstrap RuleConfigId should resolve through FinalDataRegistry."), Context.DataRegistry->FindRuleConfig(BootstrapDefinition.RuleConfigId));
		bValid &= Test.TestNotNull(TEXT("Bootstrap EncounterId should resolve through FinalDataRegistry."), Context.DataRegistry->FindEncounterDefinition(BootstrapDefinition.EncounterId));
		bValid &= Test.TestNotNull(TEXT("Bootstrap RunRouteId should resolve through FinalDataRegistry."), Context.DataRegistry->FindRunRouteDefinition(BootstrapDefinition.RunRouteId));

		for (int32 Index = 0; Index < BootstrapDefinition.PartyCharacterIds.Num(); ++Index)
		{
			if (Context.DataRegistry->FindCharacterDefinition(BootstrapDefinition.PartyCharacterIds[Index]) == nullptr)
			{
				AddMissingReferenceError(Test, FString::Printf(TEXT("PartyCharacterIds[%d]"), Index), BootstrapDefinition.PartyCharacterIds[Index].ToString());
				bValid = false;
			}
		}

		for (int32 Index = 0; Index < BootstrapDefinition.InitialCharacterStates.Num(); ++Index)
		{
			if (Context.DataRegistry->FindCharacterDefinition(BootstrapDefinition.InitialCharacterStates[Index].CharacterId) == nullptr)
			{
				AddMissingReferenceError(Test, FString::Printf(TEXT("InitialCharacterStates[%d].CharacterId"), Index), BootstrapDefinition.InitialCharacterStates[Index].CharacterId.ToString());
				bValid = false;
			}
		}

		for (int32 Index = 0; Index < BootstrapDefinition.StarterDeckCardIds.Num(); ++Index)
		{
			if (Context.DataRegistry->FindCardDefinition(BootstrapDefinition.StarterDeckCardIds[Index]) == nullptr)
			{
				AddMissingReferenceError(Test, FString::Printf(TEXT("StarterDeckCardIds[%d]"), Index), BootstrapDefinition.StarterDeckCardIds[Index].ToString());
				bValid = false;
			}
		}

		return bValid;
	}

	bool SubmitOneDeterministicBattleCommand(UFinalBattleFlowSubsystem& BattleFlowSubsystem, FString& OutFailure)
	{
		const FFinalBattleSnapshot Snapshot = BattleFlowSubsystem.GetCurrentSnapshot();
		if (Snapshot.bBattleEnded)
		{
			return true;
		}

		for (const FFinalBattleUltimateViewData& UltimateView : Snapshot.CharacterUltimates)
		{
			if (!UltimateView.bCanActivate)
			{
				continue;
			}

			FFinalBattleCommand Command;
			Command.CommandType = EFinalBattleCommandType::PlayUltimate;
			Command.UltimateOwnerUnitId = UltimateView.OwnerUnitId;
			if (BattleFlowSubsystem.SubmitBattleCommand(Command))
			{
				return true;
			}
		}

		for (const FFinalBattleCardViewData& CardView : Snapshot.HandCards)
		{
			if (CardView.RuntimeCostAP > Snapshot.CurrentAP)
			{
				continue;
			}

			FFinalBattleCommand Command;
			Command.CommandType = EFinalBattleCommandType::PlayCard;
			Command.CardInstanceId = CardView.CardInstanceId;
			if (BattleFlowSubsystem.SubmitBattleCommand(Command))
			{
				return true;
			}
		}

		FFinalBattleCommand EndTurnCommand;
		EndTurnCommand.CommandType = EFinalBattleCommandType::EndTurn;
		if (BattleFlowSubsystem.SubmitBattleCommand(EndTurnCommand))
		{
			return true;
		}

		OutFailure = BattleFlowSubsystem.GetLastFailureReason().ToString();
		return false;
	}

	FFinalBattleResult BuildSyntheticVictoryResult(const UFinalRunSession& RunSession, const UFinalBattleFlowSubsystem& BattleFlowSubsystem)
	{
		const FFinalRunState RunState = RunSession.GetRunState();
		const FFinalBattleSnapshot BattleSnapshot = BattleFlowSubsystem.GetCurrentSnapshot();

		FFinalBattleResult Result;
		Result.EncounterId = RunState.CurrentEncounterId;
		Result.Outcome = EFinalBattleOutcome::Victory;
		Result.TeamCurrentHP = BattleSnapshot.TeamCurrentHP > 0 ? BattleSnapshot.TeamCurrentHP : FMath::Max(RunState.TeamCurrentHP, 1);
		Result.RewardGold = SyntheticVictoryRewardGold;
		Result.UpdatedCharacterStates = RunState.Characters;
		return Result;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalPrototypeBootstrapDiscoveryTest,
	"Final.Editor.PrototypeSmoke.BootstrapDiscovery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalPrototypeBootstrapDiscoveryTest::RunTest(const FString& Parameters)
{
	using namespace FinalPrototypeSmokeTests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalPrototypeBootstrapDiscoveryWorld")))
	{
		return false;
	}

	UFinalPrototypeBootstrapDefinition* BootstrapDefinition = Context.ResolvePrototypeBootstrapDefinition(*this);
	if (BootstrapDefinition == nullptr)
	{
		return false;
	}

	TestEqual(TEXT("Resolved bootstrap ID should match the expected prototype vertical-slice bootstrap."), BootstrapDefinition->BootstrapId, PrototypeBootstrapId);
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalPrototypeBootstrapRegistryReferenceTest,
	"Final.Editor.PrototypeSmoke.BootstrapRegistryReferences",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalPrototypeBootstrapRegistryReferenceTest::RunTest(const FString& Parameters)
{
	using namespace FinalPrototypeSmokeTests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalPrototypeBootstrapRegistryReferencesWorld")))
	{
		return false;
	}

	UFinalPrototypeBootstrapDefinition* BootstrapDefinition = Context.ResolvePrototypeBootstrapDefinition(*this);
	if (BootstrapDefinition == nullptr)
	{
		return false;
	}

	ValidateBootstrapRegistryReferences(*this, Context, *BootstrapDefinition);

	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalStarterBootstrapRegistryReferenceTest,
	"Final.Editor.PrototypeSmoke.StarterBootstrapRegistryReferences",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalStarterBootstrapRegistryReferenceTest::RunTest(const FString& Parameters)
{
	using namespace FinalPrototypeSmokeTests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalStarterBootstrapRegistryReferencesWorld")))
	{
		return false;
	}

	UFinalPrototypeBootstrapDefinition* StarterBootstrapDefinition = Context.DataRegistry->FindPrototypeBootstrapDefinition(StarterBootstrapId);
	if (!TestNotNull(TEXT("Starter bootstrap definition must be discoverable through FinalDataRegistry."), StarterBootstrapDefinition))
	{
		return false;
	}

	TestEqual(TEXT("Resolved starter bootstrap ID should match the expected starter content bundle bootstrap."), StarterBootstrapDefinition->BootstrapId, StarterBootstrapId);
	TestTrue(TEXT("Starter bootstrap definition should satisfy its own structural validity check."), StarterBootstrapDefinition->IsValidDefinition());
	ValidateBootstrapRegistryReferences(*this, Context, *StarterBootstrapDefinition);
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalPrototypeBootstrapRunStartTest,
	"Final.Editor.PrototypeSmoke.BootstrapStartsRun",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalPrototypeBootstrapRunStartTest::RunTest(const FString& Parameters)
{
	using namespace FinalPrototypeSmokeTests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalPrototypeBootstrapStartsRunWorld")))
	{
		return false;
	}

	UFinalPrototypeBootstrapDefinition* BootstrapDefinition = Context.ResolvePrototypeBootstrapDefinition(*this);
	if (BootstrapDefinition == nullptr)
	{
		return false;
	}

	UFinalRunSession* RunSession = Context.BootstrapRunFromDefinition(*this, *BootstrapDefinition);
	if (RunSession == nullptr)
	{
		return false;
	}

	const FFinalBattleStartRequest StartRequest = RunSession->BuildBattleStartRequest();
	const FFinalRunSnapshot Snapshot = RunSession->GetSnapshot();

	TestEqual(TEXT("Bootstrapped run should use the encounter from the prototype bootstrap definition."), StartRequest.EncounterId, BootstrapDefinition->EncounterId);
	TestEqual(TEXT("Bootstrapped run should use the rule config from the prototype bootstrap definition."), StartRequest.RuleConfigId, BootstrapDefinition->RuleConfigId);
	TestEqual(TEXT("Bootstrapped run should expose the starter deck size from the bootstrap definition."), StartRequest.DeckCardIds.Num(), BootstrapDefinition->StarterDeckCardIds.Num());
	TestEqual(TEXT("Bootstrapped run should expose the prototype roster size from the bootstrap definition."), StartRequest.PartyStates.Num(), BootstrapDefinition->InitialCharacterStates.Num());
	TestEqual(TEXT("Bootstrapped run should enter PreparingBattle on the route entry battle node."), Snapshot.Progression.FlowStage, EFinalRunFlowStage::PreparingBattle);
	TestTrue(TEXT("Run snapshot should keep a current node after bootstrapping the prototype route."), !Snapshot.Progression.CurrentNodeId.IsNone());
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalPrototypeBattleProgressionSmokeTest,
	"Final.Editor.PrototypeSmoke.RunStartsBattleAndProgresses",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalPrototypeBattleProgressionSmokeTest::RunTest(const FString& Parameters)
{
	using namespace FinalPrototypeSmokeTests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalPrototypeBattleProgressionWorld")))
	{
		return false;
	}

	UFinalPrototypeBootstrapDefinition* BootstrapDefinition = Context.ResolvePrototypeBootstrapDefinition(*this);
	if (BootstrapDefinition == nullptr)
	{
		return false;
	}

	if (Context.BootstrapRunFromDefinition(*this, *BootstrapDefinition) == nullptr)
	{
		return false;
	}

	if (Context.StartBattleFromRun(*this) == nullptr)
	{
		return false;
	}

	const int32 InitialBattleEventSequence = Context.BattleFlowSubsystem->GetLatestBattleEventSequence();
	const FFinalBattleSnapshot InitialSnapshot = Context.BattleFlowSubsystem->GetCurrentSnapshot();
	TestTrue(TEXT("Prototype battle should start with at least one party member."), InitialSnapshot.Characters.Num() > 0);
	TestTrue(TEXT("Prototype battle should start with at least one enemy."), InitialSnapshot.Enemies.Num() > 0);

	FString FailureMessage;
	TestTrue(TEXT("Prototype battle smoke test should accept one deterministic command through the public battle flow API."), SubmitOneDeterministicBattleCommand(*Context.BattleFlowSubsystem, FailureMessage));
	if (!FailureMessage.IsEmpty())
	{
		AddError(FString::Printf(TEXT("Battle progression failed: %s"), *FailureMessage));
	}

	TestTrue(TEXT("Battle event ledger should advance after the first smoke-test command."), Context.BattleFlowSubsystem->GetLatestBattleEventSequence() > InitialBattleEventSequence);
	TestTrue(TEXT("Last command event should not be a rejection in the prototype smoke battle."), Context.BattleFlowSubsystem->GetLastCommandEvent().EventType != EFinalBattleEventType::CommandRejected);
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalPrototypeBattleWritebackAndSaveRestoreTest,
	"Final.Editor.PrototypeSmoke.BattleWritebackAndSaveRestore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalPrototypeBattleWritebackAndSaveRestoreTest::RunTest(const FString& Parameters)
{
	using namespace FinalPrototypeSmokeTests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalPrototypeBattleWritebackAndSaveRestoreWorld")))
	{
		return false;
	}

	UFinalPrototypeBootstrapDefinition* BootstrapDefinition = Context.ResolvePrototypeBootstrapDefinition(*this);
	if (BootstrapDefinition == nullptr)
	{
		return false;
	}

	UFinalRunSession* RunSession = Context.BootstrapRunFromDefinition(*this, *BootstrapDefinition);
	if (RunSession == nullptr)
	{
		return false;
	}

	if (Context.StartBattleFromRun(*this) == nullptr)
	{
		return false;
	}

	const FFinalBattleResult SyntheticVictoryResult = BuildSyntheticVictoryResult(*RunSession, *Context.BattleFlowSubsystem);
	TestTrue(TEXT("Prototype battle result should write back into the run through FinalGameFlowSubsystem."), Context.GameFlowSubsystem->CompleteBattleAndApplyResult(SyntheticVictoryResult));
	TestNull(TEXT("Active battle session should be cleared after battle result write-back."), Context.GameFlowSubsystem->GetActiveBattleSession());

	const FFinalRunSnapshot SnapshotAfterBattle = RunSession->GetSnapshot();
	TestTrue(
		TEXT("Battle result write-back should return the run to a post-battle stage."),
		SnapshotAfterBattle.Progression.FlowStage == EFinalRunFlowStage::PendingBattleReward
			|| SnapshotAfterBattle.Progression.FlowStage == EFinalRunFlowStage::AwaitingNodeAdvance);
	TestTrue(
		TEXT("Prototype battle victory should surface a pending reward or a progressed post-battle state."),
		SnapshotAfterBattle.PendingBattleReward.bHasPendingReward
			|| SnapshotAfterBattle.Progression.FlowStage == EFinalRunFlowStage::AwaitingNodeAdvance);

	const FFinalRunSaveData SaveData = RunSession->ExportSaveData();
	TestTrue(TEXT("Prototype run save data should keep the current supported save version."), SaveData.IsSupportedVersion());

	FText StructuralFailureReason;
	TestTrue(TEXT("Prototype run save data should remain structurally valid after the battle write-back path."), SaveData.IsStructurallyValid(&StructuralFailureReason));
	if (!StructuralFailureReason.IsEmpty())
	{
		AddError(FString::Printf(TEXT("Exported run save data was reported invalid: %s"), *StructuralFailureReason.ToString()));
	}

	FText RestoreFailureReason;
	UFinalRunSession* RestoredRunSession = NewObject<UFinalRunSession>(Context.GameInstance.Get());
	TestNotNull(TEXT("A restored RunSession object should be constructible for save/load smoke validation."), RestoredRunSession);
	if (RestoredRunSession == nullptr)
	{
		return false;
	}

	TestTrue(TEXT("Exported prototype run save data should restore through UFinalRunSession::RestoreFromSaveData()."), RestoredRunSession->RestoreFromSaveData(SaveData, RestoreFailureReason));
	if (!RestoreFailureReason.IsEmpty())
	{
		AddError(FString::Printf(TEXT("RestoreFromSaveData failed: %s"), *RestoreFailureReason.ToString()));
	}

	const FFinalRunSnapshot RestoredSnapshot = RestoredRunSession->GetSnapshot();
	TestEqual(TEXT("Restored snapshot flow stage should match the exported run snapshot."), RestoredSnapshot.Progression.FlowStage, SnapshotAfterBattle.Progression.FlowStage);
	TestEqual(TEXT("Restored snapshot current node should match the exported run snapshot."), RestoredSnapshot.Progression.CurrentNodeId, SnapshotAfterBattle.Progression.CurrentNodeId);
	TestEqual(TEXT("Restored snapshot pending reward flag should match the exported run snapshot."), RestoredSnapshot.PendingBattleReward.bHasPendingReward, SnapshotAfterBattle.PendingBattleReward.bHasPendingReward);
	TestEqual(TEXT("Restored snapshot gold should match the exported run snapshot."), RestoredSnapshot.Gold, SnapshotAfterBattle.Gold);
	TestEqual(TEXT("Restored snapshot deck count should match the exported run snapshot."), RestoredSnapshot.DeckCount, SnapshotAfterBattle.DeckCount);
	TestEqual(TEXT("Restored snapshot relic count should match the exported run snapshot."), RestoredSnapshot.RelicCount, SnapshotAfterBattle.RelicCount);
	TestTrue(TEXT("Restored run event sequence should remain self-consistent after save/load."), RestoredRunSession->GetLatestRunEventSequence() >= RunSession->GetLatestRunEventSequence());
	return !HasAnyErrors();
}

#endif
