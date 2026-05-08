#include "Validation/FinalDataValidationProjectIndex.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalEnemyDefinition.h"
#include "Battle/Definitions/FinalEnemyIntentDefinition.h"
#include "Battle/Definitions/FinalPassiveDefinition.h"
#include "Battle/Definitions/FinalStatusDefinition.h"
#include "Battle/Definitions/FinalUltimateDefinition.h"
#include "First/FirstCardDefinition.h"
#include "Run/Definitions/FinalPrototypeBootstrapDefinition.h"
#include "Modules/ModuleManager.h"
#include "Run/Definitions/FinalRelicDefinition.h"
#include "Run/Definitions/FinalRunRouteDefinition.h"

namespace FinalDataValidationProjectIndexInternal
{
	template<typename AssetType, typename IdResolverType>
	void CollectDefinitionIds(IAssetRegistry& AssetRegistry, TMap<FName, TArray<FString>>& OutPathsById, const IdResolverType& ResolveStableId)
	{
		TArray<FAssetData> AssetDataList;
		AssetRegistry.GetAssetsByClass(AssetType::StaticClass()->GetClassPathName(), AssetDataList, true);

		for (const FAssetData& AssetData : AssetDataList)
		{
			const AssetType* Asset = Cast<AssetType>(AssetData.GetAsset());
			if (Asset == nullptr)
			{
				continue;
			}

			const FName StableId = ResolveStableId(*Asset);
			if (StableId.IsNone())
			{
				continue;
			}

			OutPathsById.FindOrAdd(StableId).AddUnique(AssetData.GetSoftObjectPath().ToString());
		}
	}

	const TArray<FString>* FindPaths(const TMap<FName, TArray<FString>>& PathsById, const FName StableId)
	{
		return !StableId.IsNone() ? PathsById.Find(StableId) : nullptr;
	}

	bool HasStableId(const TMap<FName, TArray<FString>>& PathsById, const FName StableId)
	{
		const TArray<FString>* Paths = FindPaths(PathsById, StableId);
		return Paths != nullptr && Paths->Num() > 0;
	}

	TArray<FString> FindConflictingPaths(const TMap<FName, TArray<FString>>& PathsById, const FName StableId, const FString& CurrentAssetPath)
	{
		TArray<FString> ConflictingPaths;
		const TArray<FString>* Paths = FindPaths(PathsById, StableId);
		if (Paths == nullptr || Paths->Num() <= 1)
		{
			return ConflictingPaths;
		}

		for (const FString& AssetPath : *Paths)
		{
			if (CurrentAssetPath.IsEmpty() || AssetPath != CurrentAssetPath)
			{
				ConflictingPaths.Add(AssetPath);
			}
		}

		if (ConflictingPaths.Num() == 0)
		{
			ConflictingPaths = *Paths;
		}

		return ConflictingPaths;
	}
}

FFinalDataValidationProjectIndex FFinalDataValidationProjectIndex::Build()
{
	FFinalDataValidationProjectIndex ProjectIndex;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	AssetRegistry.WaitForCompletion();

	using namespace FinalDataValidationProjectIndexInternal;

	CollectDefinitionIds<UFinalCardDefinition>(AssetRegistry, ProjectIndex.CardDefinitionPathsById, [](const UFinalCardDefinition& Asset)
	{
		return Asset.CardId.Value;
	});

	CollectDefinitionIds<UFirstCardDefinition>(AssetRegistry, ProjectIndex.FirstCardDefinitionPathsById, [](const UFirstCardDefinition& Asset)
	{
		return Asset.CardId;
	});

	CollectDefinitionIds<UFinalCharacterDefinition>(AssetRegistry, ProjectIndex.CharacterDefinitionPathsById, [](const UFinalCharacterDefinition& Asset)
	{
		return Asset.CharacterId.Value;
	});

	CollectDefinitionIds<UFinalEnemyDefinition>(AssetRegistry, ProjectIndex.EnemyDefinitionPathsById, [](const UFinalEnemyDefinition& Asset)
	{
		return Asset.EnemyId.Value;
	});

	CollectDefinitionIds<UFinalEnemyIntentDefinition>(AssetRegistry, ProjectIndex.EnemyIntentDefinitionPathsById, [](const UFinalEnemyIntentDefinition& Asset)
	{
		return Asset.IntentId;
	});

	CollectDefinitionIds<UFinalBattleEncounterDefinition>(AssetRegistry, ProjectIndex.EncounterDefinitionPathsById, [](const UFinalBattleEncounterDefinition& Asset)
	{
		return Asset.EncounterId.Value;
	});

	CollectDefinitionIds<UFinalPassiveDefinition>(AssetRegistry, ProjectIndex.PassiveDefinitionPathsById, [](const UFinalPassiveDefinition& Asset)
	{
		return Asset.PassiveId.Value;
	});

	CollectDefinitionIds<UFinalPrototypeBootstrapDefinition>(AssetRegistry, ProjectIndex.PrototypeBootstrapDefinitionPathsById, [](const UFinalPrototypeBootstrapDefinition& Asset)
	{
		return Asset.BootstrapId;
	});

	CollectDefinitionIds<UFinalRelicDefinition>(AssetRegistry, ProjectIndex.RelicDefinitionPathsById, [](const UFinalRelicDefinition& Asset)
	{
		return Asset.RelicId.Value;
	});

	CollectDefinitionIds<UFinalRunRouteDefinition>(AssetRegistry, ProjectIndex.RunRouteDefinitionPathsById, [](const UFinalRunRouteDefinition& Asset)
	{
		return Asset.RouteId;
	});

	CollectDefinitionIds<UFinalStatusDefinition>(AssetRegistry, ProjectIndex.StatusDefinitionPathsById, [](const UFinalStatusDefinition& Asset)
	{
		return Asset.StatusId.Value;
	});

	CollectDefinitionIds<UFinalUltimateDefinition>(AssetRegistry, ProjectIndex.UltimateDefinitionPathsById, [](const UFinalUltimateDefinition& Asset)
	{
		return Asset.UltimateId.Value;
	});

	CollectDefinitionIds<UFinalBattleRuleConfig>(AssetRegistry, ProjectIndex.RuleConfigDefinitionPathsById, [](const UFinalBattleRuleConfig& Asset)
	{
		return Asset.RuleConfigId.Value;
	});

	return ProjectIndex;
}

TArray<FString> FFinalDataValidationProjectIndex::FindDuplicateCardDefinitionPaths(const FFinalCardId& CardId, const FString& CurrentAssetPath) const
{
	return FinalDataValidationProjectIndexInternal::FindConflictingPaths(CardDefinitionPathsById, CardId.Value, CurrentAssetPath);
}

TArray<FString> FFinalDataValidationProjectIndex::FindDuplicateFirstCardDefinitionPaths(const FName CardId, const FString& CurrentAssetPath) const
{
	return FinalDataValidationProjectIndexInternal::FindConflictingPaths(FirstCardDefinitionPathsById, CardId, CurrentAssetPath);
}

TArray<FString> FFinalDataValidationProjectIndex::FindDuplicateCharacterDefinitionPaths(const FFinalCharacterId& CharacterId, const FString& CurrentAssetPath) const
{
	return FinalDataValidationProjectIndexInternal::FindConflictingPaths(CharacterDefinitionPathsById, CharacterId.Value, CurrentAssetPath);
}

TArray<FString> FFinalDataValidationProjectIndex::FindDuplicateEnemyDefinitionPaths(const FFinalEnemyId& EnemyId, const FString& CurrentAssetPath) const
{
	return FinalDataValidationProjectIndexInternal::FindConflictingPaths(EnemyDefinitionPathsById, EnemyId.Value, CurrentAssetPath);
}

TArray<FString> FFinalDataValidationProjectIndex::FindDuplicateEnemyIntentDefinitionPaths(const FName IntentId, const FString& CurrentAssetPath) const
{
	return FinalDataValidationProjectIndexInternal::FindConflictingPaths(EnemyIntentDefinitionPathsById, IntentId, CurrentAssetPath);
}

TArray<FString> FFinalDataValidationProjectIndex::FindDuplicateEncounterDefinitionPaths(const FFinalEncounterId& EncounterId, const FString& CurrentAssetPath) const
{
	return FinalDataValidationProjectIndexInternal::FindConflictingPaths(EncounterDefinitionPathsById, EncounterId.Value, CurrentAssetPath);
}

TArray<FString> FFinalDataValidationProjectIndex::FindDuplicatePassiveDefinitionPaths(const FFinalPassiveId& PassiveId, const FString& CurrentAssetPath) const
{
	return FinalDataValidationProjectIndexInternal::FindConflictingPaths(PassiveDefinitionPathsById, PassiveId.Value, CurrentAssetPath);
}

TArray<FString> FFinalDataValidationProjectIndex::FindDuplicatePrototypeBootstrapDefinitionPaths(const FName BootstrapId, const FString& CurrentAssetPath) const
{
	return FinalDataValidationProjectIndexInternal::FindConflictingPaths(PrototypeBootstrapDefinitionPathsById, BootstrapId, CurrentAssetPath);
}

TArray<FString> FFinalDataValidationProjectIndex::FindDuplicateRelicDefinitionPaths(const FFinalRelicId& RelicId, const FString& CurrentAssetPath) const
{
	return FinalDataValidationProjectIndexInternal::FindConflictingPaths(RelicDefinitionPathsById, RelicId.Value, CurrentAssetPath);
}

TArray<FString> FFinalDataValidationProjectIndex::FindDuplicateRunRouteDefinitionPaths(const FName RouteId, const FString& CurrentAssetPath) const
{
	return FinalDataValidationProjectIndexInternal::FindConflictingPaths(RunRouteDefinitionPathsById, RouteId, CurrentAssetPath);
}

TArray<FString> FFinalDataValidationProjectIndex::FindDuplicateStatusDefinitionPaths(const FFinalStatusId& StatusId, const FString& CurrentAssetPath) const
{
	return FinalDataValidationProjectIndexInternal::FindConflictingPaths(StatusDefinitionPathsById, StatusId.Value, CurrentAssetPath);
}

TArray<FString> FFinalDataValidationProjectIndex::FindDuplicateUltimateDefinitionPaths(const FFinalUltimateId& UltimateId, const FString& CurrentAssetPath) const
{
	return FinalDataValidationProjectIndexInternal::FindConflictingPaths(UltimateDefinitionPathsById, UltimateId.Value, CurrentAssetPath);
}

TArray<FString> FFinalDataValidationProjectIndex::FindDuplicateRuleConfigDefinitionPaths(const FFinalRuleConfigId& RuleConfigId, const FString& CurrentAssetPath) const
{
	return FinalDataValidationProjectIndexInternal::FindConflictingPaths(RuleConfigDefinitionPathsById, RuleConfigId.Value, CurrentAssetPath);
}

bool FFinalDataValidationProjectIndex::HasCardDefinition(const FFinalCardId& CardId) const
{
	return FinalDataValidationProjectIndexInternal::HasStableId(CardDefinitionPathsById, CardId.Value);
}

bool FFinalDataValidationProjectIndex::HasFirstCardDefinition(const FName CardId) const
{
	return FinalDataValidationProjectIndexInternal::HasStableId(FirstCardDefinitionPathsById, CardId);
}

bool FFinalDataValidationProjectIndex::HasCharacterDefinition(const FFinalCharacterId& CharacterId) const
{
	return FinalDataValidationProjectIndexInternal::HasStableId(CharacterDefinitionPathsById, CharacterId.Value);
}

bool FFinalDataValidationProjectIndex::HasEncounterDefinition(const FFinalEncounterId& EncounterId) const
{
	return FinalDataValidationProjectIndexInternal::HasStableId(EncounterDefinitionPathsById, EncounterId.Value);
}

bool FFinalDataValidationProjectIndex::HasPassiveDefinition(const FFinalPassiveId& PassiveId) const
{
	return FinalDataValidationProjectIndexInternal::HasStableId(PassiveDefinitionPathsById, PassiveId.Value);
}

bool FFinalDataValidationProjectIndex::HasPrototypeBootstrapDefinition(const FName BootstrapId) const
{
	return FinalDataValidationProjectIndexInternal::HasStableId(PrototypeBootstrapDefinitionPathsById, BootstrapId);
}

bool FFinalDataValidationProjectIndex::HasRelicDefinition(const FFinalRelicId& RelicId) const
{
	return FinalDataValidationProjectIndexInternal::HasStableId(RelicDefinitionPathsById, RelicId.Value);
}

bool FFinalDataValidationProjectIndex::HasRuleConfigDefinition(const FFinalRuleConfigId& RuleConfigId) const
{
	return FinalDataValidationProjectIndexInternal::HasStableId(RuleConfigDefinitionPathsById, RuleConfigId.Value);
}

bool FFinalDataValidationProjectIndex::HasRunRouteDefinition(const FName RouteId) const
{
	return FinalDataValidationProjectIndexInternal::HasStableId(RunRouteDefinitionPathsById, RouteId);
}

bool FFinalDataValidationProjectIndex::HasUltimateDefinition(const FFinalUltimateId& UltimateId) const
{
	return FinalDataValidationProjectIndexInternal::HasStableId(UltimateDefinitionPathsById, UltimateId.Value);
}

bool FFinalDataValidationProjectIndex::HasStatusDefinition(const FFinalStatusId& StatusId) const
{
	return FinalDataValidationProjectIndexInternal::HasStableId(StatusDefinitionPathsById, StatusId.Value);
}
