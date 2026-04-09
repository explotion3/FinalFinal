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
	DrawCards,
	GainAP,
	GainEP,
	BonusBreak,
	ApplyPassive,
	GenerateCard,
	CopyCard
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
