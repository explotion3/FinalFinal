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

## 2026-04-29 Step 7：成长初始化接入 DataAsset，并提供 starter 手工验收入口

- `PrototypeBootstrap` 当前已扩展初始成长状态：`Level / BreakthroughValue / BreakthroughRequiredValue / RootBone / Insight / KillingIntent`。
- `CharacterDefinition` 当前已可选指向 `CharacterGrowthConfig`；`CharacterGrowthConfig` 也已补默认突破阈值字段，用于 run 初始化时提供角色默认成长配置。
- `FinalGameInstance` 当前会在启动 prototype run 时合并 `InitialCharacterStates + CharacterGrowthConfig`，生成完整 `FFinalRunPersistentCharacterState`，不再只传压力相关字段。
- `UFinalRunSession::ConfigureBattleStartState()` 当前会在装配完角色与牌组后预热一次初始成长：若某角色初始突破值已达到阈值，则立刻生成一个 `PendingGrowthChoice`，但仍只保留同一时刻一个 pending choice。
- `FinalGameFlowSubsystem` 当前不会在存在 pending growth 时跳过成长直接自动开战；这一阶段曾用于验证“开局即成长选择”的最小链路。
- starter bundle 当前已补三份 `CharacterGrowthConfig`，并新增霍断岳 `断岳斩 -> 断岳斩·破阵` 的最小真实进化内容链，便于直接手工验收属性成长与卡牌进化两条路径。

## 2026-04-29 Step 8：BattleFact 驱动战中突破值增长、HUD 突破槽显示与安全窗口 Growth overlay

- `FinalBattle` 当前已新增结构化 `BattleGrowthFact` 协议，并在卡牌 / 奥义结算与战斗胜利时产出 `OwnedCardResolved / BreakDamageDealt / EffectiveHealingDone / EnemyKilled / BattleVictoryBaseReward`。
- `FinalGameFlowSubsystem` 当前负责消费 battle growth fact batches，按 `CharacterGrowthConfig` 把 facts 转成突破值，并统一调用 `RunSession.AddBreakthroughValue()`；`FinalBattle` 不直接升级角色。
- 当前第一版采用稳定型来源：只处理高信号事实，不把抽牌、受击、暴击等高频细碎行为直接转成突破值。
- 当前 Growth overlay 立即弹出规则已收口：只有玩家主动命令完整结算后首次满槽，才立即展示成长页；敌方阶段、被动链或战斗胜利导致的 pending growth 会延后到安全窗口。
- Battle HUD 当前已直接从 `RunSnapshot.Characters` 投影角色等级与突破槽，并在满槽时做高亮；HUD 不新增第二套成长真相缓存，也不把突破值写进 `BattleSnapshot`。
- `RunSession::ApplyBattleResult()` 当前已改为按 `CharacterId` 合并 battle-owned 字段，保留 `Level / BreakthroughValue / BreakthroughRequiredValue / RootBone / Insight / KillingIntent / bHasPendingGrowthChoice` 等 run-owned 成长状态。
- starter 首章默认已从“开局即 pending growth”调整为“霍断岳 80 / 100 接近突破进入首战”，用于手工验收战中突破、即时 Growth overlay 与卡牌进化路径。

## 2026-04-29 Step 9：属性成长正式进入 battle runtime，并支持战中即时刷新

- `FinalApp` 当前新增统一的成长数值投影 helper，用于把 `RunPersistentCharacterState + CharacterDefinition + CharacterGrowthConfig` 折算成 battle runtime 数值。
- `FinalBattleCharacterInitData` 当前已显式承载 `VitalShare / StressCap / RuntimeAttack / RuntimeDefense / RuntimeBreakRate / RuntimeCritChance / RuntimeCritDamage`，`FinalBattleInitializationService` 不再自行从角色静态定义重新拼这些字段。
- 当前首版属性成长投影规则已固定：
  - `RootBone -> VitalShare / StressCap / RuntimeDefense`
  - `KillingIntent -> RuntimeAttack / RuntimeCritChance / RuntimeCritDamage`
  - `Insight` 继续只影响突破值获取倍率
- `FinalBattle` 当前已把 crit 正式纳入权威结算：每个伤害 hit 独立判定暴击，暴击时按 `RuntimeCritDamage` 放大伤害。
- 当前在 active battle 中成功选择属性成长后，`FinalRunFlowSubsystem -> FinalGameFlowSubsystem -> FinalBattleFlowSubsystem` 会立即把最新 runtime 投影回刷到当前 battle session，而不是只等到下一场战斗。
- 本步骤仍然没有处理“战中卡牌进化刷新当前 hand / draw / discard / consume 实例”；这条链路仍留给后续单独收口。

## 2026-04-29 Step 10：战中卡牌进化刷新当前 battle card instance

- `FFinalBattleStartRequest` 当前已补显式 `DeckEntries`，至少承载 `SourceRunCardInstanceId / EffectiveCardId / OwnerCharacterId`，并继续兼容旧的 `DeckCardIds`。
- `FinalBattle` 当前已把 `SourceRunCardInstanceId` 写入直接来源的 `BattleCardInstance`，供 active battle 中按来源实例定位刷新。
- 选择卡牌进化候选成功后，`FinalRunFlowSubsystem -> FinalGameFlowSubsystem -> FinalBattleFlowSubsystem` 会立即桥接一次 active battle card refresh；`FinalBattle` 不直接读取 `RunState`。
- 当前首版刷新范围只覆盖直接来源于目标 `RunCardInstance` 的 battle 实例，因此 hand / draw / discard / consume / ongoing 中的同一实例都会原地更新。
- generated / temporary / copied cards 当前默认不联动，因为它们不保留来源 `RunCardInstanceId`。

## 2026-04-29 Step 11：BattleCard 临时修正 / 投影层

- `FinalBattle` 当前已把 `BattleCardInstance` 正式收成 `base definition + modifier records + projected runtime` 三层口径，不再把裸 runtime 字段当成唯一真相。
- battle 内每张卡当前都会基于 base `CardDefinition` 构建运行时定义副本，再按 `ApplyOrder` 顺序应用 modifier records，生成当前的 `ProjectedDefinition / RuntimeCostAP / RuntimeKeywords / RuntimeBehavior`。
- 当前首版统一支持以下卡牌临时修正持续窗口：`UntilPlayed / EndOfTurn / EndOfRound / EndOfBattle / ManualClear`。
- `RefreshCardsForRunCardInstance(...)` 当前不再直接重建一组 runtime 字段，而是更新 base `CurrentCardId / BaseDefinition` 后保留既有 `ModifierRecords` 并重新投影；因此战中进化已能保留临时减费、临时关键词和 `retain / consume / recycle` 覆盖。
- 运行时图 patch 当前仍只允许作用于卡牌自身运行时图：effect / requirement / condition 的替换、插入、移除和常见载荷覆写；静态 `UFinalCardDefinition` 不会被运行时修改。
- `generated / temporary / copied` cards 当前仍然不自动继承 `RunCardInstanceId` 来源语义，这条规则继续留给后续单独收口。
- 本步骤新增 `Final.Battle.CardProjection.*` 自动化测试，并保持 `Final.Editor.RunFlow.Growth` 里的进化刷新回归通过。

## 2026-04-29 Step 12：`锋锐` 接入 BattleCard modifier / projection

- `锋锐` 当前已从通用状态伤害修正路径迁移为“状态驱动的 BattleCard modifier”；玩法语义保持为“下一张攻击牌伤害提高 20%”。
- `StatusDefinition` 当前已补最小 card projection 字段：`bProjectToOwnedHandCards / ProjectedCardTypeFilter / ProjectedOutgoingDamagePercentPerStack`。
- `FinalBattle` 当前会根据角色身上的 `锋锐` 层数，把拥有者当前手牌中的攻击牌同步为 derived `BattleCard` modifiers，并在抽牌进手、生成牌进手、手牌移动、状态叠层/消耗/过期后立即重建同步。
- `锋锐` 当前只作用于手牌中的攻击牌；抽牌堆、弃牌堆、消耗区和持续区中的牌不投影这条状态。
- `士气` 继续保留通用状态伤害修正路径，因此当前 `士气 + 锋锐` 会正确叠加为“全局状态伤害加成 + 当前出牌卡局部加成”。
- 本步骤扩展了 `Final.Battle.CardProjection.*` 自动化测试，覆盖 `锋锐` 投影、与 `士气` 叠加、成功造成伤害后的层数消耗与重算、以及生成进手攻击牌的即时投影。

## 2026-04-29 Step 13：`阵门木签` 接入 relic-driven BattleCard modifier

- `阵门木签` 当前已从“第一次打出 0 AP 牌后抽 1”扩展为第一条正式的 `Relic -> RuntimeTriggers -> TriggeredCardModifiers -> BattleCard projection` 链。
- 共享 `FFinalRuntimeTriggerDefinition` 当前已补 `TriggeredCardModifiers`，首版支持在 trigger `Effects` 之后，基于 `DrawnCardsFromExecutedEffects` 目标来源把 follow-up modifier 挂到 battle card instance。
- `FFinalBattleEffectExecutionSummary` 当前已补 `DrawnCardInstanceIds`，用于把本次 trigger 实际抽到的牌实例稳定传给 follow-up modifier 逻辑，而不通过文本日志推断。
- `FinalBattleTriggerService` 当前会在 relic runtime trigger effects 执行成功后，直接按 effect summary 把 `阵门木签` 的 `-1 AP / +20% 伤害` modifier 投影到抽到的攻击牌；若该牌带 `SourceRunCardInstanceId`，还会同步投影到当前 battle 中全部同源实例。
- `FFinalBattleCardModifierRecord` 当前已补 `bExpireAtPlayerTurnEnd`；因此 `阵门木签` 的临时修正现在同时支持“打出后清除”与“若未打出则在玩家回合结束后清除”。
- 本步骤扩展了 `Final.Battle.CardProjection.*` 自动化测试，覆盖攻击牌抽到后投影、生成为非攻击牌时不投影、打出后清除、回合结束后清除这四条基础行为。

## 2026-04-29 Step 14：统一遗物固定效果与 `RuntimeTriggers` authoring

- `UFinalRelicDefinition` 当前已删除 `BattleStartEffects / PlayerTurnStartEffects`，遗物 battle 内 authoring 统一只保留 `RuntimeTriggers`。
- `EFinalRuntimeTriggerWindow` 当前已补 `BattleStart / PlayerTurnStart`，固定相位与条件触发都统一通过 `Window + Conditions + Effects + TriggeredCardModifiers` 表达。
- `FFinalBattleStartRelicInput` 当前只桥接 `RelicId / DisplayId / DisplayName / RuntimeTriggers`；Run 不再复制旧 fixed-effect payload。
- `FinalBattleRelicService` 当前不再自带 AP / Shield 的独立 switch 执行路径，而是在 `BattleStart / PlayerTurnStart` 两个窗口直接复用共享 runtime trigger 执行链。
- `FinalBattleTriggerService` 当前已能直接执行 `BattleStart / PlayerTurnStart` relic triggers，并继续产出统一的 `RelicTriggered` 事件。
- starter / test relic 当前已迁到新 schema：
  - `试作护符`：`BattleStart -> GainAP(1)`、`PlayerTurnStart -> GainAP(1)`
  - `试作修理包`：`BattleStart -> GainShield(4)`、`PlayerTurnStart -> GainShield(2)`
- `FinalRewardResolver`、`PrototypeRunDebugScreen` 与 `FinalDataAssetValidator` 当前也已统一改成从 `RuntimeTriggers` 汇总与校验，不再读取旧 fixed-effect 字段。

## 2026-04-29 Step 15：建立通用被动系统框架，并用能力卡牌验证 `ApplyPassive`

- `FinalData` 当前已新增正式 `PassiveDefinition`，并在 `FinalDataRegistry` 中接通 `FindPassiveDefinition()` 查询。
- `ApplyPassive` 当前已从 effect type 占位变成 battle 内可执行 effect；能力牌可以通过 `ApplyPassive(Self)` 正式创建 `BattlePassiveInstance`。
- `FinalBattle` 当前已建立正式被动真相：
  - `FFinalBattleState.PassiveInstances`
  - `FFinalBattlePassiveInstance`
  - `FinalBattlePassiveService`
- passive 首条验证链路当前分成两条：
  - 霍断岳 innate passive“受压得刀势”：共享生命受损后获得 1 层刀势
  - 霍断岳 starter 能力牌“受压蓄势”：打牌后授予 passive“压势追刀”，在每回合第一次打出攻击牌后获得 1 层刀势
- 本步骤已切断旧 `CharacterDefinition.BattleTriggers` 运行时链，角色自带被动正式改为 `InitialPassiveGrants -> PassiveDefinition -> BattlePassiveInstance`。
- 霍断岳当前同时具备两条正式被动链：
  - innate passive“受压得刀势”：共享生命受损后获得 1 层刀势
  - 能力牌“受压蓄势”授予 passive“压势追刀”：每回合第一次打出攻击牌后获得 1 层刀势
- battle snapshot / debug 查询当前已补最小 passive 视图，便于验证 innate passive 与能力牌赋予被动是否生效。
