#include "Facade/FinalBattleSession.h"

#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Conditions/FinalBattleConditionDefinition.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "Commands/FinalBattleCommand.h"
#include "Facade/FinalBattleSessionTypes.h"
#include "Resolver/FinalBattleResolver.h"
#include "Runtime/FinalBattleCardInstance.h"
#include "Runtime/FinalBattleState.h"
#include "UObject/UObjectGlobals.h"
#include "Systems/FinalBattleCardService.h"

UFinalBattleSession::UFinalBattleSession()
{
	Resolver = new FFinalBattleResolver();
}

UFinalBattleSession::~UFinalBattleSession()
{
	delete State;
	State = nullptr;

	delete Resolver;
	Resolver = nullptr;
}

void UFinalBattleSession::InitializeSession(UFinalBattleEncounterDefinition* InEncounterDefinition, UFinalBattleRuleConfig* InRuleConfig, const FFinalBattleInitContext& InitContext)
{
	EncounterDefinition = InEncounterDefinition;
	RuleConfig = InRuleConfig;
	delete State;
	State = new FFinalBattleState();
	State->RuntimeProjectionOwner = this;

	delete Resolver;
	Resolver = new FFinalBattleResolver();
	Resolver->Initialize(*State, EncounterDefinition, RuleConfig, InitContext);
}

FFinalBattleEvent UFinalBattleSession::SubmitCommand(const FFinalBattleCommand& Command)
{
	if (State == nullptr || Resolver == nullptr)
	{
		FFinalBattleEvent Event;
		Event.EventType = EFinalBattleEventType::CommandRejected;
		Event.RejectReason = EFinalBattleCommandRejectReason::BattleNotInitialized;
		Event.ReasonTag = TEXT("battle.not_initialized");
		Event.Message = FText::FromString(TEXT("Battle session is not initialized."));
		return Event;
	}

	FFinalBattleEvent Event = Resolver->ExecuteCommand(*State, Command, RuleConfig);
	static const FFinalBattleCardService CardService;
	if (Event.RejectReason == EFinalBattleCommandRejectReason::None)
	{
		if (Command.CommandType == EFinalBattleCommandType::PlayCard && Command.CardInstanceId.IsValid())
		{
			if (FFinalBattleCardInstance* CardInstance = CardService.FindCardInstance(*State, Command.CardInstanceId))
			{
				const int32 RemovedCount = CardInstance->ModifierRecords.RemoveAll([](const FFinalBattleCardModifierRecord& ModifierRecord)
				{
					return ModifierRecord.DurationPolicy == EFinalBattleCardModifierDuration::UntilPlayed;
				});
				if (RemovedCount > 0)
				{
					CardService.ReprojectCardInstance(*State, Command.CardInstanceId, this);
				}
			}
		}
		else if (Command.CommandType == EFinalBattleCommandType::EndTurn)
		{
			CardService.ClearCardModifiersExpiringAtPlayerTurnEnd(*State, this);
			CardService.ClearCardModifiersByDuration(*State, this, EFinalBattleCardModifierDuration::EndOfTurn);
			CardService.ClearCardModifiersByDuration(*State, this, EFinalBattleCardModifierDuration::EndOfRound);
		}
	}

	return Event;
}

FFinalBattleSnapshot UFinalBattleSession::GetSnapshot() const
{
	if (State == nullptr || Resolver == nullptr)
	{
		return FFinalBattleSnapshot{};
	}

	return Resolver->BuildSnapshot(*State);
}

TArray<FFinalBattleEvent> UFinalBattleSession::GetBattleLogEntries() const
{
	return State != nullptr ? State->BattleLogEntries : TArray<FFinalBattleEvent>{};
}

TArray<FFinalBattleEvent> UFinalBattleSession::GetBattleEventsSince(const int32 LastSeenEventSequence) const
{
	if (State == nullptr)
	{
		return {};
	}

	TArray<FFinalBattleEvent> Events;
	for (const FFinalBattleEvent& Event : State->BattleLogEntries)
	{
		if (Event.EventSequence > LastSeenEventSequence)
		{
			Events.Add(Event);
		}
	}

	return Events;
}

int32 UFinalBattleSession::GetLatestBattleEventSequence() const
{
	return State != nullptr ? State->LastEventSequence : 0;
}

TArray<FFinalBattleGrowthFactBatch> UFinalBattleSession::GetGrowthFactBatchesSince(const int32 LastSeenBatchSequence) const
{
	if (State == nullptr)
	{
		return {};
	}

	TArray<FFinalBattleGrowthFactBatch> Batches;
	for (const FFinalBattleGrowthFactBatch& Batch : State->GrowthFactBatches)
	{
		if (Batch.BatchSequence > LastSeenBatchSequence)
		{
			Batches.Add(Batch);
		}
	}

	return Batches;
}

int32 UFinalBattleSession::GetLatestGrowthFactBatchSequence() const
{
	return State != nullptr ? State->LastGrowthFactBatchSequence : 0;
}

bool UFinalBattleSession::AddCardModifier(const FGuid& CardInstanceId, const FFinalBattleCardModifierRecord& ModifierRecord)
{
	if (State == nullptr || !CardInstanceId.IsValid())
	{
		return false;
	}

	static const FFinalBattleCardService CardService;
	return CardService.AddCardModifier(*State, CardInstanceId, this, ModifierRecord);
}

bool UFinalBattleSession::RemoveCardModifier(const FGuid& CardInstanceId, const FName ModifierId)
{
	if (State == nullptr || !CardInstanceId.IsValid())
	{
		return false;
	}

	static const FFinalBattleCardService CardService;
	return CardService.RemoveCardModifier(*State, CardInstanceId, this, ModifierId);
}

int32 UFinalBattleSession::ClearCardModifiersByDuration(const EFinalBattleCardModifierDuration DurationPolicy)
{
	if (State == nullptr)
	{
		return 0;
	}

	static const FFinalBattleCardService CardService;
	return CardService.ClearCardModifiersByDuration(*State, this, DurationPolicy);
}

bool UFinalBattleSession::ReprojectCardInstance(const FGuid& CardInstanceId)
{
	if (State == nullptr || !CardInstanceId.IsValid())
	{
		return false;
	}

	static const FFinalBattleCardService CardService;
	return CardService.ReprojectCardInstance(*State, CardInstanceId, this);
}

bool UFinalBattleSession::RefreshCharacterRuntimeStats(const FFinalBattleCharacterRuntimeStats& RuntimeStats)
{
	if (State == nullptr || !RuntimeStats.CharacterId.IsValid())
	{
		return false;
	}

	FFinalBattleCharacterState* CharacterState = State->Characters.FindByPredicate([&RuntimeStats](const FFinalBattleCharacterState& Candidate)
	{
		return Candidate.CharacterId == RuntimeStats.CharacterId;
	});
	if (CharacterState == nullptr)
	{
		return false;
	}

	CharacterState->VitalShare = RuntimeStats.VitalShare;
	CharacterState->StressCap = RuntimeStats.StressCap;
	CharacterState->CurrentStress = FMath::Min(CharacterState->CurrentStress, CharacterState->StressCap);
	CharacterState->RuntimeAttack = RuntimeStats.RuntimeAttack;
	CharacterState->RuntimeDefense = RuntimeStats.RuntimeDefense;
	CharacterState->RuntimeBreakRate = RuntimeStats.RuntimeBreakRate;
	CharacterState->RuntimeCritChance = RuntimeStats.RuntimeCritChance;
	CharacterState->RuntimeCritDamage = RuntimeStats.RuntimeCritDamage;

	int32 UpdatedTeamMaxHP = 0;
	for (const FFinalBattleCharacterState& TeamCharacterState : State->Characters)
	{
		if (!TeamCharacterState.bCollapsed)
		{
			UpdatedTeamMaxHP += TeamCharacterState.VitalShare;
		}
	}

	State->TeamMaxHP = FMath::Max(UpdatedTeamMaxHP, 0);
	State->TeamCurrentHP = FMath::Min(State->TeamCurrentHP, State->TeamMaxHP);
	return true;
}

int32 UFinalBattleSession::RefreshCardsForRunCardInstance(const FFinalBattleCardRefreshRequest& RefreshRequest)
{
	if (State == nullptr)
	{
		return 0;
	}

	static const FFinalBattleCardService CardService;
	return CardService.RefreshCardsForRunCardInstance(*State, this, RefreshRequest);
}

FFinalBattleCardProjectionView UFinalBattleSession::GetCardProjectionView(const FGuid& CardInstanceId) const
{
	if (State == nullptr || !CardInstanceId.IsValid())
	{
		return FFinalBattleCardProjectionView{};
	}

	static const FFinalBattleCardService CardService;
	return CardService.BuildProjectionView(*State, CardInstanceId);
}

void UFinalBattleSession::ResetSession()
{
	delete State;
	State = nullptr;

	delete Resolver;
	Resolver = new FFinalBattleResolver();
	EncounterDefinition = nullptr;
	RuleConfig = nullptr;
}

bool UFinalBattleSession::HasActiveBattle() const
{
	return State != nullptr;
}

void UFinalBattleSession::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	Super::AddReferencedObjects(InThis, Collector);

	UFinalBattleSession* This = CastChecked<UFinalBattleSession>(InThis);
	if (This->State == nullptr)
	{
		return;
	}

	auto AddRuntimeTriggerReferences = [&Collector](TArray<FFinalBattleRuntimeTriggerState>& TriggerStates)
	{
		for (FFinalBattleRuntimeTriggerState& TriggerState : TriggerStates)
		{
			for (TObjectPtr<UFinalBattleConditionDefinition>& Condition : TriggerState.TriggerDefinition.Conditions)
			{
				Collector.AddReferencedObject(Condition);
			}

			for (TObjectPtr<UFinalBattleEffectDefinition>& Effect : TriggerState.TriggerDefinition.Effects)
			{
				Collector.AddReferencedObject(Effect);
			}
		}
	};

	for (FFinalBattleStartRelicInput& ActiveRelic : This->State->ActiveRelics)
	{
		for (FFinalRuntimeTriggerDefinition& TriggerDefinition : ActiveRelic.RuntimeTriggers)
		{
			for (TObjectPtr<UFinalBattleConditionDefinition>& Condition : TriggerDefinition.Conditions)
			{
				Collector.AddReferencedObject(Condition);
			}

			for (TObjectPtr<UFinalBattleEffectDefinition>& Effect : TriggerDefinition.Effects)
			{
				Collector.AddReferencedObject(Effect);
			}
		}
	}

	for (FFinalBattleRelicRuntimeState& RuntimeState : This->State->RelicRuntimeStates)
	{
		AddRuntimeTriggerReferences(RuntimeState.TriggerStates);
	}

	for (FFinalBattleCharacterState& CharacterState : This->State->Characters)
	{
		AddRuntimeTriggerReferences(CharacterState.TriggerStates);
	}

	for (FFinalBattleCardInstance& CardInstance : This->State->CardInstances)
	{
		Collector.AddReferencedObject(CardInstance.BaseDefinition);
		Collector.AddReferencedObject(CardInstance.ProjectedDefinition);

		for (FFinalBattleCardModifierRecord& ModifierRecord : CardInstance.ModifierRecords)
		{
			for (TObjectPtr<UFinalBattleEffectDefinition>& ReplacementEffect : ModifierRecord.ReplacementEffects)
			{
				Collector.AddReferencedObject(ReplacementEffect);
			}

			for (FFinalBattleCardEffectPatch& EffectPatch : ModifierRecord.EffectPatches)
			{
				Collector.AddReferencedObject(EffectPatch.EffectDefinition);
			}

			for (FFinalBattleCardConditionPatch& ConditionPatch : ModifierRecord.ConditionPatches)
			{
				Collector.AddReferencedObject(ConditionPatch.ConditionDefinition);
			}
		}
	}
}
