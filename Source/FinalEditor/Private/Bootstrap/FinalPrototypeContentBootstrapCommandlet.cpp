#include "Bootstrap/FinalPrototypeContentBootstrapCommandlet.h"

#include "Bootstrap/FirstPrototypeContentBundleBuilder.h"
#include "Bootstrap/FinalPrototypeContentBootstrapAssetUtils.h"
#include "Bootstrap/FinalPrototypeTestBundleBuilder.h"
#include "Bootstrap/FinalStarterContentBundleBuilder.h"

DEFINE_LOG_CATEGORY_STATIC(LogFinalPrototypeContentBootstrap, Log, All);

UFinalPrototypeContentBootstrapCommandlet::UFinalPrototypeContentBootstrapCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
}

int32 UFinalPrototypeContentBootstrapCommandlet::Main(const FString& Params)
{
	TSet<UPackage*> PackagesToSave;

	FFinalPrototypeTestBundleBuilder::Build(PackagesToSave);
	FFinalStarterContentBundleBuilder::Build(PackagesToSave);
	FFirstPrototypeContentBundleBuilder::Build(PackagesToSave);

	FinalPrototypeContentBootstrap::SavePackages(PackagesToSave);

	UE_LOG(LogFinalPrototypeContentBootstrap, Display, TEXT("Prototype/starter content bootstrap completed. Saved %d packages."), PackagesToSave.Num());
	return 0;
}
