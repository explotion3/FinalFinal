#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Battle/Definitions/FinalCardDefinition.h"
#include "Engine/GameInstance.h"
#include "Facade/FinalRunSession.h"
#include "Queries/FinalDataRegistry.h"
#include "Queries/FinalRunSnapshot.h"
#include "Run/Definitions/FinalCardEvolutionDefinition.h"
#include "UObject/StrongObjectPtr.h"

namespace FinalRunGrowthStateTests
{
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
				Test.AddError(TEXT("Failed to allocate a transient UGameInstance for run growth automation."));
				return false;
			}

			GameInstance.Reset(RawGameInstance);
			RawGameInstance->InitializeStandalone(WorldName);
			DataRegistry = RawGameInstance->GetSubsystem<UFinalDataRegistry>();
			return Test.TestNotNull(TEXT("FinalDataRegistry should initialize in standalone automation context."), DataRegistry);
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

		UFinalCardEvolutionDefinition* RegisterEvolutionDefinition(
			const FFinalCardEvolutionId& EvolutionId,
			const FFinalCardId& FromCardId,
			const FFinalCardId& ToCardId,
			const FFinalCharacterId& CharacterId,
			const FString& DisplayName) const
		{
			UFinalCardEvolutionDefinition* EvolutionDefinition = NewObject<UFinalCardEvolutionDefinition>(GameInstance.Get());
			EvolutionDefinition->EvolutionId = EvolutionId;
			EvolutionDefinition->FromCardId = FromCardId;
			EvolutionDefinition->ToCardId = ToCardId;
			EvolutionDefinition->RequiredOwnerCharacterId = CharacterId;
			EvolutionDefinition->DisplayName = FText::FromString(DisplayName);
			EvolutionDefinition->Description = FText::FromString(TEXT("Evolve this card."));
			DataRegistry->RegisterCardEvolutionDefinition(EvolutionDefinition);
			return EvolutionDefinition;
		}

		UFinalRunSession* CreateRunSessionWithSingleCharacter(
			const FFinalCharacterId& CharacterId,
			const FFinalCardId& StarterCardId) const
		{
			UFinalRunSession* RunSession = NewObject<UFinalRunSession>(GameInstance.Get());
			RunSession->InitializeRun();

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

		TStrongObjectPtr<UGameInstance> GameInstance;
		UFinalDataRegistry* DataRegistry = nullptr;
	};

	bool ContainsChoiceType(const TArray<FFinalRunGrowthChoiceInstance>& Choices, const EFinalGrowthChoiceType ChoiceType)
	{
		return Choices.ContainsByPredicate([ChoiceType](const FFinalRunGrowthChoiceInstance& Choice)
		{
			return Choice.ChoiceType == ChoiceType;
		});
	}

	int32 CountChoiceType(const TArray<FFinalRunGrowthChoiceInstance>& Choices, const EFinalGrowthChoiceType ChoiceType)
	{
		int32 Count = 0;
		for (const FFinalRunGrowthChoiceInstance& Choice : Choices)
		{
			if (Choice.ChoiceType == ChoiceType)
			{
				++Count;
			}
		}

		return Count;
	}

	FFinalRunCommand MakeSelectGrowthChoiceCommand(const FName ChoiceInstanceId)
	{
		FFinalRunCommand Command;
		Command.CommandType = EFinalRunCommandType::SelectGrowthChoice;
		Command.PayloadId = ChoiceInstanceId;
		return Command;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalRunGrowthAttributeChoicesTest,
	"Final.Editor.RunGrowth.AttributeChoicesWhenNoEvolutionAvailable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalRunGrowthAttributeChoicesTest::RunTest(const FString& Parameters)
{
	using namespace FinalRunGrowthStateTests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalRunGrowthAttributeChoicesTest")))
	{
		return false;
	}

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.huo")));
	const FFinalCardId StarterCardId(FName(TEXT("card.test.base")));
	Context.RegisterCardDefinition(StarterCardId, CharacterId, TEXT("Starter Card"));

	UFinalRunSession* RunSession = Context.CreateRunSessionWithSingleCharacter(CharacterId, StarterCardId);
	if (!TestNotNull(TEXT("RunSession should be created for attribute-only growth test."), RunSession))
	{
		return false;
	}

	TestTrue(TEXT("Breakthrough gain should be accepted for a valid character."), RunSession->AddBreakthroughValue(CharacterId, 100));

	const FFinalRunState RunState = RunSession->GetRunState();
	const FFinalRunPersistentCharacterState* CharacterState = RunState.Characters.FindByPredicate([&CharacterId](const FFinalRunPersistentCharacterState& State)
	{
		return State.CharacterId == CharacterId;
	});
	if (!TestNotNull(TEXT("Character state should still be present after breakthrough gain."), CharacterState))
	{
		return false;
	}

	TestEqual(TEXT("Character should level up once when breakthrough reaches the threshold."), CharacterState->Level, 2);
	TestEqual(TEXT("Breakthrough value should spend one threshold when leveling up."), CharacterState->BreakthroughValue, 0);
	TestTrue(TEXT("Character should be marked as waiting for a growth choice."), CharacterState->bHasPendingGrowthChoice);
	TestTrue(TEXT("RunSession should expose a pending growth choice after leveling up."), RunSession->HasPendingGrowthChoice());
	TestEqual(TEXT("Pending growth choice should target the leveled character."), RunState.PendingGrowthChoice.CharacterId.Value, CharacterId.Value);
	TestEqual(TEXT("Attribute-only growth generation should still produce three choices."), RunState.PendingGrowthChoice.Choices.Num(), 3);
	TestEqual(TEXT("All generated choices should be attribute growth when no evolution is available."), CountChoiceType(RunState.PendingGrowthChoice.Choices, EFinalGrowthChoiceType::AttributeGrowth), 3);
	TestEqual(TEXT("Run deck size should remain unchanged while choices are pending."), RunState.RunDeck.Num(), 1);
	TestEqual(TEXT("Run deck should not change card ids before a choice is applied."), RunState.RunDeck[0].CurrentCardId.Value, StarterCardId.Value);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalRunGrowthEvolutionChoiceTest,
	"Final.Editor.RunGrowth.EvolutionChoiceGeneratedAndBreakthroughAccumulatesWhilePending",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalRunGrowthEvolutionChoiceTest::RunTest(const FString& Parameters)
{
	using namespace FinalRunGrowthStateTests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalRunGrowthEvolutionChoiceTest")))
	{
		return false;
	}

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.huo")));
	const FFinalCardId StarterCardId(FName(TEXT("card.test.base")));
	const FFinalCardId EvolvedCardId(FName(TEXT("card.test.evolved")));
	const FFinalCardEvolutionId EvolutionId(FName(TEXT("evolution.test.base_to_evolved")));

	Context.RegisterCardDefinition(StarterCardId, CharacterId, TEXT("Starter Card"));
	Context.RegisterCardDefinition(EvolvedCardId, CharacterId, TEXT("Evolved Card"));
	Context.RegisterEvolutionDefinition(EvolutionId, StarterCardId, EvolvedCardId, CharacterId, TEXT("Evolve Starter Card"));

	UFinalRunSession* RunSession = Context.CreateRunSessionWithSingleCharacter(CharacterId, StarterCardId);
	if (!TestNotNull(TEXT("RunSession should be created for evolution growth test."), RunSession))
	{
		return false;
	}

	TestTrue(TEXT("Breakthrough gain should be accepted for a valid character."), RunSession->AddBreakthroughValue(CharacterId, 100));

	const FFinalRunState FirstRunState = RunSession->GetRunState();
	TestTrue(TEXT("Growth generation should produce a pending choice set."), FirstRunState.PendingGrowthChoice.bIsValid);
	TestEqual(TEXT("Growth generation should produce exactly three choices."), FirstRunState.PendingGrowthChoice.Choices.Num(), 3);
	TestTrue(TEXT("An eligible evolution should appear as one of the generated choices."), ContainsChoiceType(FirstRunState.PendingGrowthChoice.Choices, EFinalGrowthChoiceType::CardEvolution));

	const FFinalRunGrowthChoiceInstance* EvolutionChoice = FirstRunState.PendingGrowthChoice.Choices.FindByPredicate([](const FFinalRunGrowthChoiceInstance& Choice)
	{
		return Choice.ChoiceType == EFinalGrowthChoiceType::CardEvolution;
	});
	if (!TestNotNull(TEXT("The generated pending choice set should contain the evolution entry."), EvolutionChoice))
	{
		return false;
	}

	TestEqual(TEXT("Evolution choice should reference the expected evolution definition."), EvolutionChoice->CardEvolutionId.Value, EvolutionId.Value);
	TestEqual(TEXT("Evolution choice should capture the source card id."), EvolutionChoice->FromCardId.Value, StarterCardId.Value);
	TestEqual(TEXT("Evolution choice should capture the destination card id."), EvolutionChoice->ToCardId.Value, EvolvedCardId.Value);
	TestEqual(TEXT("Pending evolution should target the current run card instance."), EvolutionChoice->TargetRunCardInstanceId, FirstRunState.RunDeck[0].InstanceId);
	TestEqual(TEXT("Run deck should remain unchanged until a growth choice is applied."), FirstRunState.RunDeck[0].CurrentCardId.Value, StarterCardId.Value);

	TestTrue(TEXT("Breakthrough should continue accumulating while a pending growth choice exists."), RunSession->AddBreakthroughValue(CharacterId, 30));
	const FFinalRunState SecondRunState = RunSession->GetRunState();
	const FFinalRunPersistentCharacterState* CharacterState = SecondRunState.Characters.FindByPredicate([&CharacterId](const FFinalRunPersistentCharacterState& State)
	{
		return State.CharacterId == CharacterId;
	});
	if (!TestNotNull(TEXT("Character state should still be present after accumulating breakthrough behind a pending choice."), CharacterState))
	{
		return false;
	}

	TestEqual(TEXT("Character level should not advance again while a pending choice blocks additional level-ups."), CharacterState->Level, 2);
	TestEqual(TEXT("Breakthrough should accumulate even when the pending growth choice remains unresolved."), CharacterState->BreakthroughValue, 30);
	TestTrue(TEXT("Pending growth choice should remain valid after additional breakthrough gain."), SecondRunState.PendingGrowthChoice.bIsValid);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalRunGrowthSelectWithoutPendingTest,
	"Final.Editor.RunGrowth.SelectGrowthChoiceRejectsWithoutPending",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalRunGrowthSelectWithoutPendingTest::RunTest(const FString& Parameters)
{
	using namespace FinalRunGrowthStateTests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalRunGrowthSelectWithoutPendingTest")))
	{
		return false;
	}

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.huo")));
	const FFinalCardId StarterCardId(FName(TEXT("card.test.base")));
	Context.RegisterCardDefinition(StarterCardId, CharacterId, TEXT("Starter Card"));

	UFinalRunSession* RunSession = Context.CreateRunSessionWithSingleCharacter(CharacterId, StarterCardId);
	if (!TestNotNull(TEXT("RunSession should be created for select-without-pending test."), RunSession))
	{
		return false;
	}

	TestFalse(TEXT("Selecting a growth choice should fail when there is no pending choice set."), RunSession->SubmitRunCommand(MakeSelectGrowthChoiceCommand(TEXT("GrowthChoice_Missing"))));

	const TArray<FFinalRunEvent> RunLogEntries = RunSession->GetRunLogEntries();
	const FFinalRunEvent* RejectEvent = RunLogEntries.FindByPredicate([](const FFinalRunEvent& Event)
	{
		return Event.EventType == EFinalRunEventType::RunCommandRejected
			&& Event.CommandType == EFinalRunCommandType::SelectGrowthChoice;
	});

	if (!TestNotNull(TEXT("Selecting without pending growth should emit a rejected command event."), RejectEvent))
	{
		return false;
	}

	TestEqual(TEXT("Reject reason should explain that no pending growth choice exists."), RejectEvent->RejectReason, EFinalRunCommandRejectReason::MissingPendingGrowthChoice);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalRunGrowthSelectUnknownChoiceTest,
	"Final.Editor.RunGrowth.SelectGrowthChoiceRejectsUnknownChoice",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalRunGrowthSelectUnknownChoiceTest::RunTest(const FString& Parameters)
{
	using namespace FinalRunGrowthStateTests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalRunGrowthSelectUnknownChoiceTest")))
	{
		return false;
	}

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.huo")));
	const FFinalCardId StarterCardId(FName(TEXT("card.test.base")));
	Context.RegisterCardDefinition(StarterCardId, CharacterId, TEXT("Starter Card"));

	UFinalRunSession* RunSession = Context.CreateRunSessionWithSingleCharacter(CharacterId, StarterCardId);
	if (!TestNotNull(TEXT("RunSession should be created for unknown-choice test."), RunSession))
	{
		return false;
	}

	TestTrue(TEXT("Breakthrough gain should create a pending growth choice for unknown-choice rejection test."), RunSession->AddBreakthroughValue(CharacterId, 100));
	const FFinalRunState BeforeRejectState = RunSession->GetRunState();
	TestTrue(TEXT("The setup should produce a valid pending choice set."), BeforeRejectState.PendingGrowthChoice.bIsValid);

	TestFalse(TEXT("Selecting an unknown choice id should fail."), RunSession->SubmitRunCommand(MakeSelectGrowthChoiceCommand(TEXT("GrowthChoice_Unknown"))));

	const FFinalRunState AfterRejectState = RunSession->GetRunState();
	TestTrue(TEXT("Pending choice should remain valid after rejecting an unknown choice id."), AfterRejectState.PendingGrowthChoice.bIsValid);
	TestEqual(TEXT("Pending choice count should remain unchanged after rejection."), AfterRejectState.PendingGrowthChoice.Choices.Num(), BeforeRejectState.PendingGrowthChoice.Choices.Num());

	const TArray<FFinalRunEvent> RunLogEntries = RunSession->GetRunLogEntries();
	const FFinalRunEvent* RejectEvent = RunLogEntries.FindByPredicate([](const FFinalRunEvent& Event)
	{
		return Event.EventType == EFinalRunEventType::RunCommandRejected
			&& Event.CommandType == EFinalRunCommandType::SelectGrowthChoice;
	});

	if (!TestNotNull(TEXT("Rejecting an unknown choice should emit a rejected command event."), RejectEvent))
	{
		return false;
	}

	TestEqual(TEXT("Reject reason should be UnknownGrowthChoice."), RejectEvent->RejectReason, EFinalRunCommandRejectReason::UnknownGrowthChoice);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalRunGrowthApplyAttributeChoiceTest,
	"Final.Editor.RunGrowth.ApplyAttributeChoiceClearsPending",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalRunGrowthApplyAttributeChoiceTest::RunTest(const FString& Parameters)
{
	using namespace FinalRunGrowthStateTests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalRunGrowthApplyAttributeChoiceTest")))
	{
		return false;
	}

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.huo")));
	const FFinalCardId StarterCardId(FName(TEXT("card.test.base")));
	Context.RegisterCardDefinition(StarterCardId, CharacterId, TEXT("Starter Card"));

	UFinalRunSession* RunSession = Context.CreateRunSessionWithSingleCharacter(CharacterId, StarterCardId);
	if (!TestNotNull(TEXT("RunSession should be created for attribute-apply test."), RunSession))
	{
		return false;
	}

	TestTrue(TEXT("Breakthrough gain should create a pending growth choice for attribute apply."), RunSession->AddBreakthroughValue(CharacterId, 100));
	const FFinalRunSnapshot PendingSnapshot = RunSession->GetSnapshot();
	TestTrue(TEXT("Snapshot should expose a pending growth choice view."), PendingSnapshot.PendingGrowthChoice.bHasPendingChoice);

	const FFinalRunGrowthChoiceInstance* AttributeChoice = PendingSnapshot.PendingGrowthChoice.Choices.FindByPredicate([](const FFinalRunGrowthChoiceInstance& Choice)
	{
		return Choice.ChoiceType == EFinalGrowthChoiceType::AttributeGrowth
			&& Choice.AttributeType == EFinalGrowthAttributeType::RootBone;
	});
	if (!TestNotNull(TEXT("Pending growth choice set should contain the Root Bone attribute choice."), AttributeChoice))
	{
		return false;
	}

	TestTrue(TEXT("Selecting the Root Bone growth choice should succeed."), RunSession->SubmitRunCommand(MakeSelectGrowthChoiceCommand(AttributeChoice->ChoiceInstanceId)));

	const FFinalRunState RunState = RunSession->GetRunState();
	const FFinalRunPersistentCharacterState* CharacterState = RunState.Characters.FindByPredicate([&CharacterId](const FFinalRunPersistentCharacterState& State)
	{
		return State.CharacterId == CharacterId;
	});
	if (!TestNotNull(TEXT("Character state should still be present after applying the growth choice."), CharacterState))
	{
		return false;
	}

	TestEqual(TEXT("Root Bone should increase by exactly one when that attribute choice is selected."), CharacterState->RootBone, 1);
	TestFalse(TEXT("Character should no longer be marked as waiting for a growth choice after applying one."), CharacterState->bHasPendingGrowthChoice);
	TestFalse(TEXT("RunState pending growth choice should be cleared after applying one."), RunState.PendingGrowthChoice.bIsValid);
	TestEqual(TEXT("RunDeck should remain unchanged after applying an attribute choice."), RunState.RunDeck[0].CurrentCardId.Value, StarterCardId.Value);

	const FFinalRunSnapshot ResolvedSnapshot = RunSession->GetSnapshot();
	TestFalse(TEXT("Snapshot should clear the pending growth choice view after application."), ResolvedSnapshot.PendingGrowthChoice.bHasPendingChoice);
	TestEqual(TEXT("Snapshot character view should reflect the updated Root Bone value."), ResolvedSnapshot.Characters[0].RootBone, 1);

	const TArray<FFinalRunEvent> RunLogEntries = RunSession->GetRunLogEntries();
	const FFinalRunEvent* AppliedEvent = RunLogEntries.FindByPredicate([](const FFinalRunEvent& Event)
	{
		return Event.EventType == EFinalRunEventType::GrowthChoiceApplied;
	});
	if (!TestNotNull(TEXT("Applying a growth choice should emit a GrowthChoiceApplied detail event."), AppliedEvent))
	{
		return false;
	}

	TestEqual(TEXT("Applied event should target the correct character."), AppliedEvent->TargetCharacterId.Value, CharacterId.Value);
	TestEqual(TEXT("Applied event should report the selected choice id in PayloadId."), AppliedEvent->PayloadId, AttributeChoice->ChoiceInstanceId);
	TestEqual(TEXT("Applied event should include one affected character result."), AppliedEvent->AffectedCharacterResults.Num(), 1);
	TestEqual(TEXT("Affected character result should reflect the updated Root Bone value."), AppliedEvent->AffectedCharacterResults[0].RootBone, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalRunGrowthApplyEvolutionChoiceTest,
	"Final.Editor.RunGrowth.ApplyEvolutionChoiceUpdatesRunDeckWithoutAutoRollover",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalRunGrowthApplyEvolutionChoiceTest::RunTest(const FString& Parameters)
{
	using namespace FinalRunGrowthStateTests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalRunGrowthApplyEvolutionChoiceTest")))
	{
		return false;
	}

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.huo")));
	const FFinalCardId StarterCardId(FName(TEXT("card.test.base")));
	const FFinalCardId EvolvedCardId(FName(TEXT("card.test.evolved")));
	const FFinalCardEvolutionId EvolutionId(FName(TEXT("evolution.test.base_to_evolved")));

	Context.RegisterCardDefinition(StarterCardId, CharacterId, TEXT("Starter Card"));
	Context.RegisterCardDefinition(EvolvedCardId, CharacterId, TEXT("Evolved Card"));
	Context.RegisterEvolutionDefinition(EvolutionId, StarterCardId, EvolvedCardId, CharacterId, TEXT("Evolve Starter Card"));

	UFinalRunSession* RunSession = Context.CreateRunSessionWithSingleCharacter(CharacterId, StarterCardId);
	if (!TestNotNull(TEXT("RunSession should be created for evolution-apply test."), RunSession))
	{
		return false;
	}

	TestTrue(TEXT("Breakthrough gain should create the first pending choice set."), RunSession->AddBreakthroughValue(CharacterId, 100));
	TestTrue(TEXT("Breakthrough should continue accumulating behind the pending choice."), RunSession->AddBreakthroughValue(CharacterId, 130));

	const FFinalRunState PendingState = RunSession->GetRunState();
	const FFinalRunGrowthChoiceInstance* EvolutionChoice = PendingState.PendingGrowthChoice.Choices.FindByPredicate([](const FFinalRunGrowthChoiceInstance& Choice)
	{
		return Choice.ChoiceType == EFinalGrowthChoiceType::CardEvolution;
	});
	if (!TestNotNull(TEXT("Pending growth choice set should contain an evolution choice."), EvolutionChoice))
	{
		return false;
	}

	TestTrue(TEXT("Selecting the evolution growth choice should succeed."), RunSession->SubmitRunCommand(MakeSelectGrowthChoiceCommand(EvolutionChoice->ChoiceInstanceId)));

	const FFinalRunState ResolvedState = RunSession->GetRunState();
	const FFinalRunPersistentCharacterState* CharacterState = ResolvedState.Characters.FindByPredicate([&CharacterId](const FFinalRunPersistentCharacterState& State)
	{
		return State.CharacterId == CharacterId;
	});
	if (!TestNotNull(TEXT("Character state should still be present after applying evolution."), CharacterState))
	{
		return false;
	}

	TestFalse(TEXT("Pending choice should be cleared after applying the evolution option."), ResolvedState.PendingGrowthChoice.bIsValid);
	TestFalse(TEXT("Character should no longer be marked as waiting for a growth choice after applying evolution."), CharacterState->bHasPendingGrowthChoice);
	TestEqual(TEXT("Stored breakthrough should remain after applying the current choice without auto-rollover."), CharacterState->BreakthroughValue, 130);
	TestEqual(TEXT("Character level should remain unchanged after applying the already-earned choice."), CharacterState->Level, 2);
	TestEqual(TEXT("RunDeck should still contain one card instance."), ResolvedState.RunDeck.Num(), 1);
	TestEqual(TEXT("Card evolution should preserve BaseCardId."), ResolvedState.RunDeck[0].BaseCardId.Value, StarterCardId.Value);
	TestEqual(TEXT("Card evolution should update CurrentCardId."), ResolvedState.RunDeck[0].CurrentCardId.Value, EvolvedCardId.Value);
	TestEqual(TEXT("Card evolution should advance the evolution stage."), ResolvedState.RunDeck[0].EvolutionStage, EFinalCardEvolutionStage::Evolved);

	const FFinalRunSnapshot ResolvedSnapshot = RunSession->GetSnapshot();
	TestFalse(TEXT("Snapshot should not generate a new pending growth choice automatically after apply."), ResolvedSnapshot.PendingGrowthChoice.bHasPendingChoice);
	TestEqual(TEXT("Snapshot current build should now reflect the evolved card id."), ResolvedSnapshot.CurrentBuild.DeckEntries[0].CardId.Value, EvolvedCardId.Value);
	return true;
}

#endif
