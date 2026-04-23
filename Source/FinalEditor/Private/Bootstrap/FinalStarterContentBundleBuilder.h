#pragma once

#include "CoreMinimal.h"

class UPackage;

struct FFinalStarterContentBundleBuilder
{
	static void Build(TSet<UPackage*>& PackagesToSave);
};
