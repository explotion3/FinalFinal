#include "Systems/FinalBattleCardService.h"

#include "Battle/Conditions/FinalBattleConditionDefinition.h"
#include "Algo/RandomShuffle.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Effects/FinalBattleEffectApplyCardModifiers.h"
#include "Battle/Effects/FinalBattleEffectApplyStatus.h"
#include "Battle/Effects/FinalBattleEffectBonusBreak.h"
#include "Battle/Effects/FinalBattleEffectConsumeStatusResource.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "Battle/Effects/FinalBattleEffectDamage.h"
#include "Battle/Effects/FinalBattleEffectDrawCards.h"
#include "Battle/Effects/FinalBattleEffectGainAP.h"
#include "Battle/Effects/FinalBattleEffectGainShield.h"
#include "Battle/Effects/FinalBattleEffectGenerateCard.h"
#include "Battle/Effects/FinalBattleEffectHeal.h"
#include "Battle/Effects/FinalBattleTargetedEffectDefinition.h"
#include "Battle/Definitions/FinalStatusDefinition.h"
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

FGameplayTag GetSwordArrayKeyword()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Final.Keyword.SwordArray"));
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

EFinalBattleCardTargetRequirement ResolveCardTargetRequirement(const UFinalCardDefinition* CardDefinition)
{
	if (CardDefinition == nullptr)
	{
		return EFinalBattleCardTargetRequirement::None;
	}

	for (const TObjectPtr<UFinalBattleEffectDefinition>& EffectDefinition : CardDefinition->Effects)
	{
		const UFinalBattleTargetedEffectDefinition* TargetedEffect = Cast<UFinalBattleTargetedEffectDefinition>(EffectDefinition);
		if (TargetedEffect != nullptr && TargetedEffect->UnitTargetRule == EFinalBattleUnitTargetRule::SelectedEnemy)
		{
			return EFinalBattleCardTargetRequirement::Enemy;
		}
	}

	return EFinalBattleCardTargetRequirement::None;
}

int32 ResolveInitialRecycleCount(const FGameplayTagContainer& Keywords)
{
	return 0;
}

FString WrapRichText(const FString& Tag, const FString& Value)
{
	return FString::Printf(TEXT("<%s>%s</>"), *Tag, *Value);
}

FString FormatInteger(const int32 Value, const FString Tag = TEXT("num"))
{
	return WrapRichText(Tag, FString::FromInt(Value));
}

FString FormatPercent(const int32 Percent, const FString Tag = TEXT("num"))
{
	return WrapRichText(Tag, FString::Printf(TEXT("%d%%"), Percent));
}

FString FormatSignedPercent(const int32 Percent)
{
	const FString Tag = Percent >= 0 ? TEXT("good") : TEXT("bad");
	return WrapRichText(Tag, FString::Printf(TEXT("%s%d%%"), Percent > 0 ? TEXT("+") : TEXT(""), Percent));
}

FString FormatStatName(const EFinalBattleSourceStat SourceStat)
{
	switch (SourceStat)
	{
	case EFinalBattleSourceStat::Attack:
		return TEXT("攻击力");
	case EFinalBattleSourceStat::Defense:
		return TEXT("防御力");
	case EFinalBattleSourceStat::BaseDamagePower:
		return TEXT("攻击力");
	default:
		return TEXT("数值");
	}
}

int32 ResolveScalarBaseDisplayValue(const FFinalBattleScalarValue& Scalar)
{
	float ResultValue = Scalar.FlatBonus;
	if (Scalar.ScaleMode == EFinalBattleScalarMode::Flat)
	{
		ResultValue += Scalar.BaseValue;
	}
	return FMath::Max(FMath::RoundToInt(ResultValue), 0);
}

FString FormatScalarValue(const FFinalBattleScalarValue& Scalar)
{
	if (Scalar.ScaleMode == EFinalBattleScalarMode::SourceStatMultiplier)
	{
		const int32 Percent = FMath::RoundToInt(Scalar.BaseValue * 100.0f);
		return FString::Printf(
			TEXT("%s %s"),
			*WrapRichText(TEXT("stat"), FormatStatName(Scalar.SourceStat)),
			*FormatPercent(Percent));
	}

	return FormatInteger(ResolveScalarBaseDisplayValue(Scalar));
}

FString FormatDamageScalarValue(
	const FFinalBattleScalarValue& Scalar,
	const int32 DamagePowerPercentPointDelta,
	const int32 FinalDamagePercentDelta)
{
	if (Scalar.ScaleMode == EFinalBattleScalarMode::SourceStatMultiplier)
	{
		const int32 BasePercent = FMath::RoundToInt(Scalar.BaseValue * 100.0f);
		const int32 PointAdjustedPercent = BasePercent + (Scalar.SourceStat == EFinalBattleSourceStat::Attack ? DamagePowerPercentPointDelta : 0);
		const float FinalScale = 1.0f + static_cast<float>(FinalDamagePercentDelta) / 100.0f;
		const int32 DisplayPercent = FMath::Max(FMath::RoundToInt(static_cast<float>(PointAdjustedPercent) * FinalScale), 0);
		const FString ValueTag = DisplayPercent > BasePercent ? TEXT("good") : (DisplayPercent < BasePercent ? TEXT("bad") : TEXT("num"));
		return FString::Printf(
			TEXT("%s %s"),
			*WrapRichText(TEXT("stat"), FormatStatName(Scalar.SourceStat)),
			*FormatPercent(DisplayPercent, ValueTag));
	}

	const int32 BaseValue = ResolveScalarBaseDisplayValue(Scalar);
	const float FinalScale = 1.0f + static_cast<float>(FinalDamagePercentDelta) / 100.0f;
	const int32 DisplayValue = FMath::Max(FMath::RoundToInt(static_cast<float>(BaseValue) * FinalScale), 0);
	const FString ValueTag = DisplayValue > BaseValue ? TEXT("good") : (DisplayValue < BaseValue ? TEXT("bad") : TEXT("num"));
	return FormatInteger(DisplayValue, ValueTag);
}

FString FormatStatusDisplayName(const FFinalStatusId& StatusId, const UFinalStatusDefinition* StatusDefinition)
{
	if (StatusDefinition != nullptr && !StatusDefinition->DisplayName.IsEmpty())
	{
		return StatusDefinition->DisplayName.ToString();
	}
	return StatusId.Value.ToString();
}

FString FormatCardDisplayName(const FFinalCardId& CardId, const UFinalCardDefinition* CardDefinition)
{
	if (CardDefinition != nullptr && !CardDefinition->DisplayName.IsEmpty())
	{
		return CardDefinition->DisplayName.ToString();
	}
	return CardId.Value.ToString();
}

bool AreAllCandidateCardsSwordArray(const TArray<TObjectPtr<UFinalCardDefinition>>& CandidateCardDefinitions)
{
	if (CandidateCardDefinitions.Num() == 0)
	{
		return false;
	}

	for (const UFinalCardDefinition* CandidateCardDefinition : CandidateCardDefinitions)
	{
		if (CandidateCardDefinition == nullptr || !CandidateCardDefinition->Keywords.HasTagExact(GetSwordArrayKeyword()))
		{
			return false;
		}
	}
	return true;
}

FString ToInlineFragment(FString Fragment)
{
	Fragment.TrimStartAndEndInline();
	if (Fragment.EndsWith(TEXT("。")))
	{
		Fragment = Fragment.LeftChop(1);
	}
	return Fragment;
}

FString FormatCardTypeFilter(const FFinalTriggeredCardModifierDefinition& ModifierDefinition)
{
	if (!ModifierDefinition.bRequireCardType)
	{
		return TEXT("牌");
	}

	switch (ModifierDefinition.RequiredCardType)
	{
	case EFinalCardType::Attack:
		return TEXT("攻击牌");
	case EFinalCardType::Skill:
		return TEXT("技能牌");
	case EFinalCardType::Ability:
		return TEXT("能力牌");
	default:
		return TEXT("牌");
	}
}

FString FormatModifierTargetSource(const EFinalTriggeredCardModifierTargetSource TargetSource)
{
	switch (TargetSource)
	{
	case EFinalTriggeredCardModifierTargetSource::CurrentAllyHandCards:
		return TEXT("其他友方当前手牌");
	case EFinalTriggeredCardModifierTargetSource::CurrentOwnedHandCards:
		return TEXT("当前手牌");
	case EFinalTriggeredCardModifierTargetSource::DrawnCardsFromExecutedEffects:
		return TEXT("本次抽到的");
	default:
		return TEXT("目标");
	}
}

bool FindTextFragmentOverride(
	const UFinalCardDefinition& CardDefinition,
	const FName EffectId,
	const EFinalCardTextFragmentKind FragmentKind,
	FString& OutOverrideText)
{
	for (const FFinalCardTextFragmentOverride& Override : CardDefinition.TextFragmentOverrides)
	{
		if (Override.EffectId == EffectId && Override.FragmentKind == FragmentKind && !Override.OverrideText.IsEmpty())
		{
			OutOverrideText = Override.OverrideText.ToString();
			return true;
		}
	}
	return false;
}

const UFinalBattleEffectDefinition* FindEffectById(const UFinalCardDefinition& CardDefinition, const FName EffectId)
{
	for (const UFinalBattleEffectDefinition* EffectDefinition : CardDefinition.Effects)
	{
		if (EffectDefinition != nullptr && EffectDefinition->EffectId == EffectId)
		{
			return EffectDefinition;
		}
	}
	return nullptr;
}

bool BuildAutomaticEffectTextFragment(
	const UFinalBattleEffectDefinition& EffectDefinition,
	const FFinalBattleCardInstance& CardInstance,
	const EFinalCardTextFragmentKind FragmentKind,
	FString& OutFragment)
{
	if (const UFinalBattleEffectDamage* DamageEffect = Cast<const UFinalBattleEffectDamage>(&EffectDefinition))
	{
		OutFragment = FString::Printf(
			TEXT("造成 %s 伤害"),
			*FormatDamageScalarValue(DamageEffect->Scalar, CardInstance.RuntimeDamagePowerPercentPointDelta, CardInstance.RuntimeFinalDamagePercentDelta));
		if (DamageEffect->HitCount > 1)
		{
			OutFragment += FString::Printf(TEXT(" ×%s"), *FormatInteger(DamageEffect->HitCount));
		}
	}
	else if (const UFinalBattleEffectGainShield* ShieldEffect = Cast<const UFinalBattleEffectGainShield>(&EffectDefinition))
	{
		OutFragment = FString::Printf(TEXT("获得 %s 护盾"), *FormatScalarValue(ShieldEffect->Scalar));
	}
	else if (const UFinalBattleEffectHeal* HealEffect = Cast<const UFinalBattleEffectHeal>(&EffectDefinition))
	{
		OutFragment = FString::Printf(TEXT("回复 %s 共享生命"), *FormatScalarValue(HealEffect->Scalar));
	}
	else if (const UFinalBattleEffectBonusBreak* BonusBreakEffect = Cast<const UFinalBattleEffectBonusBreak>(&EffectDefinition))
	{
		OutFragment = FString::Printf(TEXT("额外 %s 削韧"), *FormatScalarValue(BonusBreakEffect->Scalar));
	}
	else if (const UFinalBattleEffectApplyStatus* ApplyStatusEffect = Cast<const UFinalBattleEffectApplyStatus>(&EffectDefinition))
	{
		OutFragment = FString::Printf(
			TEXT("+%s %s"),
			*FormatInteger(FMath::Max(ApplyStatusEffect->Stacks, 1)),
			*WrapRichText(TEXT("status"), FormatStatusDisplayName(ApplyStatusEffect->StatusId, ApplyStatusEffect->StatusDefinition)));
	}
	else if (const UFinalBattleEffectConsumeStatusResource* ConsumeEffect = Cast<const UFinalBattleEffectConsumeStatusResource>(&EffectDefinition))
	{
		OutFragment = FString::Printf(
			TEXT("消耗 %s %s"),
			*FormatInteger(FMath::Max(ConsumeEffect->StacksToConsume, 1)),
			*WrapRichText(TEXT("status"), FormatStatusDisplayName(ConsumeEffect->StatusId, ConsumeEffect->StatusDefinition)));
	}
	else if (const UFinalBattleEffectDrawCards* DrawCardsEffect = Cast<const UFinalBattleEffectDrawCards>(&EffectDefinition))
	{
		OutFragment = FString::Printf(TEXT("抽 %s 张牌"), *FormatInteger(FMath::Max(DrawCardsEffect->DrawCount, 1)));
	}
	else if (const UFinalBattleEffectGenerateCard* GenerateCardEffect = Cast<const UFinalBattleEffectGenerateCard>(&EffectDefinition))
	{
		FString GeneratedCardLabel;
		if (GenerateCardEffect->GeneratedCardDefinition != nullptr || GenerateCardEffect->GeneratedCardId.IsValid())
		{
			GeneratedCardLabel = WrapRichText(TEXT("keyword"), FormatCardDisplayName(GenerateCardEffect->GeneratedCardId, GenerateCardEffect->GeneratedCardDefinition));
		}
		else if (AreAllCandidateCardsSwordArray(GenerateCardEffect->CandidateCardDefinitions))
		{
			GeneratedCardLabel = WrapRichText(TEXT("keyword"), TEXT("剑阵"));
		}
		else
		{
			GeneratedCardLabel = WrapRichText(TEXT("keyword"), TEXT("随机牌"));
		}

		OutFragment = FString::Printf(TEXT("生成 %s 张 %s 到手牌"), *FormatInteger(FMath::Max(GenerateCardEffect->GenerateCount, 1)), *GeneratedCardLabel);
	}
	else if (const UFinalBattleEffectGainAP* GainAPEffect = Cast<const UFinalBattleEffectGainAP>(&EffectDefinition))
	{
		OutFragment = FString::Printf(TEXT("回复 %s"), *WrapRichText(TEXT("cost"), FString::Printf(TEXT("%d AP"), FMath::Max(GainAPEffect->GainValue, 0))));
	}
	else if (const UFinalBattleEffectApplyCardModifiers* ApplyCardModifiersEffect = Cast<const UFinalBattleEffectApplyCardModifiers>(&EffectDefinition))
	{
		TArray<FString> ModifierFragments;
		for (const FFinalTriggeredCardModifierDefinition& ModifierDefinition : ApplyCardModifiersEffect->CardModifiers)
		{
			TArray<FString> PayloadFragments;
			if (ModifierDefinition.CostDeltaAP != 0)
			{
				const FString Sign = ModifierDefinition.CostDeltaAP > 0 ? TEXT("+") : TEXT("-");
				const FString Tag = ModifierDefinition.CostDeltaAP > 0 ? TEXT("bad") : TEXT("cost");
				PayloadFragments.Add(FString::Printf(TEXT("%s%s"), *Sign, *WrapRichText(Tag, FString::Printf(TEXT("%d AP"), FMath::Abs(ModifierDefinition.CostDeltaAP)))));
			}
			if (ModifierDefinition.DamagePowerPercentPointDelta != 0)
			{
				PayloadFragments.Add(FormatSignedPercent(ModifierDefinition.DamagePowerPercentPointDelta));
			}
			if (ModifierDefinition.FinalDamagePercentDelta != 0)
			{
				PayloadFragments.Add(FString::Printf(TEXT("%s 最终伤害"), *FormatPercent(FMath::Abs(ModifierDefinition.FinalDamagePercentDelta), ModifierDefinition.FinalDamagePercentDelta >= 0 ? TEXT("good") : TEXT("bad"))));
			}

			if (PayloadFragments.Num() > 0)
			{
				ModifierFragments.Add(FString::Printf(
					TEXT("%s%s：%s"),
					*FormatModifierTargetSource(ModifierDefinition.TargetSource),
					*FormatCardTypeFilter(ModifierDefinition),
					*FString::Join(PayloadFragments, TEXT("，"))));
			}
		}

		if (ModifierFragments.Num() == 0)
		{
			return false;
		}

		OutFragment = FString::Join(ModifierFragments, TEXT("；"));
	}
	else
	{
		return false;
	}

	if (FragmentKind == EFinalCardTextFragmentKind::FullLine)
	{
		OutFragment += TEXT("。");
	}
	else
	{
		OutFragment = ToInlineFragment(OutFragment);
	}
	return true;
}

bool BuildEffectTextFragment(
	const UFinalCardDefinition& CardDefinition,
	const FFinalBattleCardInstance& CardInstance,
	const FName EffectId,
	const EFinalCardTextFragmentKind FragmentKind,
	FString& OutFragment)
{
	if (FindTextFragmentOverride(CardDefinition, EffectId, FragmentKind, OutFragment))
	{
		return true;
	}

	const UFinalBattleEffectDefinition* EffectDefinition = FindEffectById(CardDefinition, EffectId);
	if (EffectDefinition == nullptr)
	{
		return false;
	}

	return BuildAutomaticEffectTextFragment(*EffectDefinition, CardInstance, FragmentKind, OutFragment);
}

bool ReplaceNextTextToken(
	const UFinalCardDefinition& CardDefinition,
	const FFinalBattleCardInstance& CardInstance,
	FString& InOutLine)
{
	const int32 EffectTokenIndex = InOutLine.Find(TEXT("{effect:"));
	const int32 InlineTokenIndex = InOutLine.Find(TEXT("{inline:"));
	if (EffectTokenIndex == INDEX_NONE && InlineTokenIndex == INDEX_NONE)
	{
		return true;
	}

	const bool bUseEffectToken = InlineTokenIndex == INDEX_NONE || (EffectTokenIndex != INDEX_NONE && EffectTokenIndex < InlineTokenIndex);
	const FString TokenPrefix = bUseEffectToken ? TEXT("{effect:") : TEXT("{inline:");
	const int32 TokenStart = bUseEffectToken ? EffectTokenIndex : InlineTokenIndex;
	const int32 EffectIdStart = TokenStart + TokenPrefix.Len();
	const int32 TokenEnd = InOutLine.Find(TEXT("}"), ESearchCase::CaseSensitive, ESearchDir::FromStart, EffectIdStart);
	if (TokenEnd == INDEX_NONE)
	{
		return false;
	}

	const FString EffectIdString = InOutLine.Mid(EffectIdStart, TokenEnd - EffectIdStart);
	const FName EffectId(*EffectIdString);
	FString Fragment;
	if (!BuildEffectTextFragment(CardDefinition, CardInstance, EffectId, bUseEffectToken ? EFinalCardTextFragmentKind::FullLine : EFinalCardTextFragmentKind::Inline, Fragment))
	{
		return false;
	}

	InOutLine = InOutLine.Left(TokenStart) + Fragment + InOutLine.Mid(TokenEnd + 1);
	return true;
}

FText BuildResolvedRulesTextForCard(const FFinalBattleCardInstance& CardInstance)
{
	const UFinalCardDefinition* CardDefinition = CardInstance.ProjectedDefinition;
	if (CardDefinition == nullptr)
	{
		return FText::GetEmpty();
	}

	if (CardDefinition->TextMode != EFinalCardTextMode::EffectLayout)
	{
		return CardDefinition->RulesText;
	}

	TArray<FString> ResolvedLines;
	for (const FFinalCardTextLayoutLine& LayoutLine : CardDefinition->TextLayoutLines)
	{
		FString ResolvedLine = LayoutLine.Template;
		while (ResolvedLine.Contains(TEXT("{effect:")) || ResolvedLine.Contains(TEXT("{inline:")))
		{
			if (!ReplaceNextTextToken(*CardDefinition, CardInstance, ResolvedLine))
			{
				return CardDefinition->RulesText;
			}
		}
		ResolvedLines.Add(ResolvedLine);
	}

	return ResolvedLines.Num() > 0
		? FText::FromString(FString::Join(ResolvedLines, TEXT("\n")))
		: CardDefinition->RulesText;
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
		CardView.TargetRequirement = ResolveCardTargetRequirement(CardInstance->ProjectedDefinition);
		CardView.BaseCostAP = CardInstance->ProjectedDefinition != nullptr
			? CardInstance->ProjectedDefinition->BaseCostAP
			: CardInstance->RuntimeCostAP;
		CardView.RuntimeCostAP = CardInstance->RuntimeCostAP;
		CardView.RuntimeDamagePowerPercentPointDelta = CardInstance->RuntimeDamagePowerPercentPointDelta;
		CardView.RuntimeFinalDamagePercentDelta = CardInstance->RuntimeFinalDamagePercentDelta;
		CardView.RuntimeKeywords = CardInstance->RuntimeKeywords;
		CardView.ResolvedRulesText = BuildResolvedRulesTextForCard(*CardInstance);
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
