#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Final_Command_PlayCard);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Final_Command_PlayUltimate);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Final_Command_EndTurn);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Final_Keyword_Fast);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Final_Keyword_Retain);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Final_Keyword_Expend);

struct FINALCORE_API FFinalGameplayTags
{
	static void InitializeNativeTags();
};
