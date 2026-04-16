#include "Validation/FinalDataAssetValidator.h"

#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalEnemyDefinition.h"
#include "Battle/Definitions/FinalEnemyIntentDefinition.h"
#include "Battle/Definitions/FinalStatusDefinition.h"
#include "Battle/Definitions/FinalUltimateDefinition.h"
#include "Battle/Effects/FinalBattleEffectDamage.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "Battle/Effects/FinalBattleEffectDrawCards.h"
#include "Battle/Effects/FinalBattleEffectGainShield.h"
#include "Misc/DataValidation.h"
#include "Run/Definitions/FinalRelicDefinition.h"

namespace FinalDataAssetValidation
{
	void AddError(FDataValidationContext& Context, bool& bIsValid, const FString& Message)
	{
		Context.AddError(FText::FromString(Message));
		bIsValid = false;
	}

	void RequireName(FDataValidationContext& Context, bool& bIsValid, const FName Value, const TCHAR* FieldName)
	{
		if (Value.IsNone())
		{
			AddError(Context, bIsValid, FString::Printf(TEXT("%s must be set."), FieldName));
		}
	}

	void RequireText(FDataValidationContext& Context, bool& bIsValid, const FText& Value, const TCHAR* FieldName)
	{
		if (Value.IsEmptyOrWhitespace())
		{
			AddError(Context, bIsValid, FString::Printf(TEXT("%s must not be empty."), FieldName));
		}
	}

	void ValidateNonNegative(FDataValidationContext& Context, bool& bIsValid, const int32 Value, const TCHAR* FieldName)
	{
		if (Value < 0)
		{
			AddError(Context, bIsValid, FString::Printf(TEXT("%s must be >= 0, but is %d."), FieldName, Value));
		}
	}

	void ValidatePositive(FDataValidationContext& Context, bool& bIsValid, const int32 Value, const TCHAR* FieldName)
	{
		if (Value <= 0)
		{
			AddError(Context, bIsValid, FString::Printf(TEXT("%s must be > 0, but is %d."), FieldName, Value));
		}
	}

	void ValidateNonNegativeFloat(FDataValidationContext& Context, bool& bIsValid, const float Value, const TCHAR* FieldName)
	{
		if (Value < 0.0f)
		{
			AddError(Context, bIsValid, FString::Printf(TEXT("%s must be >= 0, but is %.3f."), FieldName, Value));
		}
	}

	void ValidateProbability(FDataValidationContext& Context, bool& bIsValid, const float Value, const TCHAR* FieldName)
	{
		if (Value < 0.0f || Value > 1.0f)
		{
			AddError(Context, bIsValid, FString::Printf(TEXT("%s must be between 0 and 1, but is %.3f."), FieldName, Value));
		}
	}

	template<typename SoftObjectType>
	void ValidateRequiredSoftObject(
		FDataValidationContext& Context,
		bool& bIsValid,
		const TSoftObjectPtr<SoftObjectType>& Value,
		const FString& FieldName)
	{
		if (Value.IsNull())
		{
			AddError(Context, bIsValid, FString::Printf(TEXT("%s must reference an asset."), *FieldName));
			return;
		}

		if (!Value.LoadSynchronous())
		{
			AddError(Context, bIsValid, FString::Printf(TEXT("%s references an asset that could not be loaded: %s."), *FieldName, *Value.ToSoftObjectPath().ToString()));
		}
	}

	void ValidateScalar(
		FDataValidationContext& Context,
		bool& bIsValid,
		const FFinalBattleScalarValue& Scalar,
		const FString& FieldName,
		const bool bRequirePositiveMagnitude)
	{
		if (Scalar.BaseValue < 0.0f)
		{
			AddError(Context, bIsValid, FString::Printf(TEXT("%s.BaseValue must be >= 0, but is %.3f."), *FieldName, Scalar.BaseValue));
		}

		if (Scalar.FlatBonus < 0.0f)
		{
			AddError(Context, bIsValid, FString::Printf(TEXT("%s.FlatBonus must be >= 0, but is %.3f."), *FieldName, Scalar.FlatBonus));
		}

		if (Scalar.Cap < 0.0f)
		{
			AddError(Context, bIsValid, FString::Printf(TEXT("%s.Cap must be >= 0, but is %.3f."), *FieldName, Scalar.Cap));
		}

		if (Scalar.ScaleMode == EFinalBattleScalarMode::SourceStatMultiplier && Scalar.SourceStat == EFinalBattleSourceStat::None)
		{
			AddError(Context, bIsValid, FString::Printf(TEXT("%s.SourceStat must be set when ScaleMode is SourceStatMultiplier."), *FieldName));
		}

		if (bRequirePositiveMagnitude && Scalar.BaseValue <= 0.0f && Scalar.FlatBonus <= 0.0f)
		{
			AddError(Context, bIsValid, FString::Printf(TEXT("%s must provide a positive BaseValue or FlatBonus."), *FieldName));
		}
	}

	void ValidateBattleEffect(
		FDataValidationContext& Context,
		bool& bIsValid,
		const UFinalBattleEffectDefinition* Effect,
		const FString& FieldName)
	{
		if (!Effect)
		{
			AddError(Context, bIsValid, FString::Printf(TEXT("%s must not be null."), *FieldName));
			return;
		}

		RequireName(Context, bIsValid, Effect->EffectId, *FString::Printf(TEXT("%s.EffectId"), *FieldName));

		if (Effect->FlatValue < 0.0f)
		{
			AddError(Context, bIsValid, FString::Printf(TEXT("%s.FlatValue must be >= 0, but is %.3f."), *FieldName, Effect->FlatValue));
		}

		if (const UFinalBattleEffectDamage* DamageEffect = Cast<const UFinalBattleEffectDamage>(Effect))
		{
			if (DamageEffect->EffectType != EFinalBattleEffectType::Damage)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s EffectType must be Damage for UFinalBattleEffectDamage."), *FieldName));
			}

			ValidateScalar(Context, bIsValid, DamageEffect->Scalar, FString::Printf(TEXT("%s.Scalar"), *FieldName), true);
			ValidatePositive(Context, bIsValid, DamageEffect->HitCount, *FString::Printf(TEXT("%s.HitCount"), *FieldName));
			return;
		}

		if (const UFinalBattleEffectGainShield* ShieldEffect = Cast<const UFinalBattleEffectGainShield>(Effect))
		{
			if (ShieldEffect->EffectType != EFinalBattleEffectType::GainShield)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s EffectType must be GainShield for UFinalBattleEffectGainShield."), *FieldName));
			}

			ValidateScalar(Context, bIsValid, ShieldEffect->Scalar, FString::Printf(TEXT("%s.Scalar"), *FieldName), true);
			return;
		}

		if (const UFinalBattleEffectDrawCards* DrawCardsEffect = Cast<const UFinalBattleEffectDrawCards>(Effect))
		{
			if (DrawCardsEffect->EffectType != EFinalBattleEffectType::DrawCards)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s EffectType must be DrawCards for UFinalBattleEffectDrawCards."), *FieldName));
			}

			ValidatePositive(Context, bIsValid, DrawCardsEffect->DrawCount, *FString::Printf(TEXT("%s.DrawCount"), *FieldName));
		}
	}

	template<typename EffectArrayType>
	void ValidateEffectArray(
		FDataValidationContext& Context,
		bool& bIsValid,
		const EffectArrayType& Effects,
		const TCHAR* FieldName,
		const bool bRequireAtLeastOne)
	{
		if (bRequireAtLeastOne && Effects.IsEmpty())
		{
			AddError(Context, bIsValid, FString::Printf(TEXT("%s must contain at least one effect."), FieldName));
		}

		for (int32 Index = 0; Index < Effects.Num(); ++Index)
		{
			ValidateBattleEffect(
				Context,
				bIsValid,
				Effects[Index].Get(),
				FString::Printf(TEXT("%s[%d]"), FieldName, Index));
		}
	}

	void ValidateCardDefinition(FDataValidationContext& Context, bool& bIsValid, const UFinalCardDefinition* Card)
	{
		if (!Card->CardId.IsValid())
		{
			AddError(Context, bIsValid, TEXT("CardId must be set."));
		}

		RequireText(Context, bIsValid, Card->DisplayName, TEXT("DisplayName"));
		ValidateNonNegative(Context, bIsValid, Card->BaseCostAP, TEXT("BaseCostAP"));
		ValidateEffectArray(Context, bIsValid, Card->Effects, TEXT("Effects"), true);
	}

	void ValidateCharacterDefinition(FDataValidationContext& Context, bool& bIsValid, const UFinalCharacterDefinition* Character)
	{
		if (!Character->CharacterId.IsValid())
		{
			AddError(Context, bIsValid, TEXT("CharacterId must be set."));
		}

		RequireText(Context, bIsValid, Character->DisplayName, TEXT("DisplayName"));
		ValidatePositive(Context, bIsValid, Character->BaseVitalShare, TEXT("BaseVitalShare"));
		ValidatePositive(Context, bIsValid, Character->BaseStressCap, TEXT("BaseStressCap"));
		ValidateNonNegative(Context, bIsValid, Character->BaseAttack, TEXT("BaseAttack"));
		ValidateNonNegative(Context, bIsValid, Character->BaseDefense, TEXT("BaseDefense"));
		ValidateNonNegativeFloat(Context, bIsValid, Character->BaseBreakRate, TEXT("BaseBreakRate"));
		ValidateProbability(Context, bIsValid, Character->BaseCritChance, TEXT("BaseCritChance"));

		if (Character->BaseCritDamage < 1.0f)
		{
			AddError(Context, bIsValid, FString::Printf(TEXT("BaseCritDamage must be >= 1, but is %.3f."), Character->BaseCritDamage));
		}

		ValidateNonNegative(Context, bIsValid, Character->EpGainPerAP, TEXT("EpGainPerAP"));

		if (Character->InitialLoadoutCards.IsEmpty())
		{
			AddError(Context, bIsValid, TEXT("InitialLoadoutCards must contain at least one card entry."));
		}

		for (int32 Index = 0; Index < Character->InitialLoadoutCards.Num(); ++Index)
		{
			const FFinalInitialLoadoutCardEntry& Entry = Character->InitialLoadoutCards[Index];
			if (!Entry.CardId.IsValid())
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("InitialLoadoutCards[%d].CardId must be set."), Index));
			}
			ValidatePositive(Context, bIsValid, Entry.Count, *FString::Printf(TEXT("InitialLoadoutCards[%d].Count"), Index));
		}

		if (Character->CharacterCardPoolIds.IsEmpty())
		{
			AddError(Context, bIsValid, TEXT("CharacterCardPoolIds must contain at least one card id."));
		}

		for (int32 Index = 0; Index < Character->CharacterCardPoolIds.Num(); ++Index)
		{
			if (!Character->CharacterCardPoolIds[Index].IsValid())
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("CharacterCardPoolIds[%d] must be set."), Index));
			}
		}

		if (!Character->UltimateId.IsValid())
		{
			AddError(Context, bIsValid, TEXT("UltimateId must be set."));
		}

		if (!Character->SignatureStatusId.IsValid())
		{
			AddError(Context, bIsValid, TEXT("SignatureStatusId must be set."));
		}
	}

	void ValidateEnemyDefinition(FDataValidationContext& Context, bool& bIsValid, const UFinalEnemyDefinition* Enemy)
	{
		if (!Enemy->EnemyId.IsValid())
		{
			AddError(Context, bIsValid, TEXT("EnemyId must be set."));
		}

		RequireText(Context, bIsValid, Enemy->DisplayName, TEXT("DisplayName"));
		ValidatePositive(Context, bIsValid, Enemy->MaxHP, TEXT("MaxHP"));
		ValidatePositive(Context, bIsValid, Enemy->MaxBreakValue, TEXT("MaxBreakValue"));
		ValidateNonNegative(Context, bIsValid, Enemy->BaseDamagePower, TEXT("BaseDamagePower"));
		ValidatePositive(Context, bIsValid, Enemy->InitialInitiativeValue, TEXT("InitialInitiativeValue"));
		ValidatePositive(Context, bIsValid, Enemy->InitiativeResponse, TEXT("InitiativeResponse"));

		for (int32 Index = 0; Index < Enemy->PhaseSequence.Num(); ++Index)
		{
			const FFinalEnemyPhaseDefinition& Phase = Enemy->PhaseSequence[Index];
			if (Phase.MaxHpPercent < 0.0f || Phase.MaxHpPercent > 1.0f)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("PhaseSequence[%d].MaxHpPercent must be between 0 and 1, but is %.3f."), Index, Phase.MaxHpPercent));
			}
		}

		if (Enemy->IntentPool.IsEmpty())
		{
			AddError(Context, bIsValid, TEXT("IntentPool must contain at least one intent reference."));
		}

		for (int32 Index = 0; Index < Enemy->IntentPool.Num(); ++Index)
		{
			ValidateRequiredSoftObject(Context, bIsValid, Enemy->IntentPool[Index], FString::Printf(TEXT("IntentPool[%d]"), Index));
		}
	}

	void ValidateEnemyIntentDefinition(FDataValidationContext& Context, bool& bIsValid, const UFinalEnemyIntentDefinition* Intent)
	{
		RequireName(Context, bIsValid, Intent->IntentId, TEXT("IntentId"));
		RequireText(Context, bIsValid, Intent->DisplayName, TEXT("DisplayName"));
		ValidatePositive(Context, bIsValid, Intent->Weight, TEXT("Weight"));
		ValidateNonNegative(Context, bIsValid, Intent->CooldownTurns, TEXT("CooldownTurns"));
		ValidateNonNegative(Context, bIsValid, Intent->UseLimitPerBattle, TEXT("UseLimitPerBattle"));
		ValidateEffectArray(Context, bIsValid, Intent->Effects, TEXT("Effects"), true);
	}

	void ValidateEncounterDefinition(FDataValidationContext& Context, bool& bIsValid, const UFinalBattleEncounterDefinition* Encounter)
	{
		if (!Encounter->EncounterId.IsValid())
		{
			AddError(Context, bIsValid, TEXT("EncounterId must be set."));
		}

		RequireText(Context, bIsValid, Encounter->DisplayName, TEXT("DisplayName"));
		ValidateRequiredSoftObject(Context, bIsValid, Encounter->RuleConfig, TEXT("RuleConfig"));

		if (Encounter->EnemyRoster.IsEmpty())
		{
			AddError(Context, bIsValid, TEXT("EnemyRoster must contain at least one enemy entry."));
		}

		for (int32 Index = 0; Index < Encounter->EnemyRoster.Num(); ++Index)
		{
			const FFinalEnemyRosterEntry& Entry = Encounter->EnemyRoster[Index];
			ValidateRequiredSoftObject(Context, bIsValid, Entry.EnemyDefinition, FString::Printf(TEXT("EnemyRoster[%d].EnemyDefinition"), Index));
			ValidateNonNegative(Context, bIsValid, Entry.PositionIndex, *FString::Printf(TEXT("EnemyRoster[%d].PositionIndex"), Index));
			ValidatePositive(Context, bIsValid, Entry.SpawnWave, *FString::Printf(TEXT("EnemyRoster[%d].SpawnWave"), Index));
		}
	}

	void ValidateRelicDefinition(FDataValidationContext& Context, bool& bIsValid, const UFinalRelicDefinition* Relic)
	{
		if (!Relic->RelicId.IsValid())
		{
			AddError(Context, bIsValid, TEXT("RelicId must be set."));
		}

		RequireText(Context, bIsValid, Relic->DisplayName, TEXT("DisplayName"));

		for (int32 Index = 0; Index < Relic->BattleStartEffects.Num(); ++Index)
		{
			const FFinalRelicBattleStartEffectDefinition& Effect = Relic->BattleStartEffects[Index];
			if (Effect.EffectType == EFinalRelicBattleStartEffectType::None)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("BattleStartEffects[%d].EffectType must not be None."), Index));
			}
			ValidatePositive(Context, bIsValid, Effect.Value, *FString::Printf(TEXT("BattleStartEffects[%d].Value"), Index));
		}

		for (int32 Index = 0; Index < Relic->PlayerTurnStartEffects.Num(); ++Index)
		{
			const FFinalRelicPlayerTurnStartEffectDefinition& Effect = Relic->PlayerTurnStartEffects[Index];
			if (Effect.EffectType == EFinalRelicPlayerTurnStartEffectType::None)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("PlayerTurnStartEffects[%d].EffectType must not be None."), Index));
			}
			ValidatePositive(Context, bIsValid, Effect.Value, *FString::Printf(TEXT("PlayerTurnStartEffects[%d].Value"), Index));
		}
	}

	void ValidateRuleConfig(FDataValidationContext& Context, bool& bIsValid, const UFinalBattleRuleConfig* RuleConfig)
	{
		if (!RuleConfig->RuleConfigId.IsValid())
		{
			AddError(Context, bIsValid, TEXT("RuleConfigId must be set."));
		}

		ValidateNonNegative(Context, bIsValid, RuleConfig->InitialAP, TEXT("InitialAP"));
		ValidateNonNegative(Context, bIsValid, RuleConfig->InitialHandSize, TEXT("InitialHandSize"));
		ValidatePositive(Context, bIsValid, RuleConfig->HandLimit, TEXT("HandLimit"));
		ValidatePositive(Context, bIsValid, RuleConfig->MaxEP, TEXT("MaxEP"));
		ValidateNonNegative(Context, bIsValid, RuleConfig->EndTurnEpGain, TEXT("EndTurnEpGain"));
		ValidateNonNegative(Context, bIsValid, RuleConfig->OnHitEpGain, TEXT("OnHitEpGain"));
		ValidateNonNegative(Context, bIsValid, RuleConfig->BaseCardEpGain, TEXT("BaseCardEpGain"));
		ValidateNonNegative(Context, bIsValid, RuleConfig->BreakRewardAP, TEXT("BreakRewardAP"));
		ValidateNonNegative(Context, bIsValid, RuleConfig->NormalCardInitiativeEventCount, TEXT("NormalCardInitiativeEventCount"));
		ValidateNonNegative(Context, bIsValid, RuleConfig->CollapsedCardInitiativeEventCount, TEXT("CollapsedCardInitiativeEventCount"));
		ValidateNonNegative(Context, bIsValid, RuleConfig->AwakenStressResetValue, TEXT("AwakenStressResetValue"));
		ValidateNonNegative(Context, bIsValid, RuleConfig->CollapseCardAwakenGain, TEXT("CollapseCardAwakenGain"));
		ValidateNonNegative(Context, bIsValid, RuleConfig->StressHpLossPerPoint, TEXT("StressHpLossPerPoint"));
		ValidateNonNegative(Context, bIsValid, RuleConfig->StressHealPerPoint, TEXT("StressHealPerPoint"));
		ValidateNonNegative(Context, bIsValid, RuleConfig->MinStressChangePerEvent, TEXT("MinStressChangePerEvent"));
		ValidateNonNegative(Context, bIsValid, RuleConfig->MaxStressGainPerHit, TEXT("MaxStressGainPerHit"));
		ValidateNonNegative(Context, bIsValid, RuleConfig->StressRandomProtectionCount, TEXT("StressRandomProtectionCount"));
		ValidateNonNegative(Context, bIsValid, RuleConfig->DamageToBreakCap, TEXT("DamageToBreakCap"));

		if (RuleConfig->HandLimit < RuleConfig->InitialHandSize)
		{
			AddError(Context, bIsValid, FString::Printf(TEXT("HandLimit (%d) must be >= InitialHandSize (%d)."), RuleConfig->HandLimit, RuleConfig->InitialHandSize));
		}

		for (const TPair<int32, int32>& Threshold : RuleConfig->AwakenThresholdByCollapseCount)
		{
			if (Threshold.Key < 0)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("AwakenThresholdByCollapseCount has a negative key: %d."), Threshold.Key));
			}
			if (Threshold.Value <= 0)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("AwakenThresholdByCollapseCount[%d] must be > 0, but is %d."), Threshold.Key, Threshold.Value));
			}
		}

		for (const TPair<int32, float>& Chance : RuleConfig->DirectAwakenChanceByRemainingCount)
		{
			if (Chance.Key < 0)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("DirectAwakenChanceByRemainingCount has a negative key: %d."), Chance.Key));
			}
			if (Chance.Value < 0.0f || Chance.Value > 1.0f)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("DirectAwakenChanceByRemainingCount[%d] must be between 0 and 1, but is %.3f."), Chance.Key, Chance.Value));
			}
		}
	}

	void ValidateStatusDefinition(FDataValidationContext& Context, bool& bIsValid, const UFinalStatusDefinition* Status)
	{
		if (!Status->StatusId.IsValid())
		{
			AddError(Context, bIsValid, TEXT("StatusId must be set."));
		}

		RequireText(Context, bIsValid, Status->DisplayName, TEXT("DisplayName"));
		ValidatePositive(Context, bIsValid, Status->MaxStacks, TEXT("MaxStacks"));
		ValidateNonNegative(Context, bIsValid, Status->DefaultDuration, TEXT("DefaultDuration"));
		ValidateEffectArray(Context, bIsValid, Status->OnTickEffects, TEXT("OnTickEffects"), false);
	}

	void ValidateUltimateDefinition(FDataValidationContext& Context, bool& bIsValid, const UFinalUltimateDefinition* Ultimate)
	{
		if (!Ultimate->UltimateId.IsValid())
		{
			AddError(Context, bIsValid, TEXT("UltimateId must be set."));
		}

		RequireName(Context, bIsValid, Ultimate->OwnerUnitId, TEXT("OwnerUnitId"));
		RequireText(Context, bIsValid, Ultimate->DisplayName, TEXT("DisplayName"));
		ValidateNonNegative(Context, bIsValid, Ultimate->BaseCostEP, TEXT("BaseCostEP"));
		ValidateEffectArray(Context, bIsValid, Ultimate->Effects, TEXT("Effects"), true);
	}
}

bool UFinalDataAssetValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext) const
{
	return InAsset
		&& (InAsset->IsA<UFinalCardDefinition>()
			|| InAsset->IsA<UFinalCharacterDefinition>()
			|| InAsset->IsA<UFinalEnemyDefinition>()
			|| InAsset->IsA<UFinalEnemyIntentDefinition>()
			|| InAsset->IsA<UFinalBattleEncounterDefinition>()
			|| InAsset->IsA<UFinalRelicDefinition>()
			|| InAsset->IsA<UFinalBattleRuleConfig>()
			|| InAsset->IsA<UFinalStatusDefinition>()
			|| InAsset->IsA<UFinalUltimateDefinition>());
}

EDataValidationResult UFinalDataAssetValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext)
{
	bool bIsValid = true;

	if (const UFinalCardDefinition* Card = Cast<UFinalCardDefinition>(InAsset))
	{
		FinalDataAssetValidation::ValidateCardDefinition(InContext, bIsValid, Card);
	}
	else if (const UFinalCharacterDefinition* Character = Cast<UFinalCharacterDefinition>(InAsset))
	{
		FinalDataAssetValidation::ValidateCharacterDefinition(InContext, bIsValid, Character);
	}
	else if (const UFinalEnemyDefinition* Enemy = Cast<UFinalEnemyDefinition>(InAsset))
	{
		FinalDataAssetValidation::ValidateEnemyDefinition(InContext, bIsValid, Enemy);
	}
	else if (const UFinalEnemyIntentDefinition* Intent = Cast<UFinalEnemyIntentDefinition>(InAsset))
	{
		FinalDataAssetValidation::ValidateEnemyIntentDefinition(InContext, bIsValid, Intent);
	}
	else if (const UFinalBattleEncounterDefinition* Encounter = Cast<UFinalBattleEncounterDefinition>(InAsset))
	{
		FinalDataAssetValidation::ValidateEncounterDefinition(InContext, bIsValid, Encounter);
	}
	else if (const UFinalRelicDefinition* Relic = Cast<UFinalRelicDefinition>(InAsset))
	{
		FinalDataAssetValidation::ValidateRelicDefinition(InContext, bIsValid, Relic);
	}
	else if (const UFinalBattleRuleConfig* RuleConfig = Cast<UFinalBattleRuleConfig>(InAsset))
	{
		FinalDataAssetValidation::ValidateRuleConfig(InContext, bIsValid, RuleConfig);
	}
	else if (const UFinalStatusDefinition* Status = Cast<UFinalStatusDefinition>(InAsset))
	{
		FinalDataAssetValidation::ValidateStatusDefinition(InContext, bIsValid, Status);
	}
	else if (const UFinalUltimateDefinition* Ultimate = Cast<UFinalUltimateDefinition>(InAsset))
	{
		FinalDataAssetValidation::ValidateUltimateDefinition(InContext, bIsValid, Ultimate);
	}
	else
	{
		return EDataValidationResult::NotValidated;
	}

	return bIsValid ? EDataValidationResult::Valid : EDataValidationResult::Invalid;
}
