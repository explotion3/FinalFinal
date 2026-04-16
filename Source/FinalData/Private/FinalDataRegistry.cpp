#include "Queries/FinalDataRegistry.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalEnemyDefinition.h"
#include "Battle/Definitions/FinalEnemyIntentDefinition.h"
#include "Battle/Definitions/FinalStatusDefinition.h"
#include "Battle/Definitions/FinalUltimateDefinition.h"
#include "Run/Definitions/FinalRelicDefinition.h"
#include "Run/Definitions/FinalRunRouteDefinition.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogFinalDataRegistry, Log, All);

namespace
{
	template <typename TDefinition, typename TRegisterMemberFn>
	int32 RegisterDefinitionAssets(UFinalDataRegistry& Registry, IAssetRegistry& AssetRegistry, TRegisterMemberFn RegisterMemberFn)
	{
		TArray<FAssetData> AssetDatas;
		AssetRegistry.GetAssetsByClass(TDefinition::StaticClass()->GetClassPathName(), AssetDatas, true);

		int32 RegisteredCount = 0;
		for (const FAssetData& AssetData : AssetDatas)
		{
			if (TDefinition* Definition = Cast<TDefinition>(AssetData.GetAsset()))
			{
				(Registry.*RegisterMemberFn)(Definition);
				++RegisteredCount;
			}
		}

		return RegisteredCount;
	}
}

void UFinalDataRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CharacterDefinitions.Reset();
	CardDefinitions.Reset();
	EnemyDefinitions.Reset();
	EnemyIntentDefinitions.Reset();
	EncounterDefinitions.Reset();
	RelicDefinitions.Reset();
	RunRouteDefinitions.Reset();
	RuleConfigs.Reset();
	StatusDefinitions.Reset();
	UltimateDefinitions.Reset();

	DiscoverRuntimeDefinitions();
}

void UFinalDataRegistry::DiscoverRuntimeDefinitions()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	AssetRegistry.WaitForCompletion();

	const int32 CharacterCount = RegisterDefinitionAssets<UFinalCharacterDefinition>(*this, AssetRegistry, &UFinalDataRegistry::RegisterCharacterDefinition);
	const int32 CardCount = RegisterDefinitionAssets<UFinalCardDefinition>(*this, AssetRegistry, &UFinalDataRegistry::RegisterCardDefinition);
	const int32 EnemyCount = RegisterDefinitionAssets<UFinalEnemyDefinition>(*this, AssetRegistry, &UFinalDataRegistry::RegisterEnemyDefinition);
	const int32 EnemyIntentCount = RegisterDefinitionAssets<UFinalEnemyIntentDefinition>(*this, AssetRegistry, &UFinalDataRegistry::RegisterEnemyIntentDefinition);
	const int32 EncounterCount = RegisterDefinitionAssets<UFinalBattleEncounterDefinition>(*this, AssetRegistry, &UFinalDataRegistry::RegisterEncounterDefinition);
	const int32 RelicCount = RegisterDefinitionAssets<UFinalRelicDefinition>(*this, AssetRegistry, &UFinalDataRegistry::RegisterRelicDefinition);
	const int32 RunRouteCount = RegisterDefinitionAssets<UFinalRunRouteDefinition>(*this, AssetRegistry, &UFinalDataRegistry::RegisterRunRouteDefinition);
	const int32 RuleConfigCount = RegisterDefinitionAssets<UFinalBattleRuleConfig>(*this, AssetRegistry, &UFinalDataRegistry::RegisterRuleConfig);
	const int32 StatusCount = RegisterDefinitionAssets<UFinalStatusDefinition>(*this, AssetRegistry, &UFinalDataRegistry::RegisterStatusDefinition);
	const int32 UltimateCount = RegisterDefinitionAssets<UFinalUltimateDefinition>(*this, AssetRegistry, &UFinalDataRegistry::RegisterUltimateDefinition);

	UE_LOG(
		LogFinalDataRegistry,
		Log,
		TEXT("Discovered runtime definitions: RuleConfigs=%d Characters=%d Cards=%d Ultimates=%d Enemies=%d EnemyIntents=%d Statuses=%d Encounters=%d Relics=%d RunRoutes=%d"),
		RuleConfigCount,
		CharacterCount,
		CardCount,
		UltimateCount,
		EnemyCount,
		EnemyIntentCount,
		StatusCount,
		EncounterCount,
		RelicCount,
		RunRouteCount);
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

void UFinalDataRegistry::RegisterRunRouteDefinition(UFinalRunRouteDefinition* Definition)
{
	if (!IsValid(Definition) || Definition->RouteId.IsNone())
	{
		return;
	}

	RunRouteDefinitions.Add(Definition->RouteId, Definition);
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

UFinalRunRouteDefinition* UFinalDataRegistry::FindRunRouteDefinition(const FName& RouteId) const
{
	if (const TObjectPtr<UFinalRunRouteDefinition>* Found = RunRouteDefinitions.Find(RouteId))
	{
		return Found->Get();
	}

	UE_LOG(LogFinalDataRegistry, Verbose, TEXT("RunRouteDefinition not found for id %s"), *RouteId.ToString());
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
