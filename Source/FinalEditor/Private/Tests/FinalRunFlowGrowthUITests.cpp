#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalEnemyDefinition.h"
#include "Battle/Effects/FinalBattleEffectDamage.h"
#include "Battle/Effects/FinalBattleEffectBonusBreak.h"
#include "Battle/Effects/FinalBattleEffectGenerateCard.h"
#include "GameplayTagContainer.h"
#include "Commands/FinalBattleCommand.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"
#include "Facade/FinalRunSession.h"
#include "Queries/FinalDataRegistry.h"
#include "Requests/FinalBattleResult.h"
#include "Run/Definitions/FinalCardEvolutionDefinition.h"
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
	const FName StarterEncounterId(TEXT("encounter.starter.chapter1.roadblock"));
	const FName StarterRuleConfigId(TEXT("rule.starter.chapter1"));

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

	const FFinalBattleCharacterViewData* FindBattleCharacterView(const FFinalBattleSnapshot& Snapshot, const FFinalCharacterId& CharacterId)
	{
		return Snapshot.Characters.FindByPredicate([&CharacterId](const FFinalBattleCharacterViewData& CharacterView)
		{
			return CharacterView.CharacterId == CharacterId;
		});
	}

	const FFinalBattleEnemyViewData* FindBattleEnemyView(const FFinalBattleSnapshot& Snapshot, const FName RuntimeUnitId)
	{
		return Snapshot.Enemies.FindByPredicate([&RuntimeUnitId](const FFinalBattleEnemyViewData& EnemyView)
		{
			return EnemyView.RuntimeUnitId == RuntimeUnitId;
		});
	}

	const FFinalBattleCardViewData* FindHandCardViewByRunCardInstanceId(const FFinalBattleSnapshot& Snapshot, const FName SourceRunCardInstanceId)
	{
		return Snapshot.HandCards.FindByPredicate([&SourceRunCardInstanceId](const FFinalBattleCardViewData& CardView)
		{
			return CardView.SourceRunCardInstanceId == SourceRunCardInstanceId;
		});
	}

	const FFinalRunGrowthChoiceInstance* FindEvolutionChoiceForRunCardInstance(
		const FFinalRunSnapshot& Snapshot,
		const FName TargetRunCardInstanceId)
	{
		if (!Snapshot.PendingGrowthChoice.bHasPendingChoice)
		{
			return nullptr;
		}

		return Snapshot.PendingGrowthChoice.Choices.FindByPredicate([&TargetRunCardInstanceId](const FFinalRunGrowthChoiceInstance& Choice)
		{
			return Choice.ChoiceType == EFinalGrowthChoiceType::CardEvolution
				&& Choice.TargetRunCardInstanceId == TargetRunCardInstanceId;
		});
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

		UFinalCharacterGrowthConfig* RegisterGrowthConfig(
			const FFinalCharacterGrowthConfigId& GrowthConfigId,
			const TArray<EFinalBattleGrowthFactType>& PreferredFactTypes,
			const TMap<EFinalBattleGrowthFactType, float>& Scalars) const
		{
			UFinalCharacterGrowthConfig* GrowthConfig = NewObject<UFinalCharacterGrowthConfig>(GameInstance.Get());
			GrowthConfig->GrowthConfigId = GrowthConfigId;
			GrowthConfig->BaseBreakthroughRequiredValue = 100;
			GrowthConfig->PreferredBreakthroughFactTypes = PreferredFactTypes;
			GrowthConfig->BreakthroughGainScalarByFactType = Scalars;
			DataRegistry->RegisterCharacterGrowthConfig(GrowthConfig);
			return GrowthConfig;
		}

		UFinalCharacterDefinition* RegisterCharacterDefinition(
			const FFinalCharacterId& CharacterId,
			const FFinalCharacterGrowthConfigId& GrowthConfigId,
			const FString& DisplayName) const
		{
			UFinalCharacterDefinition* CharacterDefinition = NewObject<UFinalCharacterDefinition>(GameInstance.Get());
			CharacterDefinition->CharacterId = CharacterId;
			CharacterDefinition->DisplayName = FText::FromString(DisplayName);
			CharacterDefinition->GrowthConfigId = GrowthConfigId;
			CharacterDefinition->BaseVitalShare = 20;
			CharacterDefinition->BaseStressCap = 12;
			CharacterDefinition->BaseAttack = 5;
			CharacterDefinition->BaseDefense = 2;
			CharacterDefinition->BaseBreakRate = 1.0f;
			CharacterDefinition->BaseCritChance = 0.0f;
			CharacterDefinition->BaseCritDamage = 1.5f;
			DataRegistry->RegisterCharacterDefinition(CharacterDefinition);
			return CharacterDefinition;
		}

		UFinalBattleRuleConfig* RegisterRuleConfig(
			const FFinalRuleConfigId& RuleConfigId,
			const int32 InitialHandSize = 1,
			const int32 InitialAP = 3) const
		{
			UFinalBattleRuleConfig* RuleConfig = NewObject<UFinalBattleRuleConfig>(GameInstance.Get());
			RuleConfig->RuleConfigId = RuleConfigId;
			RuleConfig->InitialHandSize = InitialHandSize;
			RuleConfig->InitialAP = InitialAP;
			RuleConfig->TurnStartDrawCount = InitialHandSize;
			DataRegistry->RegisterRuleConfig(RuleConfig);
			return RuleConfig;
		}

		UFinalEnemyDefinition* RegisterEnemyDefinition(
			const FFinalEnemyId& EnemyId,
			const FString& DisplayName,
			const int32 MaxHP = 100) const
		{
			UFinalEnemyDefinition* EnemyDefinition = NewObject<UFinalEnemyDefinition>(GameInstance.Get());
			EnemyDefinition->EnemyId = EnemyId;
			EnemyDefinition->DisplayName = FText::FromString(DisplayName);
			EnemyDefinition->MaxHP = MaxHP;
			EnemyDefinition->MaxBreakValue = 10;
			EnemyDefinition->BaseDamagePower = 3;
			EnemyDefinition->InitialInitiativeValue = 0;
			EnemyDefinition->IntentSelectRule = EFinalIntentSelectRule::Cycle;
			DataRegistry->RegisterEnemyDefinition(EnemyDefinition);
			return EnemyDefinition;
		}

		UFinalBattleEncounterDefinition* RegisterEncounterDefinition(
			const FFinalEncounterId& EncounterId,
			UFinalBattleRuleConfig* RuleConfig,
			UFinalEnemyDefinition* EnemyDefinition) const
		{
			UFinalBattleEncounterDefinition* EncounterDefinition = NewObject<UFinalBattleEncounterDefinition>(GameInstance.Get());
			EncounterDefinition->EncounterId = EncounterId;
			EncounterDefinition->DisplayName = FText::FromString(TEXT("Automation Encounter"));
			EncounterDefinition->RuleConfig = RuleConfig;

			FFinalEnemyRosterEntry& Entry = EncounterDefinition->EnemyRoster.AddDefaulted_GetRef();
			Entry.EnemyDefinition = EnemyDefinition;
			Entry.PositionIndex = 0;
			Entry.SpawnWave = 1;

			DataRegistry->RegisterEncounterDefinition(EncounterDefinition);
			return EncounterDefinition;
		}

		UFinalCardDefinition* RegisterBreakthroughTestCard(
			const FFinalCardId& CardId,
			const FFinalCharacterId& OwnerCharacterId,
			const FString& DisplayName) const
		{
			UFinalCardDefinition* CardDefinition = RegisterCardDefinition(CardId, OwnerCharacterId, DisplayName);
			CardDefinition->BaseCostAP = 1;
			CardDefinition->CardType = EFinalCardType::Attack;
			CardDefinition->Effects.Reset();

			UFinalBattleEffectBonusBreak* BonusBreakEffect = NewObject<UFinalBattleEffectBonusBreak>(CardDefinition);
			BonusBreakEffect->EffectId = TEXT("effect.test.breakthrough.break");
			BonusBreakEffect->UnitTargetRule = EFinalBattleUnitTargetRule::SelectedEnemy;
			BonusBreakEffect->Scalar.ScaleMode = EFinalBattleScalarMode::Flat;
			BonusBreakEffect->Scalar.BaseValue = 2.0f;
			CardDefinition->Effects.Add(BonusBreakEffect);
			return CardDefinition;
		}

		UFinalCardDefinition* RegisterAttackScalingTestCard(
			const FFinalCardId& CardId,
			const FFinalCharacterId& OwnerCharacterId,
			const FString& DisplayName) const
		{
			UFinalCardDefinition* CardDefinition = RegisterCardDefinition(CardId, OwnerCharacterId, DisplayName);
			CardDefinition->BaseCostAP = 1;
			CardDefinition->CardType = EFinalCardType::Attack;
			CardDefinition->Effects.Reset();

			UFinalBattleEffectDamage* DamageEffect = NewObject<UFinalBattleEffectDamage>(CardDefinition);
			DamageEffect->EffectId = TEXT("effect.test.damage.scaling");
			DamageEffect->UnitTargetRule = EFinalBattleUnitTargetRule::SelectedEnemy;
			DamageEffect->Scalar.ScaleMode = EFinalBattleScalarMode::SourceStatMultiplier;
			DamageEffect->Scalar.SourceStat = EFinalBattleSourceStat::Attack;
			DamageEffect->Scalar.BaseValue = 1.0f;
			CardDefinition->Effects.Add(DamageEffect);
			return CardDefinition;
		}

		UFinalCardDefinition* RegisterConfigurableDamageCard(
			const FFinalCardId& CardId,
			const FFinalCharacterId& OwnerCharacterId,
			const FString& DisplayName,
			const EFinalCardType CardType,
			const int32 BaseCostAP,
			const float AttackScalar,
			const bool bConsumeOnPlay = false) const
		{
			UFinalCardDefinition* CardDefinition = RegisterCardDefinition(CardId, OwnerCharacterId, DisplayName);
			CardDefinition->BaseCostAP = BaseCostAP;
			CardDefinition->CardType = CardType;
			CardDefinition->Effects.Reset();
			CardDefinition->Keywords.Reset();
			if (bConsumeOnPlay)
			{
				CardDefinition->Keywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Expend")));
			}

			UFinalBattleEffectDamage* DamageEffect = NewObject<UFinalBattleEffectDamage>(CardDefinition);
			DamageEffect->EffectId = *FString::Printf(TEXT("effect.test.damage.%s"), *CardId.Value.ToString());
			DamageEffect->UnitTargetRule = EFinalBattleUnitTargetRule::SelectedEnemy;
			DamageEffect->Scalar.ScaleMode = EFinalBattleScalarMode::SourceStatMultiplier;
			DamageEffect->Scalar.SourceStat = EFinalBattleSourceStat::Attack;
			DamageEffect->Scalar.BaseValue = AttackScalar;
			CardDefinition->Effects.Add(DamageEffect);
			return CardDefinition;
		}

		UFinalCardEvolutionDefinition* RegisterEvolutionDefinition(
			const FFinalCardEvolutionId& EvolutionId,
			const FFinalCardId& FromCardId,
			const FFinalCardId& ToCardId,
			const FFinalCharacterId& OwnerCharacterId,
			const FString& DisplayName,
			const FString& Description = TEXT("")) const
		{
			UFinalCardEvolutionDefinition* EvolutionDefinition = NewObject<UFinalCardEvolutionDefinition>(GameInstance.Get());
			EvolutionDefinition->EvolutionId = EvolutionId;
			EvolutionDefinition->FromCardId = FromCardId;
			EvolutionDefinition->ToCardId = ToCardId;
			EvolutionDefinition->RequiredOwnerCharacterId = OwnerCharacterId;
			EvolutionDefinition->FromStage = EFinalCardEvolutionStage::Base;
			EvolutionDefinition->ToStage = EFinalCardEvolutionStage::Evolved;
			EvolutionDefinition->bAllowAsLevelUpCandidate = true;
			EvolutionDefinition->DisplayName = FText::FromString(DisplayName);
			EvolutionDefinition->Description = FText::FromString(Description);
			DataRegistry->RegisterCardEvolutionDefinition(EvolutionDefinition);
			return EvolutionDefinition;
		}

		UFinalRunSession* CreateRunSessionWithSingleCharacter(
			const FFinalCharacterId& CharacterId,
			const FFinalCardId& StarterCardId) const
		{
			FFinalRunPersistentCharacterState CharacterState;
			CharacterState.CharacterId = CharacterId;
			CharacterState.Level = 1;
			CharacterState.BreakthroughValue = 0;
			CharacterState.BreakthroughRequiredValue = 100;

			return CreateRunSessionWithCharacterState(
				CharacterState,
				{ StarterCardId },
				FFinalEncounterId(StarterEncounterId),
				FFinalRuleConfigId(StarterRuleConfigId),
				20);
		}

		UFinalRunSession* CreateRunSessionWithCharacterState(
			const FFinalRunPersistentCharacterState& CharacterState,
			const TArray<FFinalCardId>& StarterDeck,
			const FFinalEncounterId& EncounterId,
			const FFinalRuleConfigId& RuleConfigId,
			const int32 InitialTeamCurrentHP) const
		{
			UFinalRunSession* RunSession = GameFlowSubsystem->BootstrapNewRun();
			if (RunSession == nullptr)
			{
				return nullptr;
			}

			TArray<FFinalRunPersistentCharacterState> PartyStates;
			PartyStates.Add(CharacterState);

			RunSession->ConfigureBattleStartState(
				EncounterId,
				RuleConfigId,
				PartyStates,
				StarterDeck,
				InitialTeamCurrentHP);

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
	"Final.Editor.RunFlow.GrowthStarterBootstrapDefersUntilBattle",
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
	TestFalse(TEXT("Starter bootstrap should no longer create an initial pending growth choice before battle."), Snapshot.PendingGrowthChoice.bHasPendingChoice);
	TestFalse(TEXT("Starter bootstrap should not present GrowthChoice before the first battle naturally triggers growth."), Context.RunFlowSubsystem->GetPresentedOverlay() == EFinalRunPresentedOverlay::GrowthChoice);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalRunFlowBattleGrowthBridgePresentationTest,
	"Final.Editor.RunFlow.GrowthBattleBridgeCreatesImmediateOverlay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalRunFlowBattleGrowthBridgePresentationTest::RunTest(const FString& Parameters)
{
	using namespace FinalRunFlowGrowthUITests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalRunFlowBattleGrowthBridgePresentationTest")))
	{
		return false;
	}

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.bridge_huo")));
	const FFinalCharacterGrowthConfigId GrowthConfigId(FName(TEXT("growth.config.test.bridge_huo")));
	const FFinalCardId StarterCardId(FName(TEXT("card.test.bridge_huo")));
	Context.RegisterGrowthConfig(
		GrowthConfigId,
		{ EFinalBattleGrowthFactType::OwnedCardResolved, EFinalBattleGrowthFactType::BattleVictoryBaseReward },
		{
			{ EFinalBattleGrowthFactType::OwnedCardResolved, 5.0f },
			{ EFinalBattleGrowthFactType::BattleVictoryBaseReward, 1.0f }
		});
	Context.RegisterCharacterDefinition(CharacterId, GrowthConfigId, TEXT("Bridge Huo"));
	Context.RegisterBreakthroughTestCard(StarterCardId, CharacterId, TEXT("Bridge Slash"));

	UFinalRunSession* RunSession = Context.CreateRunSessionWithSingleCharacter(CharacterId, StarterCardId);
	if (!TestNotNull(TEXT("RunSession should be created for battle growth bridge test."), RunSession))
	{
		return false;
	}

	FFinalRunState SeedState = RunSession->GetRunState();
	SeedState.Characters[0].BreakthroughValue = 95;
	RunSession->ConfigureBattleStartState(
		FFinalEncounterId(StarterEncounterId),
		FFinalRuleConfigId(StarterRuleConfigId),
		SeedState.Characters,
		{ StarterCardId },
		20);

	UFinalBattleSession* BattleSession = Context.GameFlowSubsystem->StartBattleFromRunSession();
	if (!TestNotNull(TEXT("Battle session should start from the custom breakthrough bridge run."), BattleSession))
	{
		return false;
	}

	const FFinalBattleSnapshot BattleSnapshot = Context.BattleFlowSubsystem->GetCurrentSnapshot();
	if (!TestTrue(TEXT("Custom breakthrough bridge battle should draw the only deck card into hand."), BattleSnapshot.HandCards.Num() == 1))
	{
		return false;
	}

	const FName TargetUnitId = !BattleSnapshot.CurrentTargetUnitId.IsNone()
		? BattleSnapshot.CurrentTargetUnitId
		: (BattleSnapshot.Enemies.Num() > 0 ? BattleSnapshot.Enemies[0].RuntimeUnitId : NAME_None);
	TestFalse(TEXT("Custom breakthrough bridge battle should expose a valid enemy target."), TargetUnitId.IsNone());

	FFinalBattleCommand Command;
	Command.CommandType = EFinalBattleCommandType::PlayCard;
	Command.CardInstanceId = BattleSnapshot.HandCards[0].CardInstanceId;
	Command.TargetUnitId = TargetUnitId;
	TestTrue(TEXT("Playing the deterministic bridge card should succeed."), Context.BattleFlowSubsystem->SubmitBattleCommand(Command));

	const FFinalRunSnapshot RunSnapshot = RunSession->GetSnapshot();
	TestTrue(TEXT("Player-command growth bridge should create a pending growth choice immediately after the command resolves."), RunSnapshot.PendingGrowthChoice.bHasPendingChoice);
	TestEqual(TEXT("Growth overlay should immediately take over run flow presentation after the bridge creates pending growth."), Context.RunFlowSubsystem->GetPresentedOverlay(), EFinalRunPresentedOverlay::GrowthChoice);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleGrowthAttributeProjectionAndRefreshTest,
	"Final.Editor.RunFlow.GrowthAttributeProjectionAndMidBattleRefresh",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleGrowthAttributeProjectionAndRefreshTest::RunTest(const FString& Parameters)
{
	using namespace FinalRunFlowGrowthUITests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalBattleGrowthAttributeProjectionAndRefreshTest")))
	{
		return false;
	}

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.attribute_projection")));
	const FFinalCharacterGrowthConfigId GrowthConfigId(FName(TEXT("growth.config.test.attribute_projection")));
	const FFinalCardId StarterCardId(FName(TEXT("card.test.attribute_projection")));
	const FFinalRuleConfigId RuleConfigId(FName(TEXT("rule.test.attribute_projection")));
	const FFinalEncounterId EncounterId(FName(TEXT("encounter.test.attribute_projection")));
	const FFinalEnemyId EnemyId(FName(TEXT("enemy.test.attribute_projection")));

	UFinalCharacterGrowthConfig* GrowthConfig = Context.RegisterGrowthConfig(GrowthConfigId, {}, {});
	Context.RegisterCharacterDefinition(CharacterId, GrowthConfigId, TEXT("Attribute Projection Hero"));
	Context.RegisterRuleConfig(RuleConfigId, 1, 3);
	UFinalEnemyDefinition* EnemyDefinition = Context.RegisterEnemyDefinition(EnemyId, TEXT("Projection Target"), 100);
	Context.RegisterEncounterDefinition(EncounterId, Context.DataRegistry->FindRuleConfig(RuleConfigId), EnemyDefinition);
	Context.RegisterAttackScalingTestCard(StarterCardId, CharacterId, TEXT("Projection Slash"));

	FFinalRunPersistentCharacterState CharacterState;
	CharacterState.CharacterId = CharacterId;
	CharacterState.Level = 1;
	CharacterState.BreakthroughRequiredValue = 100;
	CharacterState.RootBone = 2;

	UFinalRunSession* RunSession = Context.CreateRunSessionWithCharacterState(
		CharacterState,
		{ StarterCardId },
		EncounterId,
		RuleConfigId,
		20);
	if (!TestNotNull(TEXT("RunSession should be created for attribute projection test."), RunSession))
	{
		return false;
	}

	if (!TestNotNull(TEXT("Battle session should start for attribute projection test."), Context.GameFlowSubsystem->StartBattleFromRunSession()))
	{
		return false;
	}

	FFinalBattleSnapshot BattleSnapshot = Context.BattleFlowSubsystem->GetCurrentSnapshot();
	const FFinalBattleCharacterViewData* CharacterView = FindBattleCharacterView(BattleSnapshot, CharacterId);
	if (!TestNotNull(TEXT("Battle snapshot should expose the projected character view."), CharacterView))
	{
		return false;
	}

	TestEqual(TEXT("Root Bone should increase VitalShare during battle initialization."), CharacterView->VitalShare, 32);
	TestEqual(TEXT("Root Bone should increase StressCap during battle initialization."), CharacterView->StressCap, 14);
	TestEqual(TEXT("Team max HP should include projected VitalShare."), BattleSnapshot.TeamMaxHP, 32);

	TestTrue(TEXT("Breakthrough gain should create a pending growth choice during the active battle."), RunSession->AddBreakthroughValue(CharacterId, 100));
	Context.RunFlowSubsystem->RefreshRunFlow(true);

	const FFinalRunSnapshot PendingSnapshot = Context.RunFlowSubsystem->GetCurrentRunSnapshot();
	const FFinalRunGrowthChoiceInstance* RootBoneChoice = PendingSnapshot.PendingGrowthChoice.Choices.FindByPredicate([](const FFinalRunGrowthChoiceInstance& Choice)
	{
		return Choice.ChoiceType == EFinalGrowthChoiceType::AttributeGrowth
			&& Choice.AttributeType == EFinalGrowthAttributeType::RootBone;
	});
	if (!TestNotNull(TEXT("Pending growth choice set should contain the Root Bone option."), RootBoneChoice))
	{
		return false;
	}

	TestTrue(TEXT("Selecting Root Bone through RunFlowSubsystem should succeed during an active battle."), Context.RunFlowSubsystem->SelectGrowthChoice(RootBoneChoice->ChoiceInstanceId));

	BattleSnapshot = Context.BattleFlowSubsystem->GetCurrentSnapshot();
	CharacterView = FindBattleCharacterView(BattleSnapshot, CharacterId);
	if (!TestNotNull(TEXT("Battle snapshot should still expose the character after Root Bone refresh."), CharacterView))
	{
		return false;
	}

	TestEqual(TEXT("Mid-battle Root Bone growth should immediately refresh VitalShare."), CharacterView->VitalShare, 38);
	TestEqual(TEXT("Mid-battle Root Bone growth should immediately refresh StressCap."), CharacterView->StressCap, 15);
	TestEqual(TEXT("Team max HP should refresh with the projected VitalShare after growth."), BattleSnapshot.TeamMaxHP, 38);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleGrowthKillingIntentDamageAndCritTest,
	"Final.Editor.RunFlow.GrowthKillingIntentRefreshAffectsDamageAndCrit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleGrowthKillingIntentDamageAndCritTest::RunTest(const FString& Parameters)
{
	using namespace FinalRunFlowGrowthUITests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalBattleGrowthKillingIntentDamageAndCritTest")))
	{
		return false;
	}

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.killing_intent")));
	const FFinalCharacterGrowthConfigId GrowthConfigId(FName(TEXT("growth.config.test.killing_intent")));
	const FFinalCardId StarterCardId(FName(TEXT("card.test.killing_intent")));
	const FFinalRuleConfigId RuleConfigId(FName(TEXT("rule.test.killing_intent")));
	const FFinalEncounterId EncounterId(FName(TEXT("encounter.test.killing_intent")));
	const FFinalEnemyId EnemyId(FName(TEXT("enemy.test.killing_intent")));

	UFinalCharacterGrowthConfig* GrowthConfig = Context.RegisterGrowthConfig(GrowthConfigId, {}, {});
	GrowthConfig->KillingIntentAttackPerPoint = 2;
	GrowthConfig->KillingIntentCritChancePerPoint = 1.0f;
	GrowthConfig->KillingIntentCritDamagePerPoint = 1.0f;
	Context.RegisterCharacterDefinition(CharacterId, GrowthConfigId, TEXT("Killing Intent Hero"));
	Context.RegisterRuleConfig(RuleConfigId, 2, 3);
	UFinalEnemyDefinition* EnemyDefinition = Context.RegisterEnemyDefinition(EnemyId, TEXT("Crit Target"), 100);
	Context.RegisterEncounterDefinition(EncounterId, Context.DataRegistry->FindRuleConfig(RuleConfigId), EnemyDefinition);
	Context.RegisterAttackScalingTestCard(StarterCardId, CharacterId, TEXT("Intent Slash"));

	FFinalRunPersistentCharacterState CharacterState;
	CharacterState.CharacterId = CharacterId;
	CharacterState.Level = 1;
	CharacterState.BreakthroughRequiredValue = 100;

	UFinalRunSession* RunSession = Context.CreateRunSessionWithCharacterState(
		CharacterState,
		{ StarterCardId, StarterCardId },
		EncounterId,
		RuleConfigId,
		20);
	if (!TestNotNull(TEXT("RunSession should be created for Killing Intent damage test."), RunSession))
	{
		return false;
	}

	if (!TestNotNull(TEXT("Battle session should start for Killing Intent damage test."), Context.GameFlowSubsystem->StartBattleFromRunSession()))
	{
		return false;
	}

	FFinalBattleSnapshot BattleSnapshot = Context.BattleFlowSubsystem->GetCurrentSnapshot();
	if (!TestEqual(TEXT("The custom rule config should draw both attack cards into hand."), BattleSnapshot.HandCards.Num(), 2))
	{
		return false;
	}

	const FName TargetUnitId = BattleSnapshot.CurrentTargetUnitId.IsNone()
		? BattleSnapshot.Enemies[0].RuntimeUnitId
		: BattleSnapshot.CurrentTargetUnitId;
	const FFinalBattleEnemyViewData* EnemyView = FindBattleEnemyView(BattleSnapshot, TargetUnitId);
	if (!TestNotNull(TEXT("Battle snapshot should expose the selected target enemy."), EnemyView))
	{
		return false;
	}

	const int32 EnemyHpBeforeBaselineAttack = EnemyView->CurrentHP;
	FFinalBattleCommand Command;
	Command.CommandType = EFinalBattleCommandType::PlayCard;
	Command.CardInstanceId = BattleSnapshot.HandCards[0].CardInstanceId;
	Command.TargetUnitId = TargetUnitId;
	TestTrue(TEXT("Baseline attack should resolve successfully before Killing Intent grows."), Context.BattleFlowSubsystem->SubmitBattleCommand(Command));

	BattleSnapshot = Context.BattleFlowSubsystem->GetCurrentSnapshot();
	EnemyView = FindBattleEnemyView(BattleSnapshot, TargetUnitId);
	if (!TestNotNull(TEXT("Enemy should still exist after the baseline attack."), EnemyView))
	{
		return false;
	}

	TestEqual(TEXT("Without Killing Intent growth, the baseline attack should deal damage equal to RuntimeAttack."), EnemyHpBeforeBaselineAttack - EnemyView->CurrentHP, 5);

	TestTrue(TEXT("Breakthrough gain should create a pending growth choice before selecting Killing Intent."), RunSession->AddBreakthroughValue(CharacterId, 100));
	Context.RunFlowSubsystem->RefreshRunFlow(true);

	const FFinalRunSnapshot PendingSnapshot = Context.RunFlowSubsystem->GetCurrentRunSnapshot();
	const FFinalRunGrowthChoiceInstance* KillingIntentChoice = PendingSnapshot.PendingGrowthChoice.Choices.FindByPredicate([](const FFinalRunGrowthChoiceInstance& Choice)
	{
		return Choice.ChoiceType == EFinalGrowthChoiceType::AttributeGrowth
			&& Choice.AttributeType == EFinalGrowthAttributeType::KillingIntent;
	});
	if (!TestNotNull(TEXT("Pending growth choice set should contain the Killing Intent option."), KillingIntentChoice))
	{
		return false;
	}

	TestTrue(TEXT("Selecting Killing Intent through RunFlowSubsystem should succeed during the active battle."), Context.RunFlowSubsystem->SelectGrowthChoice(KillingIntentChoice->ChoiceInstanceId));

	BattleSnapshot = Context.BattleFlowSubsystem->GetCurrentSnapshot();
	EnemyView = FindBattleEnemyView(BattleSnapshot, TargetUnitId);
	if (!TestNotNull(TEXT("Enemy should still exist before the post-growth attack."), EnemyView))
	{
		return false;
	}

	const int32 EnemyHpBeforeCriticalAttack = EnemyView->CurrentHP;
	Command.CardInstanceId = BattleSnapshot.HandCards[0].CardInstanceId;
	TestTrue(TEXT("Post-growth attack should resolve successfully after Killing Intent refresh."), Context.BattleFlowSubsystem->SubmitBattleCommand(Command));

	BattleSnapshot = Context.BattleFlowSubsystem->GetCurrentSnapshot();
	EnemyView = FindBattleEnemyView(BattleSnapshot, TargetUnitId);
	if (!TestNotNull(TEXT("Enemy should still exist after the post-growth attack."), EnemyView))
	{
		return false;
	}

	TestEqual(TEXT("Killing Intent should increase RuntimeAttack and force crits when crit chance reaches 1."), EnemyHpBeforeCriticalAttack - EnemyView->CurrentHP, 18);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleGrowthEvolutionRefreshesHandCardTest,
	"Final.Editor.RunFlow.GrowthEvolutionRefreshesHandCardInstance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleGrowthEvolutionRefreshesHandCardTest::RunTest(const FString& Parameters)
{
	using namespace FinalRunFlowGrowthUITests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalBattleGrowthEvolutionRefreshesHandCardTest")))
	{
		return false;
	}

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.evo_hand")));
	const FFinalCharacterGrowthConfigId GrowthConfigId(FName(TEXT("growth.config.test.evo_hand")));
	const FFinalCardId BaseCardId(FName(TEXT("card.test.evo_hand.base")));
	const FFinalCardId EvolvedCardId(FName(TEXT("card.test.evo_hand.evolved")));
	const FFinalCardEvolutionId EvolutionId(FName(TEXT("evo.test.evo_hand")));
	const FFinalRuleConfigId RuleConfigId(FName(TEXT("rule.test.evo_hand")));
	const FFinalEncounterId EncounterId(FName(TEXT("encounter.test.evo_hand")));
	const FFinalEnemyId EnemyId(FName(TEXT("enemy.test.evo_hand")));

	Context.RegisterGrowthConfig(GrowthConfigId, {}, {});
	Context.RegisterCharacterDefinition(CharacterId, GrowthConfigId, TEXT("Evolution Hand Hero"));
	Context.RegisterRuleConfig(RuleConfigId, 1, 3);
	UFinalEnemyDefinition* EnemyDefinition = Context.RegisterEnemyDefinition(EnemyId, TEXT("Evolution Target"), 100);
	Context.RegisterEncounterDefinition(EncounterId, Context.DataRegistry->FindRuleConfig(RuleConfigId), EnemyDefinition);
	Context.RegisterConfigurableDamageCard(BaseCardId, CharacterId, TEXT("Base Slash"), EFinalCardType::Attack, 1, 1.0f);
	Context.RegisterConfigurableDamageCard(EvolvedCardId, CharacterId, TEXT("Evolved Slash"), EFinalCardType::Skill, 2, 2.0f);
	Context.RegisterEvolutionDefinition(EvolutionId, BaseCardId, EvolvedCardId, CharacterId, TEXT("Evolve Base Slash"), TEXT("Refresh the active hand instance."));

	UFinalRunSession* RunSession = Context.CreateRunSessionWithSingleCharacter(CharacterId, BaseCardId);
	if (!TestNotNull(TEXT("RunSession should be created for hand evolution refresh test."), RunSession))
	{
		return false;
	}

	if (!TestNotNull(TEXT("Battle session should start for hand evolution refresh test."), Context.GameFlowSubsystem->StartBattleFromRunSession()))
	{
		return false;
	}

	const FFinalRunState InitialRunState = RunSession->GetRunState();
	const FName TargetRunCardInstanceId = InitialRunState.RunDeck[0].InstanceId;
	FFinalBattleSnapshot BattleSnapshot = Context.BattleFlowSubsystem->GetCurrentSnapshot();
	const FFinalBattleCardViewData* HandCard = FindHandCardViewByRunCardInstanceId(BattleSnapshot, TargetRunCardInstanceId);
	if (!TestNotNull(TEXT("Active hand should expose the source run card instance id before evolution."), HandCard))
	{
		return false;
	}

	TestEqual(TEXT("Pre-evolution hand card should still use the base card id."), HandCard->CardId.Value, BaseCardId.Value);
	TestTrue(TEXT("Breakthrough gain should create a pending growth choice for the hand refresh test."), RunSession->AddBreakthroughValue(CharacterId, 100));
	Context.RunFlowSubsystem->RefreshRunFlow(true);

	const FFinalRunSnapshot PendingSnapshot = Context.RunFlowSubsystem->GetCurrentRunSnapshot();
	const FFinalRunGrowthChoiceInstance* EvolutionChoice = FindEvolutionChoiceForRunCardInstance(PendingSnapshot, TargetRunCardInstanceId);
	if (!TestNotNull(TEXT("Pending growth choices should include the evolution choice for the in-hand run card instance."), EvolutionChoice))
	{
		return false;
	}

	TestTrue(TEXT("Selecting the in-hand evolution choice should succeed through RunFlowSubsystem."), Context.RunFlowSubsystem->SelectGrowthChoice(EvolutionChoice->ChoiceInstanceId));

	BattleSnapshot = Context.BattleFlowSubsystem->GetCurrentSnapshot();
	HandCard = FindHandCardViewByRunCardInstanceId(BattleSnapshot, TargetRunCardInstanceId);
	if (!TestNotNull(TEXT("The refreshed hand should still expose the evolved run card instance."), HandCard))
	{
		return false;
	}

	TestEqual(TEXT("Hand refresh should swap the visible card id to the evolved definition."), HandCard->CardId.Value, EvolvedCardId.Value);
	TestEqual(TEXT("Hand refresh should update the visible card type."), HandCard->CardType, EFinalCardType::Skill);
	TestEqual(TEXT("Hand refresh should update the visible runtime AP cost."), HandCard->RuntimeCostAP, 2);
	TestTrue(TEXT("Hand refresh should update the visible display name from the evolved definition."), HandCard->DisplayName.ToString().Contains(TEXT("Evolved")));

	const FName TargetUnitId = BattleSnapshot.CurrentTargetUnitId.IsNone()
		? BattleSnapshot.Enemies[0].RuntimeUnitId
		: BattleSnapshot.CurrentTargetUnitId;
	const FFinalBattleEnemyViewData* EnemyView = FindBattleEnemyView(BattleSnapshot, TargetUnitId);
	if (!TestNotNull(TEXT("The battle snapshot should expose a live enemy target after hand refresh."), EnemyView))
	{
		return false;
	}

	const int32 EnemyHpBeforeEvolvedAttack = EnemyView->CurrentHP;
	FFinalBattleCommand Command;
	Command.CommandType = EFinalBattleCommandType::PlayCard;
	Command.CardInstanceId = HandCard->CardInstanceId;
	Command.TargetUnitId = TargetUnitId;
	TestTrue(TEXT("Playing the evolved in-hand card should succeed."), Context.BattleFlowSubsystem->SubmitBattleCommand(Command));

	BattleSnapshot = Context.BattleFlowSubsystem->GetCurrentSnapshot();
	EnemyView = FindBattleEnemyView(BattleSnapshot, TargetUnitId);
	if (!TestNotNull(TEXT("Enemy should still exist after the evolved hand card resolves."), EnemyView))
	{
		return false;
	}

	TestEqual(TEXT("The evolved hand card should resolve using the evolved card definition."), EnemyHpBeforeEvolvedAttack - EnemyView->CurrentHP, 10);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleGrowthEvolutionRefreshesDrawPileCardTest,
	"Final.Editor.RunFlow.GrowthEvolutionRefreshesDrawPileCardInstance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleGrowthEvolutionRefreshesDrawPileCardTest::RunTest(const FString& Parameters)
{
	using namespace FinalRunFlowGrowthUITests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalBattleGrowthEvolutionRefreshesDrawPileCardTest")))
	{
		return false;
	}

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.evo_draw")));
	const FFinalCharacterGrowthConfigId GrowthConfigId(FName(TEXT("growth.config.test.evo_draw")));
	const FFinalCardId OpeningCardId(FName(TEXT("card.test.evo_draw.opening")));
	const FFinalCardId BaseCardId(FName(TEXT("card.test.evo_draw.base")));
	const FFinalCardId EvolvedCardId(FName(TEXT("card.test.evo_draw.evolved")));
	const FFinalCardEvolutionId EvolutionId(FName(TEXT("evo.test.evo_draw")));
	const FFinalRuleConfigId RuleConfigId(FName(TEXT("rule.test.evo_draw")));
	const FFinalEncounterId EncounterId(FName(TEXT("encounter.test.evo_draw")));
	const FFinalEnemyId EnemyId(FName(TEXT("enemy.test.evo_draw")));

	Context.RegisterGrowthConfig(GrowthConfigId, {}, {});
	Context.RegisterCharacterDefinition(CharacterId, GrowthConfigId, TEXT("Evolution Draw Hero"));
	Context.RegisterRuleConfig(RuleConfigId, 1, 3);
	UFinalEnemyDefinition* EnemyDefinition = Context.RegisterEnemyDefinition(EnemyId, TEXT("Draw Target"), 100);
	Context.RegisterEncounterDefinition(EncounterId, Context.DataRegistry->FindRuleConfig(RuleConfigId), EnemyDefinition);
	UFinalCardDefinition* OpeningCardDefinition = Context.RegisterConfigurableDamageCard(OpeningCardId, CharacterId, TEXT("Opening Slash"), EFinalCardType::Attack, 1, 1.0f);
	OpeningCardDefinition->Keywords.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Opening")));
	Context.RegisterConfigurableDamageCard(BaseCardId, CharacterId, TEXT("Draw Base Slash"), EFinalCardType::Attack, 1, 1.0f);
	Context.RegisterConfigurableDamageCard(EvolvedCardId, CharacterId, TEXT("Draw Evolved Slash"), EFinalCardType::Skill, 2, 2.0f);
	Context.RegisterEvolutionDefinition(EvolutionId, BaseCardId, EvolvedCardId, CharacterId, TEXT("Evolve Draw Base Slash"));

	FFinalRunPersistentCharacterState CharacterState;
	CharacterState.CharacterId = CharacterId;
	CharacterState.Level = 1;
	CharacterState.BreakthroughRequiredValue = 100;
	UFinalRunSession* RunSession = Context.CreateRunSessionWithCharacterState(
		CharacterState,
		{ OpeningCardId, BaseCardId },
		EncounterId,
		RuleConfigId,
		20);
	if (!TestNotNull(TEXT("RunSession should be created for draw-pile evolution refresh test."), RunSession))
	{
		return false;
	}

	if (!TestNotNull(TEXT("Battle session should start for draw-pile evolution refresh test."), Context.GameFlowSubsystem->StartBattleFromRunSession()))
	{
		return false;
	}

	const FFinalRunState InitialRunState = RunSession->GetRunState();
	const FName TargetRunCardInstanceId = InitialRunState.RunDeck[1].InstanceId;
	FFinalBattleSnapshot BattleSnapshot = Context.BattleFlowSubsystem->GetCurrentSnapshot();
	TestNull(TEXT("The evolvable target card should still be in the draw pile before refresh."), FindHandCardViewByRunCardInstanceId(BattleSnapshot, TargetRunCardInstanceId));

	TestTrue(TEXT("Breakthrough gain should create a pending growth choice for the draw-pile refresh test."), RunSession->AddBreakthroughValue(CharacterId, 100));
	Context.RunFlowSubsystem->RefreshRunFlow(true);

	const FFinalRunGrowthChoiceInstance* EvolutionChoice = FindEvolutionChoiceForRunCardInstance(Context.RunFlowSubsystem->GetCurrentRunSnapshot(), TargetRunCardInstanceId);
	if (!TestNotNull(TEXT("Pending growth choices should include the evolution choice for the draw-pile run card instance."), EvolutionChoice))
	{
		return false;
	}

	TestTrue(TEXT("Selecting the draw-pile evolution choice should succeed."), Context.RunFlowSubsystem->SelectGrowthChoice(EvolutionChoice->ChoiceInstanceId));

	FFinalBattleCommand EndTurnCommand;
	EndTurnCommand.CommandType = EFinalBattleCommandType::EndTurn;
	TestTrue(TEXT("Ending the turn should advance battle flow so the refreshed draw-pile card can be drawn."), Context.BattleFlowSubsystem->SubmitBattleCommand(EndTurnCommand));

	BattleSnapshot = Context.BattleFlowSubsystem->GetCurrentSnapshot();
	const FFinalBattleCardViewData* DrawnEvolvedCard = FindHandCardViewByRunCardInstanceId(BattleSnapshot, TargetRunCardInstanceId);
	if (!TestNotNull(TEXT("The previously draw-pile card should become visible in hand after it is drawn."), DrawnEvolvedCard))
	{
		return false;
	}

	TestEqual(TEXT("A draw-pile refreshed card should draw into hand using the evolved card id."), DrawnEvolvedCard->CardId.Value, EvolvedCardId.Value);
	TestEqual(TEXT("A draw-pile refreshed card should draw into hand using the evolved runtime AP cost."), DrawnEvolvedCard->RuntimeCostAP, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleGrowthEvolutionRefreshesDiscardPileCardTest,
	"Final.Editor.RunFlow.GrowthEvolutionRefreshesDiscardPileCardInstance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleGrowthEvolutionRefreshesDiscardPileCardTest::RunTest(const FString& Parameters)
{
	using namespace FinalRunFlowGrowthUITests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalBattleGrowthEvolutionRefreshesDiscardPileCardTest")))
	{
		return false;
	}

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.evo_discard")));
	const FFinalCharacterGrowthConfigId GrowthConfigId(FName(TEXT("growth.config.test.evo_discard")));
	const FFinalCardId BaseCardId(FName(TEXT("card.test.evo_discard.base")));
	const FFinalCardId EvolvedCardId(FName(TEXT("card.test.evo_discard.evolved")));
	const FFinalCardEvolutionId EvolutionId(FName(TEXT("evo.test.evo_discard")));
	const FFinalRuleConfigId RuleConfigId(FName(TEXT("rule.test.evo_discard")));
	const FFinalEncounterId EncounterId(FName(TEXT("encounter.test.evo_discard")));
	const FFinalEnemyId EnemyId(FName(TEXT("enemy.test.evo_discard")));

	Context.RegisterGrowthConfig(GrowthConfigId, {}, {});
	Context.RegisterCharacterDefinition(CharacterId, GrowthConfigId, TEXT("Evolution Discard Hero"));
	Context.RegisterRuleConfig(RuleConfigId, 1, 3);
	UFinalEnemyDefinition* EnemyDefinition = Context.RegisterEnemyDefinition(EnemyId, TEXT("Discard Target"), 100);
	Context.RegisterEncounterDefinition(EncounterId, Context.DataRegistry->FindRuleConfig(RuleConfigId), EnemyDefinition);
	Context.RegisterConfigurableDamageCard(BaseCardId, CharacterId, TEXT("Discard Base Slash"), EFinalCardType::Attack, 1, 1.0f);
	Context.RegisterConfigurableDamageCard(EvolvedCardId, CharacterId, TEXT("Discard Evolved Slash"), EFinalCardType::Skill, 2, 2.0f);
	Context.RegisterEvolutionDefinition(EvolutionId, BaseCardId, EvolvedCardId, CharacterId, TEXT("Evolve Discard Base Slash"));

	UFinalRunSession* RunSession = Context.CreateRunSessionWithSingleCharacter(CharacterId, BaseCardId);
	if (!TestNotNull(TEXT("RunSession should be created for discard-pile evolution refresh test."), RunSession))
	{
		return false;
	}

	if (!TestNotNull(TEXT("Battle session should start for discard-pile evolution refresh test."), Context.GameFlowSubsystem->StartBattleFromRunSession()))
	{
		return false;
	}

	const FFinalRunState InitialRunState = RunSession->GetRunState();
	const FName TargetRunCardInstanceId = InitialRunState.RunDeck[0].InstanceId;
	FFinalBattleSnapshot BattleSnapshot = Context.BattleFlowSubsystem->GetCurrentSnapshot();
	const FFinalBattleCardViewData* StartingHandCard = FindHandCardViewByRunCardInstanceId(BattleSnapshot, TargetRunCardInstanceId);
	if (!TestNotNull(TEXT("Discard-pile refresh test should begin with the target card in hand."), StartingHandCard))
	{
		return false;
	}

	const FName TargetUnitId = BattleSnapshot.CurrentTargetUnitId.IsNone()
		? BattleSnapshot.Enemies[0].RuntimeUnitId
		: BattleSnapshot.CurrentTargetUnitId;
	FFinalBattleCommand PlayCardCommand;
	PlayCardCommand.CommandType = EFinalBattleCommandType::PlayCard;
	PlayCardCommand.CardInstanceId = StartingHandCard->CardInstanceId;
	PlayCardCommand.TargetUnitId = TargetUnitId;
	TestTrue(TEXT("Playing the base card should move it into discard before evolution."), Context.BattleFlowSubsystem->SubmitBattleCommand(PlayCardCommand));

	TestTrue(TEXT("Breakthrough gain should create a pending growth choice for the discard refresh test."), RunSession->AddBreakthroughValue(CharacterId, 100));
	Context.RunFlowSubsystem->RefreshRunFlow(true);

	const FFinalRunGrowthChoiceInstance* EvolutionChoice = FindEvolutionChoiceForRunCardInstance(Context.RunFlowSubsystem->GetCurrentRunSnapshot(), TargetRunCardInstanceId);
	if (!TestNotNull(TEXT("Pending growth choices should include the evolution choice for the discard-pile run card instance."), EvolutionChoice))
	{
		return false;
	}

	TestTrue(TEXT("Selecting the discard-pile evolution choice should succeed."), Context.RunFlowSubsystem->SelectGrowthChoice(EvolutionChoice->ChoiceInstanceId));

	FFinalBattleCommand EndTurnCommand;
	EndTurnCommand.CommandType = EFinalBattleCommandType::EndTurn;
	TestTrue(TEXT("Ending the turn should reshuffle discard into draw and redraw the evolved card."), Context.BattleFlowSubsystem->SubmitBattleCommand(EndTurnCommand));

	BattleSnapshot = Context.BattleFlowSubsystem->GetCurrentSnapshot();
	const FFinalBattleCardViewData* RedrawnEvolvedCard = FindHandCardViewByRunCardInstanceId(BattleSnapshot, TargetRunCardInstanceId);
	if (!TestNotNull(TEXT("The evolved discard-pile card should be redrawn into hand on the next turn."), RedrawnEvolvedCard))
	{
		return false;
	}

	TestEqual(TEXT("A discard-pile refreshed card should redraw using the evolved card id."), RedrawnEvolvedCard->CardId.Value, EvolvedCardId.Value);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleGrowthEvolutionRefreshesConsumePileCardTest,
	"Final.Editor.RunFlow.GrowthEvolutionRefreshesConsumePileCardInstance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleGrowthEvolutionRefreshesConsumePileCardTest::RunTest(const FString& Parameters)
{
	using namespace FinalRunFlowGrowthUITests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalBattleGrowthEvolutionRefreshesConsumePileCardTest")))
	{
		return false;
	}

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.evo_consume")));
	const FFinalCharacterGrowthConfigId GrowthConfigId(FName(TEXT("growth.config.test.evo_consume")));
	const FFinalCardId BaseCardId(FName(TEXT("card.test.evo_consume.base")));
	const FFinalCardId EvolvedCardId(FName(TEXT("card.test.evo_consume.evolved")));
	const FFinalCardEvolutionId EvolutionId(FName(TEXT("evo.test.evo_consume")));
	const FFinalRuleConfigId RuleConfigId(FName(TEXT("rule.test.evo_consume")));
	const FFinalEncounterId EncounterId(FName(TEXT("encounter.test.evo_consume")));
	const FFinalEnemyId EnemyId(FName(TEXT("enemy.test.evo_consume")));

	Context.RegisterGrowthConfig(GrowthConfigId, {}, {});
	Context.RegisterCharacterDefinition(CharacterId, GrowthConfigId, TEXT("Evolution Consume Hero"));
	Context.RegisterRuleConfig(RuleConfigId, 1, 3);
	UFinalEnemyDefinition* EnemyDefinition = Context.RegisterEnemyDefinition(EnemyId, TEXT("Consume Target"), 100);
	Context.RegisterEncounterDefinition(EncounterId, Context.DataRegistry->FindRuleConfig(RuleConfigId), EnemyDefinition);
	Context.RegisterConfigurableDamageCard(BaseCardId, CharacterId, TEXT("Consume Base Slash"), EFinalCardType::Attack, 1, 1.0f, true);
	Context.RegisterConfigurableDamageCard(EvolvedCardId, CharacterId, TEXT("Consume Evolved Slash"), EFinalCardType::Skill, 2, 2.0f, true);
	Context.RegisterEvolutionDefinition(EvolutionId, BaseCardId, EvolvedCardId, CharacterId, TEXT("Evolve Consume Base Slash"));

	UFinalRunSession* RunSession = Context.CreateRunSessionWithSingleCharacter(CharacterId, BaseCardId);
	if (!TestNotNull(TEXT("RunSession should be created for consume-pile evolution refresh test."), RunSession))
	{
		return false;
	}

	if (!TestNotNull(TEXT("Battle session should start for consume-pile evolution refresh test."), Context.GameFlowSubsystem->StartBattleFromRunSession()))
	{
		return false;
	}

	const FFinalRunState InitialRunState = RunSession->GetRunState();
	const FName TargetRunCardInstanceId = InitialRunState.RunDeck[0].InstanceId;
	FFinalBattleSnapshot BattleSnapshot = Context.BattleFlowSubsystem->GetCurrentSnapshot();
	const FFinalBattleCardViewData* StartingHandCard = FindHandCardViewByRunCardInstanceId(BattleSnapshot, TargetRunCardInstanceId);
	if (!TestNotNull(TEXT("Consume-pile refresh test should begin with the target card in hand."), StartingHandCard))
	{
		return false;
	}

	const FName TargetUnitId = BattleSnapshot.CurrentTargetUnitId.IsNone()
		? BattleSnapshot.Enemies[0].RuntimeUnitId
		: BattleSnapshot.CurrentTargetUnitId;
	FFinalBattleCommand PlayCardCommand;
	PlayCardCommand.CommandType = EFinalBattleCommandType::PlayCard;
	PlayCardCommand.CardInstanceId = StartingHandCard->CardInstanceId;
	PlayCardCommand.TargetUnitId = TargetUnitId;
	TestTrue(TEXT("Playing the consume card should move it into the consume pile before evolution."), Context.BattleFlowSubsystem->SubmitBattleCommand(PlayCardCommand));

	TestTrue(TEXT("Breakthrough gain should create a pending growth choice for the consume refresh test."), RunSession->AddBreakthroughValue(CharacterId, 100));
	Context.RunFlowSubsystem->RefreshRunFlow(true);

	const FFinalRunGrowthChoiceInstance* EvolutionChoice = FindEvolutionChoiceForRunCardInstance(Context.RunFlowSubsystem->GetCurrentRunSnapshot(), TargetRunCardInstanceId);
	if (!TestNotNull(TEXT("Pending growth choices should include the evolution choice for the consume-pile run card instance."), EvolutionChoice))
	{
		return false;
	}

	TestTrue(TEXT("Selecting the consume-pile evolution choice should succeed."), Context.RunFlowSubsystem->SelectGrowthChoice(EvolutionChoice->ChoiceInstanceId));
	TestEqual(TEXT("Reissuing the explicit battle-card refresh bridge should still locate the consumed battle instance."), Context.GameFlowSubsystem->TryRefreshActiveBattleCardFromRunState(TargetRunCardInstanceId), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFinalBattleGrowthEvolutionDoesNotRefreshGeneratedCardTest,
	"Final.Editor.RunFlow.GrowthEvolutionDoesNotRefreshGeneratedCardInstance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFinalBattleGrowthEvolutionDoesNotRefreshGeneratedCardTest::RunTest(const FString& Parameters)
{
	using namespace FinalRunFlowGrowthUITests;

	FAutomationContext Context;
	if (!Context.Initialize(*this, TEXT("FinalBattleGrowthEvolutionDoesNotRefreshGeneratedCardTest")))
	{
		return false;
	}

	const FFinalCharacterId CharacterId(FName(TEXT("character.test.evo_generated")));
	const FFinalCharacterGrowthConfigId GrowthConfigId(FName(TEXT("growth.config.test.evo_generated")));
	const FFinalCardId BaseCardId(FName(TEXT("card.test.evo_generated.base")));
	const FFinalCardId EvolvedCardId(FName(TEXT("card.test.evo_generated.evolved")));
	const FFinalCardId GeneratedCardId(FName(TEXT("card.test.evo_generated.temp")));
	const FFinalCardEvolutionId EvolutionId(FName(TEXT("evo.test.evo_generated")));
	const FFinalRuleConfigId RuleConfigId(FName(TEXT("rule.test.evo_generated")));
	const FFinalEncounterId EncounterId(FName(TEXT("encounter.test.evo_generated")));
	const FFinalEnemyId EnemyId(FName(TEXT("enemy.test.evo_generated")));

	Context.RegisterGrowthConfig(GrowthConfigId, {}, {});
	Context.RegisterCharacterDefinition(CharacterId, GrowthConfigId, TEXT("Evolution Generated Hero"));
	Context.RegisterRuleConfig(RuleConfigId, 1, 3);
	UFinalEnemyDefinition* EnemyDefinition = Context.RegisterEnemyDefinition(EnemyId, TEXT("Generated Target"), 100);
	Context.RegisterEncounterDefinition(EncounterId, Context.DataRegistry->FindRuleConfig(RuleConfigId), EnemyDefinition);

	UFinalCardDefinition* GeneratedCardDefinition = Context.RegisterConfigurableDamageCard(GeneratedCardId, CharacterId, TEXT("Generated Temp Slash"), EFinalCardType::Skill, 0, 1.0f);
	UFinalCardDefinition* BaseCardDefinition = Context.RegisterCardDefinition(BaseCardId, CharacterId, TEXT("Generator Base Slash"));
	BaseCardDefinition->BaseCostAP = 1;
	BaseCardDefinition->CardType = EFinalCardType::Skill;
	BaseCardDefinition->Effects.Reset();
	UFinalBattleEffectGenerateCard* GenerateCardEffect = NewObject<UFinalBattleEffectGenerateCard>(BaseCardDefinition);
	GenerateCardEffect->EffectId = TEXT("effect.test.generate_temp_card");
	GenerateCardEffect->GenerateCount = 1;
	GenerateCardEffect->GeneratedCardDefinition = GeneratedCardDefinition;
	GenerateCardEffect->bGeneratedCard = true;
	GenerateCardEffect->bTemporaryCard = true;
	BaseCardDefinition->Effects.Add(GenerateCardEffect);

	Context.RegisterConfigurableDamageCard(EvolvedCardId, CharacterId, TEXT("Generator Evolved Slash"), EFinalCardType::Attack, 2, 2.0f);
	Context.RegisterEvolutionDefinition(EvolutionId, BaseCardId, EvolvedCardId, CharacterId, TEXT("Evolve Generator Base Slash"));

	UFinalRunSession* RunSession = Context.CreateRunSessionWithSingleCharacter(CharacterId, BaseCardId);
	if (!TestNotNull(TEXT("RunSession should be created for generated-card exclusion test."), RunSession))
	{
		return false;
	}

	if (!TestNotNull(TEXT("Battle session should start for generated-card exclusion test."), Context.GameFlowSubsystem->StartBattleFromRunSession()))
	{
		return false;
	}

	const FFinalRunState InitialRunState = RunSession->GetRunState();
	const FName TargetRunCardInstanceId = InitialRunState.RunDeck[0].InstanceId;
	FFinalBattleSnapshot BattleSnapshot = Context.BattleFlowSubsystem->GetCurrentSnapshot();
	const FFinalBattleCardViewData* StartingHandCard = FindHandCardViewByRunCardInstanceId(BattleSnapshot, TargetRunCardInstanceId);
	if (!TestNotNull(TEXT("Generated-card exclusion test should begin with the generator card in hand."), StartingHandCard))
	{
		return false;
	}

	FFinalBattleCommand PlayCardCommand;
	PlayCardCommand.CommandType = EFinalBattleCommandType::PlayCard;
	PlayCardCommand.CardInstanceId = StartingHandCard->CardInstanceId;
	PlayCardCommand.TargetUnitId = BattleSnapshot.CurrentTargetUnitId.IsNone() ? BattleSnapshot.Enemies[0].RuntimeUnitId : BattleSnapshot.CurrentTargetUnitId;
	TestTrue(TEXT("Playing the generator card should create a generated temporary card."), Context.BattleFlowSubsystem->SubmitBattleCommand(PlayCardCommand));

	BattleSnapshot = Context.BattleFlowSubsystem->GetCurrentSnapshot();
	if (!TestEqual(TEXT("After the generator resolves, the temporary generated card should be the only hand card."), BattleSnapshot.HandCards.Num(), 1))
	{
		return false;
	}

	const FFinalBattleCardViewData& GeneratedHandCard = BattleSnapshot.HandCards[0];
	TestTrue(TEXT("Generated temporary cards should not inherit SourceRunCardInstanceId by default."), GeneratedHandCard.SourceRunCardInstanceId.IsNone());
	TestEqual(TEXT("Generated temporary card should initially use its generated card id."), GeneratedHandCard.CardId.Value, GeneratedCardId.Value);

	TestTrue(TEXT("Breakthrough gain should create a pending growth choice for the generated-card exclusion test."), RunSession->AddBreakthroughValue(CharacterId, 100));
	Context.RunFlowSubsystem->RefreshRunFlow(true);

	const FFinalRunGrowthChoiceInstance* EvolutionChoice = FindEvolutionChoiceForRunCardInstance(Context.RunFlowSubsystem->GetCurrentRunSnapshot(), TargetRunCardInstanceId);
	if (!TestNotNull(TEXT("Pending growth choices should include the evolution choice for the original generator run card instance."), EvolutionChoice))
	{
		return false;
	}

	TestTrue(TEXT("Selecting the evolution choice should not fail when a generated card is currently in hand."), Context.RunFlowSubsystem->SelectGrowthChoice(EvolutionChoice->ChoiceInstanceId));

	BattleSnapshot = Context.BattleFlowSubsystem->GetCurrentSnapshot();
	if (!TestEqual(TEXT("The generated temporary hand card should remain in hand after the original run card evolves."), BattleSnapshot.HandCards.Num(), 1))
	{
		return false;
	}

	TestTrue(TEXT("The generated temporary hand card should still have no SourceRunCardInstanceId after evolution refresh."), BattleSnapshot.HandCards[0].SourceRunCardInstanceId.IsNone());
	TestEqual(TEXT("Generated temporary cards should not be refreshed to the evolved run card definition."), BattleSnapshot.HandCards[0].CardId.Value, GeneratedCardId.Value);
	return true;
}

#endif
