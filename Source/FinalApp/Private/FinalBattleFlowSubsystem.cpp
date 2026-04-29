#include "Subsystems/FinalBattleFlowSubsystem.h"

#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalPassiveDefinition.h"
#include "BattleBridge/FinalBattleGrowthStatProjection.h"
#include "Facade/FinalBattleSession.h"
#include "Queries/FinalDataRegistry.h"

void UFinalBattleFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LastCommandEvent = FFinalBattleEvent{};
	BroadcastBattleLogCount = 0;
}

UFinalBattleSession* UFinalBattleFlowSubsystem::CreateBattleSessionFromStartRequest(const FFinalBattleStartRequest& StartRequest)
{
	LastFailureReason = FText::GetEmpty();
	LastCommandEvent = FFinalBattleEvent{};
	LastStartRequest = StartRequest;

	UFinalDataRegistry* DataRegistry = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalDataRegistry>() : nullptr;
	if (DataRegistry == nullptr)
	{
		LastFailureReason = FText::FromString(TEXT("FinalDataRegistry is unavailable."));
		return nullptr;
	}

	UFinalBattleEncounterDefinition* EncounterDefinition = DataRegistry->FindEncounterDefinition(StartRequest.EncounterId);
	if (EncounterDefinition == nullptr)
	{
		LastFailureReason = FText::Format(NSLOCTEXT("FinalBattleFlow", "MissingEncounter", "Encounter {0} is not registered."), FText::FromName(StartRequest.EncounterId.Value));
		return nullptr;
	}

	UFinalBattleRuleConfig* RuleConfig = DataRegistry->FindRuleConfig(StartRequest.RuleConfigId);
	if (RuleConfig == nullptr && !StartRequest.RuleConfigId.IsValid())
	{
		RuleConfig = EncounterDefinition->RuleConfig.LoadSynchronous();
	}

	if (RuleConfig == nullptr)
	{
		LastFailureReason = FText::Format(NSLOCTEXT("FinalBattleFlow", "MissingRuleConfig", "Rule config {0} is not registered."), FText::FromName(StartRequest.RuleConfigId.Value));
		return nullptr;
	}

	FFinalBattleInitContext InitContext;
	if (!BuildInitContext(StartRequest, InitContext))
	{
		if (LastFailureReason.IsEmpty())
		{
			LastFailureReason = FText::FromString(TEXT("Failed to build battle init context from run state."));
		}
		return nullptr;
	}

	LastStartRequest = StartRequest;
	if (ActiveBattleSession)
	{
		ClearActiveBattleSession();
	}

	ActiveBattleSession = NewObject<UFinalBattleSession>(this);
	ActiveBattleSession->InitializeSession(EncounterDefinition, RuleConfig, InitContext);
	BroadcastBattleLogCount = 0;
	BroadcastPendingBattleEvents();
	BroadcastSnapshot();
	return ActiveBattleSession;
}

bool UFinalBattleFlowSubsystem::SubmitBattleCommand(const FFinalBattleCommand& Command)
{
	if (!ActiveBattleSession)
	{
		LastCommandEvent = FFinalBattleEvent{};
		LastCommandEvent.EventType = EFinalBattleEventType::CommandRejected;
		LastCommandEvent.RejectReason = EFinalBattleCommandRejectReason::BattleNotInitialized;
		LastCommandEvent.ReasonTag = TEXT("battle.not_initialized");
		LastCommandEvent.Message = FText::FromString(TEXT("当前没有可操作的战斗。"));
		LastFailureReason = LastCommandEvent.Message;
		return false;
	}

	LastCommandEvent = ActiveBattleSession->SubmitCommand(Command);
	LastFailureReason = LastCommandEvent.EventType == EFinalBattleEventType::CommandRejected
		? LastCommandEvent.Message
		: FText::GetEmpty();
	BroadcastPendingBattleEvents();
	BroadcastSnapshot();
	return LastCommandEvent.EventType != EFinalBattleEventType::CommandRejected;
}

void UFinalBattleFlowSubsystem::ClearActiveBattleSession()
{
	if (ActiveBattleSession)
	{
		ActiveBattleSession->ResetSession();
	}

	ActiveBattleSession = nullptr;
	LastCommandEvent = FFinalBattleEvent{};
	BroadcastBattleLogCount = 0;
	OnBattleSnapshotChanged.Broadcast(FFinalBattleSnapshot{});
}

FFinalBattleSnapshot UFinalBattleFlowSubsystem::GetCurrentSnapshot() const
{
	return ActiveBattleSession ? ActiveBattleSession->GetSnapshot() : FFinalBattleSnapshot{};
}

UFinalBattleSession* UFinalBattleFlowSubsystem::GetActiveBattleSession() const
{
	return ActiveBattleSession;
}

FText UFinalBattleFlowSubsystem::GetLastFailureReason() const
{
	return LastFailureReason;
}

FFinalBattleEvent UFinalBattleFlowSubsystem::GetLastCommandEvent() const
{
	return LastCommandEvent;
}

TArray<FFinalBattleEvent> UFinalBattleFlowSubsystem::GetBattleLogEntries() const
{
	return ActiveBattleSession ? ActiveBattleSession->GetBattleLogEntries() : TArray<FFinalBattleEvent>{};
}

TArray<FFinalBattleEvent> UFinalBattleFlowSubsystem::GetBattleEventsSince(const int32 LastSeenEventSequence) const
{
	return ActiveBattleSession ? ActiveBattleSession->GetBattleEventsSince(LastSeenEventSequence) : TArray<FFinalBattleEvent>{};
}

int32 UFinalBattleFlowSubsystem::GetLatestBattleEventSequence() const
{
	return ActiveBattleSession ? ActiveBattleSession->GetLatestBattleEventSequence() : 0;
}

bool UFinalBattleFlowSubsystem::RefreshCharacterRuntimeStats(const FFinalBattleCharacterRuntimeStats& RuntimeStats)
{
	if (ActiveBattleSession == nullptr)
	{
		return false;
	}

	const bool bRefreshed = ActiveBattleSession->RefreshCharacterRuntimeStats(RuntimeStats);
	if (bRefreshed)
	{
		BroadcastSnapshot();
	}

	return bRefreshed;
}

int32 UFinalBattleFlowSubsystem::RefreshCardsForRunCardInstance(const FFinalBattleCardRefreshRequest& RefreshRequest)
{
	if (ActiveBattleSession == nullptr)
	{
		return 0;
	}

	const int32 RefreshedCount = ActiveBattleSession->RefreshCardsForRunCardInstance(RefreshRequest);
	if (RefreshedCount > 0)
	{
		BroadcastSnapshot();
	}

	return RefreshedCount;
}

void UFinalBattleFlowSubsystem::BroadcastPendingBattleEvents()
{
	if (ActiveBattleSession == nullptr)
	{
		BroadcastBattleLogCount = 0;
		return;
	}

	const TArray<FFinalBattleEvent> BattleLogEntries = ActiveBattleSession->GetBattleLogEntries();
	const int32 SafeStartIndex = FMath::Clamp(BroadcastBattleLogCount, 0, BattleLogEntries.Num());
	for (int32 EventIndex = SafeStartIndex; EventIndex < BattleLogEntries.Num(); ++EventIndex)
	{
		OnBattleEventBroadcast.Broadcast(BattleLogEntries[EventIndex]);
	}

	BroadcastBattleLogCount = BattleLogEntries.Num();
}

void UFinalBattleFlowSubsystem::BroadcastSnapshot()
{
	if (ActiveBattleSession == nullptr)
	{
		return;
	}

	OnBattleSnapshotChanged.Broadcast(ActiveBattleSession->GetSnapshot());
}

bool UFinalBattleFlowSubsystem::BuildInitContext(const FFinalBattleStartRequest& StartRequest, FFinalBattleInitContext& OutInitContext)
{
	UFinalDataRegistry* DataRegistry = GetGameInstance() ? GetGameInstance()->GetSubsystem<UFinalDataRegistry>() : nullptr;
	if (DataRegistry == nullptr)
	{
		LastFailureReason = FText::FromString(TEXT("FinalDataRegistry is unavailable."));
		return false;
	}

	OutInitContext = FFinalBattleInitContext{};
	OutInitContext.TeamCurrentHP = StartRequest.TeamCurrentHP;
	OutInitContext.BattleStartRelics = StartRequest.BattleStartRelics;

	const TArray<FFinalRunPersistentCharacterState>& SourcePartyStates = StartRequest.PartyStates;
	for (const FFinalRunPersistentCharacterState& PartyState : SourcePartyStates)
	{
		UFinalCharacterDefinition* CharacterDefinition = DataRegistry->FindCharacterDefinition(PartyState.CharacterId);
		if (CharacterDefinition == nullptr)
		{
			LastFailureReason = FText::Format(NSLOCTEXT("FinalBattleFlow", "MissingCharacter", "Character {0} is not registered."), FText::FromName(PartyState.CharacterId.Value));
			return false;
		}

		FFinalBattleCharacterInitData InitData;
		InitData.CharacterDefinition = CharacterDefinition;
		InitData.UltimateDefinition = CharacterDefinition->UltimateId.IsValid()
			? DataRegistry->FindUltimateDefinition(CharacterDefinition->UltimateId)
			: nullptr;
		for (const FFinalInitialPassiveGrantDefinition& InitialPassiveGrant : CharacterDefinition->InitialPassiveGrants)
		{
			if (!InitialPassiveGrant.PassiveId.IsValid())
			{
				continue;
			}

			UFinalPassiveDefinition* PassiveDefinition = DataRegistry->FindPassiveDefinition(InitialPassiveGrant.PassiveId);
			if (PassiveDefinition == nullptr)
			{
				LastFailureReason = FText::Format(
					NSLOCTEXT("FinalBattleFlow", "MissingPassive", "Passive {0} is not registered."),
					FText::FromName(InitialPassiveGrant.PassiveId.Value));
				return false;
			}

			FFinalBattleInitialPassiveGrantData& PassiveGrantData = InitData.InitialPassiveGrants.AddDefaulted_GetRef();
			PassiveGrantData.PassiveId = InitialPassiveGrant.PassiveId;
			PassiveGrantData.PassiveDefinition = PassiveDefinition;
			PassiveGrantData.InitialStacks = FMath::Max(InitialPassiveGrant.InitialStacks, 1);
			PassiveGrantData.DurationOverride = FMath::Max(InitialPassiveGrant.DurationOverride, 0);
		}
		InitData.CurrentStress = PartyState.CurrentStress;
		InitData.bCollapsed = PartyState.bCollapsed;
		InitData.CurrentAwakenCount = PartyState.CurrentAwakenCount;
		InitData.CollapseCount = PartyState.CollapseCount;
		const UFinalCharacterGrowthConfig* GrowthConfig = CharacterDefinition->GrowthConfigId.IsValid()
			? DataRegistry->FindCharacterGrowthConfig(CharacterDefinition->GrowthConfigId)
			: nullptr;
		const FFinalBattleCharacterRuntimeStats RuntimeStats =
			FinalBattleGrowthStatProjection::BuildRuntimeStats(PartyState, *CharacterDefinition, GrowthConfig);
		InitData.VitalShare = RuntimeStats.VitalShare;
		InitData.StressCap = RuntimeStats.StressCap;
		InitData.RuntimeAttack = RuntimeStats.RuntimeAttack;
		InitData.RuntimeDefense = RuntimeStats.RuntimeDefense;
		InitData.RuntimeBreakRate = RuntimeStats.RuntimeBreakRate;
		InitData.RuntimeCritChance = RuntimeStats.RuntimeCritChance;
		InitData.RuntimeCritDamage = RuntimeStats.RuntimeCritDamage;
		OutInitContext.PartyMembers.Add(InitData);
	}

	const bool bHasExplicitDeckEntries = StartRequest.DeckEntries.Num() > 0;
	if (bHasExplicitDeckEntries)
	{
		for (const FFinalBattleStartDeckEntry& DeckEntry : StartRequest.DeckEntries)
		{
			if (!DeckEntry.EffectiveCardId.IsValid())
			{
				continue;
			}

			UFinalCardDefinition* CardDefinition = DataRegistry->FindCardDefinition(DeckEntry.EffectiveCardId);
			if (CardDefinition == nullptr)
			{
				LastFailureReason = FText::Format(NSLOCTEXT("FinalBattleFlow", "MissingDeckEntryCard", "Card {0} is not registered."), FText::FromName(DeckEntry.EffectiveCardId.Value));
				return false;
			}

			FFinalBattleCardInitData CardInitData;
			CardInitData.CardDefinition = CardDefinition;
			CardInitData.CardId = DeckEntry.EffectiveCardId;
			CardInitData.OwnerCharacterId = DeckEntry.OwnerCharacterId;
			CardInitData.SourceRunCardInstanceId = DeckEntry.SourceRunCardInstanceId;
			OutInitContext.DeckCards.Add(MoveTemp(CardInitData));
			OutInitContext.DeckDefinitions.Add(CardDefinition);
		}
	}
	else
	{
		for (const FFinalCardId& CardId : StartRequest.DeckCardIds)
		{
			UFinalCardDefinition* CardDefinition = DataRegistry->FindCardDefinition(CardId);
			if (CardDefinition == nullptr)
			{
				LastFailureReason = FText::Format(NSLOCTEXT("FinalBattleFlow", "MissingCard", "Card {0} is not registered."), FText::FromName(CardId.Value));
				return false;
			}

			FFinalBattleCardInitData CardInitData;
			CardInitData.CardDefinition = CardDefinition;
			CardInitData.CardId = CardId;
			if (!CardDefinition->OwnerUnitId.IsNone())
			{
				CardInitData.OwnerCharacterId = FFinalCharacterId(CardDefinition->OwnerUnitId);
			}
			OutInitContext.DeckCards.Add(MoveTemp(CardInitData));
			OutInitContext.DeckDefinitions.Add(CardDefinition);
		}
	}

	return OutInitContext.PartyMembers.Num() > 0 && OutInitContext.DeckCards.Num() > 0;
}
