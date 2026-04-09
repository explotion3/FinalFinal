#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Ids/FinalIds.h"
#include "FinalDataRegistry.generated.h"

class UFinalBattleEncounterDefinition;
class UFinalBattleRuleConfig;
class UFinalCardDefinition;
class UFinalCharacterDefinition;

UCLASS()
class FINALDATA_API UFinalDataRegistry : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void RegisterCharacterDefinition(UFinalCharacterDefinition* Definition);
	void RegisterCardDefinition(UFinalCardDefinition* Definition);
	void RegisterEncounterDefinition(UFinalBattleEncounterDefinition* Definition);
	void RegisterRuleConfig(UFinalBattleRuleConfig* Definition);

	UFinalCharacterDefinition* FindCharacterDefinition(const FFinalCharacterId& CharacterId) const;
	UFinalCardDefinition* FindCardDefinition(const FFinalCardId& CardId) const;
	UFinalBattleEncounterDefinition* FindEncounterDefinition(const FFinalEncounterId& EncounterId) const;
	UFinalBattleRuleConfig* FindRuleConfig(const FFinalRuleConfigId& RuleConfigId) const;

private:
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UFinalCharacterDefinition>> CharacterDefinitions;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UFinalCardDefinition>> CardDefinitions;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UFinalBattleEncounterDefinition>> EncounterDefinitions;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UFinalBattleRuleConfig>> RuleConfigs;
};
