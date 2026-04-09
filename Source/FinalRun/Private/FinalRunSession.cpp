#include "Facade/FinalRunSession.h"

void UFinalRunSession::InitializeRun()
{
	CurrentState = FFinalRunState{};
	CurrentState.TeamCurrentHP = 0;
	CurrentState.Gold = 0;
}

void UFinalRunSession::ConfigureBattleStartState(const FFinalEncounterId& EncounterId, const FFinalRuleConfigId& RuleConfigId, const TArray<FFinalRunPersistentCharacterState>& PartyStates, const TArray<FFinalCardId>& DeckCardIds, int32 InTeamCurrentHP)
{
	CurrentState.CurrentEncounterId = EncounterId;
	CurrentState.CurrentRuleConfigId = RuleConfigId;
	CurrentState.Characters = PartyStates;
	CurrentState.RunDeck = DeckCardIds;
	CurrentState.TeamCurrentHP = InTeamCurrentHP;
}

bool UFinalRunSession::SubmitRunCommand(const FFinalRunCommand& Command)
{
	switch (Command.CommandType)
	{
	case EFinalRunCommandType::AdvanceNode:
	case EFinalRunCommandType::ResolveReward:
	case EFinalRunCommandType::ResolveEvent:
	case EFinalRunCommandType::ResolveShop:
		return true;

	default:
		return false;
	}
}

bool UFinalRunSession::HasValidBattleStartState() const
{
	return CurrentState.CurrentEncounterId.IsValid()
		&& CurrentState.Characters.Num() > 0
		&& CurrentState.RunDeck.Num() > 0;
}

FFinalBattleStartRequest UFinalRunSession::BuildBattleStartRequest() const
{
	FFinalBattleStartRequest Request;
	Request.EncounterId = CurrentState.CurrentEncounterId;
	Request.RuleConfigId = CurrentState.CurrentRuleConfigId;
	Request.TeamCurrentHP = CurrentState.TeamCurrentHP;
	Request.DeckCardIds = CurrentState.RunDeck;
	Request.PartyStates = CurrentState.Characters;

	for (const FFinalRunPersistentCharacterState& CharacterState : CurrentState.Characters)
	{
		Request.PartyCharacterIds.Add(CharacterState.CharacterId);
	}

	return Request;
}

void UFinalRunSession::ApplyBattleResult(const FFinalBattleResult& Result)
{
	CurrentState.TeamCurrentHP = Result.TeamCurrentHP;

	if (Result.Outcome == EFinalBattleOutcome::Victory)
	{
		CurrentState.Gold += Result.RewardGold;
	}

	if (Result.UpdatedCharacterStates.Num() > 0)
	{
		CurrentState.Characters = Result.UpdatedCharacterStates;
	}
}

FFinalRunState UFinalRunSession::GetRunState() const
{
	return CurrentState;
}
