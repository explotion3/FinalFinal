#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalEnemyDefinition.h"
#include "Battle/Definitions/FinalEnemyIntentDefinition.h"
#include "Commands/FinalBattleCommand.h"
#include "Engine/GameInstance.h"
#include "Facade/FinalBattleSession.h"
#include "Queries/FinalBattleSnapshot.h"
#include "Queries/FinalDataRegistry.h"
#include "Run/Definitions/FinalPrototypeBootstrapDefinition.h"
#include "UObject/StrongObjectPtr.h"

namespace FinalStarterIntentContentTests
{
	const FName StarterBootstrapId(TEXT("prototype.bootstrap.starter.chapter1"));
	const FName StarterRuleConfigId(TEXT("rule.starter.chapter1"));
	const FName StarterBladeEnemyId(TEXT("enemy.starter.bandit.blade"));
	const FName StarterCrossbowEnemyId(TEXT("enemy.starter.bandit.crossbow"));
	const FName StarterShieldGuardEnemyId(TEXT("enemy.starter.bandit.shield_guard"));
	const FName StarterSmokeEnemyId(TEXT("enemy.starter.bandit.medicine_smoke"));
	const FName StarterInstructorEnemyId(TEXT("enemy.starter.blackwind.instructor"));
	const FName StarterToxicEnemyId(TEXT("enemy.starter.toxic_smoke_adept"));
	const FName StarterBossEnemyId(TEXT("enemy.starter.blackwind.chief.likui"));

	const FName StarterNormalEncounterId(TEXT("encounter.starter.chapter1.roadblock"));
	const FName StarterNormalDoubleBladeEncounterId(TEXT("encounter.starter.chapter1.double_blade"));
	const FName StarterNormalSmokeBladeEncounterId(TEXT("encounter.starter.chapter1.smoke_blade"));
	const FName StarterNormalShieldBladeEncounterId(TEXT("encounter.starter.chapter1.shield_blade"));
	const FName StarterNormalSmokeShieldEncounterId(TEXT("encounter.starter.chapter1.smoke_shield"));
	const FName StarterNormalToxicArrowsEncounterId(TEXT("encounter.starter.chapter1.toxic_arrows"));
	const FName StarterEliteEncounterId(TEXT("encounter.starter.chapter1.instructor"));
	const FName StarterEliteIronWallEncounterId(TEXT("encounter.starter.chapter1.iron_wall_instructor"));
	const FName StarterEliteToxicRitualEncounterId(TEXT("encounter.starter.chapter1.toxic_ritual"));
	const FName StarterBossEncounterId(TEXT("encounter.starter.chapter1.blackwind_chief"));

	struct FStarterIntentAutomationContext
	{
		~FStarterIntentAutomationContext()
		{
			Shutdown();
		}

		bool Initialize(FAutomationTestBase& Test, const TCHAR* ContextName)
		{
			if (GEngine == nullptr)
			{
				Test.AddError(TEXT("GEngine is unavailable; cannot create a standalone automation GameInstance."));
				return false;
			}

			const FName WorldName(*FString::Printf(TEXT("/Temp/%s"), ContextName));
			UGameInstance* RawGameInstance = NewObject<UGameInstance>(GEngine, UGameInstance::StaticClass());
			if (RawGameInstance == nullptr)
			{
				Test.AddError(TEXT("Failed to allocate a transient UGameInstance."));
				return false;
			}

			GameInstance.Reset(RawGameInstance);
			RawGameInstance->InitializeStandalone(WorldName);
			DataRegistry = RawGameInstance->GetSubsystem<UFinalDataRegistry>();
			return Test.TestNotNull(TEXT("FinalDataRegistry should initialize in starter intent content tests."), DataRegistry);
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

	UFinalEnemyDefinition* FindEnemy(FAutomationTestBase& Test, UFinalDataRegistry& Registry, const FName EnemyId)
	{
		UFinalEnemyDefinition* Enemy = Registry.FindEnemyDefinition(FFinalEnemyId(EnemyId));
		Test.TestNotNull(
			*FString::Printf(TEXT("Starter enemy should exist: %s"), *EnemyId.ToString()),
			Enemy);
		return Enemy;
	}

	UFinalBattleEncounterDefinition* FindEncounter(FAutomationTestBase& Test, UFinalDataRegistry& Registry, const FName EncounterId)
	{
		UFinalBattleEncounterDefinition* Encounter = Registry.FindEncounterDefinition(FFinalEncounterId(EncounterId));
		Test.TestNotNull(
			*FString::Printf(TEXT("Starter encounter should exist: %s"), *EncounterId.ToString()),
			Encounter);
		return Encounter;
	}

	bool ValidateEnemyRule(
		FAutomationTestBase& Test,
		UFinalEnemyDefinition* Enemy,
		const EFinalIntentSelectRule ExpectedRule,
		const int32 MinimumIntentCount)
	{
		if (Enemy == nullptr)
		{
			return false;
		}

		bool bValid = true;
		bValid &= Test.TestEqual(
			*FString::Printf(TEXT("%s should use the expected intent selection rule."), *Enemy->EnemyId.ToString()),
			Enemy->IntentSelectRule,
			ExpectedRule);
		bValid &= Test.TestTrue(
			*FString::Printf(TEXT("%s should have enough intents."), *Enemy->EnemyId.ToString()),
			Enemy->IntentPool.Num() >= MinimumIntentCount);

		TSet<FName> IntentIds;
		for (const TSoftObjectPtr<UFinalEnemyIntentDefinition>& IntentReference : Enemy->IntentPool)
		{
			UFinalEnemyIntentDefinition* IntentDefinition = IntentReference.LoadSynchronous();
			if (!Test.TestNotNull(TEXT("Starter enemy intent reference should load."), IntentDefinition))
			{
				bValid = false;
				continue;
			}

			IntentIds.Add(IntentDefinition->IntentId);
		}

		for (const FFinalEnemyScriptedIntentStep& ScriptedStep : Enemy->ScriptedIntentSequence)
		{
			bValid &= Test.TestTrue(
				*FString::Printf(TEXT("Scripted intent step should reference an intent in pool: %s"), *ScriptedStep.IntentId.ToString()),
				IntentIds.Contains(ScriptedStep.IntentId));
		}

		return bValid;
	}

	bool BuildStarterInitContext(
		FAutomationTestBase& Test,
		UFinalDataRegistry& Registry,
		const UFinalPrototypeBootstrapDefinition& Bootstrap,
		FFinalBattleInitContext& OutInitContext)
	{
		OutInitContext = FFinalBattleInitContext{};
		OutInitContext.TeamCurrentHP = Bootstrap.InitialTeamCurrentHP;

		for (const FFinalPrototypeBootstrapCharacterState& CharacterState : Bootstrap.InitialCharacterStates)
		{
			UFinalCharacterDefinition* CharacterDefinition = Registry.FindCharacterDefinition(CharacterState.CharacterId);
			if (!Test.TestNotNull(TEXT("Starter character should resolve for battle init context."), CharacterDefinition))
			{
				return false;
			}

			FFinalBattleCharacterInitData InitData;
			InitData.CharacterDefinition = CharacterDefinition;
			InitData.UltimateDefinition = CharacterDefinition->UltimateId.IsValid()
				? Registry.FindUltimateDefinition(CharacterDefinition->UltimateId)
				: nullptr;
			InitData.CurrentStress = CharacterState.CurrentStress;
			InitData.bCollapsed = CharacterState.bCollapsed;
			InitData.CurrentAwakenCount = CharacterState.CurrentAwakenCount;
			InitData.CollapseCount = CharacterState.CollapseCount;
			OutInitContext.PartyMembers.Add(InitData);
		}

		for (const FFinalCardId& CardId : Bootstrap.StarterDeckCardIds)
		{
			UFinalCardDefinition* CardDefinition = Registry.FindCardDefinition(CardId);
			if (!Test.TestNotNull(TEXT("Starter deck card should resolve for battle init context."), CardDefinition))
			{
				return false;
			}

			OutInitContext.DeckDefinitions.Add(CardDefinition);
		}

		return true;
	}

	bool SmokeStartEncounter(
		FAutomationTestBase& Test,
		UFinalDataRegistry& Registry,
		const UFinalPrototypeBootstrapDefinition& Bootstrap,
		const FName EncounterId)
	{
		UFinalBattleEncounterDefinition* Encounter = FindEncounter(Test, Registry, EncounterId);
		UFinalBattleRuleConfig* RuleConfig = Registry.FindRuleConfig(FFinalRuleConfigId(StarterRuleConfigId));
		if (Encounter == nullptr || !Test.TestNotNull(TEXT("Starter rule config should resolve."), RuleConfig))
		{
			return false;
		}

		FFinalBattleInitContext InitContext;
		if (!BuildStarterInitContext(Test, Registry, Bootstrap, InitContext))
		{
			return false;
		}

		TStrongObjectPtr<UFinalBattleSession> BattleSession(NewObject<UFinalBattleSession>());
		BattleSession->InitializeSession(Encounter, RuleConfig, InitContext);

		FFinalBattleSnapshot Snapshot = BattleSession->GetSnapshot();
		bool bValid = true;
		bValid &= Test.TestTrue(TEXT("Starter encounter should start with player characters."), Snapshot.Characters.Num() > 0);
		bValid &= Test.TestTrue(TEXT("Starter encounter should start with enemies."), Snapshot.Enemies.Num() > 0);
		for (const FFinalBattleEnemyViewData& EnemyView : Snapshot.Enemies)
		{
			bValid &= Test.TestFalse(TEXT("Starter enemy should expose an intent id."), EnemyView.CurrentIntentId.IsNone());
			bValid &= Test.TestFalse(TEXT("Starter enemy should expose readable intent text."), EnemyView.IntentText.IsEmpty());
			bValid &= Test.TestTrue(TEXT("Starter enemy should expose structured current intent."), EnemyView.CurrentIntent.bHasIntent);
			bValid &= Test.TestEqual(TEXT("Structured intent id should match legacy intent id."), EnemyView.CurrentIntent.IntentId, EnemyView.CurrentIntentId);
			bValid &= Test.TestFalse(TEXT("Structured intent should expose a display name."), EnemyView.CurrentIntent.DisplayName.IsEmpty());
			bValid &= Test.TestFalse(TEXT("Structured intent should expose preview text."), EnemyView.CurrentIntent.PreviewText.IsEmpty());
		}

		for (int32 TurnIndex = 0; TurnIndex < 2 && !Snapshot.bBattleEnded; ++TurnIndex)
		{
			FFinalBattleCommand EndTurnCommand;
			EndTurnCommand.CommandType = EFinalBattleCommandType::EndTurn;
			const FFinalBattleEvent ResultEvent = BattleSession->SubmitCommand(EndTurnCommand);
			bValid &= Test.TestTrue(TEXT("EndTurn should be accepted while smoking starter intent encounter cadence."), ResultEvent.EventType != EFinalBattleEventType::CommandRejected);
			Snapshot = BattleSession->GetSnapshot();
		}

		bValid &= Test.TestTrue(TEXT("Starter encounter smoke should advance the event ledger."), BattleSession->GetLatestBattleEventSequence() > 0);
		return bValid;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalStarterIntentContentRegistryTest,
	"Final.Editor.StarterIntentContent.RegistryAndRules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalStarterIntentContentRegistryTest::RunTest(const FString& Parameters)
{
	using namespace FinalStarterIntentContentTests;

	FStarterIntentAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalStarterIntentContentRegistryWorld")))
	{
		return false;
	}

	UFinalDataRegistry& Registry = *Context.DataRegistry;
	bool bValid = true;
	bValid &= ValidateEnemyRule(*this, FindEnemy(*this, Registry, StarterBladeEnemyId), EFinalIntentSelectRule::Cycle, 3);
	bValid &= ValidateEnemyRule(*this, FindEnemy(*this, Registry, StarterCrossbowEnemyId), EFinalIntentSelectRule::WeightedRandom, 3);
	bValid &= ValidateEnemyRule(*this, FindEnemy(*this, Registry, StarterShieldGuardEnemyId), EFinalIntentSelectRule::Cycle, 3);
	bValid &= ValidateEnemyRule(*this, FindEnemy(*this, Registry, StarterSmokeEnemyId), EFinalIntentSelectRule::WeightedRandom, 3);
	bValid &= ValidateEnemyRule(*this, FindEnemy(*this, Registry, StarterInstructorEnemyId), EFinalIntentSelectRule::PhaseSequence, 3);
	bValid &= ValidateEnemyRule(*this, FindEnemy(*this, Registry, StarterToxicEnemyId), EFinalIntentSelectRule::Cycle, 3);
	bValid &= ValidateEnemyRule(*this, FindEnemy(*this, Registry, StarterBossEnemyId), EFinalIntentSelectRule::Scripted, 4);

	const FName EncounterIds[] = {
		StarterNormalEncounterId,
		StarterNormalDoubleBladeEncounterId,
		StarterNormalSmokeBladeEncounterId,
		StarterNormalShieldBladeEncounterId,
		StarterNormalSmokeShieldEncounterId,
		StarterNormalToxicArrowsEncounterId,
		StarterEliteEncounterId,
		StarterEliteIronWallEncounterId,
		StarterEliteToxicRitualEncounterId,
		StarterBossEncounterId
	};

	for (const FName EncounterId : EncounterIds)
	{
		bValid &= FindEncounter(*this, Registry, EncounterId) != nullptr;
	}

	return bValid && !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalStarterIntentContentCadenceTest,
	"Final.Editor.StarterIntentContent.EncounterCadence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalStarterIntentContentCadenceTest::RunTest(const FString& Parameters)
{
	using namespace FinalStarterIntentContentTests;

	FStarterIntentAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalStarterIntentContentCadenceWorld")))
	{
		return false;
	}

	UFinalPrototypeBootstrapDefinition* StarterBootstrap = Context.DataRegistry->FindPrototypeBootstrapDefinition(StarterBootstrapId);
	if (!TestNotNull(TEXT("Starter bootstrap should resolve for cadence tests."), StarterBootstrap))
	{
		return false;
	}

	bool bValid = true;
	bValid &= SmokeStartEncounter(*this, *Context.DataRegistry, *StarterBootstrap, StarterNormalEncounterId);
	bValid &= SmokeStartEncounter(*this, *Context.DataRegistry, *StarterBootstrap, StarterEliteToxicRitualEncounterId);
	bValid &= SmokeStartEncounter(*this, *Context.DataRegistry, *StarterBootstrap, StarterBossEncounterId);
	return bValid && !HasAnyErrors();
}

#endif
