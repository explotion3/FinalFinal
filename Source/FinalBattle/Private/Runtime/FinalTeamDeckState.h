#pragma once

#include "CoreMinimal.h"

struct FFinalTeamDeckState
{
	TArray<FGuid> DrawPileCardInstanceIds;
	TArray<FGuid> HandCardInstanceIds;
	TArray<FGuid> DiscardPileCardInstanceIds;
	TArray<FGuid> OngoingZoneCardInstanceIds;
	TArray<FGuid> ConsumePileCardInstanceIds;
};
