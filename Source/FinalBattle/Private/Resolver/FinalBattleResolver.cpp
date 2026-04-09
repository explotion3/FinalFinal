#include "Resolver/FinalBattleResolver.h"

#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalEnemyDefinition.h"
#include "Runtime/FinalBattleState.h"

DEFINE_LOG_CATEGORY_STATIC(LogFinalBattleResolver, Log, All);

void FFinalBattleResolver::Initialize(FFinalBattleState& State, const UFinalBattleEncounterDefinition* EncounterDefinition, const UFinalBattleRuleConfig* RuleConfig, const FFinalBattleInitContext& InitContext) const
{
	State = FFinalBattleState{};
	State.BattleId = FGuid::NewGuid();
	State.CurrentRound = 1;
	State.CurrentAP = RuleConfig ? RuleConfig->InitialAP : 0;
	State.CurrentEP = 0;
	State.TeamCurrentHP = 0;
	State.TeamMaxHP = 0;
	TMap<FName, FName> TemplateToRuntimeUnitMap;

	if (EncounterDefinition)
	{
		State.EncounterId = EncounterDefinition->EncounterId;
	}

	if (RuleConfig)
	{
		State.RuleConfigId = RuleConfig->RuleConfigId;
	}

	for (int32 Index = 0; Index < InitContext.PartyMembers.Num(); ++Index)
	{
		const FFinalBattleCharacterInitData& PartyEntry = InitContext.PartyMembers[Index];
		if (PartyEntry.CharacterDefinition == nullptr || !PartyEntry.CharacterDefinition->CharacterId.IsValid())
		{
			continue;
		}

		FFinalBattleCharacterState CharacterState;
		CharacterState.RuntimeUnitId = MakePlayerUnitId(Index);
		CharacterState.CharacterId = PartyEntry.CharacterDefinition->CharacterId;
		CharacterState.CurrentStress = PartyEntry.CurrentStress;
		CharacterState.bCollapsed = PartyEntry.bCollapsed;
		CharacterState.CurrentAwakenCount = PartyEntry.CurrentAwakenCount;
		CharacterState.CollapseCount = PartyEntry.CollapseCount;
		CharacterState.RuntimeAttack = PartyEntry.CharacterDefinition->BaseAttack;
		CharacterState.RuntimeDefense = PartyEntry.CharacterDefinition->BaseDefense;
		CharacterState.RuntimeBreakRate = PartyEntry.CharacterDefinition->BaseBreakRate;
		State.Characters.Add(MoveTemp(CharacterState));

		TemplateToRuntimeUnitMap.Add(PartyEntry.CharacterDefinition->CharacterId.Value, State.Characters.Last().RuntimeUnitId);

		if (!PartyEntry.bCollapsed)
		{
			State.TeamMaxHP += PartyEntry.CharacterDefinition->BaseVitalShare;
		}
	}

	State.TeamCurrentHP = InitContext.TeamCurrentHP > 0
		? FMath::Min(InitContext.TeamCurrentHP, State.TeamMaxHP)
		: State.TeamMaxHP;

	for (UFinalCardDefinition* CardDefinition : InitContext.DeckDefinitions)
	{
		if (CardDefinition == nullptr || !CardDefinition->CardId.IsValid())
		{
			continue;
		}

		FFinalBattleCardInstance CardInstance;
		CardInstance.CardInstanceId = FGuid::NewGuid();
		CardInstance.CardId = CardDefinition->CardId;
		CardInstance.RuntimeCostAP = CardDefinition->BaseCostAP;
		CardInstance.RuntimeKeywords = CardDefinition->Keywords;

		if (const FName* RuntimeOwnerUnitId = TemplateToRuntimeUnitMap.Find(CardDefinition->OwnerUnitId))
		{
			CardInstance.RuntimeOwnerUnitId = *RuntimeOwnerUnitId;
		}
		else
		{
			CardInstance.RuntimeOwnerUnitId = CardDefinition->OwnerUnitId;
		}

		State.CardInstances.Add(CardInstance);
		State.DeckState.DrawPileCardInstanceIds.Add(CardInstance.CardInstanceId);
	}

	const int32 InitialHandSize = RuleConfig ? FMath::Max(RuleConfig->InitialHandSize, 0) : 0;
	const int32 CardsToDraw = FMath::Min(InitialHandSize, State.DeckState.DrawPileCardInstanceIds.Num());
	for (int32 DrawIndex = 0; DrawIndex < CardsToDraw; ++DrawIndex)
	{
		const FGuid DrawnCardId = State.DeckState.DrawPileCardInstanceIds[0];
		State.DeckState.DrawPileCardInstanceIds.RemoveAt(0);
		State.DeckState.HandCardInstanceIds.Add(DrawnCardId);
	}

	if (!EncounterDefinition)
	{
		return;
	}

	for (int32 Index = 0; Index < EncounterDefinition->EnemyRoster.Num(); ++Index)
	{
		const FFinalEnemyRosterEntry& Entry = EncounterDefinition->EnemyRoster[Index];
		UFinalEnemyDefinition* LoadedEnemy = Entry.EnemyDefinition.LoadSynchronous();

		FFinalBattleEnemyState EnemyState;
		EnemyState.RuntimeUnitId = MakeEnemyUnitId(Index);
		EnemyState.PositionIndex = Entry.PositionIndex;
		EnemyState.SpawnWave = Entry.SpawnWave;

		if (LoadedEnemy)
		{
			EnemyState.EnemyId = LoadedEnemy->EnemyId;
			EnemyState.DisplayName = LoadedEnemy->DisplayName;
			EnemyState.CurrentHP = LoadedEnemy->MaxHP;
			EnemyState.CurrentBreakValue = LoadedEnemy->MaxBreakValue;
			EnemyState.CurrentInitiative = LoadedEnemy->InitialInitiativeValue;
			EnemyState.CurrentIntentText = FText::FromString(TEXT("Intent Pending"));
		}
		else
		{
			EnemyState.DisplayName = FText::FromString(TEXT("Missing Enemy Definition"));
		}

		State.Enemies.Add(MoveTemp(EnemyState));
	}

	UE_LOG(LogFinalBattleResolver, Log, TEXT("Initialized battle session with %d enemy entries."), State.Enemies.Num());
}

FName FFinalBattleResolver::MakePlayerUnitId(int32 Index)
{
	return FName(*FString::Printf(TEXT("unit_player_%d"), Index + 1));
}

FFinalBattleEvent FFinalBattleResolver::ExecuteCommand(FFinalBattleState& State, const FFinalBattleCommand& Command, const UFinalBattleRuleConfig* RuleConfig) const
{
	FFinalBattleEvent Event;
	Event.Round = State.CurrentRound;

	switch (Command.CommandType)
	{
	case EFinalBattleCommandType::PlayCard:
		if (State.CurrentAP <= 0)
		{
			Event.EventType = EFinalBattleEventType::CommandRejected;
			Event.Message = FText::FromString(TEXT("No AP available."));
			return Event;
		}

		--State.CurrentAP;
		Event.EventType = EFinalBattleEventType::CommandAccepted;
		Event.Message = FText::FromString(TEXT("PlayCard accepted."));
		break;

	case EFinalBattleCommandType::PlayUltimate:
		Event.EventType = EFinalBattleEventType::CommandAccepted;
		Event.Message = FText::FromString(TEXT("PlayUltimate accepted."));
		break;

	case EFinalBattleCommandType::EndTurn:
		++State.CurrentRound;
		State.CurrentAP = RuleConfig ? RuleConfig->InitialAP : 0;
		Event.EventType = EFinalBattleEventType::StateChanged;
		Event.Message = FText::FromString(TEXT("Turn advanced."));
		Event.Round = State.CurrentRound;
		break;

	case EFinalBattleCommandType::SelectTarget:
		State.CurrentTargetUnitId = Command.TargetUnitId;
		Event.EventType = EFinalBattleEventType::StateChanged;
		Event.Message = FText::FromString(TEXT("Target updated."));
		break;

	default:
		Event.EventType = EFinalBattleEventType::CommandRejected;
		Event.Message = FText::FromString(TEXT("Unsupported command."));
		break;
	}

	State.BattleLogEntries.Add(Event);
	return Event;
}

FFinalBattleSnapshot FFinalBattleResolver::BuildSnapshot(const FFinalBattleState& State) const
{
	FFinalBattleSnapshot Snapshot;
	Snapshot.CurrentRound = State.CurrentRound;
	Snapshot.CurrentAP = State.CurrentAP;
	Snapshot.CurrentEP = State.CurrentEP;
	Snapshot.TeamCurrentHP = State.TeamCurrentHP;
	Snapshot.TeamMaxHP = State.TeamMaxHP;

	for (const FFinalBattleCharacterState& CharacterState : State.Characters)
	{
		FFinalBattleCharacterViewData CharacterView;
		CharacterView.RuntimeUnitId = CharacterState.RuntimeUnitId;
		CharacterView.CharacterId = CharacterState.CharacterId;
		CharacterView.CurrentStress = CharacterState.CurrentStress;
		CharacterView.bCollapsed = CharacterState.bCollapsed;
		Snapshot.Characters.Add(MoveTemp(CharacterView));
	}

	for (const FFinalBattleEnemyState& EnemyState : State.Enemies)
	{
		FFinalBattleEnemyViewData EnemyView;
		EnemyView.RuntimeUnitId = EnemyState.RuntimeUnitId;
		EnemyView.EnemyId = EnemyState.EnemyId;
		EnemyView.DisplayName = EnemyState.DisplayName;
		EnemyView.CurrentHP = EnemyState.CurrentHP;
		EnemyView.CurrentBreakValue = EnemyState.CurrentBreakValue;
		EnemyView.CurrentInitiative = EnemyState.CurrentInitiative;
		EnemyView.IntentText = EnemyState.CurrentIntentText;
		Snapshot.Enemies.Add(MoveTemp(EnemyView));
	}

	for (const FGuid& CardInstanceId : State.DeckState.HandCardInstanceIds)
	{
		const FFinalBattleCardInstance* CardInstance = State.CardInstances.FindByPredicate(
			[&CardInstanceId](const FFinalBattleCardInstance& Candidate)
			{
				return Candidate.CardInstanceId == CardInstanceId;
			});

		if (!CardInstance)
		{
			continue;
		}

		FFinalBattleCardViewData CardView;
		CardView.CardInstanceId = CardInstance->CardInstanceId;
		CardView.CardId = CardInstance->CardId;
		CardView.RuntimeCostAP = CardInstance->RuntimeCostAP;
		Snapshot.HandCards.Add(MoveTemp(CardView));
	}

	return Snapshot;
}

FName FFinalBattleResolver::MakeEnemyUnitId(int32 Index)
{
	return FName(*FString::Printf(TEXT("unit_enemy_%d"), Index + 1));
}
