#include "Systems/FinalBattleStatusService.h"

#include "Queries/FinalBattleQueryTypes.h"
#include "Runtime/FinalBattleCharacterState.h"
#include "Runtime/FinalBattleState.h"
#include "Runtime/FinalBattleStatusInstance.h"

namespace
{
const FName TeamPlayerUnitId(TEXT("team_player"));
}

void FFinalBattleStatusService::TickStatusWindows(FFinalBattleState& BattleState) const
{
	// Status timing windows are still intentionally minimal at this stage.
	// The service exists to keep tick ownership out of the main resolver.
	(void)BattleState;
}

void FFinalBattleStatusService::BuildStatusSnapshotData(
	const FFinalBattleState& BattleState,
	TArray<FFinalBattleCharacterStatusesViewData>& OutCharacterStatuses,
	TArray<FFinalBattleStatusViewData>& OutTeamStatuses,
	TArray<FFinalBattleStatusViewData>& OutStatuses) const
{
	for (const FFinalBattleCharacterState& CharacterState : BattleState.Characters)
	{
		FFinalBattleCharacterStatusesViewData CharacterStatusEntry;
		CharacterStatusEntry.OwnerUnitId = CharacterState.RuntimeUnitId;
		CharacterStatusEntry.CharacterId = CharacterState.CharacterId;
		OutCharacterStatuses.Add(MoveTemp(CharacterStatusEntry));
	}

	for (const FFinalBattleStatusInstance& StatusInstance : BattleState.StatusInstances)
	{
		FFinalBattleStatusViewData StatusView;
		StatusView.StatusInstanceId = StatusInstance.StatusInstanceId;
		StatusView.StatusId = StatusInstance.StatusId;
		StatusView.OwnerUnitId = StatusInstance.OwnerUnitId;
		StatusView.SourceUnitId = StatusInstance.SourceUnitId;
		StatusView.DisplayName = FText::FromName(StatusInstance.StatusId.Value);
		StatusView.CurrentStacks = StatusInstance.CurrentStacks;
		StatusView.RemainingDuration = StatusInstance.RemainingDuration;

		if (StatusInstance.OwnerUnitId == TeamPlayerUnitId)
		{
			OutTeamStatuses.Add(StatusView);
		}
		else if (FFinalBattleCharacterStatusesViewData* CharacterStatuses = OutCharacterStatuses.FindByPredicate(
			[&StatusInstance](const FFinalBattleCharacterStatusesViewData& Candidate)
			{
				return Candidate.OwnerUnitId == StatusInstance.OwnerUnitId;
			}))
		{
			CharacterStatuses->StatusEntries.Add(StatusView);
		}

		OutStatuses.Add(MoveTemp(StatusView));
	}
}
