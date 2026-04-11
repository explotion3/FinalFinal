#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Ids/FinalIds.h"
#include "Run/Definitions/FinalRelicDefinition.h"
#include "FinalDataRegistry.generated.h"

class UFinalBattleEncounterDefinition;
class UFinalBattleRuleConfig;
class UFinalCardDefinition;
class UFinalCharacterDefinition;
class UFinalEnemyDefinition;
class UFinalEnemyIntentDefinition;
class UFinalStatusDefinition;
class UFinalUltimateDefinition;

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
	void RegisterRelicDefinition(UFinalRelicDefinition* Definition);
	void RegisterRuleConfig(UFinalBattleRuleConfig* Definition);
	void RegisterStatusDefinition(UFinalStatusDefinition* Definition);
	void RegisterUltimateDefinition(UFinalUltimateDefinition* Definition);

	UFinalCharacterDefinition* FindCharacterDefinition(const FFinalCharacterId& CharacterId) const;
	UFinalCardDefinition* FindCardDefinition(const FFinalCardId& CardId) const;
	UFinalEnemyDefinition* FindEnemyDefinition(const FFinalEnemyId& EnemyId) const;
	UFinalEnemyIntentDefinition* FindEnemyIntentDefinition(const FName& IntentId) const;
	UFinalBattleEncounterDefinition* FindEncounterDefinition(const FFinalEncounterId& EncounterId) const;
	UFinalRelicDefinition* FindRelicDefinition(const FFinalRelicId& RelicId) const;
	UFinalBattleRuleConfig* FindRuleConfig(const FFinalRuleConfigId& RuleConfigId) const;
	UFinalStatusDefinition* FindStatusDefinition(const FFinalStatusId& StatusId) const;
	UFinalUltimateDefinition* FindUltimateDefinition(const FFinalUltimateId& UltimateId) const;

private:
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UFinalCharacterDefinition>> CharacterDefinitions;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UFinalCardDefinition>> CardDefinitions;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UFinalEnemyDefinition>> EnemyDefinitions;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UFinalEnemyIntentDefinition>> EnemyIntentDefinitions;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UFinalBattleEncounterDefinition>> EncounterDefinitions;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UFinalRelicDefinition>> RelicDefinitions;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UFinalBattleRuleConfig>> RuleConfigs;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UFinalStatusDefinition>> StatusDefinitions;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UFinalUltimateDefinition>> UltimateDefinitions;
};
