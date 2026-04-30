#pragma once

#include "CoreMinimal.h"
#include "FinalCoreTypes.generated.h"

UENUM(BlueprintType)
enum class EFinalCardType : uint8
{
	Attack,
	Skill,
	Ability
};

UENUM(BlueprintType)
enum class EFinalRarity : uint8
{
	Common,
	Rare,
	Epic,
	Legendary
};

UENUM(BlueprintType)
enum class EFinalLoadoutRole : uint8
{
	BaseAttack,
	BaseDefense,
	BaseTactic,
	InitialSignature,
	ExtraStartCard
};

UENUM(BlueprintType)
enum class EFinalBattleEffectType : uint8
{
	Damage,
	GainShield,
	Heal,
	ApplyStatus,
	RemoveStatus,
	ConsumeStatusResource,
	DrawCards,
	GainAP,
	GainEP,
	BonusBreak,
	ApplyPassive,
	GenerateCard,
	MoveCards,
	CopyCard
};

UENUM(BlueprintType)
enum class EFinalBattleCardZoneRule : uint8
{
	Hand,
	DrawPileTop,
	DrawPileBottom,
	DiscardPile,
	OngoingZone,
	ConsumePile
};

UENUM(BlueprintType)
enum class EFinalBattleUnitTargetRule : uint8
{
	None,
	Self,
	TeamPlayer,
	SelectedEnemy,
	FirstAliveEnemy,
	AllEnemies,
	AllPlayerCharacters
};

UENUM(BlueprintType)
enum class EFinalBattleScalarMode : uint8
{
	Flat,
	SourceStatMultiplier
};

UENUM(BlueprintType)
enum class EFinalBattleSourceStat : uint8
{
	None,
	Attack,
	Defense,
	BaseDamagePower
};

USTRUCT(BlueprintType)
struct FINALCORE_API FFinalBattleScalarValue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Effect")
	float BaseValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Effect")
	EFinalBattleScalarMode ScaleMode = EFinalBattleScalarMode::Flat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Effect")
	EFinalBattleSourceStat SourceStat = EFinalBattleSourceStat::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Effect")
	float FlatBonus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Effect")
	float Cap = 0.0f;
};

UENUM(BlueprintType)
enum class EFinalStatusCategory : uint8
{
	Buff,
	Debuff,
	Signature,
	Mechanic
};

UENUM(BlueprintType)
enum class EFinalPassiveStackPolicy : uint8
{
	RefreshExisting,
	IndependentInstances
};

UENUM(BlueprintType)
enum class EFinalPassiveDurationType : uint8
{
	Battle,
	PlayerTurns,
	Rounds
};

UENUM(BlueprintType)
enum class EFinalIntentType : uint8
{
	Attack,
	Defense,
	Buff,
	Debuff,
	Summon,
	Charge,
	Special
};

UENUM(BlueprintType)
enum class EFinalIntentSelectRule : uint8
{
	WeightedRandom,
	Cycle,
	PhaseSequence,
	Scripted
};

USTRUCT(BlueprintType)
struct FINALCORE_API FFinalEnemyPhaseDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Enemy")
	FName PhaseTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Enemy", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxHpPercent = 1.0f;
};
