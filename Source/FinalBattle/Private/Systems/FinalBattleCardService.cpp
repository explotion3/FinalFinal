#include "Systems/FinalBattleCardService.h"

#include "Battle/Conditions/FinalBattleConditionDefinition.h"
#include "Algo/RandomShuffle.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "Facade/FinalBattleSessionTypes.h"
#include "Queries/FinalBattleQueryTypes.h"
#include "Runtime/FinalBattleCardInstance.h"
#include "Runtime/FinalBattleCharacterState.h"
#include "Runtime/FinalBattleState.h"
#include "Runtime/FinalTeamDeckState.h"
#include "Systems/FinalBattleUnitService.h"

namespace
{
FGameplayTag GetRetainKeyword()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Retain"));
}

FGameplayTag GetExpendKeyword()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Expend"));
}

FGameplayTag GetOpeningKeyword()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.Opening"));
}

bool HasRetainKeyword(const FGameplayTagContainer& Keywords)
{
	return Keywords.HasTagExact(GetRetainKeyword());
}

bool HasExpendKeyword(const FGameplayTagContainer& Keywords)
{
	return Keywords.HasTagExact(GetExpendKeyword());
}

bool HasOpeningKeyword(const FGameplayTagContainer& Keywords)
{
	return Keywords.HasTagExact(GetOpeningKeyword());
}

int32 ResolveInitialRecycleCount(const FGameplayTagContainer& Keywords)
{
	return 0;
}

FFinalBattleCardRuntimeBehavior BuildRuntimeBehaviorFromKeywords(const FGameplayTagContainer& Keywords)
{
	FFinalBattleCardRuntimeBehavior Behavior;
	Behavior.bRetained = HasRetainKeyword(Keywords);
	Behavior.bConsumeOnPlay = HasExpendKeyword(Keywords);
	Behavior.RecycleCount = ResolveInitialRecycleCount(Keywords);
	return Behavior;
}

bool RemoveCardInstanceId(TArray<FGuid>& CardInstanceIds, const FGuid& CardInstanceId)
{
	return CardInstanceIds.RemoveSingle(CardInstanceId) > 0;
}

TArray<FGuid>* ResolveZoneArray(FFinalTeamDeckState& DeckState, const EFinalBattleCardZone Zone)
{
	switch (Zone)
	{
	case EFinalBattleCardZone::Hand:
		return &DeckState.HandCardInstanceIds;

	case EFinalBattleCardZone::DrawPileTop:
	case EFinalBattleCardZone::DrawPileBottom:
		return &DeckState.DrawPileCardInstanceIds;

	case EFinalBattleCardZone::DiscardPile:
		return &DeckState.DiscardPileCardInstanceIds;

	case EFinalBattleCardZone::OngoingZone:
		return &DeckState.OngoingZoneCardInstanceIds;

	case EFinalBattleCardZone::ConsumePile:
		return &DeckState.ConsumePileCardInstanceIds;

	default:
		return nullptr;
	}
}

const TArray<FGuid>* ResolveZoneArray(const FFinalTeamDeckState& DeckState, const EFinalBattleCardZone Zone)
{
	switch (Zone)
	{
	case EFinalBattleCardZone::Hand:
		return &DeckState.HandCardInstanceIds;

	case EFinalBattleCardZone::DrawPileTop:
	case EFinalBattleCardZone::DrawPileBottom:
		return &DeckState.DrawPileCardInstanceIds;

	case EFinalBattleCardZone::DiscardPile:
		return &DeckState.DiscardPileCardInstanceIds;

	case EFinalBattleCardZone::OngoingZone:
		return &DeckState.OngoingZoneCardInstanceIds;

	case EFinalBattleCardZone::ConsumePile:
		return &DeckState.ConsumePileCardInstanceIds;

	default:
		return nullptr;
	}
}

void InsertCardInstanceIntoZoneArray(TArray<FGuid>& ZoneArray, const FGuid& CardInstanceId, const EFinalBattleCardZone Zone)
{
	if (Zone == EFinalBattleCardZone::DrawPileTop)
	{
		ZoneArray.Insert(CardInstanceId, 0);
		return;
	}

	ZoneArray.Add(CardInstanceId);
}

bool MatchesGeneratedCardFilter(
	const FFinalBattleCardInstance& CardInstance,
	const FFinalBattleCardMatchCriteria& Criteria)
{
	if (CardInstance.RuntimeOwnerUnitId != Criteria.RuntimeOwnerUnitId)
	{
		return false;
	}

	if (Criteria.bGeneratedOnly && !CardInstance.bGeneratedCard)
	{
		return false;
	}

	if (Criteria.RequiredCardId.IsValid() && CardInstance.CardId != Criteria.RequiredCardId)
	{
		return false;
	}

	if (Criteria.RequiredKeyword.IsValid() && !CardInstance.RuntimeKeywords.HasTagExact(Criteria.RequiredKeyword))
	{
		return false;
	}

	return true;
}

FFinalBattleCardInstance* ResolveCardInstanceById(FFinalBattleState& BattleState, const FGuid& CardInstanceId)
{
	const int32* CardInstanceIndex = BattleState.CardInstanceIndexById.Find(CardInstanceId);
	if (CardInstanceIndex == nullptr || !BattleState.CardInstances.IsValidIndex(*CardInstanceIndex))
	{
		return nullptr;
	}

	FFinalBattleCardInstance& CardInstance = BattleState.CardInstances[*CardInstanceIndex];
	return CardInstance.CardInstanceId == CardInstanceId ? &CardInstance : nullptr;
}

const FFinalBattleCardInstance* ResolveCardInstanceById(const FFinalBattleState& BattleState, const FGuid& CardInstanceId)
{
	const int32* CardInstanceIndex = BattleState.CardInstanceIndexById.Find(CardInstanceId);
	if (CardInstanceIndex == nullptr)
	{
		const bool bMissingIndexedCardExists = BattleState.CardInstances.ContainsByPredicate(
			[&CardInstanceId](const FFinalBattleCardInstance& Candidate)
			{
				return Candidate.CardInstanceId == CardInstanceId;
			});
		ensureMsgf(
			!bMissingIndexedCardExists,
			TEXT("Battle card instance index is missing an entry for CardInstanceId %s."),
			*CardInstanceId.ToString());
		return nullptr;
	}

	if (!BattleState.CardInstances.IsValidIndex(*CardInstanceIndex))
	{
		ensureMsgf(
			false,
			TEXT("Battle card instance index for CardInstanceId %s points to invalid index %d."),
			*CardInstanceId.ToString(),
			*CardInstanceIndex);
		return nullptr;
	}

	const FFinalBattleCardInstance& CardInstance = BattleState.CardInstances[*CardInstanceIndex];
	ensureMsgf(
		CardInstance.CardInstanceId == CardInstanceId,
		TEXT("Battle card instance index for CardInstanceId %s points to mismatched CardInstanceId %s."),
		*CardInstanceId.ToString(),
		*CardInstance.CardInstanceId.ToString());
	return CardInstance.CardInstanceId == CardInstanceId ? &CardInstance : nullptr;
}

UFinalBattleEffectDefinition* DuplicateEffectForRuntime(UFinalBattleEffectDefinition* EffectDefinition, UObject* RuntimeProjectionOwner)
{
	return EffectDefinition != nullptr && RuntimeProjectionOwner != nullptr
		? DuplicateObject<UFinalBattleEffectDefinition>(EffectDefinition, RuntimeProjectionOwner)
		: nullptr;
}

UFinalBattleConditionDefinition* DuplicateConditionForRuntime(UFinalBattleConditionDefinition* ConditionDefinition, UObject* RuntimeProjectionOwner)
{
	return ConditionDefinition != nullptr && RuntimeProjectionOwner != nullptr
		? DuplicateObject<UFinalBattleConditionDefinition>(ConditionDefinition, RuntimeProjectionOwner)
		: nullptr;
}

int32 FindEffectIndexById(const TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects, const FName EffectId)
{
	if (EffectId.IsNone())
	{
		return INDEX_NONE;
	}

	for (int32 Index = 0; Index < Effects.Num(); ++Index)
	{
		if (Effects[Index] != nullptr && Effects[Index]->EffectId == EffectId)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

int32 FindConditionIndexById(const TArray<TObjectPtr<UFinalBattleConditionDefinition>>& Conditions, const FName ConditionId)
{
	if (ConditionId.IsNone())
	{
		return INDEX_NONE;
	}

	for (int32 Index = 0; Index < Conditions.Num(); ++Index)
	{
		if (Conditions[Index] != nullptr && Conditions[Index]->ConditionId == ConditionId)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

void ApplyConditionPatchToEffect(
	UFinalBattleEffectDefinition* RuntimeEffectDefinition,
	const FFinalBattleCardConditionPatch& ConditionPatch,
	UObject* RuntimeProjectionOwner)
{
	if (RuntimeEffectDefinition == nullptr)
	{
		return;
	}

	TArray<TObjectPtr<UFinalBattleConditionDefinition>>& Conditions = RuntimeEffectDefinition->Conditions;
	const int32 TargetConditionIndex = FindConditionIndexById(Conditions, ConditionPatch.TargetConditionId);

	switch (ConditionPatch.Operation)
	{
	case EFinalBattleCardEffectPatchOperation::Replace:
		if (TargetConditionIndex != INDEX_NONE && ConditionPatch.ConditionDefinition != nullptr)
		{
			Conditions[TargetConditionIndex] = DuplicateConditionForRuntime(ConditionPatch.ConditionDefinition, RuntimeProjectionOwner);
		}
		break;

	case EFinalBattleCardEffectPatchOperation::InsertBefore:
		if (TargetConditionIndex != INDEX_NONE && ConditionPatch.ConditionDefinition != nullptr)
		{
			Conditions.Insert(DuplicateConditionForRuntime(ConditionPatch.ConditionDefinition, RuntimeProjectionOwner), TargetConditionIndex);
		}
		break;

	case EFinalBattleCardEffectPatchOperation::InsertAfter:
		if (TargetConditionIndex != INDEX_NONE && ConditionPatch.ConditionDefinition != nullptr)
		{
			Conditions.Insert(DuplicateConditionForRuntime(ConditionPatch.ConditionDefinition, RuntimeProjectionOwner), TargetConditionIndex + 1);
		}
		break;

	case EFinalBattleCardEffectPatchOperation::Remove:
		if (TargetConditionIndex != INDEX_NONE)
		{
			Conditions.RemoveAt(TargetConditionIndex);
		}
		break;

	default:
		break;
	}
}

void ApplyEffectPatchToDefinition(
	UFinalCardDefinition* RuntimeCardDefinition,
	const FFinalBattleCardEffectPatch& EffectPatch,
	UObject* RuntimeProjectionOwner)
{
	if (RuntimeCardDefinition == nullptr)
	{
		return;
	}

	TArray<TObjectPtr<UFinalBattleEffectDefinition>>& Effects = RuntimeCardDefinition->Effects;
	const int32 TargetEffectIndex = FindEffectIndexById(Effects, EffectPatch.TargetEffectId);

	switch (EffectPatch.Operation)
	{
	case EFinalBattleCardEffectPatchOperation::Replace:
		if (TargetEffectIndex != INDEX_NONE && EffectPatch.EffectDefinition != nullptr)
		{
			Effects[TargetEffectIndex] = DuplicateEffectForRuntime(EffectPatch.EffectDefinition, RuntimeProjectionOwner);
		}
		break;

	case EFinalBattleCardEffectPatchOperation::InsertBefore:
		if (TargetEffectIndex != INDEX_NONE && EffectPatch.EffectDefinition != nullptr)
		{
			Effects.Insert(DuplicateEffectForRuntime(EffectPatch.EffectDefinition, RuntimeProjectionOwner), TargetEffectIndex);
		}
		break;

	case EFinalBattleCardEffectPatchOperation::InsertAfter:
		if (TargetEffectIndex != INDEX_NONE && EffectPatch.EffectDefinition != nullptr)
		{
			Effects.Insert(DuplicateEffectForRuntime(EffectPatch.EffectDefinition, RuntimeProjectionOwner), TargetEffectIndex + 1);
		}
		break;

	case EFinalBattleCardEffectPatchOperation::Remove:
		if (TargetEffectIndex != INDEX_NONE)
		{
			Effects.RemoveAt(TargetEffectIndex);
		}
		break;

	default:
		break;
	}
}

void RemoveKeywordTags(FGameplayTagContainer& Keywords, const FGameplayTagContainer& RemovedKeywords)
{
	for (const FGameplayTag& RemovedKeyword : RemovedKeywords)
	{
		Keywords.RemoveTag(RemovedKeyword);
	}
}
}

void FFinalBattleCardService::InitializeDeckState(FFinalTeamDeckState& DeckState) const
{
	DeckState = FFinalTeamDeckState{};
}

void FFinalBattleCardService::InitializeDeckCards(
	FFinalBattleState& BattleState,
	const TArray<FFinalBattleCardInitData>& DeckCards,
	UObject* RuntimeProjectionOwner,
	const TMap<FName, FName>& TemplateToRuntimeUnitMap) const
{
	for (const FFinalBattleCardInitData& DeckCard : DeckCards)
	{
		UFinalCardDefinition* CardDefinition = DeckCard.CardDefinition;
		if (CardDefinition == nullptr || !CardDefinition->CardId.IsValid())
		{
			continue;
		}

		const FName OwnerTemplateUnitId = DeckCard.OwnerCharacterId.IsValid()
			? DeckCard.OwnerCharacterId.Value
			: CardDefinition->OwnerUnitId;
		const FName* RuntimeOwnerUnitIdPtr = TemplateToRuntimeUnitMap.Find(OwnerTemplateUnitId);
		const FName RuntimeOwnerUnitId = RuntimeOwnerUnitIdPtr != nullptr
			? *RuntimeOwnerUnitIdPtr
			: OwnerTemplateUnitId;
		const FGuid CardInstanceId = CreateCardInstance(
			BattleState,
			CardDefinition,
			RuntimeOwnerUnitId,
			RuntimeProjectionOwner,
			DeckCard.SourceRunCardInstanceId,
			false,
			false);
		if (CardInstanceId.IsValid())
		{
			MoveCardInstanceToZone(BattleState, CardInstanceId, EFinalBattleCardZone::DrawPileBottom);
		}
	}
}

void FFinalBattleCardService::PrepareInitialDrawPile(FFinalBattleState& BattleState) const
{
	if (BattleState.DeckState.DrawPileCardInstanceIds.Num() <= 0)
	{
		return;
	}

	Algo::RandomShuffle(BattleState.DeckState.DrawPileCardInstanceIds);

	TArray<FGuid> OpeningCardInstanceIds;
	TArray<FGuid> RemainingCardInstanceIds;
	OpeningCardInstanceIds.Reserve(BattleState.DeckState.DrawPileCardInstanceIds.Num());
	RemainingCardInstanceIds.Reserve(BattleState.DeckState.DrawPileCardInstanceIds.Num());

	for (const FGuid& CardInstanceId : BattleState.DeckState.DrawPileCardInstanceIds)
	{
		const FFinalBattleCardInstance* CardInstance = FindCardInstance(BattleState, CardInstanceId);
		if (CardInstance != nullptr && HasOpeningKeyword(CardInstance->RuntimeKeywords))
		{
			OpeningCardInstanceIds.Add(CardInstanceId);
			continue;
		}

		RemainingCardInstanceIds.Add(CardInstanceId);
	}

	if (OpeningCardInstanceIds.Num() <= 0)
	{
		return;
	}

	BattleState.DeckState.DrawPileCardInstanceIds.Reset();
	BattleState.DeckState.DrawPileCardInstanceIds.Append(OpeningCardInstanceIds);
	BattleState.DeckState.DrawPileCardInstanceIds.Append(RemainingCardInstanceIds);
}

FFinalBattleCardInstance* FFinalBattleCardService::FindCardInstance(FFinalBattleState& BattleState, const FGuid& CardInstanceId) const
{
	return ResolveCardInstanceById(BattleState, CardInstanceId);
}

const FFinalBattleCardInstance* FFinalBattleCardService::FindCardInstance(const FFinalBattleState& BattleState, const FGuid& CardInstanceId) const
{
	return ResolveCardInstanceById(BattleState, CardInstanceId);
}

bool FFinalBattleCardService::IsCardInHand(const FFinalBattleState& BattleState, const FGuid& CardInstanceId) const
{
	return BattleState.DeckState.HandCardInstanceIds.Contains(CardInstanceId);
}

int32 FFinalBattleCardService::CountMatchingCardsInZone(
	const FFinalBattleState& BattleState,
	const EFinalBattleCardZone SourceZone,
	const FFinalBattleCardMatchCriteria& Criteria) const
{
	TArray<FGuid> MatchingCardInstanceIds;
	CollectMatchingCardInstanceIdsInZone(
		BattleState,
		SourceZone,
		Criteria,
		MAX_int32,
		MatchingCardInstanceIds);
	return MatchingCardInstanceIds.Num();
}

int32 FFinalBattleCardService::CountMatchingCardsInHand(
	const FFinalBattleState& BattleState,
	const FName RuntimeOwnerUnitId,
	const FFinalCardId& RequiredCardId,
	const FGameplayTag& RequiredKeyword,
	const bool bGeneratedOnly) const
{
	FFinalBattleCardMatchCriteria Criteria;
	Criteria.RuntimeOwnerUnitId = RuntimeOwnerUnitId;
	Criteria.RequiredCardId = RequiredCardId;
	Criteria.RequiredKeyword = RequiredKeyword;
	Criteria.bGeneratedOnly = bGeneratedOnly;

	return CountMatchingCardsInZone(BattleState, EFinalBattleCardZone::Hand, Criteria);
}

bool FFinalBattleCardService::SatisfiesMatchCriteriaInZone(
	const FFinalBattleState& BattleState,
	const EFinalBattleCardZone SourceZone,
	const FFinalBattleCardMatchCriteria& Criteria,
	const int32 MinimumCount) const
{
	return CountMatchingCardsInZone(BattleState, SourceZone, Criteria) >= FMath::Max(MinimumCount, 1);
}

bool FFinalBattleCardService::SatisfiesHandCardRequirement(
	const FFinalBattleState& BattleState,
	const FName RuntimeOwnerUnitId,
	const FFinalBattleHandCardRequirement& Requirement) const
{
	if (!Requirement.bRequireInHand)
	{
		return true;
	}

	FFinalBattleCardMatchCriteria Criteria;
	Criteria.RuntimeOwnerUnitId = RuntimeOwnerUnitId;
	Criteria.RequiredCardId = Requirement.RequiredCardId;
	Criteria.RequiredKeyword = Requirement.RequiredKeyword;
	Criteria.bGeneratedOnly = Requirement.bGeneratedOnly;

	return SatisfiesMatchCriteriaInZone(BattleState, EFinalBattleCardZone::Hand, Criteria, Requirement.MinimumCount);
}

FGuid FFinalBattleCardService::CreateCardInstance(
	FFinalBattleState& BattleState,
	UFinalCardDefinition* CardDefinition,
	const FName RuntimeOwnerUnitId,
	UObject* RuntimeProjectionOwner,
	const FName SourceRunCardInstanceId,
	const bool bGeneratedCard,
	const bool bTemporaryCard) const
{
	if (CardDefinition == nullptr || !CardDefinition->CardId.IsValid())
	{
		return FGuid();
	}

	FFinalBattleCardInstance CardInstance;
	CardInstance.CardInstanceId = FGuid::NewGuid();
	CardInstance.CardId = CardDefinition->CardId;
	CardInstance.SourceRunCardInstanceId = SourceRunCardInstanceId;
	CardInstance.RuntimeOwnerUnitId = RuntimeOwnerUnitId;
	CardInstance.BaseDefinition = CardDefinition;
	CardInstance.bGeneratedCard = bGeneratedCard;
	CardInstance.bTemporaryCard = bTemporaryCard;
	ReprojectCardInstanceInternal(CardInstance, RuntimeProjectionOwner);

	BattleState.CardInstances.Add(CardInstance);
	BattleState.CardInstanceIndexById.Add(CardInstance.CardInstanceId, BattleState.CardInstances.Num() - 1);
	return CardInstance.CardInstanceId;
}

bool FFinalBattleCardService::AddCardModifier(
	FFinalBattleState& BattleState,
	const FGuid& CardInstanceId,
	UObject* RuntimeProjectionOwner,
	const FFinalBattleCardModifierRecord& ModifierRecord) const
{
	FFinalBattleCardInstance* CardInstance = FindCardInstance(BattleState, CardInstanceId);
	if (CardInstance == nullptr || ModifierRecord.ModifierId.IsNone())
	{
		return false;
	}

	CardInstance->ModifierRecords.Add(ModifierRecord);
	return ReprojectCardInstanceInternal(*CardInstance, RuntimeProjectionOwner);
}

bool FFinalBattleCardService::RemoveCardModifier(
	FFinalBattleState& BattleState,
	const FGuid& CardInstanceId,
	UObject* RuntimeProjectionOwner,
	const FName ModifierId) const
{
	FFinalBattleCardInstance* CardInstance = FindCardInstance(BattleState, CardInstanceId);
	if (CardInstance == nullptr || ModifierId.IsNone())
	{
		return false;
	}

	const int32 RemovedCount = CardInstance->ModifierRecords.RemoveAll([&ModifierId](const FFinalBattleCardModifierRecord& Candidate)
	{
		return Candidate.ModifierId == ModifierId;
	});
	if (RemovedCount <= 0)
	{
		return false;
	}

	return ReprojectCardInstanceInternal(*CardInstance, RuntimeProjectionOwner);
}

int32 FFinalBattleCardService::ClearCardModifiersByDuration(
	FFinalBattleState& BattleState,
	UObject* RuntimeProjectionOwner,
	const EFinalBattleCardModifierDuration DurationPolicy) const
{
	int32 ReprojectedCardCount = 0;
	for (FFinalBattleCardInstance& CardInstance : BattleState.CardInstances)
	{
		const int32 RemovedCount = CardInstance.ModifierRecords.RemoveAll([DurationPolicy](const FFinalBattleCardModifierRecord& Candidate)
		{
			return Candidate.DurationPolicy == DurationPolicy;
		});
		if (RemovedCount <= 0)
		{
			continue;
		}

		ReprojectCardInstanceInternal(CardInstance, RuntimeProjectionOwner);
		++ReprojectedCardCount;
	}

	return ReprojectedCardCount;
}

int32 FFinalBattleCardService::ClearCardModifiersExpiringAtPlayerTurnEnd(
	FFinalBattleState& BattleState,
	UObject* RuntimeProjectionOwner) const
{
	int32 ReprojectedCardCount = 0;
	for (FFinalBattleCardInstance& CardInstance : BattleState.CardInstances)
	{
		const int32 RemovedCount = CardInstance.ModifierRecords.RemoveAll([](const FFinalBattleCardModifierRecord& Candidate)
		{
			return Candidate.bExpireAtPlayerTurnEnd;
		});
		if (RemovedCount <= 0)
		{
			continue;
		}

		ReprojectCardInstanceInternal(CardInstance, RuntimeProjectionOwner);
		++ReprojectedCardCount;
	}

	return ReprojectedCardCount;
}

bool FFinalBattleCardService::ReprojectCardInstance(
	FFinalBattleState& BattleState,
	const FGuid& CardInstanceId,
	UObject* RuntimeProjectionOwner) const
{
	FFinalBattleCardInstance* CardInstance = FindCardInstance(BattleState, CardInstanceId);
	return CardInstance != nullptr && ReprojectCardInstanceInternal(*CardInstance, RuntimeProjectionOwner);
}

int32 FFinalBattleCardService::RefreshCardsForRunCardInstance(
	FFinalBattleState& BattleState,
	UObject* RuntimeProjectionOwner,
	const FFinalBattleCardRefreshRequest& RefreshRequest) const
{
	if (RefreshRequest.SourceRunCardInstanceId.IsNone() || RefreshRequest.NewDefinition == nullptr || !RefreshRequest.NewCardId.IsValid())
	{
		return 0;
	}

	int32 RefreshedCount = 0;
	for (FFinalBattleCardInstance& CardInstance : BattleState.CardInstances)
	{
		if (CardInstance.SourceRunCardInstanceId != RefreshRequest.SourceRunCardInstanceId)
		{
			continue;
		}

		CardInstance.CardId = RefreshRequest.NewCardId;
		CardInstance.BaseDefinition = RefreshRequest.NewDefinition;
		ReprojectCardInstanceInternal(CardInstance, RuntimeProjectionOwner);
		++RefreshedCount;
	}

	return RefreshedCount;
}

bool FFinalBattleCardService::MoveCardInstanceToZone(
	FFinalBattleState& BattleState,
	const FGuid& CardInstanceId,
	const EFinalBattleCardZone Zone) const
{
	if (!CardInstanceId.IsValid() || FindCardInstance(BattleState, CardInstanceId) == nullptr)
	{
		return false;
	}

	TArray<FGuid>* ZoneArray = ResolveZoneArray(BattleState.DeckState, Zone);
	if (ZoneArray == nullptr)
	{
		return false;
	}

	RemoveCardInstanceFromAllZones(BattleState, CardInstanceId);
	InsertCardInstanceIntoZoneArray(*ZoneArray, CardInstanceId, Zone);
	return true;
}

int32 FFinalBattleCardService::MoveMatchingCardsBetweenZones(
	FFinalBattleState& BattleState,
	const EFinalBattleCardZone SourceZone,
	const EFinalBattleCardZone DestinationZone,
	const FFinalBattleCardMatchCriteria& Criteria,
	const int32 MoveCount,
	TArray<FGuid>* OutMovedCardInstanceIds) const
{
	const int32 TargetMoveCount = FMath::Max(MoveCount, 0);
	if (TargetMoveCount <= 0 || Criteria.RuntimeOwnerUnitId.IsNone())
	{
		return 0;
	}

	if (OutMovedCardInstanceIds != nullptr)
	{
		OutMovedCardInstanceIds->Reset();
	}

	TArray<FGuid> MatchedCardInstanceIds;
	CollectMatchingCardInstanceIdsInZone(
		BattleState,
		SourceZone,
		Criteria,
		TargetMoveCount,
		MatchedCardInstanceIds);

	int32 MovedCount = 0;
	for (const FGuid& MatchedCardInstanceId : MatchedCardInstanceIds)
	{
		if (!MoveCardInstanceToZone(BattleState, MatchedCardInstanceId, DestinationZone))
		{
			continue;
		}

		if (OutMovedCardInstanceIds != nullptr)
		{
			OutMovedCardInstanceIds->Add(MatchedCardInstanceId);
		}
		++MovedCount;
	}

	return MovedCount;
}

void FFinalBattleCardService::ResolveEndTurnHandCleanup(FFinalBattleState& BattleState) const
{
	const TArray<FGuid> HandCardInstanceIds = BattleState.DeckState.HandCardInstanceIds;

	for (const FGuid& CardInstanceId : HandCardInstanceIds)
	{
		const FFinalBattleCardInstance* CardInstance = FindCardInstance(BattleState, CardInstanceId);
		if (CardInstance == nullptr || CardInstance->RuntimeBehavior.bRetained)
		{
			continue;
		}

		MoveCardInstanceToZone(BattleState, CardInstanceId, EFinalBattleCardZone::DiscardPile);
	}
}

void FFinalBattleCardService::MoveHandCardAfterPlay(FFinalBattleState& BattleState, const FGuid& CardInstanceId) const
{
	const FFinalBattleCardInstance* CardInstance = FindCardInstance(BattleState, CardInstanceId);
	if (CardInstance != nullptr && CardInstance->RuntimeBehavior.bConsumeOnPlay)
	{
		MoveCardInstanceToZone(BattleState, CardInstanceId, EFinalBattleCardZone::ConsumePile);
		return;
	}

	MoveCardInstanceToZone(BattleState, CardInstanceId, EFinalBattleCardZone::DiscardPile);
}

int32 FFinalBattleCardService::DrawCards(
	FFinalBattleState& BattleState,
	const int32 DrawCount,
	TArray<FGuid>* OutDrawnCardInstanceIds) const
{
	int32 DrawnCount = 0;
	if (OutDrawnCardInstanceIds != nullptr)
	{
		OutDrawnCardInstanceIds->Reset();
	}

	for (int32 DrawIndex = 0; DrawIndex < DrawCount; ++DrawIndex)
	{
		if (BattleState.DeckState.DrawPileCardInstanceIds.Num() == 0)
		{
			if (!RefillDrawPileFromDiscard(BattleState))
			{
				return DrawnCount;
			}
		}

		const FGuid DrawnCardId = BattleState.DeckState.DrawPileCardInstanceIds[0];
		MoveCardInstanceToZone(BattleState, DrawnCardId, EFinalBattleCardZone::Hand);
		if (OutDrawnCardInstanceIds != nullptr)
		{
			OutDrawnCardInstanceIds->Add(DrawnCardId);
		}
		++DrawnCount;
	}

	return DrawnCount;
}

void FFinalBattleCardService::BuildHandCardViews(
	const FFinalBattleState& BattleState,
	const FFinalBattleUnitService& UnitService,
	TArray<FFinalBattleCardViewData>& OutViews) const
{
	for (const FGuid& CardInstanceId : BattleState.DeckState.HandCardInstanceIds)
	{
		const FFinalBattleCardInstance* CardInstance = FindCardInstance(BattleState, CardInstanceId);
		if (CardInstance == nullptr)
		{
			continue;
		}

		FFinalBattleCardViewData CardView;
		CardView.CardInstanceId = CardInstance->CardInstanceId;
		CardView.SourceRunCardInstanceId = CardInstance->SourceRunCardInstanceId;
		CardView.CardId = CardInstance->CardId;
		CardView.RuntimeOwnerUnitId = CardInstance->RuntimeOwnerUnitId;
		CardView.DisplayName = CardInstance->ProjectedDefinition != nullptr
			? CardInstance->ProjectedDefinition->DisplayName
			: FText::FromName(CardInstance->CardId.Value);
		CardView.CardType = CardInstance->ProjectedDefinition != nullptr
			? CardInstance->ProjectedDefinition->CardType
			: EFinalCardType::Attack;
		CardView.RuntimeCostAP = CardInstance->RuntimeCostAP;
		CardView.RuntimeDamagePowerPercentPointDelta = CardInstance->RuntimeDamagePowerPercentPointDelta;
		CardView.RuntimeFinalDamagePercentDelta = CardInstance->RuntimeFinalDamagePercentDelta;
		CardView.RuntimeKeywords = CardInstance->RuntimeKeywords;
		CardView.bRetained = CardInstance->RuntimeBehavior.bRetained;
		if (const FFinalBattleCharacterState* OwnerCharacterState = UnitService.FindCharacterState(BattleState, CardInstance->RuntimeOwnerUnitId))
		{
			CardView.bCollapsedCard = OwnerCharacterState->bCollapsed;
		}

		OutViews.Add(MoveTemp(CardView));
	}
}

FFinalBattleCardProjectionView FFinalBattleCardService::BuildProjectionView(
	const FFinalBattleState& BattleState,
	const FGuid& CardInstanceId) const
{
	FFinalBattleCardProjectionView ProjectionView;
	const FFinalBattleCardInstance* CardInstance = FindCardInstance(BattleState, CardInstanceId);
	if (CardInstance == nullptr)
	{
		return ProjectionView;
	}

	ProjectionView.CardInstanceId = CardInstance->CardInstanceId;
	ProjectionView.CardId = CardInstance->CardId;
	ProjectionView.EffectiveCostAP = CardInstance->RuntimeCostAP;
	ProjectionView.EffectiveKeywords = CardInstance->RuntimeKeywords;
	ProjectionView.bRetained = CardInstance->RuntimeBehavior.bRetained;
	ProjectionView.bConsumeOnPlay = CardInstance->RuntimeBehavior.bConsumeOnPlay;
	ProjectionView.RecycleCount = CardInstance->RuntimeBehavior.RecycleCount;
	ProjectionView.EffectiveDamagePowerPercentPointDelta = CardInstance->RuntimeDamagePowerPercentPointDelta;
	ProjectionView.EffectiveFinalDamagePercentDelta = CardInstance->RuntimeFinalDamagePercentDelta;
	ProjectionView.EffectCount = CardInstance->ProjectedDefinition != nullptr ? CardInstance->ProjectedDefinition->Effects.Num() : 0;
	ProjectionView.ModifierCount = CardInstance->ModifierRecords.Num();
	ProjectionView.bHasProjectedDefinition = CardInstance->ProjectedDefinition != nullptr;
	return ProjectionView;
}

void FFinalBattleCardService::ApplyCardModifierRecordToDefinition(
	FFinalBattleCardInstance& CardInstance,
	const FFinalBattleCardModifierRecord& ModifierRecord,
	UFinalCardDefinition* RuntimeCardDefinition) const
{
	if (RuntimeCardDefinition == nullptr)
	{
		return;
	}

	if (ModifierRecord.bReplaceEntireEffectList)
	{
		RuntimeCardDefinition->Effects.Reset();
		for (UFinalBattleEffectDefinition* ReplacementEffect : ModifierRecord.ReplacementEffects)
		{
			if (UFinalBattleEffectDefinition* RuntimeEffect = DuplicateEffectForRuntime(ReplacementEffect, RuntimeCardDefinition))
			{
				RuntimeCardDefinition->Effects.Add(RuntimeEffect);
			}
		}
	}

	for (const FFinalBattleCardEffectPatch& EffectPatch : ModifierRecord.EffectPatches)
	{
		ApplyEffectPatchToDefinition(RuntimeCardDefinition, EffectPatch, RuntimeCardDefinition);
	}

	for (const FFinalBattleCardConditionPatch& ConditionPatch : ModifierRecord.ConditionPatches)
	{
		const int32 TargetEffectIndex = FindEffectIndexById(RuntimeCardDefinition->Effects, ConditionPatch.TargetEffectId);
		if (TargetEffectIndex == INDEX_NONE || !RuntimeCardDefinition->Effects.IsValidIndex(TargetEffectIndex))
		{
			continue;
		}

		ApplyConditionPatchToEffect(RuntimeCardDefinition->Effects[TargetEffectIndex], ConditionPatch, RuntimeCardDefinition);
	}
}

void FFinalBattleCardService::ApplyCardDefinitionProjection(
	FFinalBattleCardInstance& CardInstance,
	UFinalCardDefinition* RuntimeCardDefinition,
	const int32 EffectiveCostAP,
	const FGameplayTagContainer& EffectiveKeywords,
	const FFinalBattleCardRuntimeBehavior& EffectiveBehavior,
	const int32 EffectiveDamagePowerPercentPointDelta,
	const int32 EffectiveFinalDamagePercentDelta) const
{
	CardInstance.ProjectedDefinition = RuntimeCardDefinition;
	CardInstance.CardId = RuntimeCardDefinition != nullptr && RuntimeCardDefinition->CardId.IsValid()
		? RuntimeCardDefinition->CardId
		: CardInstance.CardId;
	CardInstance.RuntimeCostAP = EffectiveCostAP;
	CardInstance.RuntimeKeywords = EffectiveKeywords;
	CardInstance.RuntimeBehavior = EffectiveBehavior;
	CardInstance.RuntimeDamagePowerPercentPointDelta = EffectiveDamagePowerPercentPointDelta;
	CardInstance.RuntimeFinalDamagePercentDelta = EffectiveFinalDamagePercentDelta;
}

bool FFinalBattleCardService::ReprojectCardInstanceInternal(FFinalBattleCardInstance& CardInstance, UObject* RuntimeProjectionOwner) const
{
	if (CardInstance.BaseDefinition == nullptr || !CardInstance.BaseDefinition->CardId.IsValid() || RuntimeProjectionOwner == nullptr)
	{
		return false;
	}

	UFinalCardDefinition* RuntimeCardDefinition = DuplicateObject<UFinalCardDefinition>(CardInstance.BaseDefinition, RuntimeProjectionOwner);
	if (RuntimeCardDefinition == nullptr)
	{
		return false;
	}

	TArray<int32> ModifierIndices;
	ModifierIndices.Reserve(CardInstance.ModifierRecords.Num());
	for (int32 ModifierIndex = 0; ModifierIndex < CardInstance.ModifierRecords.Num(); ++ModifierIndex)
	{
		ModifierIndices.Add(ModifierIndex);
	}

	ModifierIndices.StableSort([&CardInstance](const int32 LeftIndex, const int32 RightIndex)
	{
		return CardInstance.ModifierRecords[LeftIndex].ApplyOrder < CardInstance.ModifierRecords[RightIndex].ApplyOrder;
	});

	int32 EffectiveCostAP = RuntimeCardDefinition->BaseCostAP;
	FGameplayTagContainer EffectiveKeywords = RuntimeCardDefinition->Keywords;
	int32 EffectiveDamagePowerPercentPointDelta = 0;
	int32 EffectiveFinalDamagePercentDelta = 0;
	TOptional<bool> OverrideRetained;
	TOptional<bool> OverrideConsumeOnPlay;
	TOptional<int32> OverrideRecycleCount;

	for (const int32 ModifierIndex : ModifierIndices)
	{
		const FFinalBattleCardModifierRecord& ModifierRecord = CardInstance.ModifierRecords[ModifierIndex];
		ApplyCardModifierRecordToDefinition(CardInstance, ModifierRecord, RuntimeCardDefinition);
		EffectiveCostAP += ModifierRecord.CostDeltaAP;
		EffectiveDamagePowerPercentPointDelta += ModifierRecord.DamagePowerPercentPointDelta;
		EffectiveFinalDamagePercentDelta += ModifierRecord.FinalDamagePercentDelta;
		EffectiveKeywords.AppendTags(ModifierRecord.AddedKeywords);
		RemoveKeywordTags(EffectiveKeywords, ModifierRecord.RemovedKeywords);

		if (ModifierRecord.bOverrideRetained)
		{
			OverrideRetained = ModifierRecord.bRetained;
		}

		if (ModifierRecord.bOverrideConsumeOnPlay)
		{
			OverrideConsumeOnPlay = ModifierRecord.bConsumeOnPlay;
		}

		if (ModifierRecord.bOverrideRecycleCount)
		{
			OverrideRecycleCount = ModifierRecord.RecycleCount;
		}
	}

	EffectiveCostAP = FMath::Max(EffectiveCostAP, 0);
	FFinalBattleCardRuntimeBehavior EffectiveBehavior = BuildRuntimeBehaviorFromKeywords(EffectiveKeywords);
	if (OverrideRetained.IsSet())
	{
		EffectiveBehavior.bRetained = OverrideRetained.GetValue();
	}
	if (OverrideConsumeOnPlay.IsSet())
	{
		EffectiveBehavior.bConsumeOnPlay = OverrideConsumeOnPlay.GetValue();
	}
	if (OverrideRecycleCount.IsSet())
	{
		EffectiveBehavior.RecycleCount = FMath::Max(OverrideRecycleCount.GetValue(), 0);
	}

	ApplyCardDefinitionProjection(CardInstance, RuntimeCardDefinition, EffectiveCostAP, EffectiveKeywords, EffectiveBehavior, EffectiveDamagePowerPercentPointDelta, EffectiveFinalDamagePercentDelta);
	return true;
}

bool FFinalBattleCardService::RefillDrawPileFromDiscard(FFinalBattleState& BattleState) const
{
	if (BattleState.DeckState.DiscardPileCardInstanceIds.Num() == 0)
	{
		return false;
	}

	BattleState.DeckState.DrawPileCardInstanceIds.Append(BattleState.DeckState.DiscardPileCardInstanceIds);
	BattleState.DeckState.DiscardPileCardInstanceIds.Reset();
	Algo::RandomShuffle(BattleState.DeckState.DrawPileCardInstanceIds);
	return BattleState.DeckState.DrawPileCardInstanceIds.Num() > 0;
}

void FFinalBattleCardService::RemoveCardInstanceFromAllZones(FFinalBattleState& BattleState, const FGuid& CardInstanceId) const
{
	RemoveCardInstanceId(BattleState.DeckState.DrawPileCardInstanceIds, CardInstanceId);
	RemoveCardInstanceId(BattleState.DeckState.HandCardInstanceIds, CardInstanceId);
	RemoveCardInstanceId(BattleState.DeckState.DiscardPileCardInstanceIds, CardInstanceId);
	RemoveCardInstanceId(BattleState.DeckState.OngoingZoneCardInstanceIds, CardInstanceId);
	RemoveCardInstanceId(BattleState.DeckState.ConsumePileCardInstanceIds, CardInstanceId);
}

void FFinalBattleCardService::CollectMatchingCardInstanceIdsInZone(
	const FFinalBattleState& BattleState,
	const EFinalBattleCardZone SourceZone,
	const FFinalBattleCardMatchCriteria& Criteria,
	const int32 MaxCount,
	TArray<FGuid>& OutCardInstanceIds) const
{
	OutCardInstanceIds.Reset();

	if (Criteria.RuntimeOwnerUnitId.IsNone() || MaxCount <= 0)
	{
		return;
	}

	const TArray<FGuid>* SourceZoneArray = ResolveZoneArray(BattleState.DeckState, SourceZone);
	if (SourceZoneArray == nullptr)
	{
		return;
	}

	for (int32 CardIndex = SourceZoneArray->Num() - 1;
		CardIndex >= 0 && OutCardInstanceIds.Num() < MaxCount;
		--CardIndex)
	{
		const FGuid CandidateCardInstanceId = (*SourceZoneArray)[CardIndex];
		const FFinalBattleCardInstance* CandidateCardInstance = FindCardInstance(BattleState, CandidateCardInstanceId);
		if (CandidateCardInstance == nullptr
			|| !MatchesGeneratedCardFilter(*CandidateCardInstance, Criteria))
		{
			continue;
		}

		OutCardInstanceIds.Add(CandidateCardInstanceId);
	}
}
