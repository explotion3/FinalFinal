# Implementation Progress

## 2026-05-01：能力牌默认进入持续区修正

- `FinalBattle` 当前打出手牌后的默认去向已与牌区规则对齐：带 `消耗` 的牌进入消耗区，能力牌进入持续区，其余攻击牌 / 技能牌进入弃牌堆。
- 霍断岳能力牌“受压蓄势”打出后会留在持续区，不再进入弃牌堆参与后续重洗。

## 2026-05-01：手牌 AP 不足不可用表现 v0.1

- `FinalApp` 当前会在 hand entry view data 中根据 `CurrentAP / RuntimeCostAP` 标记 `bCanPlayHint` 与 `UnplayableHintText`；该字段只用于 UI 提示，不作为战斗规则真相。
- AP 不足的 Card Entry 当前保持可点击，费用显示 bad 色，并由 Hand Panel 统一应用较低透明度。
- 扇形手牌布局当前支持 AP 不足牌下沉表现；下沉距离按手牌位置动态插值，中间牌接近 `UnplayableDropMax`，两侧牌接近 `UnplayableDropMin`。
- AP 不足的牌当前仍可 hover 命中与点击，但不会触发 hover 抬升、放大或置顶表现。
- `UFinalBattleHandPanel` 当前暴露 `UnplayableDropMin / UnplayableDropMax / UnplayableOpacity`，用于在编辑器中调不可用表现。

## 2026-05-01：出牌目标需求投影 v0.1

- `FinalBattle` 当前会在构建 `FFinalBattleCardViewData` 时投影 `TargetRequirement`，首版只区分 `None / Enemy`。
- 只有包含 `SelectedEnemy` targeted effect 的手牌会投影为 `Enemy`；`Self / TeamPlayer / FirstAliveEnemy / AllEnemies / AllPlayerCharacters / None` 不要求 UI 传入具体目标。
- `FinalApp` 的出牌请求路径当前只在 `TargetRequirement = Enemy` 时解析当前/默认敌人目标；无目标牌可以直接提交 `PlayCard` 命令，由 `FinalBattle` 做最终合法性校验。
- 本轮不新增友方单体目标或完整目标选择 UI，后续若需要 `SelectedAlly` 再单独扩 `FinalData / FinalBattle` 目标规则。

## 2026-05-01：Card Effect Text Projection v0.1 正式落地

- `UFinalCardDefinition` 新增 `TextMode / TextLayoutLines / TextFragmentOverrides`；`RulesText` 保留为 manual/fallback/资产可读兜底。
- `FinalBattleCardService` 当前会在构建 `FFinalBattleCardViewData` 时生成 `ResolvedRulesText`，支持 `Damage / GainShield / Heal / BonusBreak / ApplyStatus / ConsumeStatusResource / DrawCards / GenerateCard / ApplyCardModifiers / GainAP` 的首版自动片段。
- `Damage` 文本只投影卡牌自身运行时 modifier：`RuntimeDamagePowerPercentPointDelta` 与 `RuntimeFinalDamagePercentDelta`；不合入士气、易伤、虚弱、暴击或目标防护等上下文。
- Starter 玩家普通即时牌与剑阵衍生牌已迁到 `EffectLayout`；霍断岳 `受压蓄势` 等 `ApplyPassive` 能力牌继续使用手写 `RulesText`。
- `FinalApp` Card Entry 优先显示 `ResolvedRulesText`，费用颜色按 `RuntimeCostAP - BaseCostAP` 判定：降低为 good，升高为 bad，不显示划掉原费用。
- `FinalDataAssetValidator` 会校验 EffectLayout token 是否引用真实 effect，并对 unsupported effect 缺 override 给 warning。
- 动态括号 hint（例如“手牌攻击牌每张 +20%（80%）”）已记录为后续 `{hint:...}` 扩展方向，本轮不实现。

## 2026-04-30：卡牌文本规范 v0.1 与 starter 文案收口

- `Card_Design_Guide.md` 当前已新增 `Card Text Style v0.1`，卡牌 `RulesText` 采用短语化、RichText 标签和每行一个效果的写法。
- Starter 玩家卡牌与剑阵衍生牌的 `RulesText` 已收口为 `<num> / <stat> / <status> / <keyword> / <cost> / <good> / <bad>` 首版标签格式。
- `FinalDataAssetValidator` 当前会对 starter card 中明显旧式文本片段给 warning；这轮不改变卡牌规则、费用、卡池或 BattleCard projection。

## 2026-04-30 Step 25：跨角色改卡，沈清弦技能牌即时强化友方攻击牌

- 新增直接 effect `ApplyCardModifiers`，普通卡牌现在可以不经过 passive / relic trigger，直接向 BattleCard instance 写入临时 modifier。
- 共享 card modifier target source 新增 `CurrentAllyHandCards`：只扫描来源角色之外的其他玩家角色当前手牌，不包含来源自己、敌人或 `team_player`。
- 新增沈清弦 starter 技能牌 `援锋成阵`（`card.starter.shen.yuanfengchengzhen`）：进入沈清弦卡池，不进入初始牌组；打出后让其他友方当前手牌攻击牌获得 `-1 AP / FinalDamagePercentDelta=20`，直到打出或玩家回合结束。
- 霍断岳 `压势追刀` 保持原有 passive-driven 自我改卡语义，不受本轮影响。

## 2026-04-30 Step 24：状态系统 legacy 字段清理与 authoring 收口

- `UFinalStatusDefinition` 与 `FFinalBattleStatusInstance` 上的 legacy 状态字段已删除；状态规则当前只通过正式 schema authoring。
- `FinalBattleStatusService` 已移除 legacy fallback：
  - 直接规则读取 `RuntimeModifiers`
  - 手牌投影读取 `ProjectedCardModifiers`
  - 资源消费读取 `ConsumeStatusResource`
  - DOT 读取 `DamageOverTime` 专用字段
- 新增状态级 `ConsumptionRules`，用于承载状态扣层窗口：
  - `锋锐`：`SuccessfulOwnerDamage + require attack + consume 1`
  - `生命免疫`：`PreventedTeamHealthDamage + consume 1`
- `DurationType / ExpireWindow / StackKeyPolicy / StackRule` 已进入 battle runtime 主路径；旧 `bExpireAtPlayerTurnEnd` 不再存在。

## 2026-04-30 Step 19A：状态系统第一步，只补 schema，不改玩法

- `UFinalStatusDefinition` 当前已补终版状态框架需要的结构化 schema：
  - `RuntimeModifiers`
  - `ProjectedCardModifiers`
  - `RuntimeTriggers`
  - `ConsumptionRules`
  - `StackKeyPolicy / StackRule / DurationType / ExpireWindow`
- 旧状态字段已在 Step 24 删除；本节记录的是当时的迁移起点。
- `FinalDataAssetValidator` 当时开始校验新 schema 的基础合法性；Step 24 后已改为正式 schema 校验。
- 本步骤不改变 `FinalBattleStatusService` 的 apply/remove/expire/query/projection 行为，因此 starter 实际玩法口径保持不变。
- 当时固定的完整状态系统迁移顺序为：
  - `士气`
  - `生命免疫`
  - `锋锐`
  - `刀势 / 药引`
  - `易伤 / 虚弱`
  - `中毒 DOT`
  - 最后删除旧字段与旧读取路径

## 2026-04-30 Step 19B：迁移 `士气 + 生命免疫` 到 `RuntimeModifiers`

- `士气` 与 `生命免疫` 当前已进入新 `RuntimeModifiers` schema 的正式 authoring 路径；starter builder 不再为这两条状态回填 direct-rule legacy 字段。
- `FFinalBattleStatusInstance` 当前已新增结构化 `RuntimeModifiers` 运行时载荷，用于承载 battle 直接规则修正，不与 card projection 混用。
- `FinalBattleStatusService` 当前已开始对“已迁移状态”读取结构化 runtime modifiers。
- 生命免疫的 team HP protection 后续已改为 `RuntimeModifiers + ConsumptionRules`。
- `DurationType / ExpireWindow / StackKeyPolicy / StackRule` 后续已进入 runtime 主路径。

## 2026-04-30 Step 19C：迁移 `锋锐` 到 `ProjectedCardModifiers`

- `锋锐` 当前已进入新 `ProjectedCardModifiers` schema 的正式 authoring 路径；starter builder 不再为这条状态回填 owned-hand projection legacy 字段。
- `FFinalBattleStatusInstance` 当前已新增结构化 `ProjectedCardModifiers` 运行时载荷，用于承载状态驱动的 `BattleCard` 投影修正。
- `FinalBattleStatusService.ResyncProjectedHandCardModifiers()` 当前已开始对“已迁移状态”读取结构化 projected-card modifiers：
  - `TargetSource = CurrentOwnedHandCards`
  - `RequiredCardType = Attack`
  - `DamagePowerPercentPointDeltaPerStack = 20`
- `锋锐` 的玩法语义保持不变：
  - 只作用于拥有者当前手牌中的攻击牌
  - `+20%` 按攻击力倍率点数增加结算，例如 `130% -> 150%`
  - 新抽进手或生成进手的攻击牌，在 `锋锐` 仍存在时也会立即获得投影
  - 成功造成伤害后仍消耗 1 层
  - 玩家回合结束仍会过期并清理投影
- 其他状态当前仍保留在各自正式路径：
  - `士气 / 生命免疫` 走 `RuntimeModifiers`
- `刀势 / 药引` 已归位为正式资源型状态
- `易伤 / 虚弱` 已迁入正式 `RuntimeModifiers`
- `中毒` 已走 DOT 专用 schema/runtime；`腐蚀 / 流血` 退出当前首章有效规则

## 2026-05-01：卡牌伤害修正语义拆分

- `BattleCard` 投影中的旧 `RuntimeOutgoingDamagePercent` 语义已拆为两条：
  - `RuntimeDamagePowerPercentPointDelta`：攻击力倍率点数增加，服务 `锋锐` 这类 `+20%` 语义。
  - `RuntimeFinalDamagePercentDelta`：最终伤害百分比修正，服务 `压势追刀 / 援锋成阵 / 阵门木签` 这类 `20%` 乘算语义。
- `ProjectedCardModifiers` 的状态侧字段同步拆为 `DamagePowerPercentPointDeltaPerStack / FinalDamagePercentDeltaPerStack`。
- `TriggeredCardModifiers / ApplyCardModifiers` 同步拆为 `DamagePowerPercentPointDelta / FinalDamagePercentDelta`。

## 2026-04-30 Step 20：迁移 `易伤 / 虚弱` 到正式 `RuntimeModifiers`

- `易伤 / 虚弱` 当前已从“可挂载、可展示的负面占位状态”迁到正式 `RuntimeModifiers` 路径。
- `FFinalStatusRuntimeModifierDefinition` 与 `FFinalBattleStatusRuntimeModifierInstance` 当前已新增 `IncomingDamagePercentPerStack`，用于承载目标侧受击伤害修正。
- `FinalBattleStatusService` 当前已补 `GetIncomingDamageModifierPercent()`，并把伤害结算顺序明确为：
  - 暴击等已有放大
  - 来源方 `OutgoingDamageModifierPercent`
  - 目标方 `IncomingDamageModifierPercent`
  - 护盾、共享生命、防护等后续处理
- `虚弱` 当前已正式影响目标造成的最终伤害；`易伤` 当前已正式影响目标受到的最终伤害。
- starter 敌人内容当前继续沿用 `易伤 / 虚弱` 的施加路径，但不再把它们当作纯文本占位；`山匪教头` 的“若目标处于易伤则伤害会被进一步放大”现在由正式受击修正自然成立。
- 当前唯一正式 DOT 为 `中毒`；`腐蚀 / 流血` 已从 starter 与当前规则文档中移除。

## 2026-04-30 Step 19D：把 `刀势 / 药引` 归位成正式资源型状态

- `刀势 / 药引` 当前已从“普通状态 + RemoveStatus 临时消费”迁到正式资源型状态框架。
- `UFinalStatusDefinition` 当前已补资源型 schema：
  - `bIsResourceStatus`
  - `ResourceBehavior`
  - `bAutoAffectBattleRules`
  - `bAutoProjectToCards`
- `FFinalBattleStatusInstance` 当前已正式承载资源型状态标记；`FinalBattleStatusService` 也已新增：
  - `CanConsumeStatusResource()`
  - `ConsumeStatusResource()`
- 资源型状态当前继续复用 `StatusInstance` 容器：
  - 获得：继续 `ApplyStatus`
  - 消费：正式改为 `ConsumeStatusResource`
- `刀势 / 药引` 当前默认不参与：
  - `RuntimeModifiers`
  - `ProjectedCardModifiers`
  - `RuntimeTriggers`
- starter 内容当前已迁移：
  - `断岳斩 / 断岳斩·破阵`：改为 `ConsumeStatusResource(刀势, 1) + ResourceConsumedCondition`
  - `化引 / 回春散`：改为 `ConsumeStatusResource(药引, 1) + ResourceConsumedCondition`
- 获得路径保持不变：
  - `行针 / 调息 / 裂锋 / 铁壁回锋 / 受压得刀势` 继续通过 `ApplyStatus` 增加 `刀势 / 药引`
- 当前状态系统第一批三条主干已明确：
  - `士气 / 生命免疫`：直接规则型
  - `锋锐`：卡牌投影型
  - `刀势 / 药引`：资源型

## 2026-04-30 Step 19E：清理状态系统无用 legacy 残留

- `UFinalStatusDefinition.OnTickEffects` 当前已从 schema 中删除；该字段在 battle runtime 中没有消费者，继续保留只会形成第二套无效扩展入口。
- `FinalDataAssetValidator` 当前不再校验 `OnTickEffects`。
- starter/test bundle 当前已移除所有 `OnTickEffects.Reset()` 写入，避免 bootstrap 继续维护无效字段。
- 对已完成迁移的 starter 状态，builder 当前已移除不会再被读取的 legacy 回填：
  - `生命免疫` 不再回填 direct-rule legacy 字段
  - `士气` 不再回填 direct-rule legacy 字段
  - `锋锐` 不再回填 owned-hand projection legacy 字段
- 后续 Step 24 已删除剩余 legacy 字段与 runtime fallback。

## 2026-04-30 Step 21：补 `AppliesTo` 状态适用域约束

- `UFinalStatusDefinition` 当前已新增 `AppliesTo` 定义级约束，取值为：
  - `Shared`
  - `PlayerOnly`
  - `EnemyOnly`
- `FinalBattleStatusService::AddStatusStacks()` 当前已成为状态适用域的唯一 runtime 硬约束点：
  - `team_player` 视为玩家侧单位
  - `PlayerOnly` 允许玩家角色与 `team_player`
  - `EnemyOnly` 只允许敌人单位
  - 非法归属会在施加入口被直接拒绝，不创建、不叠层
- starter 当前已把现用状态全部补齐 `AppliesTo`：
  - `士气 / 易伤 / 虚弱`：`Shared`
  - `生命免疫 / 锋锐 / 刀势 / 药引 / 阵诀`：`PlayerOnly`
- 本步骤不扩 DOT，不改变状态规则类别；`AppliesTo` 只约束状态拥有者归属。

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

- `锋锐` 当前已从通用状态伤害修正路径迁移为“状态驱动的 BattleCard modifier”；玩法语义保持为“下一张攻击牌攻击力倍率点数 +20%”。
- `StatusDefinition` 当时补了最小 card projection 字段；该路径后续已收口为 `ProjectedCardModifiers`。
- `FinalBattle` 当前会根据角色身上的 `锋锐` 层数，把拥有者当前手牌中的攻击牌同步为 derived `BattleCard` modifiers，并在抽牌进手、生成牌进手、手牌移动、状态叠层/消耗/过期后立即重建同步。
- `锋锐` 当前只作用于手牌中的攻击牌；抽牌堆、弃牌堆、消耗区和持续区中的牌不投影这条状态。
- `士气` 继续保留通用状态伤害修正路径，因此当前 `士气 + 锋锐` 会正确叠加为“全局状态伤害加成 + 当前出牌卡局部加成”。
- 本步骤扩展了 `Final.Battle.CardProjection.*` 自动化测试，覆盖 `锋锐` 投影、与 `士气` 叠加、成功造成伤害后的层数消耗与重算、以及生成进手攻击牌的即时投影。

## 2026-04-29 Step 13：`阵门木签` 接入 relic-driven BattleCard modifier

- `阵门木签` 当前已从“第一次打出 0 AP 牌后抽 1”扩展为第一条正式的 `Relic -> RuntimeTriggers -> TriggeredCardModifiers -> BattleCard projection` 链。
- 共享 `FFinalRuntimeTriggerDefinition` 当前已补 `TriggeredCardModifiers`，首版支持在 trigger `Effects` 之后，基于 `DrawnCardsFromExecutedEffects` 目标来源把 follow-up modifier 挂到 battle card instance。
- `FFinalBattleEffectExecutionSummary` 当前已补 `DrawnCardInstanceIds`，用于把本次 trigger 实际抽到的牌实例稳定传给 follow-up modifier 逻辑，而不通过文本日志推断。
- `FinalBattleTriggerService` 当前会在 relic runtime trigger effects 执行成功后，直接按 effect summary 把 `阵门木签` 的 `-1 AP / FinalDamagePercentDelta=20` modifier 投影到抽到的攻击牌；若该牌带 `SourceRunCardInstanceId`，还会同步投影到当前 battle 中全部同源实例。
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
  - 霍断岳 starter 能力牌“受压蓄势”：打牌后授予 passive“压势追刀”，在每回合第一次打出攻击牌后为当前手牌中的攻击牌投影 `-1 AP / FinalDamagePercentDelta=20`
- 本步骤已切断旧 `CharacterDefinition.BattleTriggers` 运行时链，角色自带被动正式改为 `InitialPassiveGrants -> PassiveDefinition -> BattlePassiveInstance`。
- 霍断岳当前同时具备两条正式被动链：
  - innate passive“受压得刀势”：共享生命受损后获得 1 层刀势
  - 能力牌“受压蓄势”授予 passive“压势追刀”：每回合第一次打出攻击牌后，为当前手牌中的攻击牌投影 `-1 AP / FinalDamagePercentDelta=20`，持续到打出或玩家回合结束
- battle snapshot / debug 查询当前已补最小 passive 视图，便于验证 innate passive 与能力牌赋予被动是否生效。
- `RuntimeTriggers -> TriggeredCardModifiers` 当前已新增 `CurrentOwnedHandCards` 目标来源，第一条正式 passive-driven card projection 已由 `压势追刀` 落地。

## 2026-04-30 Step 18：补齐“被动获得 / 触发 / 失效”的正式事件可见性

- `FinalBattleEvent` 当前已正式补齐三类被动事件：
  - `PassiveApplied`
  - `PassiveTriggered`
  - `PassiveRemoved`
- `BattleEvent` 当前已直接携带 `PassiveInstanceId / PassiveId / RelatedTag / ReasonTag`，并要求 `Message` 直接包含被动显示名，避免 HUD 或日志再做反查。
- `FinalBattlePassiveService` 当前仍只负责被动实例生命周期，但其 `ApplyPassive()` 与 `ResolvePlayerTurnEndPassives()` 现在会返回结构化 apply / removal 结果，供调用方统一发射 battle event。
- battle 初始化应用 `InitialPassiveGrants` 时，当前会发出 `PassiveApplied(initial_grant)`。
- `ApplyPassive` effect 成功创建或刷新被动时，当前会发出 `PassiveApplied(effect)`。
- passive 的 `RuntimeTriggers` 成功执行时，当前会发出 `PassiveTriggered`，并通过 `RelatedTag` 记录触发窗口，例如：
  - `battle.trigger.owner_took_health_damage`
  - `battle.trigger.player_card_resolved`
- `PlayerTurns` 类型 passive 在玩家回合结束自然到期时，当前会发出 `PassiveRemoved(expired)`。
- `FinalApp` 的 battle HUD 当前已直接消费这三类事件，并映射为：
  - `被动获得`
  - `被动触发`
  - `被动失效`
- 当前仍不新增正式被动 HUD 栏位；被动的可见性继续依赖：
  - battle event / battle log
  - battle snapshot / debug passive 列表

## 2026-04-30 Step 22：给被动系统补 `AppliesTo` 定义级约束

- `UFinalPassiveDefinition` 当前已新增 `AppliesTo` 定义级约束，取值为：
  - `Shared`
  - `PlayerOnly`
  - `EnemyOnly`
- `team_player` 当前明确归为玩家侧单位，因此 `PlayerOnly` 被动允许挂到玩家角色与 `team_player`，不允许挂到敌人单位。
- `FinalBattlePassiveService::ApplyPassive()` 当前已成为被动 owner-domain 的唯一 runtime 硬拦截点；不合法归属不会创建或刷新被动实例，也不会发出 `PassiveApplied`。
- starter 当前已把现用被动补齐 `AppliesTo`：
  - `受压得刀势`：`PlayerOnly`
  - `压势追刀`：`PlayerOnly`
- 本步骤不改变被动玩法，不改变 trigger 语义；`AppliesTo` 只约束被动拥有者归属。
