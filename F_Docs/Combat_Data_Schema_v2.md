# 战斗数据字段规范 2.0

## 1. 文档定位

本文档定义战斗、Run、卡牌实例、角色成长与内容配置之间的数据边界。

与其他文档的分工：

- `GDD4.0.md`：描述完整玩法目标与世界观级系统。
- `Power_Fantasy_Direction.md`：描述当前爽游化转向与第一版范围。
- `Battle_Rules.md`：描述战斗术语、结算顺序与判定边界。
- `Card_Design_Guide.md`：描述卡牌内容设计规范。
- `Code_Function_Requirements.md`：描述模块职责与实现边界。
- 本文档：描述数据层如何承载上述系统。

本文档优先服务第一版竖切：角色升级成长三选一、卡牌实例进化、压力临界状态机。

## 2. 数据设计原则

### 2.1 三层分离

数据按三层拆分：

```text
Definition
静态定义。描述模板、配置、规则文本与展示字段。

RunPersistentState
Run 内持久状态。描述跨战斗保留的角色成长、牌组实例、卡牌进化状态。

BattleState
战斗内运行时状态。描述当前战斗中的 AP、EP、压力状态、牌区、临时修正。
```

核心原则：

- 静态模板不记录战斗临时变化。
- 战斗实例不回写静态模板。
- Run 内成长不混入单场 `BattleState`。
- 卡牌成长作用于 `RunCardInstance`，不直接修改 `CardDefinition`。
- 战斗开始时由 Run 层实例生成 Battle 层实例。

### 2.2 第一版优先级

第一版必须承载：

- 角色等级与突破值。
- 根骨、悟性、杀意。
- 角色升级成长三选一。
- 卡牌实例 `BaseCardId / CurrentCardId`。
- 卡牌进化后当前手牌即时刷新。
- 压力 `Normal / Critical / Collapse` 状态机。

第一版暂不要求完整承载：

- 强化珠即时镶嵌与长期管理。
- 完整绝学化树。
- 角色专属临界收益。
- 珠子套装、洗词条、合成树。
- 复杂技能树与门派 / 经脉系统。

## 3. 命名规范

### 3.1 基础规则

- 结构体名使用 `F` 前缀。
- 枚举名使用 `E` 前缀。
- 字段名使用 PascalCase。
- 布尔字段使用 `b` 前缀。
- 显示名与系统 ID 分离。
- 系统 ID 使用稳定英文小写前缀。

### 3.2 推荐 ID 前缀

| 类型 | 前缀 |
|---|---|
| 角色 | `char_` |
| 卡牌 | `card_` |
| 卡牌进化 | `evo_` |
| 成长候选 | `growth_` |
| 敌人 | `enemy_` |
| 状态 | `status_` |
| 被动 | `passive_` |
| 遗物 | `relic_` |
| 事件 | `event_` |
| 遭遇 | `enc_` |
| 奥义 | `ult_` |
| 强化珠 | `gem_` |

### 3.3 卡牌命名口径

- 静态卡牌所属字段统一使用 `OwnerUnitId`。
- Run 内卡牌实例语义上使用 `RunCardInstanceId`，当前首版运行时字段名为 `InstanceId`。
- 战斗内卡牌实例使用 `BattleCardInstanceId`。
- 运行时单位句柄统一使用 `RuntimeUnitId`。
- 运行时卡牌所属字段统一使用 `RuntimeOwnerUnitId`。
- 卡牌类型字段统一使用 `CardType`。
- 卡牌稀有度字段统一使用 `Rarity`。
- 关键词字段统一使用 `Keywords`。
- 文本字段统一使用 `RulesText`。

不再使用：

- `OwnerCharacterId` 作为卡牌静态归属字段。
- `bExhaust`。
- `Advanced` 作为稀有度值。
- 同义重复字段并存，例如 `bRetain / RetainKeyword`。

补充说明：

- 当前首章 roster 只包含玩家角色，因此 `RunCardInstance` 运行时字段暂用 `OwnerCharacterId`。
- 若后续需要让同一套 `RunCardInstance` 挂到非角色单位，再统一提升为更通用的 `OwnerUnitId`。

## 4. 共享协议与核心枚举

### 4.1 UnitReference

用于统一“所属单位 / 来源单位 / 目标单位”的引用方式。

建议口径：

- 模板单位：`char_*`、`enemy_*`。
- 运行时单位：`unit_player_*`、`unit_enemy_*`。
- 队伍保留值：`team_player`、`team_enemy`。

运行时字段不得直接引用 `char_* / enemy_*` 这类模板 ID。

### 4.2 CardType

```text
Attack
Skill
Ability
```

后续如果加入奥义牌、临界牌、状态牌，应先在设计文档中确认边界，再加入枚举。

### 4.3 Rarity

```text
Common
Rare
Epic
Legendary
```

文档口径对应：

```text
普通 / 稀有 / 罕见 / 传说
```

### 4.4 CardZone

```text
Hand
DrawPile
DiscardPile
OngoingZone
ConsumePile
```

首版不单独维护 `Graveyard`。消耗、消灭、移出战斗统一进入 `ConsumePile`。

### 4.5 StressState

```text
Normal
Critical
Collapse
```

说明：

- `Normal`：当前压力低于临界阈值。
- `Critical`：当前压力达到或超过临界阈值，但尚未崩溃。
- `Collapse`：临界期间继续受压或规则判定失败后的失控状态。

### 4.6 GrowthChoiceType

```text
AttributeGrowth
CardEvolution
Special
```

第一版只要求实现：

```text
AttributeGrowth
CardEvolution
```

### 4.7 GrowthAttributeType

```text
RootBone
Insight
KillingIntent
```

文档显示口径：

```text
根骨 / 悟性 / 杀意
```

### 4.8 EvolutionStage

```text
Base
Evolved
Mastered
```

说明：

- `Base`：基础卡。
- `Evolved`：进化卡。
- `Mastered`：绝学化卡。第一版可只预留，不要求完整内容。

### 4.9 CardEvolutionType

```text
ImmediatePower
GrowthPotential
ArchetypeShift
```

第一版必须支持 `ImmediatePower`。  
其余类型先作为扩展口径保留，不要求完整内容池。

### 4.10 GemSlotLevel 与 GemTier

强化珠系统不进入第一版完整实现，但数据口径先保留。

```text
GemSlotLevel: 1 / 2 / 3 / 4
GemTier:      1 / 2 / 3 / 4
```

原则：

- 一个强化槽只能放一个强化珠。
- 槽级限制珠阶。
- 珠级提升不改变珠阶。
- 第一版不要求实现强化珠背包、拆卸、合成树。

### 4.11 BattleGrowthFact Bridge

用于承载 `FinalBattle -> FinalApp -> FinalRun` 的突破值桥接协议。

当前首版事实类型：

```text
OwnedCardResolved
BreakDamageDealt
EffectiveHealingDone
EnemyKilled
BattleVictoryBaseReward
```

核心字段：

| 字段 | 说明 |
|---|---|
| `CharacterId` | 触发该事实的角色 |
| `FactType` | 事实类型 |
| `Magnitude` | 强度或数量 |
| `SourceCardId` | 来源卡牌，可为空 |
| `Round` | 发生回合 |
| `bCausedByPlayerCommand` | 是否由玩家主动命令直接导致 |
| `SourceBattleCommandType` | 来源命令类型或窗口 |

说明：

- 这是薄协议，不是第二套成长真相。
- `FinalBattle` 只产出 facts，不直接给角色加突破值。
- `FinalApp` 负责消费 facts 并调用 `FinalRun::AddBreakthroughValue()`。
- `FinalRun` 仍是突破值、升级与成长候选的唯一真相源。
- 当前 `CharacterGrowthConfig` 还同时提供首版属性投影参数：
  - `RootBoneVitalSharePerPoint`
  - `RootBoneStressCapPerPoint`
  - `RootBoneDefensePerPoint`
  - `KillingIntentAttackPerPoint`
  - `KillingIntentCritChancePerPoint`
  - `KillingIntentCritDamagePerPoint`
  - `InsightBreakthroughGainMultiplierPerPoint`
- 当前 battle runtime 投影统一由 `FinalApp` 侧 helper 计算，并用于：
  - 新 battle 初始化
  - active battle 中属性成长后的即时刷新

## 5. 静态定义

### 5.1 BattleRuleConfig

用途：描述战斗基础规则配置。

核心字段：

| 字段 | 说明 |
|---|---|
| `RuleConfigId` | 规则配置 ID |
| `BaseAPPerTurn` | 每回合基础 AP |
| `BaseEP` | 初始 EP |
| `CriticalThresholdRatio` | 临界阈值比例，默认 `0.9` |
| `bClampStressOnEnterCritical` | 首次越过临界时是否钳制压力到崩溃阈值 |
| `BreakActionCancelRule` | Break 后敌方行动取消规则 |
| `MaxHandSize` | 手牌上限 |

说明：

- `CriticalThreshold = ceil(StressCap * CriticalThresholdRatio)`。
- `CollapseThreshold = StressCap`。
- 压力状态机在 `FinalBattle` 中结算。

### 5.2 CharacterDefinition

用途：描述角色静态模板。

核心字段：

| 字段 | 说明 |
|---|---|
| `CharacterId` | 角色 ID |
| `DisplayName` | 显示名 |
| `BaseVitalShare` | 基础生命份额 |
| `BaseAttack` | 基础攻击 |
| `BaseDefense` | 基础防御 |
| `BaseStressCap` | 基础压力上限 |
| `BaseBreakRate` | 基础削韧修正 |
| `StarterCardIds` | 起始牌列表 |
| `UltimateId` | 奥义 ID |
| `RoleTags` | 角色定位标签 |
| `ContentTags` | 内容主题标签 |
| `GrowthConfigId` | 角色默认成长配置，可为空 |
| `InitialPassiveGrants` | 角色开战时挂载的初始被动列表 |

第一版角色成长属性不直接写进 `CharacterDefinition`，而是写入 `RunPersistentCharacterState`。  
当前 `CharacterDefinition` 只允许通过 `GrowthConfigId` 指向角色默认成长配置，用于提供诸如默认突破阈值这类“角色成长口径”，不承担本局初始成长状态真相。

补充说明：

- 当前 battle 内已经建立正式 `PassiveDefinition / BattlePassiveInstance` 框架。
- 角色自带被动的正式 authoring 入口为 `CharacterDefinition.InitialPassiveGrants`。
- 新的“能力牌赋予被动”链路统一通过 `ApplyPassive -> PassiveDefinition -> BattlePassiveInstance -> RuntimeTriggers` 落地。

### 5.3 CardDefinition

用途：描述卡牌静态模板。

核心字段：

| 字段 | 说明 |
|---|---|
| `CardId` | 卡牌模板 ID |
| `DisplayName` | 卡牌显示名 |
| `OwnerUnitId` | 静态所属单位，可为空 |
| `CardType` | 卡牌类型 |
| `Rarity` | 稀有度 |
| `BaseCostAP` | 基础 AP 消耗 |
| `BaseCostEP` | 基础 EP 消耗 |
| `TargetType` | 玩家交互目标类型 |
| `Keywords` | 规则关键词 |
| `CardContentTags` | 构筑主题标签 |
| `CardAccessTags` | 获取限制标签 |
| `Effects` | 战斗效果列表 |
| `RulesText` | 规则文本 |
| `EvolutionGroupId` | 进化组 ID，可为空 |
| `DefaultGemSlots` | 默认强化槽结构，可为空 |

说明：

- `CardDefinition` 是模板，不保存强化、进化、珠子镶嵌等 Run 内结果。
- 进化卡和绝学卡仍然可以是独立 `CardDefinition`。
- `RunCardInstance.CurrentCardId` 决定这一张实例当前使用哪个模板。

### 5.4 BattleEffectDefinition

用途：描述卡牌、敌人、状态、遗物在战斗内产生的具体效果。

核心字段：

| 字段 | 说明 |
|---|---|
| `EffectId` | 效果 ID |
| `EffectType` | 效果类型 |
| `TargetRule` | 实际结算目标 |
| `Value` | 基础数值 |
| `ScaleMode` | 缩放方式 |
| `SourceStat` | 引用来源属性 |
| `Conditions` | 条件列表 |
| `TriggerCondition` | 触发时点 |

常见 `EffectType`：

```text
Damage
GainShield
Heal
ApplyStatus
RemoveStatus
ConsumeStatusResource
DrawCards
GainAP
GainEP
BonusBreak
GenerateCard
ApplyCardModifiers
CopyCard
ApplyPassive
```

`ApplyPassive` 当前已成为 battle 内正式 effect 类型；首版用于把能力牌效果转成 `BattlePassiveInstance`，再由被动自己的 `RuntimeTriggers` 继续驱动后续效果。

`ConsumeStatusResource` 当前已成为资源型状态的正式消费 effect；首版用于 `刀势 / 药引` 这类 `Signature Resource` 状态的显式扣减，不再把通用 `RemoveStatus` 作为长期资源消费协议。

`ApplyCardModifiers` 用于卡牌效果即时向 battle hand card instance 挂临时 modifier。当前用于沈清弦 `援锋成阵`，目标来源为 `CurrentAllyHandCards`，只影响其他友方当前手牌中的攻击牌，不追踪后续新抽入手的牌。

### 5.4 PassiveDefinition

用途：描述一个 battle 内正式被动模板。

核心字段：

| 字段 | 说明 |
|---|---|
| `PassiveId` | 被动模板 ID |
| `DisplayId` | 调试 / 展示稳定 ID |
| `DisplayName` | 被动显示名 |
| `SummaryText` | 简述文本 |
| `StackPolicy` | 叠层或刷新策略 |
| `DurationType` | 持续类型 |
| `MaxStacks` | 最大层数 |
| `AppliesTo` | 被动定义级拥有者约束。`Shared` 允许玩家与敌人单位拥有；`PlayerOnly` 允许玩家角色与 `team_player`；`EnemyOnly` 只允许敌人单位 |
| `RuntimeTriggers` | 被动触发定义列表 |

说明：

- 被动本身不发明第三套 trigger schema，直接复用 `RuntimeTriggerDefinition`。
- `AppliesTo` 当前只约束被动拥有者归属，不改变 trigger / effect 语义。battle runtime 会在 `ApplyPassive()` 入口硬拦截不合法归属。
- 当前 starter 已有两条正式被动链：
  - 霍断岳 innate passive：`OwnerTookHealthDamage -> ApplyStatus(刀势 +1)`
  - 霍断岳能力牌“受压蓄势”：`ApplyPassive(Self)`，授予 `PlayerCardResolved + ResolvedCard(Attack) + OncePerPlayerTurn -> TriggeredCardModifiers(CurrentOwnedHandCards, Attack, -1 AP, +20% damage, UntilPlayed + EndOfTurn cleanup)`。
- `CharacterDefinition.BattleTriggers` 已从当前代码真相中移除；角色 battle 规则统一由 `InitialPassiveGrants` 与 `PassiveDefinition.RuntimeTriggers` 承载。

### 5.5 CardEvolutionDefinition

用途：描述一张卡牌实例可以如何从当前形态进化到另一形态。

核心字段：

| 字段 | 说明 |
|---|---|
| `EvolutionId` | 进化定义 ID |
| `FromCardId` | 原卡牌模板 ID |
| `ToCardId` | 目标卡牌模板 ID |
| `EvolutionType` | 进化类型 |
| `FromStage` | 要求当前阶段 |
| `ToStage` | 进化后阶段 |
| `RequiredOwnerCharacterId` | 限定所属角色，可为空 |
| `RequiredCardTags` | 要求卡牌标签，可为空 |
| `bAllowAsLevelUpCandidate` | 是否允许作为升级候选 |
| `DisplayName` | 候选展示标题 |
| `Description` | 候选展示描述 |

说明：

- 第一版卡牌进化通过替换 `RunCardInstance.CurrentCardId` 实现。
- 进化不修改 `BaseCardId`。
- 绝学化可先复用该定义，但第一版不要求完整内容池。
- starter 当前已补最小真实内容链：霍断岳 `断岳斩 -> 断岳斩·破阵`，用于直接验收成长 overlay 的卡牌进化路径。

示例：

```text
BaseCardId: card_huo_liefeng
CurrentCardId: card_huo_liefeng
选择进化后：
BaseCardId: card_huo_liefeng
CurrentCardId: card_huo_liefeng_poshi
EvolutionStage: Evolved
```

### 5.6 GrowthChoiceDefinition

用途：描述角色升级时可能出现的成长候选。

核心字段：

| 字段 | 说明 |
|---|---|
| `GrowthChoiceId` | 成长候选定义 ID |
| `ChoiceType` | 候选类型 |
| `DisplayName` | 展示标题 |
| `Description` | 展示说明 |
| `Weight` | 基础出现权重 |
| `AttributeType` | 属性成长类型，属性候选使用 |
| `AttributeDelta` | 属性增加值 |
| `CardEvolutionId` | 卡牌进化候选使用 |

说明：

- 第一版可以不为所有属性候选建表，也可以由规则直接生成。
- 当前 Step 4 运行时属性候选就是由 `FinalRunSession` 直接程序化生成，并未强依赖 `GrowthChoiceDefinition` 表。
- 卡牌进化候选应引用 `CardEvolutionDefinition`。
- 候选实例由 `FinalRun` 生成，不由 `FinalBattle` 生成。

### 5.7 CardGemDefinition（后续扩展）

用途：描述强化珠模板。

核心字段：

| 字段 | 说明 |
|---|---|
| `GemId` | 强化珠 ID |
| `DisplayName` | 显示名 |
| `GemTier` | 珠阶，决定需要的槽级 |
| `GemLevel` | 珠级，决定效果强度 |
| `AllowedCardTypes` | 可镶嵌卡牌类型 |
| `AllowedTags` | 可镶嵌标签 |
| `Effects` | 强化效果 |

第一版仅预留，不要求完整实现。

### 5.8 其他静态定义

以下定义沿用旧文档口径，第一版只要求满足现有首章内容：

- `EnemyDefinition`
- `EnemyIntentDefinition`
- `StatusDefinition`
- `PassiveDefinition`
- `RelicDefinition`
- `UltimateDefinition`
- `EventDefinition`
- `BattleEncounterDefinition`
- `RunEffectDefinition`

这些定义不应承载角色升级三选一或 Run 内卡牌进化的主逻辑。

其中 `StatusDefinition` 当前已完成 legacy 字段收口，正式规则入口如下：

- **新 schema** 已补齐终版结构入口：
  - `RuntimeModifiers`
  - `ProjectedCardModifiers`
  - `RuntimeTriggers`
  - `StackKeyPolicy / StackRule / DurationType / ExpireWindow`
  - `AppliesTo`
| 字段组 | 说明 |
|---|---|
| `RuntimeModifiers` | 承载直接 battle 规则修正，例如伤害百分比、防护百分比、按攻击牌过滤等 |
| `ProjectedCardModifiers` | 承载状态驱动的 `BattleCard` 投影修正，例如 owned-hand 攻击牌增伤、减费等；其生命周期 schema 现在使用状态专属 `LifetimePolicy`，而不是复用 trigger 的 `DurationPolicy` |
| `ConsumptionRules` | 承载状态级扣层规则，例如成功造成伤害后消耗、成功抵消共享生命伤害后消耗 |
| `RuntimeTriggers` | 预留为状态终版的正式触发入口；首版先只进入 schema，不切换 runtime 消费 |
| `StackKeyPolicy / StackRule / DurationType / ExpireWindow` | 终版状态归并、叠层和持续时间模型 |
| `bIsResourceStatus / ResourceBehavior / bAutoAffectBattleRules / bAutoProjectToCards` | 承载正式资源型状态 schema；用于明确 `刀势 / 药引` 这类专属层数资源不自动进入 battle 规则修正和卡牌投影 |
| `AppliesTo` | 状态定义级拥有者约束。`Shared` 允许玩家与敌人单位拥有；`PlayerOnly` 允许玩家角色与 `team_player`；`EnemyOnly` 只允许敌人单位 |

当前 `AppliesTo` 只约束状态拥有者归属，不改变状态的具体规则类别。battle runtime 会在 `AddStatusStacks()` 入口硬拦截不合法归属。

当前首版运行时约束：

- `士气` 当前已走结构化 `RuntimeModifiers` 的通用状态伤害修正路径。
- `生命免疫` 当前已走结构化 `RuntimeModifiers` 的 team HP protection 路径，并通过 `ConsumptionRules(PreventedTeamHealthDamage)` 扣层。
- `锋锐` 当前已通过 `ProjectedCardModifiers` 驱动 owned-hand `BattleCard` modifier，并通过 `ConsumptionRules(SuccessfulOwnerDamage)` 扣层。
- `刀势 / 药引` 当前已归位为正式资源型状态：获得继续走 `ApplyStatus`，消费改走 `ConsumeStatusResource`，且默认不参与 `RuntimeModifiers / ProjectedCardModifiers / RuntimeTriggers`。
- `易伤 / 虚弱` 当前已走结构化 `RuntimeModifiers` 路径。
- `中毒` 已进入正式 DOT schema，按 `ByOwnerAndSource` 叠层；`腐蚀 / 流血` 已退出当前首章有效规则口径。
- `OnTickEffects` 旧入口已删除；后续持续/触发型状态统一收口到 `RuntimeTriggers`，不再保留第二套未消费 schema。
- `DurationType / ExpireWindow / StackKeyPolicy / StackRule` 已进入 battle runtime 主路径。
- 旧状态字段和旧读取路径已删除，状态规则不再允许新旧字段双真相。

资源型状态当前的正式消费条件口径：

- `ConsumeStatusResource` effect 会把成功消费结果写入 effect chain record。
- `ResourceConsumedCondition` 用于判断本次 effect chain 中，指定 `StatusId` 是否成功消费了至少 N 层。
- starter 当前已把：
  - `断岳斩 / 断岳斩·破阵`
  - `化引 / 回春散`
 迁到 `ConsumeStatusResource + ResourceConsumedCondition` 链。

## 6. Run 内持久结构

### 6.1 RunState

用途：描述一次爬塔的持久状态。

核心字段：

| 字段 | 说明 |
|---|---|
| `RunId` | Run ID |
| `CurrentNodeId` | 当前节点 |
| `Characters` | 角色持久状态 |
| `RunDeck` | Run 内卡牌实例列表 |
| `RelicIds` | 已拥有遗物 |
| `Gold` | 金币 |
| `PendingGrowthChoice` | 当前待处理成长候选，可为空 |
| `RunFlags` | Run 内标记 |

### 6.2 RunPersistentCharacterState

用途：描述跨战斗保留的角色成长与压力状态。

核心字段：

| 字段 | 说明 |
|---|---|
| `CharacterId` | 角色模板 ID |
| `Level` | 角色等级 |
| `BreakthroughValue` | 当前突破值 / 经验值 |
| `BreakthroughRequiredValue` | 升级所需突破值 |
| `RootBone` | 根骨 |
| `Insight` | 悟性 |
| `KillingIntent` | 杀意 |
| `CurrentStress` | 战斗外保留压力 |
| `bCollapsed` | 是否处于崩溃状态 |
| `CurrentAwakenCount` | 已苏醒次数 |
| `CollapseCount` | 崩溃次数 |
| `bHasPendingGrowthChoice` | 是否有待处理升级选择 |

说明：

- 根骨、悟性、杀意是 Run 内成长属性。
- 悟性第一版可只影响突破值获取倍率。
- 压力状态的战斗内 `Normal / Critical / Collapse` 由 `BattleCharacterState` 维护。
- 战斗结束后是否保留压力值，由 Run 结算规则决定。

### 6.3 RunCardInstance

用途：描述玩家在本次 Run 中真正拥有的一张卡。

核心字段：

| 字段 | 说明 |
|---|---|
| `InstanceId` | Run 内卡牌实例 ID |
| `BaseCardId` | 初始基础卡牌 ID |
| `CurrentCardId` | 当前实际卡牌 ID |
| `OwnerCharacterId` | Run 内所属角色 |
| `EvolutionStage` | 当前进化阶段 |
| `TimesPlayedThisRun` | 本 Run 使用次数 |

说明：

- `BaseCardId` 用于保留这张牌的原始身份。
- `CurrentCardId` 用于决定当前展示和结算模板。
- 第一版卡牌进化只要求替换 `CurrentCardId` 和 `EvolutionStage`。
- 当前 `RunDeck` 已是 Run 内唯一牌组真相源；战斗起始请求首版仍兼容 `DeckCardIds`，但已补显式 `DeckEntries`，其中至少包含 `SourceRunCardInstanceId / EffectiveCardId / OwnerCharacterId`。
- 如果这张卡当前也存在于 active battle，进化后会按 `SourceRunCardInstanceId` 同步刷新对应 `BattleCardInstance` 的展示与结算引用；当前覆盖范围包含 hand / draw / discard / consume / ongoing 中的直接来源实例。

### 6.4 PendingGrowthChoiceState

用途：描述一次角色升级待处理的成长三选一。

核心字段：

| 字段 | 说明 |
|---|---|
| `bIsValid` | 当前待处理结构是否有效 |
| `CharacterId` | 升级角色 |
| `Choices` | 候选列表 |

说明：

- 当前首版运行时同一时刻只维护一个 `PendingGrowthChoice`。
- Step 4 已落地的最小结构只保存 `bIsValid / CharacterId / Choices`。
- `PendingChoiceId / GeneratedAtNodeId / bResolved` 这类审计字段可以在接入 UI、命令与事件后再补。
- 当前 starter 首章不再使用“开局即 pending growth”；默认改为霍断岳以 `80 / 100` 突破值进入首战，在战斗中自然触发第一次成长选择。

### 6.5 GrowthChoiceInstance

用途：描述一次实际生成出来的成长候选。

核心字段：

| 字段 | 说明 |
|---|---|
| `ChoiceInstanceId` | 候选实例 ID |
| `ChoiceType` | 候选类型 |
| `CharacterId` | 候选归属角色 |
| `DisplayName` | 展示标题 |
| `Description` | 展示说明 |
| `AttributeType` | 属性成长候选使用 |
| `AttributeDelta` | 属性增量 |
| `TargetRunCardInstanceId` | 卡牌进化候选目标实例 |
| `CardEvolutionId` | 进化定义 ID |
| `FromCardId` | 展示用原卡 |
| `ToCardId` | 展示用目标卡 |

说明：

- 一次角色升级默认生成 3 个候选。
- 候选应混合属性成长与卡牌进化。
- 当前 Step 4 运行时口径为 deterministic：
  - 默认生成 `RootBone +1` 与 `Insight +1`
  - 若存在可用进化，则第 3 个候选为卡牌进化
  - 否则第 3 个候选为 `KillingIntent +1`

## 7. 战斗运行时结构

### 7.1 BattleState

用途：描述一场战斗的完整运行时状态。

核心字段：

| 字段 | 说明 |
|---|---|
| `BattleId` | 战斗实例 ID |
| `RunId` | 来源 Run ID |
| `EncounterId` | 遭遇 ID |
| `RuleConfigId` | 规则配置 ID |
| `CurrentRound` | 当前回合 |
| `CurrentAP` | 当前 AP |
| `CurrentEP` | 当前 EP |
| `TeamCurrentHP` | 队伍当前生命 |
| `TeamMaxHP` | 队伍最大生命 |
| `Characters` | 角色战斗状态 |
| `Enemies` | 敌人战斗状态 |
| `DeckState` | 战斗牌区状态 |
| `CardInstances` | 战斗内卡牌实例 |
| `StatusInstances` | 状态实例 |
| `PassiveInstances` | 被动实例 |
| `RelicRuntimeStates` | 遗物运行时状态 |
| `BattleLogEntries` | 战斗日志 |

说明：

- `BattleState` 不直接保存角色升级候选。
- 战斗中发生的行为可记录为成长事实，由战斗结束后交给 `FinalRun` 处理。

### 7.2 BattleCharacterState

用途：描述角色在当前战斗中的状态。

核心字段：

| 字段 | 说明 |
|---|---|
| `RuntimeUnitId` | 运行时单位 ID |
| `CharacterId` | 角色模板 ID |
| `CurrentStress` | 当前压力 |
| `StressCap` | 当前压力上限 |
| `StressState` | `Normal / Critical / Collapse` |
| `CriticalThreshold` | 当前临界阈值 |
| `CollapseThreshold` | 当前崩溃阈值 |
| `bEnteredCriticalThisEvent` | 本次压力事件是否已触发临界保护 |
| `RuntimeAttack` | 战斗内攻击 |
| `RuntimeDefense` | 战斗内防御 |
| `RuntimeBreakRate` | 战斗内削韧修正 |
| `bActedThisRound` | 本回合是否行动过 |

说明：

- `CriticalThreshold = ceil(StressCap * CriticalThresholdRatio)`。
- `CollapseThreshold = StressCap`。
- 首次越过临界阈值优先进入 `Critical`。
- 角色专属临界收益第一版不要求实现。

### 7.3 BattleEnemyState

用途：描述敌人在当前战斗中的状态。

核心字段：

| 字段 | 说明 |
|---|---|
| `RuntimeUnitId` | 运行时单位 ID |
| `EnemyId` | 敌人模板 ID |
| `PositionIndex` | 当前站位 |
| `SpawnWave` | 进入战斗波次 |
| `CurrentHP` | 当前生命 |
| `CurrentBreakValue` | 当前韧性 |
| `CurrentInitiative` | 当前先机 |
| `CurrentIntentId` | 当前意图 |
| `bActedThisRound` | 本回合是否已行动 |

### 7.4 TeamDeckState

用途：描述共享牌组各牌区的当前状态。

核心字段：

| 字段 | 说明 |
|---|---|
| `DrawPileCardInstanceIds` | 抽牌堆 |
| `HandCardInstanceIds` | 手牌 |
| `DiscardPileCardInstanceIds` | 弃牌堆 |
| `OngoingZoneCardInstanceIds` | 持续区 |
| `ConsumePileCardInstanceIds` | 消耗区 |

这些 ID 指向 `BattleCardInstance`，而不是 `RunCardInstance`。

### 7.5 BattleCardInstance

用途：描述一张牌在当前战斗中的实例状态。

核心字段：

| 字段 | 说明 |
|---|---|
| `BattleCardInstanceId` | 战斗内卡牌实例 ID |
| `RunCardInstanceId` | 来源 Run 内实例 ID，可为空 |
| `BaseCardId` | 原始基础卡 ID |
| `CurrentCardId` | 当前使用模板 ID |
| `RuntimeOwnerUnitId` | 当前所属单位 |
| `BaseDefinition` | 当前基础模板定义 |
| `ModifierRecords` | battle 内临时修正记录集合 |
| `ProjectedDefinition` | 当前投影后的运行时定义副本 |
| `RuntimeCostAP` | 当前投影 AP 消耗 |
| `RuntimeKeywords` | 当前投影关键词 |
| `RuntimeBehavior` | 当前投影牌区行为位 |

说明：

- 战斗开始时由 `RunCardInstance` 生成基础战斗实例。
- 衍生牌、复制牌、敌方塞入牌可以没有 `RunCardInstanceId`。
- 若 Run 内卡牌进化发生在战斗中，当前 battle 中直接来源于目标 `RunCardInstanceId` 的对应实例都会同步刷新 base `CurrentCardId / BaseDefinition`，并重新投影当前运行时定义。
- 临时费用、临时关键词、临时行为位和运行时图 patch 只写入 `BattleCardInstance.ModifierRecords`，不回写 Run。
- 生成牌、复制牌、衍生牌、临时牌默认不联动这次刷新，因为它们不保留来源 `RunCardInstanceId`。

`ModifierRecords` 首版统一包含以下协议字段：

| 字段 | 说明 |
|---|---|
| `ModifierId` | 修正记录自身 ID |
| `SourceType` | 来源类型：Card / Status / Passive / Relic / System |
| `DurationPolicy` | 持续窗口：`UntilPlayed / EndOfTurn / EndOfRound / EndOfBattle / ManualClear` |
| `bExpireAtPlayerTurnEnd` | 是否在玩家回合结束时额外清理 |
| `ApplyOrder` | 同一卡上的稳定应用顺序 |
| `OutgoingDamagePercentDelta` | 当前卡实例独有的伤害百分比修正 |
| `PatchPayload` | 费用、关键词、行为位或运行时图 patch 载荷 |

当前首版允许的 patch 作用范围只覆盖卡牌自身运行时图：

- AP 费用修正
- 关键词增删
- `retain / consume / recycle` 行为位覆盖
- effect 节点替换 / 插入 / 移除 / 常见载荷覆写
- requirement / condition 节点替换 / 插入 / 移除 / 常见载荷覆写

### 7.6 BattleStatusInstance

用途：描述一个状态在当前战斗中的挂载情况。

核心字段：

| 字段 | 说明 |
|---|---|
| `StatusInstanceId` | 状态实例 ID |
| `StatusId` | 状态模板 ID |
| `OwnerUnitId` | 状态拥有者 |
| `SourceUnitId` | 状态来源 |
| `CurrentStacks` | 当前层数 |
| `RemainingDuration` | 剩余持续值 |
| `DurationType` | 持续类型 |
| `AppliedSequence` | 应用顺序 |

### 7.7 BattlePassiveInstance

用途：描述一个被动在当前战斗中的挂载情况。

核心字段：

| 字段 | 说明 |
|---|---|
| `PassiveInstanceId` | 被动实例 ID |
| `PassiveId` | 被动模板 ID |
| `DisplayId` | 调试显示 ID |
| `DisplayName` | 调试显示名 |
| `OwnerUnitId` | 被动拥有者 |
| `SourceUnitId` | 被动来源 |
| `CurrentStacks` | 当前层数 |
| `RemainingDuration` | 剩余持续值 |
| `DurationType` | 持续类型 |
| `AppliedSequence` | 应用顺序 |
| `TriggerStates` | 该被动实例持有的运行时 trigger 状态 |

说明：

- `BattlePassiveInstance` 当前已经成为代码中的真实 runtime truth，不再只是预留占位。
- `PassiveInstance` 只存在于 `FinalBattle`；`FinalRun` 当前不持有被动真相。
- battle snapshot / debug query 当前已暴露最小 passive 视图，用于验证能力牌赋予被动链路。

### 7.8 BattleRelicRuntimeState

用途：描述遗物在当前战斗中的触发记录。

核心字段：

| 字段 | 说明 |
|---|---|
| `RelicId` | 遗物 ID |
| `TriggerStates` | 触发状态列表 |
| `CustomCounters` | 特殊计数，可为空 |

`RuntimeTriggers` 当前已支持在 `Effects` 之后追加 `TriggeredCardModifiers`。  
当前 target source 已支持：
- `DrawnCardsFromExecutedEffects`：把 trigger follow-up modifier 直接挂到本次 effect summary 中抽到的 battle card instance 上
- `CurrentOwnedHandCards`：在 trigger 结算时扫描来源角色当前手牌中的实例，并把 modifier 直接挂到这些 hand card instance 上
- `CurrentAllyHandCards`：扫描来源角色之外的其他玩家角色当前手牌实例；不包含来源角色自己、敌人或 `team_player`

### 7.9 BattleEffectExecutionSummary

用途：描述一组 effect 在本次结算中产出的聚合结果，供 trigger / growth fact / battle event follow-up 继续消费。

核心字段补充：

| 字段 | 说明 |
|---|---|
| `TotalCardsDrawn` | 本次结算总抽牌数量 |
| `DrawnCardInstanceIds` | 本次结算实际抽到的 `BattleCardInstanceId` 列表 |

说明：

- `DrawnCardInstanceIds` 是 `RuntimeTriggers -> TriggeredCardModifiers` 的稳定目标协议之一。
- 对 `CurrentOwnedHandCards / CurrentAllyHandCards` 这类目标来源，`FinalBattle` 不依赖 effect summary，而是直接按 source owner 扫描当前手牌实例。

### 7.10 BattleEvent

用途：描述战斗内发生的结构化事件，并作为 HUD、战斗日志与调试界面的统一消费入口。

当前与被动可见性直接相关的事件类型已补齐：

```text
PassiveApplied
PassiveTriggered
PassiveRemoved
```

`BattleEvent` 中与被动相关的最小字段语义固定为：

| 字段 | 说明 |
|---|---|
| `PassiveInstanceId` | 被动实例 ID |
| `PassiveId` | 被动模板 ID |
| `SourceUnitId` | 被动来源单位 |
| `TargetUnitId` | 被动拥有者 |
| `RelatedTag` | 触发窗口标签，仅 `PassiveTriggered` 使用 |
| `ReasonTag` | 应用/移除原因标签 |
| `PrimaryValue` | 当前层数 |
| `SecondaryValue` | 当前剩余持续值 |
| `Message` | 直接可展示的事件文本，必须带被动显示名 |

当前首版原因标签口径：

```text
passive.applied.initial_grant
passive.applied.effect
passive.removed.expired
```

说明：

- `PassiveApplied` 会在 battle 初始化应用 `InitialPassiveGrants` 时发出，也会在 `ApplyPassive` effect 成功挂上或刷新被动时发出。
- `PassiveTriggered` 会在被动的某条 `RuntimeTriggerDefinition` 条件满足且成功执行后发出，无论后续效果是 `Effects` 还是 `TriggeredCardModifiers`。
- `PassiveRemoved` 当前统一承载“失效 / 移除”事件；本首版先通过 `ReasonTag` 区分原因，不再拆单独的 `Expired` 事件类型。
- battle snapshot / debug query 继续保留被动实例视图；事件流只负责“发生了什么”，不创建第二套被动真相。

## 8. 关键数据流

### 8.1 战斗初始化

```text
RunState
-> RunCardInstance
-> BattleCardInstance
-> BattleState.DeckState
```

规则：

- `RunCardInstance.CurrentCardId` 决定战斗中卡牌当前模板。
- 通用卡在入战时必须解析 `RuntimeOwnerUnitId`。
- 角色 Run 内成长属性折算为 `BattleCharacterState` 的运行时属性。

### 8.2 角色升级成长三选一

```text
BattleGrowthFact / 节点奖励
-> 增加 BreakthroughValue
-> 达到阈值
-> FinalRun 生成 PendingGrowthChoiceState
-> 玩家选择 GrowthChoiceInstance
-> 应用属性成长或卡牌进化
```

说明：

- `FinalBattle` 不直接生成成长候选。
- `FinalBattle` 只产出结构化 `BattleGrowthFactBatch`。
- `FinalApp` 桥接层根据角色 `CharacterGrowthConfig` 把 facts 转成突破值，并调用 `FinalRun::AddBreakthroughValue()`。
- `FinalRun` 根据规则处理突破值、升级和候选应用。
- 当前属性成长应用成功后，`FinalApp` 还会把最新的 battle runtime 投影回刷到 active battle：
  - `RootBone -> VitalShare / StressCap / RuntimeDefense`
  - `KillingIntent -> RuntimeAttack / RuntimeCritChance / RuntimeCritDamage`
  - `Insight` 仍不进入 battle runtime 面板
- 当前 Step 4 已落地的最小规则还包括：
  - 同一时刻只允许一个 `PendingGrowthChoice`
  - 若已有待处理成长候选，新的突破值仍会累计，但不会再次触发升级
- 当前 Step 5 已补 `SelectGrowthChoice` 命令入口：
  - 选择成功后立即应用属性成长或卡牌进化
  - 应用后立即清空 `PendingGrowthChoice`
  - 当前不会因为剩余突破值足够而自动继续生成下一组候选
- 当前 Step 8 已补安全窗口口径：
  - 只有玩家命令完整结算后首次满槽，才立即弹出 Growth overlay
  - 敌方阶段、被动链或战斗胜利导致的满槽，会延后到下一个安全窗口展示

### 8.3 卡牌进化

```text
GrowthChoiceInstance
-> CardEvolutionDefinition
-> RunCardInstance.CurrentCardId = ToCardId
-> RunCardInstance.EvolutionStage = ToStage
-> FinalApp 桥接 active battle refresh
-> 按 SourceRunCardInstanceId 刷新当前 battle 中的直接来源 BattleCardInstance
```

必须保持：

```text
BaseCardId 不变
CurrentCardId 改变
BattleCardInstance 更新 base definition 后保留 ModifierRecords 并重新投影
历史已经完成的结算不回滚
```

### 8.4 压力临界

```text
压力变化事件
-> 更新 CurrentStress
-> 检查 CriticalThreshold
-> Normal / Critical / Collapse 状态流转
```

规则：

- 第一次越过临界阈值优先进入 `Critical`。
- 同一次压力事件不直接跳过临界进入崩溃。
- 临界期间继续受压，进入崩溃判定。
- 角色专属临界收益后续扩展，不是状态机成立的前提。

## 9. 第一版最小落地范围

第一版建议优先实现以下数据结构：

```text
BattleRuleConfig
CharacterDefinition
CardDefinition
BattleEffectDefinition
CardEvolutionDefinition
GrowthChoiceDefinition
RunState
RunPersistentCharacterState
RunCardInstance
PendingGrowthChoiceState
GrowthChoiceInstance
BattleState
BattleCharacterState
BattleEnemyState
TeamDeckState
BattleCardInstance
BattleStatusInstance
BattlePassiveInstance
BattleRelicRuntimeState
```

第一版可以暂缓：

```text
CardGemDefinition 完整实现
强化珠即时镶嵌流程
强化槽升级和珠子三合一
完整绝学化内容池
角色专属临界收益
复杂事件成长树
门派 / 经脉 / 技能树
```

## 10. 设计风险与约束

### 10.1 避免 Battle 层越权

不允许：

```text
FinalBattle 直接升级角色
FinalBattle 直接替换 RunCardInstance.CurrentCardId
FinalBattle 直接生成成长三选一候选
```

应该：

```text
FinalBattle 记录事实
FinalRun 处理成长
FinalData 提供定义
```

### 10.2 避免 CardDefinition 被污染

不允许把以下内容写入静态模板作为运行时结果：

```text
当前进化状态
当前强化珠
本局使用次数
临时费用变化
临时关键词变化
```

这些应分别写入：

```text
RunCardInstance
BattleCardInstance
ModifierRecords
```

### 10.3 避免第一版范围膨胀

第一版只验证：

```text
角色升级成长三选一
卡牌实例进化
临界状态机
当前手牌即时刷新
```

强化珠、绝学化、角色专属临界收益可以保留数据口径，但不进入第一版必须实现范围。
