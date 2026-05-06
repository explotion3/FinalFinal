#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalEnemyIntentDefinition.h"
#include "GameplayTagContainer.h"
#include "Runtime/FinalBattleEnemyState.h"
#include "Runtime/FinalBattleState.h"
#include "Systems/FinalBattleEffectExecutionService.h"
#include "Systems/FinalBattleEnemyActionService.h"
#include "Systems/FinalBattleInitiativeService.h"
#include "Systems/FinalBattleTriggerService.h"
#include "Systems/FinalBattleUnitService.h"
#include "UObject/StrongObjectPtr.h"

namespace FinalBattleInitiativeServiceTests
{
	const FName EnemyAUnitId(TEXT("enemy_a"));
	const FName EnemyBUnitId(TEXT("enemy_b"));

	FFinalBattleEnemyState MakeEnemy(
		const FName RuntimeUnitId,
		const int32 PositionIndex,
		const int32 InitialInitiative,
		const int32 InitiativeResponse,
		const int32 RuntimeDamagePower = 0)
	{
		FFinalBattleEnemyState EnemyState;
		EnemyState.RuntimeUnitId = RuntimeUnitId;
		EnemyState.DisplayName = FText::FromName(RuntimeUnitId);
		EnemyState.PositionIndex = PositionIndex;
		EnemyState.MaxHP = 20;
		EnemyState.CurrentHP = 20;
		EnemyState.MaxBreakValue = 10;
		EnemyState.CurrentBreakValue = 10;
		EnemyState.InitialInitiative = InitialInitiative;
		EnemyState.CurrentInitiative = InitialInitiative;
		EnemyState.InitiativeResponse = InitiativeResponse;
		EnemyState.InitiativeState = EFinalEnemyInitiativeState::Counting;
		EnemyState.MaxActionsPerRound = 1;
		EnemyState.RuntimeDamagePower = RuntimeDamagePower;
		return EnemyState;
	}

	TStrongObjectPtr<UFinalBattleRuleConfig> MakeRuleConfig()
	{
		TStrongObjectPtr<UFinalBattleRuleConfig> RuleConfig(NewObject<UFinalBattleRuleConfig>(GetTransientPackage()));
		RuleConfig->NormalCardInitiativeEventCount = 1;
		RuleConfig->CollapsedCardInitiativeEventCount = 1;
		RuleConfig->bUltimateTriggersInitiativeEvent = false;
		return RuleConfig;
	}

	FFinalBattleResolvedCardTriggerContext MakeCardContext(const bool bFast = false)
	{
		FFinalBattleResolvedCardTriggerContext Context;
		Context.RuntimeOwnerUnitId = TEXT("player_0");
		Context.CardType = EFinalCardType::Attack;
		if (bFast)
		{
			Context.RuntimeKeywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Fast")));
		}
		return Context;
	}

	const FFinalBattleInitiativeService& GetInitiativeService()
	{
		static const FFinalBattleInitiativeService Service;
		return Service;
	}

	const FFinalBattleEnemyActionService& GetEnemyActionService()
	{
		static const FFinalBattleEnemyActionService Service;
		return Service;
	}

	const FFinalBattleUnitService& GetUnitService()
	{
		static const FFinalBattleUnitService Service;
		return Service;
	}

	const FFinalBattleTriggerService& GetTriggerService()
	{
		static const FFinalBattleTriggerService Service;
		return Service;
	}

	const FFinalBattleEffectExecutionService& GetEffectExecutionService()
	{
		static const FFinalBattleEffectExecutionService Service;
		return Service;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleInitiativeNormalCardReducesAllEnemiesTest,
	"Final.Battle.EnemyIntent.Initiative.NormalCardReducesAllEnemies",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleInitiativeNormalCardReducesAllEnemiesTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattleInitiativeServiceTests;

	FFinalBattleState State;
	State.TeamCurrentHP = 30;
	State.TeamMaxHP = 30;
	State.Enemies.Add(MakeEnemy(EnemyAUnitId, 0, 3, 1));
	State.Enemies.Add(MakeEnemy(EnemyBUnitId, 1, 4, 2));

	const FFinalBattleEnemyActionSequenceResult Result = GetInitiativeService().ResolveCardInitiativeEvents(
		State,
		MakeRuleConfig().Get(),
		MakeCardContext(),
		false,
		GetEnemyActionService(),
		GetUnitService(),
		GetTriggerService(),
		GetEffectExecutionService());

	TestEqual(TEXT("Enemy A initiative should reduce by response."), State.Enemies[0].CurrentInitiative, 2);
	TestEqual(TEXT("Enemy B initiative should reduce by response."), State.Enemies[1].CurrentInitiative, 2);
	TestEqual(TEXT("No enemy should act when initiative remains above zero."), Result.ResolvedEffectCount, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleInitiativeFastCardSkipsReductionTest,
	"Final.Battle.EnemyIntent.Initiative.FastCardSkipsReduction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleInitiativeFastCardSkipsReductionTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattleInitiativeServiceTests;

	FFinalBattleState State;
	State.TeamCurrentHP = 30;
	State.TeamMaxHP = 30;
	State.Enemies.Add(MakeEnemy(EnemyAUnitId, 0, 3, 1));

	GetInitiativeService().ResolveCardInitiativeEvents(
		State,
		MakeRuleConfig().Get(),
		MakeCardContext(true),
		false,
		GetEnemyActionService(),
		GetUnitService(),
		GetTriggerService(),
		GetEffectExecutionService());

	TestEqual(TEXT("Fast cards should not reduce initiative."), State.Enemies[0].CurrentInitiative, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleInitiativeQueueActsAndSpendsActionTest,
	"Final.Battle.EnemyIntent.Initiative.QueueActsAndSpendsAction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleInitiativeQueueActsAndSpendsActionTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattleInitiativeServiceTests;

	FFinalBattleState State;
	State.TeamCurrentHP = 30;
	State.TeamMaxHP = 30;
	State.Enemies.Add(MakeEnemy(EnemyAUnitId, 0, 1, 1, 3));

	const FFinalBattleEnemyActionSequenceResult Result = GetInitiativeService().ResolveCardInitiativeEvents(
		State,
		MakeRuleConfig().Get(),
		MakeCardContext(),
		false,
		GetEnemyActionService(),
		GetUnitService(),
		GetTriggerService(),
		GetEffectExecutionService());

	TestEqual(TEXT("Queued enemy should damage the team through fallback action."), Result.TotalDamageToTeam, 3);
	TestEqual(TEXT("Team HP should be reduced by the queued action."), State.TeamCurrentHP, 27);
	TestEqual(TEXT("Enemy should spend its action opportunity."), State.Enemies[0].InitiativeState, EFinalEnemyInitiativeState::ActionSpent);
	TestEqual(TEXT("Enemy action count should advance."), State.Enemies[0].ActionsTakenThisRound, 1);
	TestTrue(TEXT("Queued action should emit EnemyActed."), Result.GeneratedEvents.ContainsByPredicate(
		[](const FFinalBattleEvent& Event)
		{
			return Event.EventType == EFinalBattleEventType::EnemyActed;
		}));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleInitiativeBreakSkipActionOverrideTest,
	"Final.Battle.EnemyIntent.Initiative.BreakSkipActionOverride",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleInitiativeBreakSkipActionOverrideTest::RunTest(const FString& Parameters)
{
	using namespace FinalBattleInitiativeServiceTests;

	FFinalBattleState State;
	State.TeamCurrentHP = 30;
	State.TeamMaxHP = 30;
	State.Enemies.Add(MakeEnemy(EnemyAUnitId, 0, 1, 1, 5));
	FFinalBattleEnemyState& EnemyState = State.Enemies[0];
	EnemyState.CurrentBreakValue = 0;

	TStrongObjectPtr<UFinalEnemyIntentDefinition> Intent(NewObject<UFinalEnemyIntentDefinition>(GetTransientPackage()));
	Intent->IntentId = TEXT("intent.test.skip_should_not_commit");
	FFinalBattleEnemyIntentRuntimeState& RuntimeIntent = EnemyState.IntentRuntimeStates.AddDefaulted_GetRef();
	RuntimeIntent.Definition = Intent.Get();
	RuntimeIntent.IntentId = Intent->IntentId;
	EnemyState.CurrentIntentDefinition = Intent.Get();
	EnemyState.CurrentIntentId = Intent->IntentId;
	EnemyState.CurrentIntentIndex = 0;

	GetInitiativeService().ApplyBreakSkipActionOverride(EnemyState, 10);
	GetInitiativeService().ResolveCardInitiativeEvents(
		State,
		MakeRuleConfig().Get(),
		MakeCardContext(),
		false,
		GetEnemyActionService(),
		GetUnitService(),
		GetTriggerService(),
		GetEffectExecutionService());

	TestEqual(TEXT("Break skip override should prevent initiative response."), EnemyState.CurrentInitiative, 1);
	TestEqual(TEXT("Break skip override should remain until enemy action phase."), EnemyState.ActionOverrideType, EFinalEnemyActionOverrideType::SkipNextAction);

	const FFinalBattleEnemyActionSequenceResult EndTurnResult = GetInitiativeService().ResolveEndTurnEnemyActions(
		State,
		GetEnemyActionService(),
		GetUnitService(),
		GetTriggerService(),
		GetEffectExecutionService());

	TestEqual(TEXT("Skipped action should not damage team."), EndTurnResult.TotalDamageToTeam, 0);
	TestEqual(TEXT("Skip override should clear after resolving."), EnemyState.ActionOverrideType, EFinalEnemyActionOverrideType::None);
	TestEqual(TEXT("Break should recover after the skipped action resolves."), EnemyState.CurrentBreakValue, EnemyState.MaxBreakValue);
	TestEqual(TEXT("Skipped action should spend the action opportunity."), EnemyState.InitiativeState, EFinalEnemyInitiativeState::ActionSpent);
	TestEqual(TEXT("Skipped action should not commit the normal intent."), EnemyState.IntentRuntimeStates[0].UseCount, 0);
	TestTrue(TEXT("Skipped action should emit EnemyActionSkipped."), EndTurnResult.GeneratedEvents.ContainsByPredicate(
		[](const FFinalBattleEvent& Event)
		{
			return Event.EventType == EFinalBattleEventType::EnemyActionSkipped;
		}));
	return true;
}

#endif
