#include "Run/Bridge/FinalBattleRelicBridge.h"

namespace
{
TMap<FString, TArray<FFinalBattleStartRelicInput>> GPublishedBattleRelicPayloads;
}

void FFinalBattleRelicBridgeStore::PublishPayload(const FString& BridgeKey, TArray<FFinalBattleStartRelicInput> Payload)
{
	if (BridgeKey.IsEmpty())
	{
		return;
	}

	GPublishedBattleRelicPayloads.Add(BridgeKey, MoveTemp(Payload));
}

TArray<FFinalBattleStartRelicInput> FFinalBattleRelicBridgeStore::ConsumePayload(const FString& BridgeKey)
{
	if (BridgeKey.IsEmpty())
	{
		return {};
	}

	TArray<FFinalBattleStartRelicInput> Payload;
	if (TArray<FFinalBattleStartRelicInput>* FoundPayload = GPublishedBattleRelicPayloads.Find(BridgeKey))
	{
		Payload = MoveTemp(*FoundPayload);
		GPublishedBattleRelicPayloads.Remove(BridgeKey);
	}

	return Payload;
}
