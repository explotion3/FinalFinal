#include "First/FirstCardDefinitionCompiler.h"

#include "First/FirstCardDefinition.h"

namespace
{
	EFirstCardEffectType CompileEffectType(const EFirstCardDefinitionEffectType EffectType)
	{
		switch (EffectType)
		{
		case EFirstCardDefinitionEffectType::Damage:
			return EFirstCardEffectType::Damage;
		case EFirstCardDefinitionEffectType::MoveHandCard:
			return EFirstCardEffectType::MoveHandCard;
		default:
			return EFirstCardEffectType::None;
		}
	}

	EFirstHandRole CompileHandRole(const EFirstCardDefinitionHandRole HandRole)
	{
		switch (HandRole)
		{
		case EFirstCardDefinitionHandRole::LeftHandCore:
			return EFirstHandRole::LeftHandCore;
		case EFirstCardDefinitionHandRole::RightHandCore:
			return EFirstHandRole::RightHandCore;
		default:
			return EFirstHandRole::None;
		}
	}

	EFirstHandZone CompileHandZone(const EFirstCardDefinitionHandZone HandZone)
	{
		switch (HandZone)
		{
		case EFirstCardDefinitionHandZone::Left:
			return EFirstHandZone::Left;
		case EFirstCardDefinitionHandZone::Both:
			return EFirstHandZone::Both;
		case EFirstCardDefinitionHandZone::Right:
			return EFirstHandZone::Right;
		default:
			return EFirstHandZone::None;
		}
	}

	EFirstHandMoveTargetPolicy CompileMoveTargetPolicy(const EFirstCardDefinitionHandMoveTargetPolicy Policy)
	{
		switch (Policy)
		{
		case EFirstCardDefinitionHandMoveTargetPolicy::RandomOtherThanSourceZone:
			return EFirstHandMoveTargetPolicy::RandomOtherThanSourceZone;
		case EFirstCardDefinitionHandMoveTargetPolicy::FixedZone:
			return EFirstHandMoveTargetPolicy::FixedZone;
		default:
			return EFirstHandMoveTargetPolicy::RandomValidZone;
		}
	}

	EFirstCardPlayDestination CompilePlayDestination(const EFirstCardDefinitionPlayDestination Destination)
	{
		switch (Destination)
		{
		case EFirstCardDefinitionPlayDestination::ReturnToHandRandomZone:
			return EFirstCardPlayDestination::ReturnToHandRandomZone;
		default:
			return EFirstCardPlayDestination::DiscardPile;
		}
	}
}

FFirstCardInstance FFirstCardDefinitionCompiler::CompileCardDefinition(const UFirstCardDefinition* Definition)
{
	return CompileCardDefinition(Definition, FGuid::NewGuid());
}

FFirstCardInstance FFirstCardDefinitionCompiler::CompileCardDefinition(const UFirstCardDefinition* Definition, const FGuid& CardInstanceId)
{
	FFirstCardInstance RuntimeCard;
	RuntimeCard.CardInstanceId = CardInstanceId;

	if (!IsValid(Definition))
	{
		return RuntimeCard;
	}

	RuntimeCard.CardId = Definition->CardId;
	RuntimeCard.DisplayName = Definition->DisplayName;
	RuntimeCard.BaseCost = FMath::Max(0, Definition->BaseCost);
	RuntimeCard.RuntimeCost = RuntimeCard.BaseCost;
	RuntimeCard.PlayerMaxHPBonusOnEnterBattle = FMath::Max(0, Definition->PlayerMaxHPBonusOnEnterBattle);
	RuntimeCard.PlayDestination = CompilePlayDestination(Definition->PlayDestination);
	RuntimeCard.HandRole = CompileHandRole(Definition->HandRole);
	RuntimeCard.bRequiresHandZoneToPlay = Definition->bRequiresHandZoneToPlay;
	RuntimeCard.RequiredHandZone = CompileHandZone(Definition->RequiredHandZone);
	RuntimeCard.bSkipInitiativeReductionOnPerfectReleaseInZone = Definition->bSkipInitiativeReductionOnPerfectReleaseInZone;
	RuntimeCard.PerfectReleaseInitiativeSkipZone = CompileHandZone(Definition->PerfectReleaseInitiativeSkipZone);
	RuntimeCard.Keywords = Definition->Keywords;

	RuntimeCard.Effects.Reserve(Definition->Effects.Num());
	for (const FFirstCardDefinitionEffect& DefinitionEffect : Definition->Effects)
	{
		FFirstCardEffectInstance RuntimeEffect;
		RuntimeEffect.EffectType = CompileEffectType(DefinitionEffect.EffectType);
		RuntimeEffect.EffectId = DefinitionEffect.EffectId;
		RuntimeEffect.Value = DefinitionEffect.Value;
		RuntimeEffect.MoveCardCount = DefinitionEffect.MoveCardCount;
		RuntimeEffect.bMoveRequiresSourceZone = DefinitionEffect.bMoveRequiresSourceZone;
		RuntimeEffect.MoveSourceZone = CompileHandZone(DefinitionEffect.MoveSourceZone);
		RuntimeEffect.MoveTargetPolicy = CompileMoveTargetPolicy(DefinitionEffect.MoveTargetPolicy);
		RuntimeEffect.MoveTargetZone = CompileHandZone(DefinitionEffect.MoveTargetZone);
		RuntimeEffect.MoveTargetCostDelta = DefinitionEffect.MoveTargetCostDelta;
		RuntimeEffect.bTransferActualCostReductionToSourceCard = DefinitionEffect.bTransferActualCostReductionToSourceCard;
		RuntimeCard.Effects.Add(RuntimeEffect);
	}

	return RuntimeCard;
}
