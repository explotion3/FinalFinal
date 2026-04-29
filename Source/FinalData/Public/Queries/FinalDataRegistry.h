#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Ids/FinalIds.h"
#include "Run/Definitions/FinalRelicDefinition.h"
#include "FinalDataRegistry.generated.h"

class UFinalBattleEncounterDefinition;
class UFinalBattleRuleConfig;
class UFinalCardEvolutionDefinition;
class UFinalCharacterGrowthConfig;
class UFinalGrowthChoiceDefinition;
class UFinalCardDefinition;
class UFinalCharacterDefinition;
class UFinalEnemyDefinition;
class UFinalEnemyIntentDefinition;
class UFinalPrototypeBootstrapDefinition;
class UFinalRunRouteDefinition;
class UFinalStatusDefinition;
class UFinalUltimateDefinition;

USTRUCT()
struct FINALDATA_API FFinalDataRegistryAssetEntry
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FSoftObjectPath AssetPath;

	UPROPERTY(Transient)
	TObjectPtr<UObject> LoadedAsset = nullptr;
};

UCLASS()
class FINALDATA_API UFinalDataRegistry : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void RegisterCharacterDefinition(UFinalCharacterDefinition* Definition);
	void RegisterCardDefinition(UFinalCardDefinition* Definition);
	void RegisterEnemyDefinition(UFinalEnemyDefinition* Definition);
	void RegisterEnemyIntentDefinition(UFinalEnemyIntentDefinition* Definition);
	void RegisterEncounterDefinition(UFinalBattleEncounterDefinition* Definition);
	void RegisterPrototypeBootstrapDefinition(UFinalPrototypeBootstrapDefinition* Definition);
	void RegisterRelicDefinition(UFinalRelicDefinition* Definition);
	void RegisterRunRouteDefinition(UFinalRunRouteDefinition* Definition);
	void RegisterRuleConfig(UFinalBattleRuleConfig* Definition);
	void RegisterCharacterGrowthConfig(UFinalCharacterGrowthConfig* Definition);
	void RegisterGrowthChoiceDefinition(UFinalGrowthChoiceDefinition* Definition);
	void RegisterCardEvolutionDefinition(UFinalCardEvolutionDefinition* Definition);
	void RegisterStatusDefinition(UFinalStatusDefinition* Definition);
	void RegisterUltimateDefinition(UFinalUltimateDefinition* Definition);

	UFinalCharacterDefinition* FindCharacterDefinition(const FFinalCharacterId& CharacterId) const;
	UFinalCardDefinition* FindCardDefinition(const FFinalCardId& CardId) const;
	UFinalEnemyDefinition* FindEnemyDefinition(const FFinalEnemyId& EnemyId) const;
	UFinalEnemyIntentDefinition* FindEnemyIntentDefinition(const FName& IntentId) const;
	UFinalBattleEncounterDefinition* FindEncounterDefinition(const FFinalEncounterId& EncounterId) const;
	UFinalPrototypeBootstrapDefinition* FindPrototypeBootstrapDefinition(const FName& BootstrapId) const;
	UFinalRelicDefinition* FindRelicDefinition(const FFinalRelicId& RelicId) const;
	UFinalRunRouteDefinition* FindRunRouteDefinition(const FName& RouteId) const;
	UFinalBattleRuleConfig* FindRuleConfig(const FFinalRuleConfigId& RuleConfigId) const;
	UFinalCharacterGrowthConfig* FindCharacterGrowthConfig(const FFinalCharacterGrowthConfigId& GrowthConfigId) const;
	UFinalGrowthChoiceDefinition* FindGrowthChoiceDefinition(const FFinalGrowthChoiceId& GrowthChoiceId) const;
	UFinalCardEvolutionDefinition* FindCardEvolutionDefinition(const FFinalCardEvolutionId& EvolutionId) const;
	UFinalStatusDefinition* FindStatusDefinition(const FFinalStatusId& StatusId) const;
	UFinalUltimateDefinition* FindUltimateDefinition(const FFinalUltimateId& UltimateId) const;

private:
	void DiscoverRuntimeDefinitions();

	template <typename TDefinition>
	TDefinition* FindLoadedDefinition(TMap<FName, FFinalDataRegistryAssetEntry>& DefinitionEntries, FName StableId, const TCHAR* DefinitionTypeName);

	template <typename TDefinition>
	void RegisterLoadedDefinition(TMap<FName, FFinalDataRegistryAssetEntry>& DefinitionEntries, FName StableId, TDefinition* Definition);

	UPROPERTY(Transient)
	TMap<FName, FFinalDataRegistryAssetEntry> CharacterDefinitions;

	UPROPERTY(Transient)
	TMap<FName, FFinalDataRegistryAssetEntry> CardDefinitions;

	UPROPERTY(Transient)
	TMap<FName, FFinalDataRegistryAssetEntry> EnemyDefinitions;

	UPROPERTY(Transient)
	TMap<FName, FFinalDataRegistryAssetEntry> EnemyIntentDefinitions;

	UPROPERTY(Transient)
	TMap<FName, FFinalDataRegistryAssetEntry> EncounterDefinitions;

	UPROPERTY(Transient)
	TMap<FName, FFinalDataRegistryAssetEntry> PrototypeBootstrapDefinitions;

	UPROPERTY(Transient)
	TMap<FName, FFinalDataRegistryAssetEntry> RelicDefinitions;

	UPROPERTY(Transient)
	TMap<FName, FFinalDataRegistryAssetEntry> RunRouteDefinitions;

	UPROPERTY(Transient)
	TMap<FName, FFinalDataRegistryAssetEntry> RuleConfigs;

	UPROPERTY(Transient)
	TMap<FName, FFinalDataRegistryAssetEntry> CharacterGrowthConfigs;

	UPROPERTY(Transient)
	TMap<FName, FFinalDataRegistryAssetEntry> GrowthChoiceDefinitions;

	UPROPERTY(Transient)
	TMap<FName, FFinalDataRegistryAssetEntry> CardEvolutionDefinitions;

	UPROPERTY(Transient)
	TMap<FName, FFinalDataRegistryAssetEntry> StatusDefinitions;

	UPROPERTY(Transient)
	TMap<FName, FFinalDataRegistryAssetEntry> UltimateDefinitions;
};
