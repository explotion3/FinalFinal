#include "Validation/FinalDataAssetValidator.h"

#include "Battle/Conditions/FinalBattleConditionDefinition.h"
#include "Battle/Conditions/FinalBattleConditionHandCard.h"
#include "Battle/Conditions/FinalBattleConditionMovedCards.h"
#include "Battle/Conditions/FinalBattleConditionResourceConsumed.h"
#include "Battle/Conditions/FinalBattleConditionResolvedCard.h"
#include "Battle/Conditions/FinalBattleConditionStatusChanged.h"
#include "Battle/Conditions/FinalBattleConditionTargetState.h"
#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalEnemyDefinition.h"
#include "Battle/Definitions/FinalEnemyIntentDefinition.h"
#include "Battle/Definitions/FinalPassiveDefinition.h"
#include "Battle/Definitions/FinalRuntimeTriggerDefinition.h"
#include "Battle/Definitions/FinalStatusDefinition.h"
#include "Battle/Definitions/FinalUltimateDefinition.h"
#include "Battle/Effects/FinalBattleEffectDamage.h"
#include "Battle/Effects/FinalBattleEffectDefinition.h"
#include "Battle/Effects/FinalBattleEffectDrawCards.h"
#include "Battle/Effects/FinalBattleEffectApplyPassive.h"
#include "Battle/Effects/FinalBattleEffectApplyStatus.h"
#include "Battle/Effects/FinalBattleEffectBonusBreak.h"
#include "Battle/Effects/FinalBattleEffectConsumeStatusResource.h"
#include "Battle/Effects/FinalBattleEffectGainAP.h"
#include "Battle/Effects/FinalBattleEffectGainShield.h"
#include "Battle/Effects/FinalBattleEffectGenerateCard.h"
#include "Battle/Effects/FinalBattleEffectHeal.h"
#include "Battle/Effects/FinalBattleEffectMoveCards.h"
#include "Battle/Effects/FinalBattleEffectRemoveStatus.h"
#include "Battle/Effects/FinalBattleTargetedEffectDefinition.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/DataValidation.h"
#include "Modules/ModuleManager.h"
#include "Run/Definitions/FinalPrototypeBootstrapDefinition.h"
#include "Run/Definitions/FinalRelicDefinition.h"
#include "Run/Definitions/FinalRunRouteDefinition.h"
#include "Validation/FinalDataValidationProjectIndex.h"

namespace FinalDataAssetValidation
{
	class FFinalDataValidationProjectIndexCache
	{
	public:
		const FFinalDataValidationProjectIndex& Get()
		{
			BindToAssetRegistryIfNeeded();

			if (bDirty || !CachedProjectIndex.IsSet())
			{
				CachedProjectIndex = FFinalDataValidationProjectIndex::Build();
				bDirty = false;
			}

			return CachedProjectIndex.GetValue();
		}

	private:
		void BindToAssetRegistryIfNeeded()
		{
			if (bBound)
			{
				return;
			}

			FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
			IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
			AssetAddedHandle = AssetRegistry.OnAssetAdded().AddRaw(this, &FFinalDataValidationProjectIndexCache::HandleAssetRegistryChanged);
			AssetRemovedHandle = AssetRegistry.OnAssetRemoved().AddRaw(this, &FFinalDataValidationProjectIndexCache::HandleAssetRegistryChanged);
			AssetUpdatedHandle = AssetRegistry.OnAssetUpdated().AddRaw(this, &FFinalDataValidationProjectIndexCache::HandleAssetRegistryChanged);
			AssetRenamedHandle = AssetRegistry.OnAssetRenamed().AddRaw(this, &FFinalDataValidationProjectIndexCache::HandleAssetRenamed);
			bBound = true;
		}

		void HandleAssetRegistryChanged(const FAssetData&)
		{
			bDirty = true;
		}

		void HandleAssetRenamed(const FAssetData&, const FString&)
		{
			bDirty = true;
		}

		TOptional<FFinalDataValidationProjectIndex> CachedProjectIndex;
		FDelegateHandle AssetAddedHandle;
		FDelegateHandle AssetRemovedHandle;
		FDelegateHandle AssetUpdatedHandle;
		FDelegateHandle AssetRenamedHandle;
		bool bDirty = true;
		bool bBound = false;
	};

	FFinalDataValidationProjectIndexCache& GetProjectIndexCache()
	{
		static FFinalDataValidationProjectIndexCache Cache;
		return Cache;
	}

	void AddError(FDataValidationContext& Context, bool& bIsValid, const FString& Message)
	{
		Context.AddError(FText::FromString(Message));
		bIsValid = false;
	}

	void AddWarning(FDataValidationContext& Context, const FString& Message)
	{
		Context.AddWarning(FText::FromString(Message));
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

	void ValidateBattleEffectCondition(
		FDataValidationContext& Context,
		bool& bIsValid,
		const UFinalBattleConditionDefinition* Condition,
		const FString& FieldName)
	{
		if (Condition == nullptr)
		{
			AddError(Context, bIsValid, FString::Printf(TEXT("%s must not be null."), *FieldName));
			return;
		}

		if (const UFinalBattleConditionStatusChanged* StatusChangedCondition = Cast<const UFinalBattleConditionStatusChanged>(Condition))
		{
			const FFinalBattleStatusChangeRequirement& Requirement = StatusChangedCondition->Requirement;
			if (!Requirement.RequiredStatusId.IsValid())
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s.Requirement.RequiredStatusId must be set."), *FieldName));
			}
			ValidatePositive(Context, bIsValid, Requirement.MinimumStacks, *FString::Printf(TEXT("%s.Requirement.MinimumStacks"), *FieldName));
			return;
		}

		if (const UFinalBattleConditionResourceConsumed* ResourceConsumedCondition = Cast<const UFinalBattleConditionResourceConsumed>(Condition))
		{
			const FFinalBattleResourceConsumeRequirement& Requirement = ResourceConsumedCondition->Requirement;
			if (!Requirement.RequiredStatusId.IsValid())
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s.Requirement.RequiredStatusId must be set."), *FieldName));
			}
			ValidatePositive(Context, bIsValid, Requirement.MinimumStacks, *FString::Printf(TEXT("%s.Requirement.MinimumStacks"), *FieldName));
			return;
		}

		if (const UFinalBattleConditionMovedCards* MovedCardsCondition = Cast<const UFinalBattleConditionMovedCards>(Condition))
		{
			const FFinalBattleMovedCardRequirement& Requirement = MovedCardsCondition->Requirement;
			ValidatePositive(Context, bIsValid, Requirement.MinimumCount, *FString::Printf(TEXT("%s.Requirement.MinimumCount"), *FieldName));
			if (Requirement.RequiredCardId.IsValid()
				&& !GetProjectIndexCache().Get().HasCardDefinition(Requirement.RequiredCardId))
			{
				AddError(
					Context,
					bIsValid,
					FString::Printf(
						TEXT("%s.Requirement.RequiredCardId references missing CardDefinition '%s'."),
						*FieldName,
						*Requirement.RequiredCardId.Value.ToString()));
			}
			return;
		}

		if (const UFinalBattleConditionHandCard* HandCardCondition = Cast<const UFinalBattleConditionHandCard>(Condition))
		{
			const FFinalBattleHandCardRequirement& Requirement = HandCardCondition->Requirement;
			if (Requirement.bRequireInHand)
			{
				ValidatePositive(Context, bIsValid, Requirement.MinimumCount, *FString::Printf(TEXT("%s.Requirement.MinimumCount"), *FieldName));
			}
			return;
		}

		if (const UFinalBattleConditionResolvedCard* ResolvedCardCondition = Cast<const UFinalBattleConditionResolvedCard>(Condition))
		{
			const FFinalBattleResolvedCardRequirement& Requirement = ResolvedCardCondition->Requirement;
			if (Requirement.bRequireCardCostAP)
			{
				ValidateNonNegative(Context, bIsValid, Requirement.RequiredCardCostAP, *FString::Printf(TEXT("%s.Requirement.RequiredCardCostAP"), *FieldName));
			}
			return;
		}

		if (Cast<const UFinalBattleConditionTargetState>(Condition))
		{
			return;
		}

		AddError(Context, bIsValid, FString::Printf(TEXT("%s uses unsupported battle condition class %s."), *FieldName, *Condition->GetClass()->GetName()));
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

		if (const UFinalBattleTargetedEffectDefinition* TargetedEffect = Cast<const UFinalBattleTargetedEffectDefinition>(Effect))
		{
			if (TargetedEffect->UnitTargetRule == EFinalBattleUnitTargetRule::None)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s.UnitTargetRule must not be None for targeted effects."), *FieldName));
			}
		}

		for (int32 ConditionIndex = 0; ConditionIndex < Effect->Conditions.Num(); ++ConditionIndex)
		{
			ValidateBattleEffectCondition(
				Context,
				bIsValid,
				Effect->Conditions[ConditionIndex].Get(),
				FString::Printf(TEXT("%s.Conditions[%d]"), *FieldName, ConditionIndex));
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

		if (const UFinalBattleEffectHeal* HealEffect = Cast<const UFinalBattleEffectHeal>(Effect))
		{
			if (HealEffect->EffectType != EFinalBattleEffectType::Heal)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s EffectType must be Heal for UFinalBattleEffectHeal."), *FieldName));
			}

			ValidateScalar(Context, bIsValid, HealEffect->Scalar, FString::Printf(TEXT("%s.Scalar"), *FieldName), true);
			return;
		}

		if (const UFinalBattleEffectApplyPassive* ApplyPassiveEffect = Cast<const UFinalBattleEffectApplyPassive>(Effect))
		{
			if (ApplyPassiveEffect->EffectType != EFinalBattleEffectType::ApplyPassive)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s EffectType must be ApplyPassive for UFinalBattleEffectApplyPassive."), *FieldName));
			}

			ValidatePositive(Context, bIsValid, ApplyPassiveEffect->Stacks, *FString::Printf(TEXT("%s.Stacks"), *FieldName));
			ValidateNonNegative(Context, bIsValid, ApplyPassiveEffect->DurationOverride, *FString::Printf(TEXT("%s.DurationOverride"), *FieldName));

			if (!ApplyPassiveEffect->PassiveId.IsValid() && ApplyPassiveEffect->PassiveDefinition == nullptr)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s must define PassiveId or PassiveDefinition."), *FieldName));
			}
			else if (ApplyPassiveEffect->PassiveId.IsValid()
				&& !GetProjectIndexCache().Get().HasPassiveDefinition(ApplyPassiveEffect->PassiveId)
				&& ApplyPassiveEffect->PassiveDefinition == nullptr)
			{
				AddError(
					Context,
					bIsValid,
					FString::Printf(
						TEXT("%s.PassiveId references missing PassiveDefinition '%s'."),
						*FieldName,
						*ApplyPassiveEffect->PassiveId.Value.ToString()));
			}
			return;
		}

		if (const UFinalBattleEffectApplyStatus* ApplyStatusEffect = Cast<const UFinalBattleEffectApplyStatus>(Effect))
		{
			if (ApplyStatusEffect->EffectType != EFinalBattleEffectType::ApplyStatus)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s EffectType must be ApplyStatus for UFinalBattleEffectApplyStatus."), *FieldName));
			}

			ValidatePositive(Context, bIsValid, ApplyStatusEffect->Stacks, *FString::Printf(TEXT("%s.Stacks"), *FieldName));
			return;
		}

		if (const UFinalBattleEffectRemoveStatus* RemoveStatusEffect = Cast<const UFinalBattleEffectRemoveStatus>(Effect))
		{
			if (RemoveStatusEffect->EffectType != EFinalBattleEffectType::RemoveStatus)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s EffectType must be RemoveStatus for UFinalBattleEffectRemoveStatus."), *FieldName));
			}

			ValidatePositive(Context, bIsValid, RemoveStatusEffect->Stacks, *FString::Printf(TEXT("%s.Stacks"), *FieldName));
			return;
		}

		if (const UFinalBattleEffectConsumeStatusResource* ConsumeStatusResourceEffect = Cast<const UFinalBattleEffectConsumeStatusResource>(Effect))
		{
			if (ConsumeStatusResourceEffect->EffectType != EFinalBattleEffectType::ConsumeStatusResource)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s EffectType must be ConsumeStatusResource for UFinalBattleEffectConsumeStatusResource."), *FieldName));
			}

			ValidatePositive(Context, bIsValid, ConsumeStatusResourceEffect->StacksToConsume, *FString::Printf(TEXT("%s.StacksToConsume"), *FieldName));
			return;
		}

		if (const UFinalBattleEffectDrawCards* DrawCardsEffect = Cast<const UFinalBattleEffectDrawCards>(Effect))
		{
			if (DrawCardsEffect->EffectType != EFinalBattleEffectType::DrawCards)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s EffectType must be DrawCards for UFinalBattleEffectDrawCards."), *FieldName));
			}

			ValidatePositive(Context, bIsValid, DrawCardsEffect->DrawCount, *FString::Printf(TEXT("%s.DrawCount"), *FieldName));
			return;
		}

		if (const UFinalBattleEffectGainAP* GainApEffect = Cast<const UFinalBattleEffectGainAP>(Effect))
		{
			if (GainApEffect->EffectType != EFinalBattleEffectType::GainAP)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s EffectType must be GainAP for UFinalBattleEffectGainAP."), *FieldName));
			}

			ValidateNonNegative(Context, bIsValid, GainApEffect->GainValue, *FString::Printf(TEXT("%s.GainValue"), *FieldName));
			return;
		}

		if (const UFinalBattleEffectBonusBreak* BonusBreakEffect = Cast<const UFinalBattleEffectBonusBreak>(Effect))
		{
			if (BonusBreakEffect->EffectType != EFinalBattleEffectType::BonusBreak)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s EffectType must be BonusBreak for UFinalBattleEffectBonusBreak."), *FieldName));
			}

			ValidateScalar(Context, bIsValid, BonusBreakEffect->Scalar, FString::Printf(TEXT("%s.Scalar"), *FieldName), true);
			return;
		}

		if (const UFinalBattleEffectGenerateCard* GenerateCardEffect = Cast<const UFinalBattleEffectGenerateCard>(Effect))
		{
			if (GenerateCardEffect->EffectType != EFinalBattleEffectType::GenerateCard)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s EffectType must be GenerateCard for UFinalBattleEffectGenerateCard."), *FieldName));
			}

			ValidatePositive(Context, bIsValid, GenerateCardEffect->GenerateCount, *FString::Printf(TEXT("%s.GenerateCount"), *FieldName));
			return;
		}

		if (const UFinalBattleEffectMoveCards* MoveCardsEffect = Cast<const UFinalBattleEffectMoveCards>(Effect))
		{
			if (MoveCardsEffect->EffectType != EFinalBattleEffectType::MoveCards)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s EffectType must be MoveCards for UFinalBattleEffectMoveCards."), *FieldName));
			}

			ValidatePositive(Context, bIsValid, MoveCardsEffect->MoveCount, *FString::Printf(TEXT("%s.MoveCount"), *FieldName));
			if (MoveCardsEffect->SourceZone == MoveCardsEffect->DestinationZone)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s SourceZone and DestinationZone must be different."), *FieldName));
			}
			if (MoveCardsEffect->RequiredCardId.IsValid()
				&& !GetProjectIndexCache().Get().HasCardDefinition(MoveCardsEffect->RequiredCardId))
			{
				AddError(
					Context,
					bIsValid,
					FString::Printf(
						TEXT("%s.RequiredCardId references missing CardDefinition '%s'."),
						*FieldName,
						*MoveCardsEffect->RequiredCardId.Value.ToString()));
			}
			return;
		}
	}

	FFinalStatusId ResolveRemoveStatusEffectStatusId(const UFinalBattleEffectRemoveStatus* RemoveStatusEffect)
	{
		if (RemoveStatusEffect == nullptr)
		{
			return FFinalStatusId();
		}

		if (RemoveStatusEffect->StatusId.IsValid())
		{
			return RemoveStatusEffect->StatusId;
		}

		return RemoveStatusEffect->StatusDefinition ? RemoveStatusEffect->StatusDefinition->StatusId : FFinalStatusId();
	}

	bool CanEarlierRemoveStatusSatisfy(
		const UFinalBattleEffectRemoveStatus* RemoveStatusEffect,
		const FFinalBattleStatusChangeRequirement& Requirement)
	{
		if (RemoveStatusEffect == nullptr
			|| Requirement.ChangeKind != EFinalBattleStatusChangeKind::Removed
			|| !Requirement.RequiredStatusId.IsValid())
		{
			return false;
		}

		return ResolveRemoveStatusEffectStatusId(RemoveStatusEffect) == Requirement.RequiredStatusId
			&& RemoveStatusEffect->Stacks >= FMath::Max(Requirement.MinimumStacks, 1);
	}

	FFinalStatusId ResolveConsumeStatusResourceEffectStatusId(const UFinalBattleEffectConsumeStatusResource* ConsumeStatusResourceEffect)
	{
		if (ConsumeStatusResourceEffect == nullptr)
		{
			return FFinalStatusId();
		}

		if (ConsumeStatusResourceEffect->StatusId.IsValid())
		{
			return ConsumeStatusResourceEffect->StatusId;
		}

		return ConsumeStatusResourceEffect->StatusDefinition ? ConsumeStatusResourceEffect->StatusDefinition->StatusId : FFinalStatusId();
	}

	bool CanEarlierConsumeStatusResourceSatisfy(
		const UFinalBattleEffectConsumeStatusResource* ConsumeStatusResourceEffect,
		const FFinalBattleResourceConsumeRequirement& Requirement)
	{
		if (ConsumeStatusResourceEffect == nullptr || !Requirement.RequiredStatusId.IsValid())
		{
			return false;
		}

		return ResolveConsumeStatusResourceEffectStatusId(ConsumeStatusResourceEffect) == Requirement.RequiredStatusId
			&& ConsumeStatusResourceEffect->StacksToConsume >= FMath::Max(Requirement.MinimumStacks, 1);
	}

	bool CanEarlierMoveCardsRecordMovedCardsSatisfy(
		const UFinalBattleEffectMoveCards* MoveCardsEffect,
		const FFinalBattleMovedCardRequirement& Requirement)
	{
		if (MoveCardsEffect == nullptr || !MoveCardsEffect->bRecordMovedCards)
		{
			return false;
		}

		if (MoveCardsEffect->MoveCount < FMath::Max(Requirement.MinimumCount, 1))
		{
			return false;
		}

		if (Requirement.RequiredCardId.IsValid()
			&& MoveCardsEffect->RequiredCardId.IsValid()
			&& MoveCardsEffect->RequiredCardId != Requirement.RequiredCardId)
		{
			return false;
		}

		if (Requirement.RequiredKeyword.IsValid()
			&& MoveCardsEffect->RequiredKeyword.IsValid()
			&& MoveCardsEffect->RequiredKeyword != Requirement.RequiredKeyword)
		{
			return false;
		}

		if (Requirement.bGeneratedOnly && !MoveCardsEffect->bGeneratedOnly)
		{
			return false;
		}

		if (Requirement.bRequireSourceZone && MoveCardsEffect->SourceZone != Requirement.SourceZone)
		{
			return false;
		}

		if (Requirement.bRequireDestinationZone && MoveCardsEffect->DestinationZone != Requirement.DestinationZone)
		{
			return false;
		}

		return true;
	}

	template<typename EffectArrayType>
	bool HasEarlierRemoveStatusProducer(
		const EffectArrayType& Effects,
		const int32 ConditionEffectIndex,
		const FFinalBattleStatusChangeRequirement& Requirement)
	{
		for (int32 ProducerIndex = 0; ProducerIndex < ConditionEffectIndex; ++ProducerIndex)
		{
			if (CanEarlierRemoveStatusSatisfy(Cast<const UFinalBattleEffectRemoveStatus>(Effects[ProducerIndex].Get()), Requirement))
			{
				return true;
			}
		}

		return false;
	}

	template<typename EffectArrayType>
	bool HasEarlierMoveCardsProducer(
		const EffectArrayType& Effects,
		const int32 ConditionEffectIndex,
		const FFinalBattleMovedCardRequirement& Requirement)
	{
		for (int32 ProducerIndex = 0; ProducerIndex < ConditionEffectIndex; ++ProducerIndex)
		{
			if (CanEarlierMoveCardsRecordMovedCardsSatisfy(Cast<const UFinalBattleEffectMoveCards>(Effects[ProducerIndex].Get()), Requirement))
			{
				return true;
			}
		}

		return false;
	}

	template<typename EffectArrayType>
	bool HasEarlierConsumeStatusResourceProducer(
		const EffectArrayType& Effects,
		const int32 ConditionEffectIndex,
		const FFinalBattleResourceConsumeRequirement& Requirement)
	{
		for (int32 ProducerIndex = 0; ProducerIndex < ConditionEffectIndex; ++ProducerIndex)
		{
			if (CanEarlierConsumeStatusResourceSatisfy(Cast<const UFinalBattleEffectConsumeStatusResource>(Effects[ProducerIndex].Get()), Requirement))
			{
				return true;
			}
		}

		return false;
	}

	template<typename EffectArrayType>
	void ValidateEffectConditionChain(
		FDataValidationContext& Context,
		const EffectArrayType& Effects,
		const TCHAR* FieldName)
	{
		for (int32 EffectIndex = 0; EffectIndex < Effects.Num(); ++EffectIndex)
		{
			const UFinalBattleEffectDefinition* Effect = Effects[EffectIndex].Get();
			if (Effect == nullptr)
			{
				continue;
			}

			for (int32 ConditionIndex = 0; ConditionIndex < Effect->Conditions.Num(); ++ConditionIndex)
			{
				const UFinalBattleConditionDefinition* Condition = Effect->Conditions[ConditionIndex].Get();
				const FString ConditionFieldName = FString::Printf(TEXT("%s[%d].Conditions[%d]"), FieldName, EffectIndex, ConditionIndex);

				if (const UFinalBattleConditionStatusChanged* StatusChangedCondition = Cast<const UFinalBattleConditionStatusChanged>(Condition))
				{
					const FFinalBattleStatusChangeRequirement& Requirement = StatusChangedCondition->Requirement;
					if (Requirement.ChangeKind == EFinalBattleStatusChangeKind::Removed
						&& Requirement.RequiredStatusId.IsValid()
						&& !HasEarlierRemoveStatusProducer(Effects, EffectIndex, Requirement))
					{
						AddWarning(
							Context,
							FString::Printf(
								TEXT("%s requires removed status '%s', but no earlier RemoveStatus effect in %s can obviously produce that chain record."),
								*ConditionFieldName,
								*Requirement.RequiredStatusId.Value.ToString(),
								FieldName));
					}
					continue;
				}

				if (const UFinalBattleConditionResourceConsumed* ResourceConsumedCondition = Cast<const UFinalBattleConditionResourceConsumed>(Condition))
				{
					const FFinalBattleResourceConsumeRequirement& Requirement = ResourceConsumedCondition->Requirement;
					if (Requirement.RequiredStatusId.IsValid()
						&& !HasEarlierConsumeStatusResourceProducer(Effects, EffectIndex, Requirement))
					{
						AddWarning(
							Context,
							FString::Printf(
								TEXT("%s requires consumed resource status '%s', but no earlier ConsumeStatusResource effect in %s can obviously produce that chain record."),
								*ConditionFieldName,
								*Requirement.RequiredStatusId.Value.ToString(),
								FieldName));
					}
					continue;
				}

				if (const UFinalBattleConditionMovedCards* MovedCardsCondition = Cast<const UFinalBattleConditionMovedCards>(Condition))
				{
					const FFinalBattleMovedCardRequirement& Requirement = MovedCardsCondition->Requirement;
					if (!HasEarlierMoveCardsProducer(Effects, EffectIndex, Requirement))
					{
						AddWarning(
							Context,
							FString::Printf(
								TEXT("%s requires moved card records, but no earlier MoveCards effect in %s can obviously produce that chain record."),
								*ConditionFieldName,
								FieldName));
					}
				}
			}
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

		ValidateEffectConditionChain(Context, Effects, FieldName);
	}

	template<typename TriggerArrayType>
	void ValidateRuntimeTriggerDefinitions(
		FDataValidationContext& Context,
		bool& bIsValid,
		const TriggerArrayType& Triggers,
		const TCHAR* FieldName)
	{
		for (int32 TriggerIndex = 0; TriggerIndex < Triggers.Num(); ++TriggerIndex)
		{
			const FFinalRuntimeTriggerDefinition& Trigger = Triggers[TriggerIndex];
			const FString TriggerFieldName = FString::Printf(TEXT("%s[%d]"), FieldName, TriggerIndex);

			if (Trigger.Domain == EFinalRuntimeTriggerDomain::None)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s.Domain must not be None."), *TriggerFieldName));
			}

			if (Trigger.Window == EFinalRuntimeTriggerWindow::None)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s.Window must not be None."), *TriggerFieldName));
			}

			if (Trigger.Effects.IsEmpty() && Trigger.TriggeredCardModifiers.IsEmpty())
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s must contain at least one effect or triggered card modifier."), *TriggerFieldName));
			}

			for (int32 ConditionIndex = 0; ConditionIndex < Trigger.Conditions.Num(); ++ConditionIndex)
			{
				ValidateBattleEffectCondition(
					Context,
					bIsValid,
					Trigger.Conditions[ConditionIndex].Get(),
					FString::Printf(TEXT("%s.Conditions[%d]"), *TriggerFieldName, ConditionIndex));
			}

			ValidateEffectArray(
				Context,
				bIsValid,
				Trigger.Effects,
				*FString::Printf(TEXT("%s.Effects"), *TriggerFieldName),
				true);

			for (int32 ModifierIndex = 0; ModifierIndex < Trigger.TriggeredCardModifiers.Num(); ++ModifierIndex)
			{
				const FFinalTriggeredCardModifierDefinition& ModifierDefinition = Trigger.TriggeredCardModifiers[ModifierIndex];
				const FString ModifierFieldName = FString::Printf(TEXT("%s.TriggeredCardModifiers[%d]"), *TriggerFieldName, ModifierIndex);

				if (ModifierDefinition.TargetSource == EFinalTriggeredCardModifierTargetSource::None)
				{
					AddError(Context, bIsValid, FString::Printf(TEXT("%s.TargetSource must not be None."), *ModifierFieldName));
				}

				if (ModifierDefinition.CostDeltaAP == 0 && ModifierDefinition.OutgoingDamagePercentDelta == 0)
				{
					AddError(Context, bIsValid, FString::Printf(TEXT("%s must define at least one non-zero modifier payload."), *ModifierFieldName));
				}
			}
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

		for (int32 Index = 0; Index < Character->InitialPassiveGrants.Num(); ++Index)
		{
			const FFinalInitialPassiveGrantDefinition& InitialPassiveGrant = Character->InitialPassiveGrants[Index];
			if (!InitialPassiveGrant.PassiveId.IsValid())
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("InitialPassiveGrants[%d].PassiveId must be set."), Index));
			}
			ValidatePositive(Context, bIsValid, InitialPassiveGrant.InitialStacks, *FString::Printf(TEXT("InitialPassiveGrants[%d].InitialStacks"), Index));
			ValidateNonNegative(Context, bIsValid, InitialPassiveGrant.DurationOverride, *FString::Printf(TEXT("InitialPassiveGrants[%d].DurationOverride"), Index));
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

		if (Enemy->IntentSelectRule == EFinalIntentSelectRule::PhaseSequence && Enemy->PhaseSequence.IsEmpty())
		{
			AddWarning(Context, TEXT("IntentSelectRule is PhaseSequence but PhaseSequence is empty; runtime will fall back to all eligible intents."));
		}

		if (Enemy->IntentSelectRule == EFinalIntentSelectRule::Scripted && Enemy->ScriptedIntentSequence.IsEmpty())
		{
			AddError(Context, bIsValid, TEXT("IntentSelectRule is Scripted but ScriptedIntentSequence is empty."));
		}

		if (Enemy->IntentPool.IsEmpty())
		{
			AddError(Context, bIsValid, TEXT("IntentPool must contain at least one intent reference."));
		}

		TSet<FName> IntentPoolIds;
		for (int32 Index = 0; Index < Enemy->IntentPool.Num(); ++Index)
		{
			ValidateRequiredSoftObject(Context, bIsValid, Enemy->IntentPool[Index], FString::Printf(TEXT("IntentPool[%d]"), Index));
			if (const UFinalEnemyIntentDefinition* IntentDefinition = Enemy->IntentPool[Index].LoadSynchronous())
			{
				IntentPoolIds.Add(IntentDefinition->IntentId);
			}
		}

		for (int32 Index = 0; Index < Enemy->ScriptedIntentSequence.Num(); ++Index)
		{
			const FFinalEnemyScriptedIntentStep& ScriptedStep = Enemy->ScriptedIntentSequence[Index];
			if (ScriptedStep.IntentId == NAME_None)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("ScriptedIntentSequence[%d].IntentId must be set."), Index));
				continue;
			}

			if (!IntentPoolIds.Contains(ScriptedStep.IntentId))
			{
				AddError(Context, bIsValid, FString::Printf(
					TEXT("ScriptedIntentSequence[%d].IntentId '%s' must be present in IntentPool."),
					Index,
					*ScriptedStep.IntentId.ToString()));
			}
		}
	}

	void ValidateEnemyIntentDefinition(FDataValidationContext& Context, bool& bIsValid, const UFinalEnemyIntentDefinition* Intent)
	{
		RequireName(Context, bIsValid, Intent->IntentId, TEXT("IntentId"));
		RequireText(Context, bIsValid, Intent->DisplayName, TEXT("DisplayName"));
		ValidateNonNegative(Context, bIsValid, Intent->Weight, TEXT("Weight"));
		ValidateNonNegative(Context, bIsValid, Intent->CooldownTurns, TEXT("CooldownTurns"));
		ValidateNonNegative(Context, bIsValid, Intent->UseLimitPerBattle, TEXT("UseLimitPerBattle"));
		ValidatePositive(Context, bIsValid, Intent->MinPreviewRound, TEXT("MinPreviewRound"));
		ValidateNonNegative(Context, bIsValid, Intent->MaxPreviewRound, TEXT("MaxPreviewRound"));
		if (Intent->MaxPreviewRound > 0 && Intent->MaxPreviewRound < Intent->MinPreviewRound)
		{
			AddError(Context, bIsValid, FString::Printf(
				TEXT("MaxPreviewRound must be 0 or >= MinPreviewRound, but MaxPreviewRound is %d and MinPreviewRound is %d."),
				Intent->MaxPreviewRound,
				Intent->MinPreviewRound));
		}
		ValidateProbability(Context, bIsValid, Intent->MinEnemyHpPercent, TEXT("MinEnemyHpPercent"));
		ValidateProbability(Context, bIsValid, Intent->MaxEnemyHpPercent, TEXT("MaxEnemyHpPercent"));
		if (Intent->MaxEnemyHpPercent < Intent->MinEnemyHpPercent)
		{
			AddError(Context, bIsValid, FString::Printf(
				TEXT("MaxEnemyHpPercent must be >= MinEnemyHpPercent, but MaxEnemyHpPercent is %.3f and MinEnemyHpPercent is %.3f."),
				Intent->MaxEnemyHpPercent,
				Intent->MinEnemyHpPercent));
		}
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

		ValidateRuntimeTriggerDefinitions(Context, bIsValid, Relic->RuntimeTriggers, TEXT("RuntimeTriggers"));
	}

	void ValidatePassiveDefinition(FDataValidationContext& Context, bool& bIsValid, const UFinalPassiveDefinition* Passive)
	{
		if (!Passive->PassiveId.IsValid())
		{
			AddError(Context, bIsValid, TEXT("PassiveId must be set."));
		}

		RequireText(Context, bIsValid, Passive->DisplayName, TEXT("DisplayName"));
		ValidatePositive(Context, bIsValid, Passive->MaxStacks, TEXT("MaxStacks"));
		ValidateRuntimeTriggerDefinitions(Context, bIsValid, Passive->RuntimeTriggers, TEXT("RuntimeTriggers"));

		if (Passive->AppliesTo == EFinalPassiveAppliesTo::EnemyOnly)
		{
			for (const FFinalRuntimeTriggerDefinition& TriggerDefinition : Passive->RuntimeTriggers)
			{
				for (const FFinalTriggeredCardModifierDefinition& TriggeredModifier : TriggerDefinition.TriggeredCardModifiers)
				{
					if (TriggeredModifier.TargetSource == EFinalTriggeredCardModifierTargetSource::CurrentOwnedHandCards)
					{
						AddWarning(
							Context,
							TEXT("EnemyOnly passives currently have no starter-backed hand/card ownership scenario; current-hand triggered card modifiers may be authored for future use but are unusual in the current project scope."));
						return;
					}
				}
			}
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
		ValidateNonNegative(Context, bIsValid, RuleConfig->TurnStartDrawCount, TEXT("TurnStartDrawCount"));
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
		ValidateRuntimeTriggerDefinitions(Context, bIsValid, Status->RuntimeTriggers, TEXT("RuntimeTriggers"));

		for (int32 ModifierIndex = 0; ModifierIndex < Status->RuntimeModifiers.Num(); ++ModifierIndex)
		{
			const FFinalStatusRuntimeModifierDefinition& ModifierDefinition = Status->RuntimeModifiers[ModifierIndex];
			const FString ModifierFieldName = FString::Printf(TEXT("RuntimeModifiers[%d]"), ModifierIndex);

			if (ModifierDefinition.OutgoingDamagePercentPerStack == 0
				&& ModifierDefinition.IncomingDamagePercentPerStack == 0
				&& ModifierDefinition.IncomingTeamHealthDamageReductionPercentPerStack == 0
				&& !ModifierDefinition.bConsumeOnSuccessfulOwnerDamage
				&& !ModifierDefinition.bConsumeOnPreventedTeamHealthDamage
				&& !ModifierDefinition.bOnlyAffectAttackCards)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s must define at least one non-default runtime modifier payload."), *ModifierFieldName));
			}
		}

		for (int32 ModifierIndex = 0; ModifierIndex < Status->ProjectedCardModifiers.Num(); ++ModifierIndex)
		{
			const FFinalStatusProjectedCardModifierDefinition& ModifierDefinition = Status->ProjectedCardModifiers[ModifierIndex];
			const FString ModifierFieldName = FString::Printf(TEXT("ProjectedCardModifiers[%d]"), ModifierIndex);

			if (ModifierDefinition.TargetSource == EFinalTriggeredCardModifierTargetSource::None)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s.TargetSource must not be None."), *ModifierFieldName));
			}

			if (ModifierDefinition.CostDeltaAPPerStack == 0 && ModifierDefinition.OutgoingDamagePercentPerStack == 0)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s must define at least one non-zero modifier payload."), *ModifierFieldName));
			}
		}

		if (Status->AppliesTo == EFinalStatusAppliesTo::EnemyOnly && !Status->ProjectedCardModifiers.IsEmpty())
		{
			AddWarning(
				Context,
				TEXT("EnemyOnly statuses currently have no starter-backed hand/card ownership scenario; projected card modifiers may be authored for future use but are unusual in the current project scope."));
		}

		if (Status->bIsResourceStatus)
		{
			if (Status->ResourceBehavior == EFinalStatusResourceBehavior::None)
			{
				AddError(Context, bIsValid, TEXT("Resource statuses must define ResourceBehavior."));
			}

			if (!Status->RuntimeModifiers.IsEmpty())
			{
				AddError(Context, bIsValid, TEXT("Resource statuses must not author RuntimeModifiers."));
			}

			if (!Status->ProjectedCardModifiers.IsEmpty())
			{
				AddError(Context, bIsValid, TEXT("Resource statuses must not author ProjectedCardModifiers."));
			}
		}

		if (Status->bIsDamageOverTime)
		{
			if (Status->DamageOverTimeTickWindow == EFinalStatusDamageOverTimeTickWindow::None)
			{
				AddError(Context, bIsValid, TEXT("Damage-over-time statuses must define DamageOverTimeTickWindow."));
			}

			if (Status->DamageOverTimeAttackPowerPercentPerStack <= 0)
			{
				AddError(Context, bIsValid, TEXT("Damage-over-time statuses must define a positive DamageOverTimeAttackPowerPercentPerStack."));
			}

			if (Status->bIsResourceStatus)
			{
				AddError(Context, bIsValid, TEXT("A status cannot be both a resource status and a damage-over-time status."));
			}
		}

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

	void ValidatePrototypeBootstrapDefinition(FDataValidationContext& Context, bool& bIsValid, const UFinalPrototypeBootstrapDefinition* Bootstrap)
	{
		RequireName(Context, bIsValid, Bootstrap->BootstrapId, TEXT("BootstrapId"));
		RequireText(Context, bIsValid, Bootstrap->DisplayName, TEXT("DisplayName"));

		if (!Bootstrap->RuleConfigId.IsValid())
		{
			AddError(Context, bIsValid, TEXT("RuleConfigId must be set."));
		}

		if (!Bootstrap->EncounterId.IsValid())
		{
			AddError(Context, bIsValid, TEXT("EncounterId must be set."));
		}

		RequireName(Context, bIsValid, Bootstrap->RunRouteId, TEXT("RunRouteId"));
		ValidatePositive(Context, bIsValid, Bootstrap->InitialTeamCurrentHP, TEXT("InitialTeamCurrentHP"));

		if (Bootstrap->PartyCharacterIds.IsEmpty())
		{
			AddError(Context, bIsValid, TEXT("PartyCharacterIds must contain at least one character id."));
		}

		for (int32 Index = 0; Index < Bootstrap->PartyCharacterIds.Num(); ++Index)
		{
			if (!Bootstrap->PartyCharacterIds[Index].IsValid())
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("PartyCharacterIds[%d] must be set."), Index));
			}
		}

		if (Bootstrap->InitialCharacterStates.IsEmpty())
		{
			AddError(Context, bIsValid, TEXT("InitialCharacterStates must contain at least one entry."));
		}

		TSet<FName> PartyCharacterIdSet;
		for (const FFinalCharacterId& CharacterId : Bootstrap->PartyCharacterIds)
		{
			if (CharacterId.IsValid())
			{
				PartyCharacterIdSet.Add(CharacterId.Value);
			}
		}

		TSet<FName> StateCharacterIdSet;
		for (int32 Index = 0; Index < Bootstrap->InitialCharacterStates.Num(); ++Index)
		{
			const FFinalPrototypeBootstrapCharacterState& CharacterState = Bootstrap->InitialCharacterStates[Index];
			if (!CharacterState.CharacterId.IsValid())
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("InitialCharacterStates[%d].CharacterId must be set."), Index));
			}

			ValidateNonNegative(Context, bIsValid, CharacterState.CurrentStress, *FString::Printf(TEXT("InitialCharacterStates[%d].CurrentStress"), Index));
			ValidateNonNegative(Context, bIsValid, CharacterState.CurrentAwakenCount, *FString::Printf(TEXT("InitialCharacterStates[%d].CurrentAwakenCount"), Index));
			ValidateNonNegative(Context, bIsValid, CharacterState.CollapseCount, *FString::Printf(TEXT("InitialCharacterStates[%d].CollapseCount"), Index));

			if (CharacterState.CharacterId.IsValid())
			{
				StateCharacterIdSet.Add(CharacterState.CharacterId.Value);
				if (!PartyCharacterIdSet.Contains(CharacterState.CharacterId.Value))
				{
					AddError(
						Context,
						bIsValid,
						FString::Printf(TEXT("InitialCharacterStates[%d].CharacterId '%s' is not present in PartyCharacterIds."), Index, *CharacterState.CharacterId.ToString()));
				}
			}
		}

		for (const FFinalCharacterId& CharacterId : Bootstrap->PartyCharacterIds)
		{
			if (CharacterId.IsValid() && !StateCharacterIdSet.Contains(CharacterId.Value))
			{
				AddError(
					Context,
					bIsValid,
					FString::Printf(TEXT("PartyCharacterIds is missing a matching InitialCharacterStates entry for '%s'."), *CharacterId.ToString()));
			}
		}

		if (Bootstrap->StarterDeckCardIds.IsEmpty())
		{
			AddError(Context, bIsValid, TEXT("StarterDeckCardIds must contain at least one card id."));
		}

		for (int32 Index = 0; Index < Bootstrap->StarterDeckCardIds.Num(); ++Index)
		{
			if (!Bootstrap->StarterDeckCardIds[Index].IsValid())
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("StarterDeckCardIds[%d] must be set."), Index));
			}
		}
	}

	void ValidateDuplicateStableId(
		FDataValidationContext& Context,
		bool& bIsValid,
		const FString& FieldName,
		const FString& StableId,
		const TArray<FString>& ConflictingAssetPaths)
	{
		if (StableId.IsEmpty() || ConflictingAssetPaths.IsEmpty())
		{
			return;
		}

		AddError(
			Context,
			bIsValid,
			FString::Printf(
				TEXT("%s '%s' is duplicated. Conflicting assets: %s."),
				*FieldName,
				*StableId,
				*FString::Join(ConflictingAssetPaths, TEXT(", "))));
	}

	void ValidateReferencedCardIdExists(
		FDataValidationContext& Context,
		bool& bIsValid,
		const FFinalDataValidationProjectIndex& ProjectIndex,
		const FFinalCardId& CardId,
		const FString& FieldName)
	{
		if (CardId.IsValid() && !ProjectIndex.HasCardDefinition(CardId))
		{
			AddError(
				Context,
				bIsValid,
				FString::Printf(TEXT("%s references missing CardDefinition stable ID '%s'."), *FieldName, *CardId.ToString()));
		}
	}

	void ValidateReferencedCharacterIdExists(
		FDataValidationContext& Context,
		bool& bIsValid,
		const FFinalDataValidationProjectIndex& ProjectIndex,
		const FFinalCharacterId& CharacterId,
		const FString& FieldName)
	{
		if (CharacterId.IsValid() && !ProjectIndex.HasCharacterDefinition(CharacterId))
		{
			AddError(
				Context,
				bIsValid,
				FString::Printf(TEXT("%s references missing CharacterDefinition stable ID '%s'."), *FieldName, *CharacterId.ToString()));
		}
	}

	void ValidateReferencedEncounterIdExists(
		FDataValidationContext& Context,
		bool& bIsValid,
		const FFinalDataValidationProjectIndex& ProjectIndex,
		const FFinalEncounterId& EncounterId,
		const FString& FieldName)
	{
		if (EncounterId.IsValid() && !ProjectIndex.HasEncounterDefinition(EncounterId))
		{
			AddError(
				Context,
				bIsValid,
				FString::Printf(TEXT("%s references missing BattleEncounterDefinition stable ID '%s'."), *FieldName, *EncounterId.ToString()));
		}
	}

	void ValidateReferencedRuleConfigIdExists(
		FDataValidationContext& Context,
		bool& bIsValid,
		const FFinalDataValidationProjectIndex& ProjectIndex,
		const FFinalRuleConfigId& RuleConfigId,
		const FString& FieldName)
	{
		if (RuleConfigId.IsValid() && !ProjectIndex.HasRuleConfigDefinition(RuleConfigId))
		{
			AddError(
				Context,
				bIsValid,
				FString::Printf(TEXT("%s references missing BattleRuleConfig stable ID '%s'."), *FieldName, *RuleConfigId.ToString()));
		}
	}

	void ValidateReferencedRunRouteIdExists(
		FDataValidationContext& Context,
		bool& bIsValid,
		const FFinalDataValidationProjectIndex& ProjectIndex,
		const FName RouteId,
		const FString& FieldName)
	{
		if (!RouteId.IsNone() && !ProjectIndex.HasRunRouteDefinition(RouteId))
		{
			AddError(
				Context,
				bIsValid,
				FString::Printf(TEXT("%s references missing RunRouteDefinition stable ID '%s'."), *FieldName, *RouteId.ToString()));
		}
	}

	void ValidateReferencedUltimateIdExists(
		FDataValidationContext& Context,
		bool& bIsValid,
		const FFinalDataValidationProjectIndex& ProjectIndex,
		const FFinalUltimateId& UltimateId,
		const FString& FieldName)
	{
		if (UltimateId.IsValid() && !ProjectIndex.HasUltimateDefinition(UltimateId))
		{
			AddError(
				Context,
				bIsValid,
				FString::Printf(TEXT("%s references missing UltimateDefinition stable ID '%s'."), *FieldName, *UltimateId.ToString()));
		}
	}

	void ValidateReferencedStatusIdExists(
		FDataValidationContext& Context,
		bool& bIsValid,
		const FFinalDataValidationProjectIndex& ProjectIndex,
		const FFinalStatusId& StatusId,
		const FString& FieldName)
	{
		if (StatusId.IsValid() && !ProjectIndex.HasStatusDefinition(StatusId))
		{
			AddError(
				Context,
				bIsValid,
				FString::Printf(TEXT("%s references missing StatusDefinition stable ID '%s'."), *FieldName, *StatusId.ToString()));
		}
	}

	void ValidateReferencedRelicIdExists(
		FDataValidationContext& Context,
		bool& bIsValid,
		const FFinalDataValidationProjectIndex& ProjectIndex,
		const FFinalRelicId& RelicId,
		const FString& FieldName)
	{
		if (RelicId.IsValid() && !ProjectIndex.HasRelicDefinition(RelicId))
		{
			AddError(
				Context,
				bIsValid,
				FString::Printf(TEXT("%s references missing RelicDefinition stable ID '%s'."), *FieldName, *RelicId.ToString()));
		}
	}

	void ValidateRunRewardEntry(
		FDataValidationContext& Context,
		bool& bIsValid,
		const FFinalDataValidationProjectIndex& ProjectIndex,
		const FFinalRunRewardEntry& RewardEntry,
		const FString& FieldName)
	{
		if (RewardEntry.RewardType == EFinalRunRewardType::None)
		{
			AddError(Context, bIsValid, FString::Printf(TEXT("%s.RewardType must not be None."), *FieldName));
			return;
		}

		switch (RewardEntry.RewardType)
		{
		case EFinalRunRewardType::Gold:
			ValidatePositive(Context, bIsValid, RewardEntry.Value, *FString::Printf(TEXT("%s.Value"), *FieldName));
			break;

		case EFinalRunRewardType::CardGrant:
			if (!RewardEntry.GrantedCardId.IsValid())
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s.GrantedCardId must be set for CardGrant rewards."), *FieldName));
			}
			else
			{
				ValidateReferencedCardIdExists(Context, bIsValid, ProjectIndex, RewardEntry.GrantedCardId, FString::Printf(TEXT("%s.GrantedCardId"), *FieldName));
			}
			break;

		case EFinalRunRewardType::RelicGrant:
			if (!RewardEntry.GrantedRelicId.IsValid())
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s.GrantedRelicId must be set for RelicGrant rewards."), *FieldName));
			}
			else
			{
				ValidateReferencedRelicIdExists(Context, bIsValid, ProjectIndex, RewardEntry.GrantedRelicId, FString::Printf(TEXT("%s.GrantedRelicId"), *FieldName));
			}
			break;

		case EFinalRunRewardType::RemoveCard:
			if (!RewardEntry.RemovedCardId.IsValid())
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s.RemovedCardId must be set for RemoveCard rewards."), *FieldName));
			}
			else
			{
				ValidateReferencedCardIdExists(Context, bIsValid, ProjectIndex, RewardEntry.RemovedCardId, FString::Printf(TEXT("%s.RemovedCardId"), *FieldName));
			}
			break;

		case EFinalRunRewardType::UpgradeCard:
			if (!RewardEntry.UpgradeFromCardId.IsValid())
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s.UpgradeFromCardId must be set for UpgradeCard rewards."), *FieldName));
			}
			else
			{
				ValidateReferencedCardIdExists(Context, bIsValid, ProjectIndex, RewardEntry.UpgradeFromCardId, FString::Printf(TEXT("%s.UpgradeFromCardId"), *FieldName));
			}

			if (!RewardEntry.UpgradeToCardId.IsValid())
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s.UpgradeToCardId must be set for UpgradeCard rewards."), *FieldName));
			}
			else
			{
				ValidateReferencedCardIdExists(Context, bIsValid, ProjectIndex, RewardEntry.UpgradeToCardId, FString::Printf(TEXT("%s.UpgradeToCardId"), *FieldName));
			}

			if (RewardEntry.UpgradeFromCardId.IsValid()
				&& RewardEntry.UpgradeToCardId.IsValid()
				&& RewardEntry.UpgradeFromCardId.Value == RewardEntry.UpgradeToCardId.Value)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s upgrade payload must not reference the same card for UpgradeFromCardId and UpgradeToCardId."), *FieldName));
			}
			break;

		case EFinalRunRewardType::Growth:
			if (!RewardEntry.GrowthTargetCharacterId.IsValid())
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s.GrowthTargetCharacterId must be set for Growth rewards."), *FieldName));
			}
			else
			{
				ValidateReferencedCharacterIdExists(Context, bIsValid, ProjectIndex, RewardEntry.GrowthTargetCharacterId, FString::Printf(TEXT("%s.GrowthTargetCharacterId"), *FieldName));
			}

			if (RewardEntry.GrowthEffectType == EFinalRunGrowthEffectType::None)
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s.GrowthEffectType must not be None for Growth rewards."), *FieldName));
			}

			ValidatePositive(Context, bIsValid, RewardEntry.Value, *FString::Printf(TEXT("%s.Value"), *FieldName));
			break;

		default:
			break;
		}
	}

	template<typename RewardEntryArrayType>
	void ValidateRunRewardEntries(
		FDataValidationContext& Context,
		bool& bIsValid,
		const FFinalDataValidationProjectIndex& ProjectIndex,
		const RewardEntryArrayType& RewardEntries,
		const FString& FieldName)
	{
		for (int32 RewardIndex = 0; RewardIndex < RewardEntries.Num(); ++RewardIndex)
		{
			ValidateRunRewardEntry(
				Context,
				bIsValid,
				ProjectIndex,
				RewardEntries[RewardIndex],
				FString::Printf(TEXT("%s[%d]"), *FieldName, RewardIndex));
		}
	}

	void ValidateRunRouteDefinition(
		FDataValidationContext& Context,
		bool& bIsValid,
		const UFinalRunRouteDefinition* Route,
		const FFinalDataValidationProjectIndex& ProjectIndex)
	{
		RequireName(Context, bIsValid, Route->RouteId, TEXT("RouteId"));
		RequireName(Context, bIsValid, Route->EntryNodeId, TEXT("EntryNodeId"));

		TSet<FName> NodeIds;
		bool bFoundEntryNode = false;

		for (int32 NodeIndex = 0; NodeIndex < Route->NodeDefinitions.Num(); ++NodeIndex)
		{
			const FFinalRunNodeDefinition& NodeDefinition = Route->NodeDefinitions[NodeIndex];
			const FString NodeField = FString::Printf(TEXT("NodeDefinitions[%d]"), NodeIndex);

			if (NodeDefinition.NodeId.IsNone())
			{
				AddError(Context, bIsValid, FString::Printf(TEXT("%s.NodeId must be set."), *NodeField));
			}
			else
			{
				if (NodeDefinition.NodeId == Route->EntryNodeId)
				{
					bFoundEntryNode = true;
				}

				if (NodeIds.Contains(NodeDefinition.NodeId))
				{
					AddError(Context, bIsValid, FString::Printf(TEXT("%s.NodeId '%s' is duplicated within RouteId '%s'."), *NodeField, *NodeDefinition.NodeId.ToString(), *Route->RouteId.ToString()));
				}
				else
				{
					NodeIds.Add(NodeDefinition.NodeId);
				}
			}
		}

		if (!Route->EntryNodeId.IsNone() && !bFoundEntryNode)
		{
			AddError(Context, bIsValid, FString::Printf(TEXT("EntryNodeId '%s' must match a NodeDefinitions[*].NodeId within the same route."), *Route->EntryNodeId.ToString()));
		}

		for (int32 NodeIndex = 0; NodeIndex < Route->NodeDefinitions.Num(); ++NodeIndex)
		{
			const FFinalRunNodeDefinition& NodeDefinition = Route->NodeDefinitions[NodeIndex];
			const FString NodeField = FString::Printf(TEXT("NodeDefinitions[%d]"), NodeIndex);

			for (int32 NextNodeIndex = 0; NextNodeIndex < NodeDefinition.NextNodeIds.Num(); ++NextNodeIndex)
			{
				const FName NextNodeId = NodeDefinition.NextNodeIds[NextNodeIndex];
				if (NextNodeId.IsNone())
				{
					AddError(Context, bIsValid, FString::Printf(TEXT("%s.NextNodeIds[%d] must be set."), *NodeField, NextNodeIndex));
				}
				else if (!NodeIds.Contains(NextNodeId))
				{
					AddError(Context, bIsValid, FString::Printf(TEXT("%s.NextNodeIds[%d] references missing node id '%s' within RouteId '%s'."), *NodeField, NextNodeIndex, *NextNodeId.ToString(), *Route->RouteId.ToString()));
				}
			}

			if (NodeDefinition.NodeType == EFinalRunNodeType::Battle
				|| NodeDefinition.NodeType == EFinalRunNodeType::EliteBattle
				|| NodeDefinition.NodeType == EFinalRunNodeType::BossBattle)
			{
				if (!NodeDefinition.EncounterId.IsValid())
				{
					AddError(Context, bIsValid, FString::Printf(TEXT("%s.EncounterId must be set for battle nodes."), *NodeField));
				}
				else
				{
					ValidateReferencedEncounterIdExists(Context, bIsValid, ProjectIndex, NodeDefinition.EncounterId, FString::Printf(TEXT("%s.EncounterId"), *NodeField));
				}

				if (!NodeDefinition.RuleConfigId.IsValid())
				{
					AddError(Context, bIsValid, FString::Printf(TEXT("%s.RuleConfigId must be set for battle nodes."), *NodeField));
				}
				else
				{
					ValidateReferencedRuleConfigIdExists(Context, bIsValid, ProjectIndex, NodeDefinition.RuleConfigId, FString::Printf(TEXT("%s.RuleConfigId"), *NodeField));
				}
			}

			if (NodeDefinition.NodeType == EFinalRunNodeType::Reward)
			{
				if (NodeDefinition.RewardContent.RewardEntries.IsEmpty())
				{
					AddError(Context, bIsValid, FString::Printf(TEXT("%s.RewardContent.RewardEntries must contain at least one reward entry."), *NodeField));
				}

				ValidateRunRewardEntries(Context, bIsValid, ProjectIndex, NodeDefinition.RewardContent.RewardEntries, FString::Printf(TEXT("%s.RewardContent.RewardEntries"), *NodeField));
			}

			if (NodeDefinition.NodeType == EFinalRunNodeType::Event)
			{
				if (NodeDefinition.EventContent.Options.IsEmpty())
				{
					AddError(Context, bIsValid, FString::Printf(TEXT("%s.EventContent.Options must contain at least one option."), *NodeField));
				}

				for (int32 OptionIndex = 0; OptionIndex < NodeDefinition.EventContent.Options.Num(); ++OptionIndex)
				{
					const FFinalRunEventOptionDefinition& OptionDefinition = NodeDefinition.EventContent.Options[OptionIndex];
					ValidateRunRewardEntries(
						Context,
						bIsValid,
						ProjectIndex,
						OptionDefinition.RewardEntries,
						FString::Printf(TEXT("%s.EventContent.Options[%d].RewardEntries"), *NodeField, OptionIndex));
				}
			}

			if (NodeDefinition.NodeType == EFinalRunNodeType::Shop)
			{
				if (NodeDefinition.ShopContent.Offers.IsEmpty())
				{
					AddError(Context, bIsValid, FString::Printf(TEXT("%s.ShopContent.Offers must contain at least one offer."), *NodeField));
				}

				for (int32 OfferIndex = 0; OfferIndex < NodeDefinition.ShopContent.Offers.Num(); ++OfferIndex)
				{
					const FFinalRunShopOfferDefinition& OfferDefinition = NodeDefinition.ShopContent.Offers[OfferIndex];
					ValidateRunRewardEntries(
						Context,
						bIsValid,
						ProjectIndex,
						OfferDefinition.RewardEntries,
						FString::Printf(TEXT("%s.ShopContent.Offers[%d].RewardEntries"), *NodeField, OfferIndex));
				}
			}
		}
	}

	void ValidateCardDefinitionProjectConsistency(
		FDataValidationContext& Context,
		bool& bIsValid,
		const UFinalCardDefinition* Card,
		const FString& CurrentAssetPath,
		const FFinalDataValidationProjectIndex& ProjectIndex)
	{
		ValidateDuplicateStableId(
			Context,
			bIsValid,
			TEXT("CardId"),
			Card->CardId.ToString(),
			ProjectIndex.FindDuplicateCardDefinitionPaths(Card->CardId, CurrentAssetPath));
	}

	void ValidateCharacterDefinitionProjectConsistency(
		FDataValidationContext& Context,
		bool& bIsValid,
		const UFinalCharacterDefinition* Character,
		const FString& CurrentAssetPath,
		const FFinalDataValidationProjectIndex& ProjectIndex)
	{
		ValidateDuplicateStableId(
			Context,
			bIsValid,
			TEXT("CharacterId"),
			Character->CharacterId.ToString(),
			ProjectIndex.FindDuplicateCharacterDefinitionPaths(Character->CharacterId, CurrentAssetPath));

		for (int32 Index = 0; Index < Character->InitialLoadoutCards.Num(); ++Index)
		{
			ValidateReferencedCardIdExists(
				Context,
				bIsValid,
				ProjectIndex,
				Character->InitialLoadoutCards[Index].CardId,
				FString::Printf(TEXT("InitialLoadoutCards[%d].CardId"), Index));
		}

		for (int32 Index = 0; Index < Character->CharacterCardPoolIds.Num(); ++Index)
		{
			ValidateReferencedCardIdExists(
				Context,
				bIsValid,
				ProjectIndex,
				Character->CharacterCardPoolIds[Index],
				FString::Printf(TEXT("CharacterCardPoolIds[%d]"), Index));
		}

		ValidateReferencedUltimateIdExists(Context, bIsValid, ProjectIndex, Character->UltimateId, TEXT("UltimateId"));
		ValidateReferencedStatusIdExists(Context, bIsValid, ProjectIndex, Character->SignatureStatusId, TEXT("SignatureStatusId"));
		for (int32 Index = 0; Index < Character->InitialPassiveGrants.Num(); ++Index)
		{
			const FFinalPassiveId PassiveId = Character->InitialPassiveGrants[Index].PassiveId;
			if (PassiveId.IsValid() && !ProjectIndex.HasPassiveDefinition(PassiveId))
			{
				AddError(
					Context,
					bIsValid,
					FString::Printf(
						TEXT("InitialPassiveGrants[%d].PassiveId references missing PassiveDefinition '%s'."),
						Index,
						*PassiveId.Value.ToString()));
			}
		}
	}

	void ValidateEnemyDefinitionProjectConsistency(
		FDataValidationContext& Context,
		bool& bIsValid,
		const UFinalEnemyDefinition* Enemy,
		const FString& CurrentAssetPath,
		const FFinalDataValidationProjectIndex& ProjectIndex)
	{
		ValidateDuplicateStableId(
			Context,
			bIsValid,
			TEXT("EnemyId"),
			Enemy->EnemyId.ToString(),
			ProjectIndex.FindDuplicateEnemyDefinitionPaths(Enemy->EnemyId, CurrentAssetPath));
	}

	void ValidateEnemyIntentDefinitionProjectConsistency(
		FDataValidationContext& Context,
		bool& bIsValid,
		const UFinalEnemyIntentDefinition* Intent,
		const FString& CurrentAssetPath,
		const FFinalDataValidationProjectIndex& ProjectIndex)
	{
		ValidateDuplicateStableId(
			Context,
			bIsValid,
			TEXT("IntentId"),
			Intent->IntentId.ToString(),
			ProjectIndex.FindDuplicateEnemyIntentDefinitionPaths(Intent->IntentId, CurrentAssetPath));
	}

	void ValidateEncounterDefinitionProjectConsistency(
		FDataValidationContext& Context,
		bool& bIsValid,
		const UFinalBattleEncounterDefinition* Encounter,
		const FString& CurrentAssetPath,
		const FFinalDataValidationProjectIndex& ProjectIndex)
	{
		ValidateDuplicateStableId(
			Context,
			bIsValid,
			TEXT("EncounterId"),
			Encounter->EncounterId.ToString(),
			ProjectIndex.FindDuplicateEncounterDefinitionPaths(Encounter->EncounterId, CurrentAssetPath));
	}

	void ValidatePrototypeBootstrapDefinitionProjectConsistency(
		FDataValidationContext& Context,
		bool& bIsValid,
		const UFinalPrototypeBootstrapDefinition* Bootstrap,
		const FString& CurrentAssetPath,
		const FFinalDataValidationProjectIndex& ProjectIndex)
	{
		ValidateDuplicateStableId(
			Context,
			bIsValid,
			TEXT("BootstrapId"),
			Bootstrap->BootstrapId.ToString(),
			ProjectIndex.FindDuplicatePrototypeBootstrapDefinitionPaths(Bootstrap->BootstrapId, CurrentAssetPath));

		ValidateReferencedRuleConfigIdExists(Context, bIsValid, ProjectIndex, Bootstrap->RuleConfigId, TEXT("RuleConfigId"));
		ValidateReferencedEncounterIdExists(Context, bIsValid, ProjectIndex, Bootstrap->EncounterId, TEXT("EncounterId"));
		ValidateReferencedRunRouteIdExists(Context, bIsValid, ProjectIndex, Bootstrap->RunRouteId, TEXT("RunRouteId"));

		for (int32 Index = 0; Index < Bootstrap->PartyCharacterIds.Num(); ++Index)
		{
			ValidateReferencedCharacterIdExists(
				Context,
				bIsValid,
				ProjectIndex,
				Bootstrap->PartyCharacterIds[Index],
				FString::Printf(TEXT("PartyCharacterIds[%d]"), Index));
		}

		for (int32 Index = 0; Index < Bootstrap->InitialCharacterStates.Num(); ++Index)
		{
			ValidateReferencedCharacterIdExists(
				Context,
				bIsValid,
				ProjectIndex,
				Bootstrap->InitialCharacterStates[Index].CharacterId,
				FString::Printf(TEXT("InitialCharacterStates[%d].CharacterId"), Index));
		}

		for (int32 Index = 0; Index < Bootstrap->StarterDeckCardIds.Num(); ++Index)
		{
			ValidateReferencedCardIdExists(
				Context,
				bIsValid,
				ProjectIndex,
				Bootstrap->StarterDeckCardIds[Index],
				FString::Printf(TEXT("StarterDeckCardIds[%d]"), Index));
		}
	}

	void ValidateRelicDefinitionProjectConsistency(
		FDataValidationContext& Context,
		bool& bIsValid,
		const UFinalRelicDefinition* Relic,
		const FString& CurrentAssetPath,
		const FFinalDataValidationProjectIndex& ProjectIndex)
	{
		ValidateDuplicateStableId(
			Context,
			bIsValid,
			TEXT("RelicId"),
			Relic->RelicId.ToString(),
			ProjectIndex.FindDuplicateRelicDefinitionPaths(Relic->RelicId, CurrentAssetPath));
	}

	void ValidatePassiveDefinitionProjectConsistency(
		FDataValidationContext& Context,
		bool& bIsValid,
		const UFinalPassiveDefinition* Passive,
		const FString& CurrentAssetPath,
		const FFinalDataValidationProjectIndex& ProjectIndex)
	{
		ValidateDuplicateStableId(
			Context,
			bIsValid,
			TEXT("PassiveId"),
			Passive->PassiveId.ToString(),
			ProjectIndex.FindDuplicatePassiveDefinitionPaths(Passive->PassiveId, CurrentAssetPath));
	}

	void ValidateRunRouteDefinitionProjectConsistency(
		FDataValidationContext& Context,
		bool& bIsValid,
		const UFinalRunRouteDefinition* Route,
		const FString& CurrentAssetPath,
		const FFinalDataValidationProjectIndex& ProjectIndex)
	{
		ValidateDuplicateStableId(
			Context,
			bIsValid,
			TEXT("RouteId"),
			Route->RouteId.ToString(),
			ProjectIndex.FindDuplicateRunRouteDefinitionPaths(Route->RouteId, CurrentAssetPath));
	}

	void ValidateRuleConfigProjectConsistency(
		FDataValidationContext& Context,
		bool& bIsValid,
		const UFinalBattleRuleConfig* RuleConfig,
		const FString& CurrentAssetPath,
		const FFinalDataValidationProjectIndex& ProjectIndex)
	{
		ValidateDuplicateStableId(
			Context,
			bIsValid,
			TEXT("RuleConfigId"),
			RuleConfig->RuleConfigId.ToString(),
			ProjectIndex.FindDuplicateRuleConfigDefinitionPaths(RuleConfig->RuleConfigId, CurrentAssetPath));
	}

	void ValidateStatusDefinitionProjectConsistency(
		FDataValidationContext& Context,
		bool& bIsValid,
		const UFinalStatusDefinition* Status,
		const FString& CurrentAssetPath,
		const FFinalDataValidationProjectIndex& ProjectIndex)
	{
		ValidateDuplicateStableId(
			Context,
			bIsValid,
			TEXT("StatusId"),
			Status->StatusId.ToString(),
			ProjectIndex.FindDuplicateStatusDefinitionPaths(Status->StatusId, CurrentAssetPath));
	}

	void ValidateUltimateDefinitionProjectConsistency(
		FDataValidationContext& Context,
		bool& bIsValid,
		const UFinalUltimateDefinition* Ultimate,
		const FString& CurrentAssetPath,
		const FFinalDataValidationProjectIndex& ProjectIndex)
	{
		ValidateDuplicateStableId(
			Context,
			bIsValid,
			TEXT("UltimateId"),
			Ultimate->UltimateId.ToString(),
			ProjectIndex.FindDuplicateUltimateDefinitionPaths(Ultimate->UltimateId, CurrentAssetPath));
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
			|| InAsset->IsA<UFinalPassiveDefinition>()
			|| InAsset->IsA<UFinalPrototypeBootstrapDefinition>()
			|| InAsset->IsA<UFinalRelicDefinition>()
			|| InAsset->IsA<UFinalRunRouteDefinition>()
			|| InAsset->IsA<UFinalBattleRuleConfig>()
			|| InAsset->IsA<UFinalStatusDefinition>()
			|| InAsset->IsA<UFinalUltimateDefinition>());
}

EDataValidationResult UFinalDataAssetValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& InContext)
{
	bool bIsValid = true;
	const FString CurrentAssetPath = InAssetData.GetSoftObjectPath().ToString();
	const FFinalDataValidationProjectIndex& ProjectIndex = FinalDataAssetValidation::GetProjectIndexCache().Get();

	if (const UFinalCardDefinition* Card = Cast<UFinalCardDefinition>(InAsset))
	{
		FinalDataAssetValidation::ValidateCardDefinition(InContext, bIsValid, Card);
		FinalDataAssetValidation::ValidateCardDefinitionProjectConsistency(InContext, bIsValid, Card, CurrentAssetPath, ProjectIndex);
	}
	else if (const UFinalCharacterDefinition* Character = Cast<UFinalCharacterDefinition>(InAsset))
	{
		FinalDataAssetValidation::ValidateCharacterDefinition(InContext, bIsValid, Character);
		FinalDataAssetValidation::ValidateCharacterDefinitionProjectConsistency(InContext, bIsValid, Character, CurrentAssetPath, ProjectIndex);
	}
	else if (const UFinalEnemyDefinition* Enemy = Cast<UFinalEnemyDefinition>(InAsset))
	{
		FinalDataAssetValidation::ValidateEnemyDefinition(InContext, bIsValid, Enemy);
		FinalDataAssetValidation::ValidateEnemyDefinitionProjectConsistency(InContext, bIsValid, Enemy, CurrentAssetPath, ProjectIndex);
	}
	else if (const UFinalEnemyIntentDefinition* Intent = Cast<UFinalEnemyIntentDefinition>(InAsset))
	{
		FinalDataAssetValidation::ValidateEnemyIntentDefinition(InContext, bIsValid, Intent);
		FinalDataAssetValidation::ValidateEnemyIntentDefinitionProjectConsistency(InContext, bIsValid, Intent, CurrentAssetPath, ProjectIndex);
	}
	else if (const UFinalBattleEncounterDefinition* Encounter = Cast<UFinalBattleEncounterDefinition>(InAsset))
	{
		FinalDataAssetValidation::ValidateEncounterDefinition(InContext, bIsValid, Encounter);
		FinalDataAssetValidation::ValidateEncounterDefinitionProjectConsistency(InContext, bIsValid, Encounter, CurrentAssetPath, ProjectIndex);
	}
	else if (const UFinalPassiveDefinition* Passive = Cast<UFinalPassiveDefinition>(InAsset))
	{
		FinalDataAssetValidation::ValidatePassiveDefinition(InContext, bIsValid, Passive);
		FinalDataAssetValidation::ValidatePassiveDefinitionProjectConsistency(InContext, bIsValid, Passive, CurrentAssetPath, ProjectIndex);
	}
	else if (const UFinalPrototypeBootstrapDefinition* PrototypeBootstrap = Cast<UFinalPrototypeBootstrapDefinition>(InAsset))
	{
		FinalDataAssetValidation::ValidatePrototypeBootstrapDefinition(InContext, bIsValid, PrototypeBootstrap);
		FinalDataAssetValidation::ValidatePrototypeBootstrapDefinitionProjectConsistency(InContext, bIsValid, PrototypeBootstrap, CurrentAssetPath, ProjectIndex);
	}
	else if (const UFinalRelicDefinition* Relic = Cast<UFinalRelicDefinition>(InAsset))
	{
		FinalDataAssetValidation::ValidateRelicDefinition(InContext, bIsValid, Relic);
		FinalDataAssetValidation::ValidateRelicDefinitionProjectConsistency(InContext, bIsValid, Relic, CurrentAssetPath, ProjectIndex);
	}
	else if (const UFinalRunRouteDefinition* RunRoute = Cast<UFinalRunRouteDefinition>(InAsset))
	{
		FinalDataAssetValidation::ValidateRunRouteDefinition(InContext, bIsValid, RunRoute, ProjectIndex);
		FinalDataAssetValidation::ValidateRunRouteDefinitionProjectConsistency(InContext, bIsValid, RunRoute, CurrentAssetPath, ProjectIndex);
	}
	else if (const UFinalBattleRuleConfig* RuleConfig = Cast<UFinalBattleRuleConfig>(InAsset))
	{
		FinalDataAssetValidation::ValidateRuleConfig(InContext, bIsValid, RuleConfig);
		FinalDataAssetValidation::ValidateRuleConfigProjectConsistency(InContext, bIsValid, RuleConfig, CurrentAssetPath, ProjectIndex);
	}
	else if (const UFinalStatusDefinition* Status = Cast<UFinalStatusDefinition>(InAsset))
	{
		FinalDataAssetValidation::ValidateStatusDefinition(InContext, bIsValid, Status);
		FinalDataAssetValidation::ValidateStatusDefinitionProjectConsistency(InContext, bIsValid, Status, CurrentAssetPath, ProjectIndex);
	}
	else if (const UFinalUltimateDefinition* Ultimate = Cast<UFinalUltimateDefinition>(InAsset))
	{
		FinalDataAssetValidation::ValidateUltimateDefinition(InContext, bIsValid, Ultimate);
		FinalDataAssetValidation::ValidateUltimateDefinitionProjectConsistency(InContext, bIsValid, Ultimate, CurrentAssetPath, ProjectIndex);
	}
	else
	{
		return EDataValidationResult::NotValidated;
	}

	return bIsValid ? EDataValidationResult::Valid : EDataValidationResult::Invalid;
}
