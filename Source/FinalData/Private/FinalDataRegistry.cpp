#include "Queries/FinalDataRegistry.h"

#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Logging/FinalLogChannels.h"

void UFinalDataRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CharacterDefinitions.Reset();
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

const UFinalCharacterDefinition* UFinalDataRegistry::FindCharacterDefinition(const FFinalCharacterId& CharacterId) const
{
	if (const TObjectPtr<UFinalCharacterDefinition>* Found = CharacterDefinitions.Find(CharacterId.Value))
	{
		return Found->Get();
	}

	UE_LOG(LogFinalData, Verbose, TEXT("CharacterDefinition not found for id %s"), *CharacterId.ToString());
	return nullptr;
}

const UFinalBattleEncounterDefinition* UFinalDataRegistry::FindEncounterDefinition(const FFinalEncounterId& EncounterId) const
{
	if (const TObjectPtr<UFinalBattleEncounterDefinition>* Found = EncounterDefinitions.Find(EncounterId.Value))
	{
		return Found->Get();
	}

	UE_LOG(LogFinalData, Verbose, TEXT("BattleEncounterDefinition not found for id %s"), *EncounterId.ToString());
	return nullptr;
}

const UFinalBattleRuleConfig* UFinalDataRegistry::FindRuleConfig(const FFinalRuleConfigId& RuleConfigId) const
{
	if (const TObjectPtr<UFinalBattleRuleConfig>* Found = RuleConfigs.Find(RuleConfigId.Value))
	{
		return Found->Get();
	}

	UE_LOG(LogFinalData, Verbose, TEXT("BattleRuleConfig not found for id %s"), *RuleConfigId.ToString());
	return nullptr;
}
