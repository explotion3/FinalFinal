#include "Queries/FinalDataRegistry.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Battle/Definitions/FinalBattleEncounterDefinition.h"
#include "Battle/Definitions/FinalBattleRuleConfig.h"
#include "Battle/Definitions/FinalCardDefinition.h"
#include "Battle/Definitions/FinalCharacterDefinition.h"
#include "Battle/Definitions/FinalEnemyDefinition.h"
#include "Battle/Definitions/FinalEnemyIntentDefinition.h"
#include "Battle/Definitions/FinalStatusDefinition.h"
#include "Battle/Definitions/FinalUltimateDefinition.h"
#include "Modules/ModuleManager.h"
#include "Run/Definitions/FinalCardEvolutionDefinition.h"
#include "Run/Definitions/FinalCharacterGrowthConfig.h"
#include "Run/Definitions/FinalGrowthChoiceDefinition.h"
#include "Run/Definitions/FinalPrototypeBootstrapDefinition.h"
#include "Run/Definitions/FinalRelicDefinition.h"
#include "Run/Definitions/FinalRunRouteDefinition.h"

DEFINE_LOG_CATEGORY_STATIC(LogFinalDataRegistry, Log, All);

namespace
{
	struct FFinalDataRegistryIndexStats
	{
		int32 IndexedCount = 0;
		int32 MissingStableIdTagCount = 0;
	};

	void StripMatchingQuotes(FString& Value)
	{
		Value.TrimStartAndEndInline();
		if (Value.Len() >= 2
			&& ((Value[0] == TEXT('"') && Value[Value.Len() - 1] == TEXT('"'))
				|| (Value[0] == TEXT('\'') && Value[Value.Len() - 1] == TEXT('\''))))
		{
			Value = Value.Mid(1, Value.Len() - 2);
			Value.TrimStartAndEndInline();
		}
	}

	bool TryExtractStructValueTag(const FString& TagValue, FString& OutStableIdText)
	{
		const FString ValueToken = TEXT("Value=");
		int32 ValueIndex = INDEX_NONE;
		if (!TagValue.FindChar(TEXT('='), ValueIndex) || !TagValue.Contains(ValueToken))
		{
			return false;
		}

		ValueIndex = TagValue.Find(ValueToken, ESearchCase::IgnoreCase, ESearchDir::FromStart);
		if (ValueIndex == INDEX_NONE)
		{
			return false;
		}

		FString Remainder = TagValue.Mid(ValueIndex + ValueToken.Len());
		Remainder.TrimStartAndEndInline();
		if (Remainder.IsEmpty())
		{
			return false;
		}

		if (Remainder[0] == TEXT('"') || Remainder[0] == TEXT('\''))
		{
			const TCHAR QuoteChar = Remainder[0];
			int32 ClosingQuoteIndex = INDEX_NONE;
			for (int32 Index = 1; Index < Remainder.Len(); ++Index)
			{
				if (Remainder[Index] == QuoteChar)
				{
					ClosingQuoteIndex = Index;
					break;
				}
			}
			if (ClosingQuoteIndex == INDEX_NONE)
			{
				return false;
			}

			OutStableIdText = Remainder.Mid(1, ClosingQuoteIndex - 1);
			OutStableIdText.TrimStartAndEndInline();
			return !OutStableIdText.IsEmpty();
		}

		int32 EndIndex = Remainder.Len();
		for (int32 Index = 0; Index < Remainder.Len(); ++Index)
		{
			const TCHAR Character = Remainder[Index];
			if (Character == TEXT(')') || Character == TEXT(',') || FChar::IsWhitespace(Character))
			{
				EndIndex = Index;
				break;
			}
		}

		OutStableIdText = Remainder.Left(EndIndex);
		StripMatchingQuotes(OutStableIdText);
		return !OutStableIdText.IsEmpty();
	}

	bool TryParseStableIdText(const FString& RawStableIdText, FName& OutStableId)
	{
		FString StableIdText = RawStableIdText;
		StableIdText.TrimStartAndEndInline();
		if (StableIdText.IsEmpty())
		{
			return false;
		}

		FString ExtractedStructValue;
		if (TryExtractStructValueTag(StableIdText, ExtractedStructValue))
		{
			StableIdText = ExtractedStructValue;
		}
		else
		{
			StripMatchingQuotes(StableIdText);
		}

		if (StableIdText.IsEmpty() || StableIdText.Equals(TEXT("None"), ESearchCase::IgnoreCase))
		{
			return false;
		}

		OutStableId = FName(*StableIdText);
		return !OutStableId.IsNone();
	}

	bool TryReadStableIdTag(const FAssetData& AssetData, const FName StableIdPropertyName, FName& OutStableId)
	{
		FString StableIdTagValue;
		if (AssetData.GetTagValue(StableIdPropertyName, StableIdTagValue) && TryParseStableIdText(StableIdTagValue, OutStableId))
		{
			return true;
		}

		const FName NestedValueTagName(*FString::Printf(TEXT("%s.Value"), *StableIdPropertyName.ToString()));
		if (AssetData.GetTagValue(NestedValueTagName, StableIdTagValue) && TryParseStableIdText(StableIdTagValue, OutStableId))
		{
			return true;
		}

		return false;
	}

	template <typename TDefinition>
	FFinalDataRegistryIndexStats IndexDefinitionAssets(
		IAssetRegistry& AssetRegistry,
		const FName StableIdPropertyName,
		TMap<FName, FFinalDataRegistryAssetEntry>& OutDefinitionEntries,
		const TCHAR* DefinitionTypeName)
	{
		TArray<FAssetData> AssetDatas;
		AssetRegistry.GetAssetsByClass(TDefinition::StaticClass()->GetClassPathName(), AssetDatas, true);

		FFinalDataRegistryIndexStats Stats;
		for (const FAssetData& AssetData : AssetDatas)
		{
			FName StableId = NAME_None;
			if (!TryReadStableIdTag(AssetData, StableIdPropertyName, StableId))
			{
				++Stats.MissingStableIdTagCount;
				UE_LOG(
					LogFinalDataRegistry,
					Warning,
					TEXT("Skipped %s asset %s because AssetRegistry tag %s was missing or invalid. Resave the asset after AssetRegistrySearchable metadata changes."),
					DefinitionTypeName,
					*AssetData.GetSoftObjectPath().ToString(),
					*StableIdPropertyName.ToString());
				continue;
			}

			FFinalDataRegistryAssetEntry& Entry = OutDefinitionEntries.FindOrAdd(StableId);
			if (Entry.AssetPath.IsValid() && Entry.AssetPath != AssetData.ToSoftObjectPath())
			{
				UE_LOG(
					LogFinalDataRegistry,
					Warning,
					TEXT("Duplicate %s stable id %s. Replacing %s with %s."),
					DefinitionTypeName,
					*StableId.ToString(),
					*Entry.AssetPath.ToString(),
					*AssetData.ToSoftObjectPath().ToString());
			}

			Entry.AssetPath = AssetData.ToSoftObjectPath();
			Entry.LoadedAsset = nullptr;
			++Stats.IndexedCount;
		}

		return Stats;
	}
}

void UFinalDataRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CharacterDefinitions.Reset();
	CardDefinitions.Reset();
	EnemyDefinitions.Reset();
	EnemyIntentDefinitions.Reset();
	EncounterDefinitions.Reset();
	PrototypeBootstrapDefinitions.Reset();
	RelicDefinitions.Reset();
	RunRouteDefinitions.Reset();
	RuleConfigs.Reset();
	CharacterGrowthConfigs.Reset();
	GrowthChoiceDefinitions.Reset();
	CardEvolutionDefinitions.Reset();
	StatusDefinitions.Reset();
	UltimateDefinitions.Reset();

	DiscoverRuntimeDefinitions();
}

void UFinalDataRegistry::DiscoverRuntimeDefinitions()
{
	const double StartSeconds = FPlatformTime::Seconds();

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	AssetRegistry.WaitForCompletion();

	const FFinalDataRegistryIndexStats CharacterStats = IndexDefinitionAssets<UFinalCharacterDefinition>(AssetRegistry, TEXT("CharacterId"), CharacterDefinitions, TEXT("CharacterDefinition"));
	const FFinalDataRegistryIndexStats CardStats = IndexDefinitionAssets<UFinalCardDefinition>(AssetRegistry, TEXT("CardId"), CardDefinitions, TEXT("CardDefinition"));
	const FFinalDataRegistryIndexStats EnemyStats = IndexDefinitionAssets<UFinalEnemyDefinition>(AssetRegistry, TEXT("EnemyId"), EnemyDefinitions, TEXT("EnemyDefinition"));
	const FFinalDataRegistryIndexStats EnemyIntentStats = IndexDefinitionAssets<UFinalEnemyIntentDefinition>(AssetRegistry, TEXT("IntentId"), EnemyIntentDefinitions, TEXT("EnemyIntentDefinition"));
	const FFinalDataRegistryIndexStats EncounterStats = IndexDefinitionAssets<UFinalBattleEncounterDefinition>(AssetRegistry, TEXT("EncounterId"), EncounterDefinitions, TEXT("BattleEncounterDefinition"));
	const FFinalDataRegistryIndexStats PrototypeBootstrapStats = IndexDefinitionAssets<UFinalPrototypeBootstrapDefinition>(AssetRegistry, TEXT("BootstrapId"), PrototypeBootstrapDefinitions, TEXT("PrototypeBootstrapDefinition"));
	const FFinalDataRegistryIndexStats RelicStats = IndexDefinitionAssets<UFinalRelicDefinition>(AssetRegistry, TEXT("RelicId"), RelicDefinitions, TEXT("RelicDefinition"));
	const FFinalDataRegistryIndexStats RunRouteStats = IndexDefinitionAssets<UFinalRunRouteDefinition>(AssetRegistry, TEXT("RouteId"), RunRouteDefinitions, TEXT("RunRouteDefinition"));
	const FFinalDataRegistryIndexStats RuleConfigStats = IndexDefinitionAssets<UFinalBattleRuleConfig>(AssetRegistry, TEXT("RuleConfigId"), RuleConfigs, TEXT("BattleRuleConfig"));
	const FFinalDataRegistryIndexStats CharacterGrowthConfigStats = IndexDefinitionAssets<UFinalCharacterGrowthConfig>(AssetRegistry, TEXT("GrowthConfigId"), CharacterGrowthConfigs, TEXT("CharacterGrowthConfig"));
	const FFinalDataRegistryIndexStats GrowthChoiceStats = IndexDefinitionAssets<UFinalGrowthChoiceDefinition>(AssetRegistry, TEXT("GrowthChoiceId"), GrowthChoiceDefinitions, TEXT("GrowthChoiceDefinition"));
	const FFinalDataRegistryIndexStats CardEvolutionStats = IndexDefinitionAssets<UFinalCardEvolutionDefinition>(AssetRegistry, TEXT("EvolutionId"), CardEvolutionDefinitions, TEXT("CardEvolutionDefinition"));
	const FFinalDataRegistryIndexStats StatusStats = IndexDefinitionAssets<UFinalStatusDefinition>(AssetRegistry, TEXT("StatusId"), StatusDefinitions, TEXT("StatusDefinition"));
	const FFinalDataRegistryIndexStats UltimateStats = IndexDefinitionAssets<UFinalUltimateDefinition>(AssetRegistry, TEXT("UltimateId"), UltimateDefinitions, TEXT("UltimateDefinition"));

	const double ElapsedMilliseconds = (FPlatformTime::Seconds() - StartSeconds) * 1000.0;
	const int32 MissingTagCount =
		CharacterStats.MissingStableIdTagCount
		+ CardStats.MissingStableIdTagCount
		+ EnemyStats.MissingStableIdTagCount
		+ EnemyIntentStats.MissingStableIdTagCount
		+ EncounterStats.MissingStableIdTagCount
		+ PrototypeBootstrapStats.MissingStableIdTagCount
		+ RelicStats.MissingStableIdTagCount
		+ RunRouteStats.MissingStableIdTagCount
		+ RuleConfigStats.MissingStableIdTagCount
		+ CharacterGrowthConfigStats.MissingStableIdTagCount
		+ GrowthChoiceStats.MissingStableIdTagCount
		+ CardEvolutionStats.MissingStableIdTagCount
		+ StatusStats.MissingStableIdTagCount
		+ UltimateStats.MissingStableIdTagCount;

	UE_LOG(
		LogFinalDataRegistry,
		Log,
		TEXT("Indexed runtime definitions in %.2f ms: RuleConfigs=%d Characters=%d Cards=%d Ultimates=%d Enemies=%d EnemyIntents=%d Statuses=%d Encounters=%d PrototypeBootstraps=%d Relics=%d RunRoutes=%d CharacterGrowthConfigs=%d GrowthChoices=%d CardEvolutions=%d MissingStableIdTags=%d"),
		ElapsedMilliseconds,
		RuleConfigStats.IndexedCount,
		CharacterStats.IndexedCount,
		CardStats.IndexedCount,
		UltimateStats.IndexedCount,
		EnemyStats.IndexedCount,
		EnemyIntentStats.IndexedCount,
		StatusStats.IndexedCount,
		EncounterStats.IndexedCount,
		PrototypeBootstrapStats.IndexedCount,
		RelicStats.IndexedCount,
		RunRouteStats.IndexedCount,
		CharacterGrowthConfigStats.IndexedCount,
		GrowthChoiceStats.IndexedCount,
		CardEvolutionStats.IndexedCount,
		MissingTagCount);
}

template <typename TDefinition>
TDefinition* UFinalDataRegistry::FindLoadedDefinition(TMap<FName, FFinalDataRegistryAssetEntry>& DefinitionEntries, const FName StableId, const TCHAR* DefinitionTypeName)
{
	FFinalDataRegistryAssetEntry* Entry = DefinitionEntries.Find(StableId);
	if (!Entry)
	{
		UE_LOG(LogFinalDataRegistry, Verbose, TEXT("%s not found for id %s"), DefinitionTypeName, *StableId.ToString());
		return nullptr;
	}

	if (Entry->LoadedAsset)
	{
		TDefinition* LoadedDefinition = Cast<TDefinition>(Entry->LoadedAsset.Get());
		if (!LoadedDefinition)
		{
			UE_LOG(
				LogFinalDataRegistry,
				Warning,
				TEXT("Cached %s for id %s has unexpected type: %s"),
				DefinitionTypeName,
				*StableId.ToString(),
				*GetNameSafe(Entry->LoadedAsset.Get()));
		}
		return LoadedDefinition;
	}

	if (!Entry->AssetPath.IsValid())
	{
		UE_LOG(LogFinalDataRegistry, Warning, TEXT("%s id %s has no valid asset path."), DefinitionTypeName, *StableId.ToString());
		return nullptr;
	}

	UObject* LoadedObject = Entry->AssetPath.TryLoad();
	TDefinition* LoadedDefinition = Cast<TDefinition>(LoadedObject);
	if (!LoadedDefinition)
	{
		UE_LOG(
			LogFinalDataRegistry,
			Warning,
			TEXT("Failed to lazy-load %s id %s from %s."),
			DefinitionTypeName,
			*StableId.ToString(),
			*Entry->AssetPath.ToString());
		return nullptr;
	}

	Entry->LoadedAsset = LoadedDefinition;
	UE_LOG(LogFinalDataRegistry, Verbose, TEXT("Lazy-loaded %s id %s from %s."), DefinitionTypeName, *StableId.ToString(), *Entry->AssetPath.ToString());
	return LoadedDefinition;
}

template <typename TDefinition>
void UFinalDataRegistry::RegisterLoadedDefinition(TMap<FName, FFinalDataRegistryAssetEntry>& DefinitionEntries, const FName StableId, TDefinition* Definition)
{
	FFinalDataRegistryAssetEntry& Entry = DefinitionEntries.FindOrAdd(StableId);
	Entry.AssetPath = FSoftObjectPath(Definition);
	Entry.LoadedAsset = Definition;
}

void UFinalDataRegistry::RegisterCharacterDefinition(UFinalCharacterDefinition* Definition)
{
	if (!IsValid(Definition) || !Definition->CharacterId.IsValid())
	{
		return;
	}

	RegisterLoadedDefinition(CharacterDefinitions, Definition->CharacterId.Value, Definition);
}

void UFinalDataRegistry::RegisterCardDefinition(UFinalCardDefinition* Definition)
{
	if (!IsValid(Definition) || !Definition->CardId.IsValid())
	{
		return;
	}

	RegisterLoadedDefinition(CardDefinitions, Definition->CardId.Value, Definition);
}

void UFinalDataRegistry::RegisterEnemyDefinition(UFinalEnemyDefinition* Definition)
{
	if (!IsValid(Definition) || !Definition->EnemyId.IsValid())
	{
		return;
	}

	RegisterLoadedDefinition(EnemyDefinitions, Definition->EnemyId.Value, Definition);
}

void UFinalDataRegistry::RegisterEnemyIntentDefinition(UFinalEnemyIntentDefinition* Definition)
{
	if (!IsValid(Definition) || Definition->IntentId.IsNone())
	{
		return;
	}

	RegisterLoadedDefinition(EnemyIntentDefinitions, Definition->IntentId, Definition);
}

void UFinalDataRegistry::RegisterEncounterDefinition(UFinalBattleEncounterDefinition* Definition)
{
	if (!IsValid(Definition) || !Definition->EncounterId.IsValid())
	{
		return;
	}

	RegisterLoadedDefinition(EncounterDefinitions, Definition->EncounterId.Value, Definition);
}

void UFinalDataRegistry::RegisterPrototypeBootstrapDefinition(UFinalPrototypeBootstrapDefinition* Definition)
{
	if (!IsValid(Definition) || Definition->BootstrapId.IsNone())
	{
		return;
	}

	RegisterLoadedDefinition(PrototypeBootstrapDefinitions, Definition->BootstrapId, Definition);
}

void UFinalDataRegistry::RegisterRelicDefinition(UFinalRelicDefinition* Definition)
{
	if (!IsValid(Definition) || !Definition->RelicId.IsValid())
	{
		return;
	}

	RegisterLoadedDefinition(RelicDefinitions, Definition->RelicId.Value, Definition);
}

void UFinalDataRegistry::RegisterRunRouteDefinition(UFinalRunRouteDefinition* Definition)
{
	if (!IsValid(Definition) || Definition->RouteId.IsNone())
	{
		return;
	}

	RegisterLoadedDefinition(RunRouteDefinitions, Definition->RouteId, Definition);
}

void UFinalDataRegistry::RegisterRuleConfig(UFinalBattleRuleConfig* Definition)
{
	if (!IsValid(Definition) || !Definition->RuleConfigId.IsValid())
	{
		return;
	}

	RegisterLoadedDefinition(RuleConfigs, Definition->RuleConfigId.Value, Definition);
}

void UFinalDataRegistry::RegisterCharacterGrowthConfig(UFinalCharacterGrowthConfig* Definition)
{
	if (!IsValid(Definition) || !Definition->GrowthConfigId.IsValid())
	{
		return;
	}

	RegisterLoadedDefinition(CharacterGrowthConfigs, Definition->GrowthConfigId.Value, Definition);
}

void UFinalDataRegistry::RegisterGrowthChoiceDefinition(UFinalGrowthChoiceDefinition* Definition)
{
	if (!IsValid(Definition) || !Definition->GrowthChoiceId.IsValid())
	{
		return;
	}

	RegisterLoadedDefinition(GrowthChoiceDefinitions, Definition->GrowthChoiceId.Value, Definition);
}

void UFinalDataRegistry::RegisterCardEvolutionDefinition(UFinalCardEvolutionDefinition* Definition)
{
	if (!IsValid(Definition) || !Definition->EvolutionId.IsValid())
	{
		return;
	}

	RegisterLoadedDefinition(CardEvolutionDefinitions, Definition->EvolutionId.Value, Definition);
}

void UFinalDataRegistry::RegisterStatusDefinition(UFinalStatusDefinition* Definition)
{
	if (!IsValid(Definition) || !Definition->StatusId.IsValid())
	{
		return;
	}

	RegisterLoadedDefinition(StatusDefinitions, Definition->StatusId.Value, Definition);
}

void UFinalDataRegistry::RegisterUltimateDefinition(UFinalUltimateDefinition* Definition)
{
	if (!IsValid(Definition) || !Definition->UltimateId.IsValid())
	{
		return;
	}

	RegisterLoadedDefinition(UltimateDefinitions, Definition->UltimateId.Value, Definition);
}

UFinalCharacterDefinition* UFinalDataRegistry::FindCharacterDefinition(const FFinalCharacterId& CharacterId) const
{
	UFinalDataRegistry* MutableThis = const_cast<UFinalDataRegistry*>(this);
	return MutableThis->FindLoadedDefinition<UFinalCharacterDefinition>(MutableThis->CharacterDefinitions, CharacterId.Value, TEXT("CharacterDefinition"));
}

UFinalCardDefinition* UFinalDataRegistry::FindCardDefinition(const FFinalCardId& CardId) const
{
	UFinalDataRegistry* MutableThis = const_cast<UFinalDataRegistry*>(this);
	return MutableThis->FindLoadedDefinition<UFinalCardDefinition>(MutableThis->CardDefinitions, CardId.Value, TEXT("CardDefinition"));
}

UFinalEnemyDefinition* UFinalDataRegistry::FindEnemyDefinition(const FFinalEnemyId& EnemyId) const
{
	UFinalDataRegistry* MutableThis = const_cast<UFinalDataRegistry*>(this);
	return MutableThis->FindLoadedDefinition<UFinalEnemyDefinition>(MutableThis->EnemyDefinitions, EnemyId.Value, TEXT("EnemyDefinition"));
}

UFinalEnemyIntentDefinition* UFinalDataRegistry::FindEnemyIntentDefinition(const FName& IntentId) const
{
	UFinalDataRegistry* MutableThis = const_cast<UFinalDataRegistry*>(this);
	return MutableThis->FindLoadedDefinition<UFinalEnemyIntentDefinition>(MutableThis->EnemyIntentDefinitions, IntentId, TEXT("EnemyIntentDefinition"));
}

UFinalBattleEncounterDefinition* UFinalDataRegistry::FindEncounterDefinition(const FFinalEncounterId& EncounterId) const
{
	UFinalDataRegistry* MutableThis = const_cast<UFinalDataRegistry*>(this);
	return MutableThis->FindLoadedDefinition<UFinalBattleEncounterDefinition>(MutableThis->EncounterDefinitions, EncounterId.Value, TEXT("BattleEncounterDefinition"));
}

UFinalPrototypeBootstrapDefinition* UFinalDataRegistry::FindPrototypeBootstrapDefinition(const FName& BootstrapId) const
{
	UFinalDataRegistry* MutableThis = const_cast<UFinalDataRegistry*>(this);
	return MutableThis->FindLoadedDefinition<UFinalPrototypeBootstrapDefinition>(MutableThis->PrototypeBootstrapDefinitions, BootstrapId, TEXT("PrototypeBootstrapDefinition"));
}

UFinalRelicDefinition* UFinalDataRegistry::FindRelicDefinition(const FFinalRelicId& RelicId) const
{
	UFinalDataRegistry* MutableThis = const_cast<UFinalDataRegistry*>(this);
	return MutableThis->FindLoadedDefinition<UFinalRelicDefinition>(MutableThis->RelicDefinitions, RelicId.Value, TEXT("RelicDefinition"));
}

UFinalRunRouteDefinition* UFinalDataRegistry::FindRunRouteDefinition(const FName& RouteId) const
{
	UFinalDataRegistry* MutableThis = const_cast<UFinalDataRegistry*>(this);
	return MutableThis->FindLoadedDefinition<UFinalRunRouteDefinition>(MutableThis->RunRouteDefinitions, RouteId, TEXT("RunRouteDefinition"));
}

UFinalBattleRuleConfig* UFinalDataRegistry::FindRuleConfig(const FFinalRuleConfigId& RuleConfigId) const
{
	UFinalDataRegistry* MutableThis = const_cast<UFinalDataRegistry*>(this);
	return MutableThis->FindLoadedDefinition<UFinalBattleRuleConfig>(MutableThis->RuleConfigs, RuleConfigId.Value, TEXT("BattleRuleConfig"));
}

UFinalCharacterGrowthConfig* UFinalDataRegistry::FindCharacterGrowthConfig(const FFinalCharacterGrowthConfigId& GrowthConfigId) const
{
	UFinalDataRegistry* MutableThis = const_cast<UFinalDataRegistry*>(this);
	return MutableThis->FindLoadedDefinition<UFinalCharacterGrowthConfig>(MutableThis->CharacterGrowthConfigs, GrowthConfigId.Value, TEXT("CharacterGrowthConfig"));
}

UFinalGrowthChoiceDefinition* UFinalDataRegistry::FindGrowthChoiceDefinition(const FFinalGrowthChoiceId& GrowthChoiceId) const
{
	UFinalDataRegistry* MutableThis = const_cast<UFinalDataRegistry*>(this);
	return MutableThis->FindLoadedDefinition<UFinalGrowthChoiceDefinition>(MutableThis->GrowthChoiceDefinitions, GrowthChoiceId.Value, TEXT("GrowthChoiceDefinition"));
}

UFinalCardEvolutionDefinition* UFinalDataRegistry::FindCardEvolutionDefinition(const FFinalCardEvolutionId& EvolutionId) const
{
	UFinalDataRegistry* MutableThis = const_cast<UFinalDataRegistry*>(this);
	return MutableThis->FindLoadedDefinition<UFinalCardEvolutionDefinition>(MutableThis->CardEvolutionDefinitions, EvolutionId.Value, TEXT("CardEvolutionDefinition"));
}

UFinalStatusDefinition* UFinalDataRegistry::FindStatusDefinition(const FFinalStatusId& StatusId) const
{
	UFinalDataRegistry* MutableThis = const_cast<UFinalDataRegistry*>(this);
	return MutableThis->FindLoadedDefinition<UFinalStatusDefinition>(MutableThis->StatusDefinitions, StatusId.Value, TEXT("StatusDefinition"));
}

UFinalUltimateDefinition* UFinalDataRegistry::FindUltimateDefinition(const FFinalUltimateId& UltimateId) const
{
	UFinalDataRegistry* MutableThis = const_cast<UFinalDataRegistry*>(this);
	return MutableThis->FindLoadedDefinition<UFinalUltimateDefinition>(MutableThis->UltimateDefinitions, UltimateId.Value, TEXT("UltimateDefinition"));
}
