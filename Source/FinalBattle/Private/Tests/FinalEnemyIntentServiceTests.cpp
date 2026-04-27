#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Battle/Definitions/FinalEnemyIntentDefinition.h"
#include "Runtime/FinalBattleEnemyState.h"
#include "Systems/FinalEnemyIntentService.h"
#include "UObject/StrongObjectPtr.h"

namespace FinalEnemyIntentServiceTests
{
	TStrongObjectPtr<UFinalEnemyIntentDefinition> MakeIntent(
		const FName IntentId,
		const int32 Weight = 1,
		const TArray<FName>& PhaseTags = TArray<FName>())
	{
		TStrongObjectPtr<UFinalEnemyIntentDefinition> Intent(NewObject<UFinalEnemyIntentDefinition>(GetTransientPackage()));
		Intent->IntentId = IntentId;
		Intent->DisplayName = FText::FromName(IntentId);
		Intent->PreviewText = FText::FromName(IntentId);
		Intent->Weight = Weight;
		Intent->PhaseTags = PhaseTags;
		return Intent;
	}

	void AddIntent(FFinalBattleEnemyState& EnemyState, UFinalEnemyIntentDefinition* IntentDefinition, const int32 NextAvailableRound = 1)
	{
		FFinalBattleEnemyIntentRuntimeState& RuntimeIntent = EnemyState.IntentRuntimeStates.AddDefaulted_GetRef();
		RuntimeIntent.Definition = IntentDefinition;
		RuntimeIntent.IntentId = IntentDefinition != nullptr ? IntentDefinition->IntentId : NAME_None;
		RuntimeIntent.NextAvailableRound = NextAvailableRound;
	}

	FFinalBattleEnemyState MakeEnemyState(const EFinalIntentSelectRule SelectRule)
	{
		FFinalBattleEnemyState EnemyState;
		EnemyState.RuntimeUnitId = TEXT("enemy_test");
		EnemyState.MaxHP = 100;
		EnemyState.CurrentHP = 100;
		EnemyState.RuntimeDamagePower = 10;
		EnemyState.IntentSelectRule = SelectRule;
		return EnemyState;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalEnemyIntentServiceCycleTest,
	"Final.Battle.EnemyIntent.CycleRespectsCooldownAndRepeat",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalEnemyIntentServiceCycleTest::RunTest(const FString& Parameters)
{
	using namespace FinalEnemyIntentServiceTests;

	TStrongObjectPtr<UFinalEnemyIntentDefinition> RepeatBlockedIntent = MakeIntent(TEXT("intent.repeat_blocked"));
	RepeatBlockedIntent->bDisallowRepeatLastIntent = true;
	TStrongObjectPtr<UFinalEnemyIntentDefinition> AvailableIntent = MakeIntent(TEXT("intent.available"));
	TStrongObjectPtr<UFinalEnemyIntentDefinition> CooldownIntent = MakeIntent(TEXT("intent.cooldown"));

	FFinalBattleEnemyState EnemyState = MakeEnemyState(EFinalIntentSelectRule::Cycle);
	EnemyState.LastExecutedIntentId = RepeatBlockedIntent->IntentId;
	AddIntent(EnemyState, RepeatBlockedIntent.Get());
	AddIntent(EnemyState, CooldownIntent.Get(), 2);
	AddIntent(EnemyState, AvailableIntent.Get());

	const FFinalEnemyIntentService Service;
	Service.RefreshIntent(EnemyState, 1);

	TestEqual(TEXT("Cycle should skip repeat-blocked and cooldown intents."), EnemyState.CurrentIntentId, AvailableIntent->IntentId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalEnemyIntentServiceWeightedRandomTest,
	"Final.Battle.EnemyIntent.WeightedRandomIgnoresZeroWeight",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalEnemyIntentServiceWeightedRandomTest::RunTest(const FString& Parameters)
{
	using namespace FinalEnemyIntentServiceTests;

	TStrongObjectPtr<UFinalEnemyIntentDefinition> ZeroWeightIntent = MakeIntent(TEXT("intent.zero_weight"), 0);
	TStrongObjectPtr<UFinalEnemyIntentDefinition> WeightedIntent = MakeIntent(TEXT("intent.weighted"), 1);

	FFinalBattleEnemyState EnemyState = MakeEnemyState(EFinalIntentSelectRule::WeightedRandom);
	AddIntent(EnemyState, ZeroWeightIntent.Get());
	AddIntent(EnemyState, WeightedIntent.Get());

	const FFinalEnemyIntentService Service;
	Service.RefreshIntent(EnemyState, 1);

	TestEqual(TEXT("Weighted random should not select zero-weight intents."), EnemyState.CurrentIntentId, WeightedIntent->IntentId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalEnemyIntentServicePhaseSequenceTest,
	"Final.Battle.EnemyIntent.PhaseSequencePrefersCurrentPhase",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalEnemyIntentServicePhaseSequenceTest::RunTest(const FString& Parameters)
{
	using namespace FinalEnemyIntentServiceTests;

	const FName PhaseOne(TEXT("phase.one"));
	const FName PhaseTwo(TEXT("phase.two"));
	TStrongObjectPtr<UFinalEnemyIntentDefinition> PhaseOneIntent = MakeIntent(TEXT("intent.phase_one"), 1, { PhaseOne });
	TStrongObjectPtr<UFinalEnemyIntentDefinition> PhaseTwoIntent = MakeIntent(TEXT("intent.phase_two"), 1, { PhaseTwo });

	FFinalBattleEnemyState EnemyState = MakeEnemyState(EFinalIntentSelectRule::PhaseSequence);
	EnemyState.CurrentHP = 40;
	EnemyState.PhaseSequence.Add({ PhaseOne, 1.0f });
	EnemyState.PhaseSequence.Add({ PhaseTwo, 0.5f });
	AddIntent(EnemyState, PhaseOneIntent.Get());
	AddIntent(EnemyState, PhaseTwoIntent.Get());

	const FFinalEnemyIntentService Service;
	Service.RefreshIntent(EnemyState, 1);

	TestEqual(TEXT("PhaseSequence should prefer the current phase's intent."), EnemyState.CurrentIntentId, PhaseTwoIntent->IntentId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalEnemyIntentServiceScriptedTest,
	"Final.Battle.EnemyIntent.ScriptedSelectsConfiguredStepAndFallsBack",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalEnemyIntentServiceScriptedTest::RunTest(const FString& Parameters)
{
	using namespace FinalEnemyIntentServiceTests;

	TStrongObjectPtr<UFinalEnemyIntentDefinition> ScriptedIntent = MakeIntent(TEXT("intent.scripted"));
	ScriptedIntent->MinPreviewRound = 2;
	TStrongObjectPtr<UFinalEnemyIntentDefinition> FallbackIntent = MakeIntent(TEXT("intent.fallback"));

	FFinalBattleEnemyState EnemyState = MakeEnemyState(EFinalIntentSelectRule::Scripted);
	EnemyState.ScriptedIntentSequence.Add({ ScriptedIntent->IntentId, NAME_None, false });
	AddIntent(EnemyState, ScriptedIntent.Get());
	AddIntent(EnemyState, FallbackIntent.Get());

	const FFinalEnemyIntentService Service;
	Service.RefreshIntent(EnemyState, 2);
	TestEqual(TEXT("Scripted should select the configured first step."), EnemyState.CurrentIntentId, ScriptedIntent->IntentId);

	Service.RefreshIntent(EnemyState, 1);
	TestEqual(TEXT("Scripted should use fallback when configured step is unavailable."), EnemyState.CurrentIntentId, FallbackIntent->IntentId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalEnemyIntentServiceCommitTest,
	"Final.Battle.EnemyIntent.CommitUpdatesRuntimeCounters",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalEnemyIntentServiceCommitTest::RunTest(const FString& Parameters)
{
	using namespace FinalEnemyIntentServiceTests;

	TStrongObjectPtr<UFinalEnemyIntentDefinition> Intent = MakeIntent(TEXT("intent.commit"));
	Intent->CooldownTurns = 1;

	FFinalBattleEnemyState EnemyState = MakeEnemyState(EFinalIntentSelectRule::Cycle);
	AddIntent(EnemyState, Intent.Get());

	const FFinalEnemyIntentService Service;
	Service.RefreshIntent(EnemyState, 1);
	Service.CommitCurrentIntentExecution(EnemyState, 1);

	TestEqual(TEXT("Intent use count should advance."), EnemyState.IntentRuntimeStates[0].UseCount, 1);
	TestEqual(TEXT("Cooldown should set next available round."), EnemyState.IntentRuntimeStates[0].NextAvailableRound, 3);
	TestEqual(TEXT("Last executed intent should be recorded."), EnemyState.LastExecutedIntentId, Intent->IntentId);
	TestEqual(TEXT("Intent execution count should advance."), EnemyState.IntentExecutionCount, 1);
	TestEqual(TEXT("Consecutive use count should advance."), EnemyState.ConsecutiveIntentUseCount, 1);
	return true;
}

#endif
