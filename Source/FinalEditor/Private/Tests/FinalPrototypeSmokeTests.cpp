#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Commands/FinalBattleCommand.h"
#include "Engine/GameInstance.h"
#include "Facade/FinalRunSession.h"
#include "Queries/FinalDataRegistry.h"
#include "Run/Definitions/FinalCharacterGrowthConfig.h"
#include "Run/Definitions/FinalPrototypeBootstrapDefinition.h"
#include "Run/Definitions/FinalRunRouteDefinition.h"
#include "Subsystems/FinalBattleFlowSubsystem.h"
#include "Subsystems/FinalGameFlowSubsystem.h"
#include "UObject/StrongObjectPtr.h"

namespace FinalPrototypeSmokeTests
{
	const FName PrototypeBootstrapId(TEXT("prototype.bootstrap.test"));
	const FName StarterBootstrapId(TEXT("prototype.bootstrap.starter.chapter1"));
	const int32 SyntheticVictoryRewardGold = 15;
	const int32 BattleCommandSafetyLimit = 64;

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
				const UFinalCharacterDefinition* CharacterDefinition = DataRegistry->FindCharacterDefinition(BootstrapCharacterState.CharacterId);
				const UFinalCharacterGrowthConfig* GrowthConfig =
					(CharacterDefinition != nullptr && CharacterDefinition->GrowthConfigId.IsValid())
						? DataRegistry->FindCharacterGrowthConfig(CharacterDefinition->GrowthConfigId)
						: nullptr;
				PartyStates.Add(BuildInitialRunCharacterState(BootstrapCharacterState, GrowthConfig));
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
			const UFinalCharacterDefinition* CharacterDefinition = Context.DataRegistry->FindCharacterDefinition(BootstrapDefinition.InitialCharacterStates[Index].CharacterId);
			if (CharacterDefinition == nullptr)
			{
				AddMissingReferenceError(Test, FString::Printf(TEXT("InitialCharacterStates[%d].CharacterId"), Index), BootstrapDefinition.InitialCharacterStates[Index].CharacterId.ToString());
				bValid = false;
			}
			else if (CharacterDefinition->GrowthConfigId.IsValid()
				&& Context.DataRegistry->FindCharacterGrowthConfig(CharacterDefinition->GrowthConfigId) == nullptr)
			{
				AddMissingReferenceError(Test, FString::Printf(TEXT("InitialCharacterStates[%d].GrowthConfigId"), Index), CharacterDefinition->GrowthConfigId.ToString());
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
	TestNotNull(
		TEXT("Starter growth evolution definition should be discoverable through FinalDataRegistry."),
		Context.DataRegistry->FindCardEvolutionDefinition(FFinalCardEvolutionId(FName(TEXT("evo.starter.huo.duanyuezhan.pozhen")))));
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

	FFinalRunState SeedRunState = RunSession->GetRunState();
	if (SeedRunState.Characters.Num() > 0)
	{
		SeedRunState.Characters[0].BreakthroughValue += 7;
		RunSession->ConfigureBattleStartState(
			BootstrapDefinition->EncounterId,
			BootstrapDefinition->RuleConfigId,
			SeedRunState.Characters,
			BootstrapDefinition->StarterDeckCardIds,
			BootstrapDefinition->InitialTeamCurrentHP);
		RunSession->ConfigureRunRouteById(BootstrapDefinition->RunRouteId);
	}

	if (Context.StartBattleFromRun(*this) == nullptr)
	{
		return false;
	}

	const FFinalBattleResult SyntheticVictoryResult = BuildSyntheticVictoryResult(*RunSession, *Context.BattleFlowSubsystem);
	TestTrue(TEXT("Prototype battle result should write back into the run through FinalGameFlowSubsystem."), Context.GameFlowSubsystem->CompleteBattleAndApplyResult(SyntheticVictoryResult));
	TestNull(TEXT("Active battle session should be cleared after battle result write-back."), Context.GameFlowSubsystem->GetActiveBattleSession());

	const FFinalRunSnapshot SnapshotAfterBattle = RunSession->GetSnapshot();
	if (SnapshotAfterBattle.Characters.Num() > 0)
	{
		TestEqual(TEXT("Battle result write-back should preserve run-owned breakthrough progress."), SnapshotAfterBattle.Characters[0].BreakthroughValue, SeedRunState.Characters[0].BreakthroughValue);
		TestEqual(TEXT("Battle result write-back should preserve run-owned level."), SnapshotAfterBattle.Characters[0].Level, SeedRunState.Characters[0].Level);
	}
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalPrototypePostBattleCardRewardAndLinearProgressionTest,
	"Final.Editor.PrototypeSmoke.PostBattleCardRewardAndLinearProgression",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalPrototypePostBattleCardRewardAndLinearProgressionTest::RunTest(const FString& Parameters)
{
	using namespace FinalPrototypeSmokeTests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalPrototypePostBattleRewardWorld")))
	{
		return false;
	}

	UFinalPrototypeBootstrapDefinition* BootstrapDefinition = Context.DataRegistry
		? Context.DataRegistry->FindPrototypeBootstrapDefinition(StarterBootstrapId)
		: nullptr;
	if (!TestNotNull(TEXT("Starter bootstrap definition must be discoverable for post-battle reward progression."), BootstrapDefinition))
	{
		return false;
	}

	const UFinalRunRouteDefinition* RouteDefinition = Context.DataRegistry->FindRunRouteDefinition(BootstrapDefinition->RunRouteId);
	if (!TestNotNull(TEXT("Starter route definition must be available."), RouteDefinition))
	{
		return false;
	}

	TestTrue(
		TEXT("Starter route should include a boss node for the linear endpoint."),
		RouteDefinition->NodeDefinitions.ContainsByPredicate([](const FFinalRunNodeDefinition& NodeDefinition)
		{
			return NodeDefinition.NodeType == EFinalRunNodeType::BossBattle;
		}));

	UFinalRunSession* RunSession = Context.BootstrapRunFromDefinition(*this, *BootstrapDefinition);
	if (RunSession == nullptr)
	{
		return false;
	}

	TestFalse(TEXT("Starter bootstrap should no longer begin with an already-pending growth choice."), RunSession->HasPendingGrowthChoice());

	if (Context.StartBattleFromRun(*this) == nullptr)
	{
		return false;
	}

	const int32 InitialGold = RunSession->GetSnapshot().Gold;
	const int32 InitialDeckCount = RunSession->GetSnapshot().DeckCount;

	const FFinalBattleResult OpeningVictoryResult = BuildSyntheticVictoryResult(*RunSession, *Context.BattleFlowSubsystem);
	TestTrue(TEXT("Opening battle result should apply to run."), Context.GameFlowSubsystem->CompleteBattleAndApplyResult(OpeningVictoryResult));

	FFinalRunSnapshot Snapshot = RunSession->GetSnapshot();
	TestEqual(TEXT("Victory gold should be applied immediately."), Snapshot.Gold, InitialGold + SyntheticVictoryRewardGold);
	TestTrue(TEXT("Post-battle reward should expose card candidates."), Snapshot.PendingBattleReward.bHasPendingReward);
	TestTrue(TEXT("Post-battle reward should expose no more than three card candidates."), Snapshot.PendingBattleReward.RewardEntries.Num() <= 3);
	TestTrue(TEXT("Post-battle reward candidates should be card grants."), Snapshot.PendingBattleReward.RewardEntries.ContainsByPredicate([](const FFinalRunRewardEntry& Entry)
	{
		return Entry.RewardType == EFinalRunRewardType::CardGrant && Entry.GrantedCardId.IsValid();
	}));
	for (const FFinalRunRewardEntry& RewardEntry : Snapshot.PendingBattleReward.RewardEntries)
	{
		TestEqual(TEXT("Every post-battle pending reward should be a card grant."), RewardEntry.RewardType, EFinalRunRewardType::CardGrant);
		TestTrue(TEXT("Every post-battle card grant should reference a valid card id."), RewardEntry.GrantedCardId.IsValid());
	}

	if (Snapshot.PendingBattleReward.RewardEntries.Num() > 1)
	{
		TestFalse(TEXT("Claiming a multi-candidate post-battle reward without RewardId should be rejected."), RunSession->ClaimPendingBattleReward());
	}

	const FName FirstRewardId = Snapshot.PendingBattleReward.RewardEntries.Num() > 0
		? Snapshot.PendingBattleReward.RewardEntries[0].RewardId
		: NAME_None;
	TestFalse(TEXT("First post-battle card reward id should not be None."), FirstRewardId.IsNone());
	TestTrue(TEXT("Choosing one post-battle card reward should be accepted."), RunSession->ClaimPendingBattleRewardById(FirstRewardId));

	Snapshot = RunSession->GetSnapshot();
	TestFalse(TEXT("Pending post-battle reward should clear after choosing a card."), Snapshot.PendingBattleReward.bHasPendingReward);
	TestEqual(TEXT("Choosing one card reward should add exactly one card to RunDeck."), Snapshot.DeckCount, InitialDeckCount + 1);
	TestEqual(TEXT("Run should wait for node advance after reward claim."), Snapshot.Progression.FlowStage, EFinalRunFlowStage::AwaitingNodeAdvance);
	TestTrue(TEXT("Run should expose at least one next node after opening reward claim."), Snapshot.Progression.AvailableNextNodes.Num() > 0);

	const FName RewardNodeId = Snapshot.Progression.AvailableNextNodes.Num() > 0 ? Snapshot.Progression.AvailableNextNodes[0].NodeId : NAME_None;
	TestFalse(TEXT("Reward node id should be available."), RewardNodeId.IsNone());
	TestTrue(TEXT("Run should advance to the configured reward node."), RunSession->AdvanceToNode(RewardNodeId));

	FFinalRunCommand ResolveRewardCommand;
	ResolveRewardCommand.CommandType = EFinalRunCommandType::ResolveReward;
	TestTrue(TEXT("Reward node should resolve through RunCommand."), RunSession->SubmitRunCommand(ResolveRewardCommand));

	Snapshot = RunSession->GetSnapshot();
	const FName EventNodeId = Snapshot.Progression.AvailableNextNodes.Num() > 0 ? Snapshot.Progression.AvailableNextNodes[0].NodeId : NAME_None;
	TestFalse(TEXT("Event node id should be available after reward node."), EventNodeId.IsNone());
	TestTrue(TEXT("Run should advance to the configured event node."), RunSession->AdvanceToNode(EventNodeId));

	Snapshot = RunSession->GetSnapshot();
	FName EventOptionId = NAME_None;
	for (const FFinalRunEventOptionViewData& Option : Snapshot.PendingEventNode.Options)
	{
		if (Option.bSelectable)
		{
			EventOptionId = Option.OptionId;
			break;
		}
	}
	TestFalse(TEXT("Event node should expose a selectable option."), EventOptionId.IsNone());

	FFinalRunCommand ResolveEventCommand;
	ResolveEventCommand.CommandType = EFinalRunCommandType::ResolveEvent;
	ResolveEventCommand.PayloadId = EventOptionId;
	TestTrue(TEXT("Event node option should resolve through RunCommand."), RunSession->SubmitRunCommand(ResolveEventCommand));

	Snapshot = RunSession->GetSnapshot();
	const FName ShopNodeId = Snapshot.Progression.AvailableNextNodes.Num() > 0 ? Snapshot.Progression.AvailableNextNodes[0].NodeId : NAME_None;
	TestFalse(TEXT("Shop node id should be available after event node."), ShopNodeId.IsNone());
	TestTrue(TEXT("Run should advance to the configured shop node."), RunSession->AdvanceToNode(ShopNodeId));

	Snapshot = RunSession->GetSnapshot();
	FName ShopOfferId = NAME_None;
	for (const FFinalRunShopOfferViewData& Offer : Snapshot.PendingShopNode.Offers)
	{
		if (Offer.bPurchasable)
		{
			ShopOfferId = Offer.OfferId;
			break;
		}
	}
	TestFalse(TEXT("Shop node should expose a purchasable offer."), ShopOfferId.IsNone());

	FFinalRunCommand ResolveShopCommand;
	ResolveShopCommand.CommandType = EFinalRunCommandType::ResolveShop;
	ResolveShopCommand.PayloadId = ShopOfferId;
	TestTrue(TEXT("Shop offer should resolve through RunCommand."), RunSession->SubmitRunCommand(ResolveShopCommand));

	Snapshot = RunSession->GetSnapshot();
	const FName EliteNodeId = Snapshot.Progression.AvailableNextNodes.Num() > 0 ? Snapshot.Progression.AvailableNextNodes[0].NodeId : NAME_None;
	TestFalse(TEXT("Elite node id should be available after shop node."), EliteNodeId.IsNone());
	TestTrue(TEXT("Run should advance to the configured elite node."), RunSession->AdvanceToNode(EliteNodeId));
	TestEqual(TEXT("Elite node should prepare battle."), RunSession->GetSnapshot().Progression.FlowStage, EFinalRunFlowStage::PreparingBattle);

	if (Context.StartBattleFromRun(*this) == nullptr)
	{
		return false;
	}

	const FFinalBattleResult EliteVictoryResult = BuildSyntheticVictoryResult(*RunSession, *Context.BattleFlowSubsystem);
	TestTrue(TEXT("Elite battle result should apply to run."), Context.GameFlowSubsystem->CompleteBattleAndApplyResult(EliteVictoryResult));
	if (RunSession->GetSnapshot().PendingBattleReward.bHasPendingReward)
	{
		TestTrue(TEXT("Elite post-battle card reward can be skipped."), RunSession->SkipPendingBattleReward());
	}

	Snapshot = RunSession->GetSnapshot();
	const FName BossNodeId = Snapshot.Progression.AvailableNextNodes.Num() > 0 ? Snapshot.Progression.AvailableNextNodes[0].NodeId : NAME_None;
	TestFalse(TEXT("Boss node id should be available after elite node."), BossNodeId.IsNone());
	TestTrue(TEXT("Run should advance to the configured boss node."), RunSession->AdvanceToNode(BossNodeId));
	TestEqual(TEXT("Boss node should prepare battle."), RunSession->GetSnapshot().Progression.FlowStage, EFinalRunFlowStage::PreparingBattle);

	if (Context.StartBattleFromRun(*this) == nullptr)
	{
		return false;
	}

	const FFinalBattleResult BossVictoryResult = BuildSyntheticVictoryResult(*RunSession, *Context.BattleFlowSubsystem);
	TestTrue(TEXT("Boss battle result should apply to run."), Context.GameFlowSubsystem->CompleteBattleAndApplyResult(BossVictoryResult));
	if (RunSession->GetSnapshot().PendingBattleReward.bHasPendingReward)
	{
		TestTrue(TEXT("Boss post-battle card reward can be skipped."), RunSession->SkipPendingBattleReward());
	}

	TestEqual(TEXT("Run should end after resolving the boss post-battle reward because no next node exists."), RunSession->GetSnapshot().Progression.FlowStage, EFinalRunFlowStage::RunEnded);
	return !HasAnyErrors();
}

#endif
