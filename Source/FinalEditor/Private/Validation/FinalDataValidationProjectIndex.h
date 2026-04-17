#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"

struct FFinalDataValidationProjectIndex
{
	static FFinalDataValidationProjectIndex Build();

	TArray<FString> FindDuplicateCardDefinitionPaths(const FFinalCardId& CardId, const FString& CurrentAssetPath) const;
	TArray<FString> FindDuplicateCharacterDefinitionPaths(const FFinalCharacterId& CharacterId, const FString& CurrentAssetPath) const;
	TArray<FString> FindDuplicateEnemyDefinitionPaths(const FFinalEnemyId& EnemyId, const FString& CurrentAssetPath) const;
	TArray<FString> FindDuplicateEnemyIntentDefinitionPaths(FName IntentId, const FString& CurrentAssetPath) const;
	TArray<FString> FindDuplicateEncounterDefinitionPaths(const FFinalEncounterId& EncounterId, const FString& CurrentAssetPath) const;
	TArray<FString> FindDuplicatePrototypeBootstrapDefinitionPaths(FName BootstrapId, const FString& CurrentAssetPath) const;
	TArray<FString> FindDuplicateRelicDefinitionPaths(const FFinalRelicId& RelicId, const FString& CurrentAssetPath) const;
	TArray<FString> FindDuplicateRunRouteDefinitionPaths(FName RouteId, const FString& CurrentAssetPath) const;
	TArray<FString> FindDuplicateStatusDefinitionPaths(const FFinalStatusId& StatusId, const FString& CurrentAssetPath) const;
	TArray<FString> FindDuplicateUltimateDefinitionPaths(const FFinalUltimateId& UltimateId, const FString& CurrentAssetPath) const;
	TArray<FString> FindDuplicateRuleConfigDefinitionPaths(const FFinalRuleConfigId& RuleConfigId, const FString& CurrentAssetPath) const;

	bool HasCardDefinition(const FFinalCardId& CardId) const;
	bool HasCharacterDefinition(const FFinalCharacterId& CharacterId) const;
	bool HasEncounterDefinition(const FFinalEncounterId& EncounterId) const;
	bool HasPrototypeBootstrapDefinition(FName BootstrapId) const;
	bool HasRelicDefinition(const FFinalRelicId& RelicId) const;
	bool HasRuleConfigDefinition(const FFinalRuleConfigId& RuleConfigId) const;
	bool HasRunRouteDefinition(FName RouteId) const;
	bool HasUltimateDefinition(const FFinalUltimateId& UltimateId) const;
	bool HasStatusDefinition(const FFinalStatusId& StatusId) const;

private:
	TMap<FName, TArray<FString>> CardDefinitionPathsById;
	TMap<FName, TArray<FString>> CharacterDefinitionPathsById;
	TMap<FName, TArray<FString>> EnemyDefinitionPathsById;
	TMap<FName, TArray<FString>> EnemyIntentDefinitionPathsById;
	TMap<FName, TArray<FString>> EncounterDefinitionPathsById;
	TMap<FName, TArray<FString>> PrototypeBootstrapDefinitionPathsById;
	TMap<FName, TArray<FString>> RelicDefinitionPathsById;
	TMap<FName, TArray<FString>> RunRouteDefinitionPathsById;
	TMap<FName, TArray<FString>> StatusDefinitionPathsById;
	TMap<FName, TArray<FString>> UltimateDefinitionPathsById;
	TMap<FName, TArray<FString>> RuleConfigDefinitionPathsById;
};
