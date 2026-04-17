#pragma once

#include "CoreMinimal.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "Ids/FinalIds.h"
#include "FinalBattleEffectGenerateCard.generated.h"

class UFinalCardDefinition;

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class FINALDATA_API UFinalBattleEffectGenerateCard : public UFinalBattleEffectDefinition
{
	GENERATED_BODY()

public:
	UFinalBattleEffectGenerateCard();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	FFinalCardId GeneratedCardId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	TObjectPtr<UFinalCardDefinition> GeneratedCardDefinition = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	TArray<TObjectPtr<UFinalCardDefinition>> CandidateCardDefinitions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect", meta = (ClampMin = "1"))
	int32 GenerateCount = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	bool bChooseRandomCandidate = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	bool bGeneratedCard = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	bool bTemporaryCard = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	bool bRetainInHand = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Effect")
	bool bConsumeOnPlay = true;
};
