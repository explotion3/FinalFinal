#pragma once

#include "CoreMinimal.h"
#include "Commands/FinalBattleCommand.h"
#include "Events/FinalBattleEvent.h"
#include "Facade/FinalBattleSessionTypes.h"
#include "Queries/FinalBattleSnapshot.h"
#include "UObject/Object.h"
#include "FinalBattleSession.generated.h"

class UFinalBattleEncounterDefinition;
class UFinalBattleRuleConfig;
struct FFinalBattleState;
class FFinalBattleResolver;

UCLASS(BlueprintType)
class FINALBATTLE_API UFinalBattleSession : public UObject
{
	GENERATED_BODY()

public:
	UFinalBattleSession();
	virtual ~UFinalBattleSession() override;

	void InitializeSession(UFinalBattleEncounterDefinition* InEncounterDefinition, UFinalBattleRuleConfig* InRuleConfig, const FFinalBattleInitContext& InitContext);

	UFUNCTION(BlueprintCallable, Category = "Final|Battle")
	FFinalBattleEvent SubmitCommand(const FFinalBattleCommand& Command);

	UFUNCTION(BlueprintPure, Category = "Final|Battle")
	FFinalBattleSnapshot GetSnapshot() const;

	UFUNCTION(BlueprintCallable, Category = "Final|Battle")
	void ResetSession();

	UFUNCTION(BlueprintPure, Category = "Final|Battle")
	bool HasActiveBattle() const;

private:
	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleEncounterDefinition> EncounterDefinition;

	UPROPERTY(Transient)
	TObjectPtr<UFinalBattleRuleConfig> RuleConfig;

	FFinalBattleState* State = nullptr;
	FFinalBattleResolver* Resolver = nullptr;
};
