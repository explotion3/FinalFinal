# Implementation Progress

## 2026-04-29 - FinalRun RunDeck instance migration step 1

- `FFinalRunState.RunDeck` is now the single run deck truth source and stores `FFinalRunCardInstance` entries instead of raw `FFinalCardId` values.
- Each run deck card keeps `BaseCardId`, `CurrentCardId`, `OwnerCharacterId`, `EvolutionStage`, and `TimesPlayedThisRun`.
- Battle start remains compatible with the current battle layer: `BuildBattleStartRequest()` still outputs `DeckCardIds`, derived from each card instance's `CurrentCardId`.
- Existing reward deck edits now operate on card instances:
  - card grant creates a new run card instance;
  - remove-card removes the first matching effective card id;
  - upgrade-card updates `CurrentCardId` on the matching instance.
- No growth-choice generation, attribute growth, UI, battle fact integration, or hand refresh logic is implemented in this step.
## Step 2：角色成长状态

- `FFinalRunPersistentCharacterState` 增加角色等级、突破值、突破阈值、根骨、悟性、杀意与待成长标记。
- `FFinalRunState` 增加轻量的 `PendingGrowthChoice`，用于记录当前是否有角色等待成长选择。
- 本步骤只落状态结构，不生成成长三选一，不应用属性成长，不执行卡牌进化，不接 UI。
- `RunDeck` 仍是 Run 内牌组唯一真相源；卡牌实例迁移与本步骤保持解耦。
