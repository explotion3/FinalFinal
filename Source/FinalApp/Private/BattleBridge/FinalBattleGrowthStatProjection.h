#pragma once

#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Facade/FinalBattleSessionTypes.h"
#include "Run/Definitions/FinalCharacterGrowthConfig.h"
#include "Runtime/FinalRunPersistentCharacterState.h"

namespace FinalBattleGrowthStatProjection
{
	inline FFinalBattleCharacterRuntimeStats BuildRuntimeStats(
		const FFinalRunPersistentCharacterState& CharacterState,
		const UFinalCharacterDefinition& CharacterDefinition,
		const UFinalCharacterGrowthConfig* GrowthConfig)
	{
		const int32 RootBoneVitalSharePerPoint = GrowthConfig != nullptr ? GrowthConfig->RootBoneVitalSharePerPoint : 0;
		const int32 RootBoneStressCapPerPoint = GrowthConfig != nullptr ? GrowthConfig->RootBoneStressCapPerPoint : 0;
		const int32 RootBoneDefensePerPoint = GrowthConfig != nullptr ? GrowthConfig->RootBoneDefensePerPoint : 0;
		const int32 KillingIntentAttackPerPoint = GrowthConfig != nullptr ? GrowthConfig->KillingIntentAttackPerPoint : 0;
		const float KillingIntentCritChancePerPoint = GrowthConfig != nullptr ? GrowthConfig->KillingIntentCritChancePerPoint : 0.0f;
		const float KillingIntentCritDamagePerPoint = GrowthConfig != nullptr ? GrowthConfig->KillingIntentCritDamagePerPoint : 0.0f;

		FFinalBattleCharacterRuntimeStats RuntimeStats;
		RuntimeStats.CharacterId = CharacterState.CharacterId;
		RuntimeStats.VitalShare = FMath::Max(
			0,
			CharacterDefinition.BaseVitalShare + CharacterState.RootBone * RootBoneVitalSharePerPoint);
		RuntimeStats.StressCap = FMath::Max(
			0,
			CharacterDefinition.BaseStressCap + CharacterState.RootBone * RootBoneStressCapPerPoint);
		RuntimeStats.RuntimeAttack = FMath::Max(
			0,
			CharacterDefinition.BaseAttack + CharacterState.KillingIntent * KillingIntentAttackPerPoint);
		RuntimeStats.RuntimeDefense = FMath::Max(
			0,
			CharacterDefinition.BaseDefense + CharacterState.RootBone * RootBoneDefensePerPoint);
		RuntimeStats.RuntimeBreakRate = FMath::Max(CharacterDefinition.BaseBreakRate, 0.0f);
		RuntimeStats.RuntimeCritChance = FMath::Clamp(
			CharacterDefinition.BaseCritChance + static_cast<float>(CharacterState.KillingIntent) * KillingIntentCritChancePerPoint,
			0.0f,
			1.0f);
		RuntimeStats.RuntimeCritDamage = FMath::Max(
			1.0f,
			CharacterDefinition.BaseCritDamage + static_cast<float>(CharacterState.KillingIntent) * KillingIntentCritDamagePerPoint);
		return RuntimeStats;
	}
}
