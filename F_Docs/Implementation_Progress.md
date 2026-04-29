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
## 2026-04-29 Step 3：成长候选运行时结构

- 新增 `FFinalRunGrowthChoiceInstance`，用于保存一次角色升级成长候选的运行时快照。
- 新增 `FFinalRunPendingGrowthChoice`，用于在 `RunState` 中承载某个角色待处理的成长三选一。
- `FFinalRunState` 继续只保存一个当前待处理成长选择集，后续再由 `FinalRunSession` 生成和应用候选。
- 本步骤不生成候选、不应用属性成长、不应用卡牌进化，也不接 UI。
## 2026-04-29 Step 4：突破值累积、升级触发与成长候选生成

- `UFinalRunSession` 新增最小成长入口：`AddBreakthroughValue()`、`HasPendingGrowthChoice()` 与 `GetPendingGrowthChoice()`。
- 角色突破值达到阈值时只触发一次升级：扣除一次阈值、`Level +1`，并生成一个待处理的成长候选集。
- 当前同一时刻只允许一个 `PendingGrowthChoice`；若已有待处理候选，新的突破值仍会累计，但不会再次触发升级。
- 成长候选当前为 deterministic：
  - 默认生成 `RootBone +1`、`Insight +1`
  - 若 `RunDeck` 中存在符合 `CardEvolutionDefinition` 的卡牌实例，则第 3 个候选为卡牌进化
  - 否则第 3 个候选为 `KillingIntent +1`
- 本步骤仍然不应用候选效果、不修改 `RunDeck.CurrentCardId`、不接 UI / Command / Event。

## 2026-04-29 Step 5：ApplyGrowthChoice、清理 PendingGrowthChoice、正式落地属性与进化

- `FinalRunCommand` 新增 `SelectGrowthChoice`，使用 `PayloadId` 传入 `ChoiceInstanceId`。
- `UFinalRunSession` 新增 Step 5 编排：校验当前 pending choice、应用选中候选、清理 `PendingGrowthChoice`、并清除角色上的 `bHasPendingGrowthChoice`。
- 属性成长与卡牌进化都统一复用 `FinalGrowthResolver` 落地，避免 `FinalRunSession` 再长出第二套 mutation 逻辑。
- 选择属性候选时，当前只修改对应角色的 `RootBone / Insight / KillingIntent`。
- 选择进化候选时，当前只修改目标 `RunCardInstance.CurrentCardId / EvolutionStage`，并保持 `BaseCardId` 不变。
- 当前不会在应用候选后自动连锁触发下一次升级；若角色仍保留足够突破值，也需要后续再次显式调用成长入口才会生成下一组候选。
- `RunSnapshot` 当前已补 `PendingGrowthChoice` 公开查询面；`FFinalRunCharacterViewData` 也已补等级、突破值与三维成长属性，供外层直接读取成长结果。

## 2026-04-29 Step 6：`PendingGrowthChoice` 正式接入 `FinalApp` Run flow / UI

- `FinalRunFlowSubsystem` 新增 `SelectGrowthChoice()`，继续作为 `FinalApp` 内唯一的 Run 外层流程桥接入口。
- 外层页路由现在会优先检测 `RunSnapshot.PendingGrowthChoice`；若存在待处理成长，则优先展示独立 `FinalRunGrowthChoiceOverlayScreen`。
- `FinalUISubsystem` 当前已正式管理新的 Growth overlay screen，并通过 `FinalUIWidgetClassSettings` 暴露可替换的 `RunGrowthChoiceOverlayScreenClass`。
- `FinalRunGrowthChoiceOverlayScreen` 当前只消费 `RunSnapshot.PendingGrowthChoice / Characters` 与 `GrowthChoiceApplied` 反馈，不保存成长真相，也不直接触碰 `RunSession` 私有状态。
- `BattleHUD` 的 `RunFlowPromptPanel` 现在也会识别待处理成长，并通过 `RunFlowSubsystem.RefreshRunFlow(true)` 重新打开正确的外层页，而不再硬编码只打开普通 `RunFlowOverlay`。
