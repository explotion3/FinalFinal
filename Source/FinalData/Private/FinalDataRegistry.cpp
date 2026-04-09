#include "Queries/FinalDataRegistry.h"

#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"

DEFINE_LOG_CATEGORY_STATIC(LogFinalDataRegistry, Log, All);

void UFinalDataRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CharacterDefinitions.Reset();
	CardDefinitions.Reset();
	EncounterDefinitions.Reset();
	RuleConfigs.Reset();
}

void UFinalDataRegistry::RegisterCharacterDefinition(UFinalCharacterDefinition* Definition)
{
	if (!IsValid(Definition) || !Definition->CharacterId.IsValid())
	{
		return;
	}

	CharacterDefinitions.Add(Definition->CharacterId.Value, Definition);
}

void UFinalDataRegistry::RegisterCardDefinition(UFinalCardDefinition* Definition)
{
	if (!IsValid(Definition) || !Definition->CardId.IsValid())
	{
		return;
	}

	CardDefinitions.Add(Definition->CardId.Value, Definition);
}

void UFinalDataRegistry::RegisterEncounterDefinition(UFinalBattleEncounterDefinition* Definition)
{
	if (!IsValid(Definition) || !Definition->EncounterId.IsValid())
	{
		return;
	}

	EncounterDefinitions.Add(Definition->EncounterId.Value, Definition);
}

void UFinalDataRegistry::RegisterRuleConfig(UFinalBattleRuleConfig* Definition)
{
	if (!IsValid(Definition) || !Definition->RuleConfigId.IsValid())
	{
		return;
	}

	RuleConfigs.Add(Definition->RuleConfigId.Value, Definition);
}

UFinalCharacterDefinition* UFinalDataRegistry::FindCharacterDefinition(const FFinalCharacterId& CharacterId) const
{
	if (const TObjectPtr<UFinalCharacterDefinition>* Found = CharacterDefinitions.Find(CharacterId.Value))
	{
		return Found->Get();
	}

	UE_LOG(LogFinalDataRegistry, Verbose, TEXT("CharacterDefinition not found for id %s"), *CharacterId.ToString());
	return nullptr;
}

UFinalCardDefinition* UFinalDataRegistry::FindCardDefinition(const FFinalCardId& CardId) const
{
	if (const TObjectPtr<UFinalCardDefinition>* Found = CardDefinitions.Find(CardId.Value))
	{
		return Found->Get();
	}

	UE_LOG(LogFinalDataRegistry, Verbose, TEXT("CardDefinition not found for id %s"), *CardId.ToString());
	return nullptr;
}

UFinalBattleEncounterDefinition* UFinalDataRegistry::FindEncounterDefinition(const FFinalEncounterId& EncounterId) const
{
	if (const TObjectPtr<UFinalBattleEncounterDefinition>* Found = EncounterDefinitions.Find(EncounterId.Value))
	{
		return Found->Get();
	}

	UE_LOG(LogFinalDataRegistry, Verbose, TEXT("BattleEncounterDefinition not found for id %s"), *EncounterId.ToString());
	return nullptr;
}

UFinalBattleRuleConfig* UFinalDataRegistry::FindRuleConfig(const FFinalRuleConfigId& RuleConfigId) const
{
	if (const TObjectPtr<UFinalBattleRuleConfig>* Found = RuleConfigs.Find(RuleConfigId.Value))
	{
		return Found->Get();
	}

	UE_LOG(LogFinalDataRegistry, Verbose, TEXT("BattleRuleConfig not found for id %s"), *RuleConfigId.ToString());
	return nullptr;
}
