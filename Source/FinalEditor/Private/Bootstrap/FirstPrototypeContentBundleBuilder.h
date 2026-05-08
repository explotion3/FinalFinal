#pragma once

#include "CoreMinimal.h"

class UPackage;

struct FFirstPrototypeContentBundleBuilder
{
	static void Build(TSet<UPackage*>& PackagesToSave);
};
