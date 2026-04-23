#include "Bootstrap/FinalPrototypeContentBootstrapAssetUtils.h"

#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "UObject/SavePackage.h"

namespace FinalPrototypeContentBootstrap
{
	void TrackPackage(UObject* Asset, TSet<UPackage*>& OutPackages)
	{
		if (Asset == nullptr)
		{
			return;
		}

		if (UPackage* Package = Asset->GetOutermost())
		{
			Package->MarkPackageDirty();
			OutPackages.Add(Package);
		}
	}

	void SavePackages(const TSet<UPackage*>& PackagesToSave)
	{
		for (UPackage* Package : PackagesToSave)
		{
			if (Package == nullptr)
			{
				continue;
			}

			const FString PackageName = Package->GetName();
			const FString PackageFilename = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
			const FString FullPackageFilename = FPaths::ConvertRelativePathToFull(PackageFilename);
			IFileManager::Get().MakeDirectory(*FPaths::GetPath(FullPackageFilename), true);

			FSavePackageArgs SaveArgs;
			SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
			SaveArgs.SaveFlags = SAVE_NoError;

			UPackage::SavePackage(Package, nullptr, *FullPackageFilename, SaveArgs);
		}
	}
}
