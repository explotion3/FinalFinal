#pragma once

#include "CoreMinimal.h"

class UPackage;

struct FFinalPrototypeTestBundleBuilder
{
	static void Build(TSet<UPackage*>& PackagesToSave);
};
