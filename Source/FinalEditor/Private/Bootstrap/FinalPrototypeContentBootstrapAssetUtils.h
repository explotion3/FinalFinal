#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/PackageName.h"

class UObject;
class UPackage;

namespace FinalPrototypeContentBootstrap
{
	template<typename TAsset>
	TAsset* LoadOrCreateAsset(const FString& PackagePath, bool& bOutCreated)
	{
		const FString AssetName = FPackageName::GetLongPackageAssetName(PackagePath);
		const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackagePath, *AssetName);
		if (TAsset* LoadedAsset = Cast<TAsset>(StaticLoadObject(TAsset::StaticClass(), nullptr, *ObjectPath, nullptr, LOAD_NoWarn)))
		{
			bOutCreated = false;
			return LoadedAsset;
		}

		UPackage* Package = CreatePackage(*PackagePath);
		TAsset* Asset = NewObject<TAsset>(Package, TAsset::StaticClass(), *AssetName, RF_Public | RF_Standalone);
		FAssetRegistryModule::AssetCreated(Asset);
		bOutCreated = true;
		return Asset;
	}

	void TrackPackage(UObject* Asset, TSet<UPackage*>& OutPackages);
	void SavePackages(const TSet<UPackage*>& PackagesToSave);
}
