#include "Queries/FinalDataRegistry.h"

#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalEnemyDefinition.h"
#include "Battle/Definitions/FinalEnemyIntentDefinition.h"
#include "Battle/Definitions/FinalStatusDefinition.h"
#include "Battle/Definitions/FinalUltimateDefinition.h"
#include "Run/Definitions/FinalRelicDefinition.h"

DEFINE_LOG_CATEGORY_STATIC(LogFinalDataRegistry, Log, All);

void UFinalDataRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CharacterDefinitions.Reset();
	CardDefinitions.Reset();
	EnemyDefinitions.Reset();
	EnemyIntentDefinitions.Reset();
	EncounterDefinitions.Reset();
	RelicDefinitions.Reset();
	RuleConfigs.Reset();
	StatusDefinitions.Reset();
	UltimateDefinitions.Reset();
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

void UFinalDataRegistry::RegisterEnemyDefinition(UFinalEnemyDefinition* Definition)
{
	if (!IsValid(Definition) || !Definition->EnemyId.IsValid())
	{
		return;
	}

	EnemyDefinitions.Add(Definition->EnemyId.Value, Definition);
}

void UFinalDataRegistry::RegisterEnemyIntentDefinition(UFinalEnemyIntentDefinition* Definition)
{
	if (!IsValid(Definition) || Definition->IntentId.IsNone())
	{
		return;
	}

	EnemyIntentDefinitions.Add(Definition->IntentId, Definition);
}

void UFinalDataRegistry::RegisterEncounterDefinition(UFinalBattleEncounterDefinition* Definition)
{
	if (!IsValid(Definition) || !Definition->EncounterId.IsValid())
	{
		return;
	}

	EncounterDefinitions.Add(Definition->EncounterId.Value, Definition);
}

void UFinalDataRegistry::RegisterRelicDefinition(UFinalRelicDefinition* Definition)
{
	if (!IsValid(Definition) || !Definition->RelicId.IsValid())
	{
		return;
	}

	RelicDefinitions.Add(Definition->RelicId.Value, Definition);
}

void UFinalDataRegistry::RegisterRuleConfig(UFinalBattleRuleConfig* Definition)
{
	if (!IsValid(Definition) || !Definition->RuleConfigId.IsValid())
	{
		return;
	}

	RuleConfigs.Add(Definition->RuleConfigId.Value, Definition);
}

void UFinalDataRegistry::RegisterStatusDefinition(UFinalStatusDefinition* Definition)
{
	if (!IsValid(Definition) || !Definition->StatusId.IsValid())
	{
		return;
	}

	StatusDefinitions.Add(Definition->StatusId.Value, Definition);
}

void UFinalDataRegistry::RegisterUltimateDefinition(UFinalUltimateDefinition* Definition)
{
	if (!IsValid(Definition) || !Definition->UltimateId.IsValid())
	{
		return;
	}

	UltimateDefinitions.Add(Definition->UltimateId.Value, Definition);
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

UFinalEnemyDefinition* UFinalDataRegistry::FindEnemyDefinition(const FFinalEnemyId& EnemyId) const
{
	if (const TObjectPtr<UFinalEnemyDefinition>* Found = EnemyDefinitions.Find(EnemyId.Value))
	{
		return Found->Get();
	}

	UE_LOG(LogFinalDataRegistry, Verbose, TEXT("EnemyDefinition not found for id %s"), *EnemyId.ToString());
	return nullptr;
}

UFinalEnemyIntentDefinition* UFinalDataRegistry::FindEnemyIntentDefinition(const FName& IntentId) const
{
	if (const TObjectPtr<UFinalEnemyIntentDefinition>* Found = EnemyIntentDefinitions.Find(IntentId))
	{
		return Found->Get();
	}

	UE_LOG(LogFinalDataRegistry, Verbose, TEXT("EnemyIntentDefinition not found for id %s"), *IntentId.ToString());
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

UFinalRelicDefinition* UFinalDataRegistry::FindRelicDefinition(const FFinalRelicId& RelicId) const
{
	if (const TObjectPtr<UFinalRelicDefinition>* Found = RelicDefinitions.Find(RelicId.Value))
	{
		return Found->Get();
	}

	UE_LOG(LogFinalDataRegistry, Verbose, TEXT("RelicDefinition not found for id %s"), *RelicId.ToString());
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

UFinalStatusDefinition* UFinalDataRegistry::FindStatusDefinition(const FFinalStatusId& StatusId) const
{
	if (const TObjectPtr<UFinalStatusDefinition>* Found = StatusDefinitions.Find(StatusId.Value))
	{
		return Found->Get();
	}

	UE_LOG(LogFinalDataRegistry, Verbose, TEXT("StatusDefinition not found for id %s"), *StatusId.ToString());
	return nullptr;
}

UFinalUltimateDefinition* UFinalDataRegistry::FindUltimateDefinition(const FFinalUltimateId& UltimateId) const
{
	if (const TObjectPtr<UFinalUltimateDefinition>* Found = UltimateDefinitions.Find(UltimateId.Value))
	{
		return Found->Get();
	}

	UE_LOG(LogFinalDataRegistry, Verbose, TEXT("UltimateDefinition not found for id %s"), *UltimateId.ToString());
	return nullptr;
}
