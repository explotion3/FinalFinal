#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Ids/FinalIds.h"
#include "Battle/Definitions/FinalRuntimeTriggerDefinition.h"
#include "Types/FinalCoreTypes.h"
#include "FinalStatusDefinition.generated.h"

UENUM(BlueprintType)
enum class EFinalStatusStackKeyPolicy : uint8
{
	ByOwner,
	ByOwnerAndSource,
	SeparateInstances
};

UENUM(BlueprintType)
enum class EFinalStatusStackRule : uint8
{
	AddAndClamp,
	RefreshOnly,
	Replace
};

UENUM(BlueprintType)
enum class EFinalStatusDurationType : uint8
{
	Battle,
	PlayerTurns,
	EnemyTurns,
	Infinite,
	UntilConsumed
};

UENUM(BlueprintType)
enum class EFinalStatusExpireWindow : uint8
{
	None,
	PlayerTurnStart,
	PlayerTurnEnd,
	EnemyTurnStart,
	EnemyTurnEnd
};

UENUM(BlueprintType)
enum class EFinalStatusProjectedCardModifierLifetimePolicy : uint8
{
	WhileStatusActive,
	UntilPlayed,
	ManualClear
};

UENUM(BlueprintType)
enum class EFinalStatusResourceBehavior : uint8
{
	None,
	AccumulateAndConsume
};

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalStatusRuntimeModifierDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|RuntimeModifiers")
	int32 OutgoingDamagePercentPerStack = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|RuntimeModifiers")
	int32 IncomingDamagePercentPerStack = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|RuntimeModifiers")
	bool bOnlyAffectAttackCards = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|RuntimeModifiers")
	int32 IncomingTeamHealthDamageReductionPercentPerStack = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|RuntimeModifiers")
	bool bConsumeOnSuccessfulOwnerDamage = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|RuntimeModifiers")
	bool bConsumeOnPreventedTeamHealthDamage = false;
};

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalStatusProjectedCardModifierDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|ProjectedCardModifiers")
	EFinalTriggeredCardModifierTargetSource TargetSource = EFinalTriggeredCardModifierTargetSource::CurrentOwnedHandCards;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|ProjectedCardModifiers")
	bool bRequireCardType = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|ProjectedCardModifiers", meta = (EditCondition = "bRequireCardType"))
	EFinalCardType RequiredCardType = EFinalCardType::Attack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|ProjectedCardModifiers")
	int32 CostDeltaAPPerStack = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|ProjectedCardModifiers")
	int32 OutgoingDamagePercentPerStack = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|ProjectedCardModifiers")
	EFinalStatusProjectedCardModifierLifetimePolicy LifetimePolicy = EFinalStatusProjectedCardModifierLifetimePolicy::WhileStatusActive;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|ProjectedCardModifiers")
	bool bExpireAtPlayerTurnEnd = false;
};

UCLASS(BlueprintType)
class FINALDATA_API UFinalStatusDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, AssetRegistrySearchable, Category = "Final|Status")
	FFinalStatusId StatusId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status")
	EFinalStatusCategory StatusCategory = EFinalStatusCategory::Buff;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status")
	int32 MaxStacks = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status")
	int32 DefaultDuration = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status")
	FText SummaryText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|Resource")
	bool bIsResourceStatus = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|Resource", meta = (EditCondition = "bIsResourceStatus"))
	EFinalStatusResourceBehavior ResourceBehavior = EFinalStatusResourceBehavior::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|Resource", meta = (EditCondition = "bIsResourceStatus"))
	bool bAutoAffectBattleRules = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|Resource", meta = (EditCondition = "bIsResourceStatus"))
	bool bAutoProjectToCards = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status")
	EFinalStatusStackKeyPolicy StackKeyPolicy = EFinalStatusStackKeyPolicy::ByOwner;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status")
	EFinalStatusStackRule StackRule = EFinalStatusStackRule::AddAndClamp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status")
	EFinalStatusDurationType DurationType = EFinalStatusDurationType::PlayerTurns;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status")
	EFinalStatusExpireWindow ExpireWindow = EFinalStatusExpireWindow::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status")
	TArray<FFinalStatusRuntimeModifierDefinition> RuntimeModifiers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status")
	TArray<FFinalStatusProjectedCardModifierDefinition> ProjectedCardModifiers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status", meta = (TitleProperty = "Window"))
	TArray<FFinalRuntimeTriggerDefinition> RuntimeTriggers;

	// LegacyRuntimeFields: current battle runtime still reads these fields directly.
	// Keep them until the full status system migration completes, then delete them.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|LegacyRuntimeFields")
	int32 OutgoingDamagePercentPerStack = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|LegacyRuntimeFields")
	bool bExpireAtPlayerTurnEnd = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|LegacyRuntimeFields")
	bool bConsumeOnSuccessfulOwnerDamage = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|LegacyRuntimeFields")
	bool bOnlyAffectAttackCards = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|LegacyRuntimeFields")
	int32 IncomingTeamHealthDamageReductionPercentPerStack = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|LegacyRuntimeFields")
	bool bConsumeOnPreventedTeamHealthDamage = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|LegacyRuntimeFields")
	bool bProjectToOwnedHandCards = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|LegacyRuntimeFields")
	EFinalCardType ProjectedCardTypeFilter = EFinalCardType::Attack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Final|Status|LegacyRuntimeFields")
	int32 ProjectedOutgoingDamagePercentPerStack = 0;

};
