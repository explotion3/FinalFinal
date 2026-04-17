#include "Tags/FinalGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Final_Command_PlayCard, "Final.Command.PlayCard", "Battle command tag for playing a card.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Final_Command_PlayUltimate, "Final.Command.PlayUltimate", "Battle command tag for playing an ultimate.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Final_Command_EndTurn, "Final.Command.EndTurn", "Battle command tag for ending the player turn.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Final_Keyword_Fast, "Final.Keyword.Fast", "Card keyword that skips initiative reduction.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Final_Keyword_Retain, "Final.Keyword.Retain", "Card keyword that stays in hand at end of turn.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Final_Keyword_Expend, "Final.Keyword.Expend", "Card keyword that sends a card to the consume pile.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Final_Keyword_SwordArray, "Final.Keyword.SwordArray", "Derived sword-array card family tag.");

void FFinalGameplayTags::InitializeNativeTags()
{
}
