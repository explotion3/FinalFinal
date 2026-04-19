# 战斗数据字段规范 2.0

## 1. 文档定位
本文档用于重构战斗数据层的字段结构、实体边界与运行时实例设计。  
原 `Combat_Data_Schema.md` 旧稿保留作为参照；若后续补回仓库，可继续作为历史对照。新的字段口径、实体拆分与运行时结构统一收敛到本文档。

与其他文档的分工如下：
* [GDD4.0.md](GDD4.0.md)：描述玩法设计、系统目标与范围边界
* [Battle_Rules.md](Battle_Rules.md)：描述术语定义、结算顺序与判定边界
* [Card_Design_Guide.md](Card_Design_Guide.md)：描述卡牌通用字段、类型、稀有度、关键词与文本规范
* [Starter_Characters.md](Starter_Characters.md)：描述首发角色与角色卡牌内容
* [Starter_Enemies.md](Starter_Enemies.md)：描述首章敌人与遭遇内容
* [Starter_Relics.md](Starter_Relics.md)：描述首章遗物内容
* [Starter_Events.md](Starter_Events.md)：描述首章事件内容
* 本文档：描述数据层如何承载上述设计与内容

## 2. 本次重构目标
本次重构优先解决以下问题：
* 统一旧 schema 与当前文档体系的术语口径
* 用 `所属单位` 替代旧的卡牌归属角色写法
* 补齐事件、遗物、生成牌与衍生牌的字段承载能力
* 清理重复字段与枚举冲突
* 重新收口运行时实例结构，避免悬空引用

## 3. 重构范围
本轮重构预计覆盖以下内容：
* 通用命名规范与推荐枚举
* 角色、卡牌、敌人、状态、被动、遗物、事件、奥义定义
* 运行时实例结构
* 首版最小落地范围

## 4. 目标结构草案
### 4.1 通用规则
* 文档定位
* 数据设计原则
* 命名规范
* 推荐枚举与标签

### 4.2 静态定义
* `CharacterDefinition`
* `CardDefinition`
* `BattleEffectDefinition`
* `EnemyDefinition`
* `EnemyIntentDefinition`
* `StatusDefinition`
* `PassiveDefinition`
* `RelicDefinition`
* `EventDefinition`
* `UltimateDefinition`
* `BattleEncounterDefinition`
* `BattleRuleConfig`

### 4.3 运行时结构
* `BattleState`
* `BattleCharacterState`
* `BattleEnemyState`
* `BattleCardInstance`
* `BattleStatusInstance`
* `BattlePassiveInstance`
* `BattleRelicRuntimeState`
* `RunPersistentCharacterState`

## 5. 当前状态
本文档当前先建立重构稿结构。  
后续重写时，以原 `Combat_Data_Schema.md` 旧稿为参照，但不继承其中已经过时的字段、枚举或命名口径。

## 6. 数据设计原则
### 6.1 基本原则
* 能做成配置的内容，不写死在蓝图或 C++ 流程里
* 静态模板、战斗内实例、局外持久状态分离
* 数值字段、规则字段、表现字段分离
* 同一类概念只保留一套主字段，不保留语义重复字段
* 首版优先保证可配置、可运行、可调试，不过度为远期扩展预埋复杂层

### 6.2 口径原则
* 术语统一以 [GDD4.0.md](GDD4.0.md)、[Battle_Rules.md](Battle_Rules.md)、[Card_Design_Guide.md](Card_Design_Guide.md) 为准
* 卡牌归属统一写为 `所属单位`，不再使用“归属角色”作为主字段概念
* 稀有度统一使用 `普通 / 稀有 / 罕见 / 传说`
* 关键词、状态、被动三者严格分层，不互相代替

### 6.3 分层原则
* `Definition` 负责描述静态模板，不记录战斗中的临时变化
* `Instance` / `State` 负责描述战斗中的动态值，不回写静态模板
* `RunPersistentState` 负责描述跨战斗保留的数据，不混入单场 `BattleState`

### 6.4 事件与遗物原则
* 事件、遗物、卡牌、敌人都应复用统一的效果描述思路
* 首版允许按内容类型保留少量专用字段，但避免为单条内容写一次性特判字段
* 已经在首发内容文档中出现的效果类型，schema 必须能直接承载

## 7. 命名规范
### 7.1 基础规则
* 显示名与系统 ID 分离
* 系统 ID 全局唯一，优先使用稳定英文小写前缀
* 字段名统一使用 PascalCase
* 布尔字段统一以 `b` 开头
* 枚举名统一使用 `E` 前缀
* 结构体名统一使用 `F` 前缀

### 7.2 实体 ID 建议
* `CharacterId`
* `CardId`
* `EnemyId`
* `StatusId`
* `PassiveId`
* `RelicId`
* `EventId`
* `EncounterId`
* `UltimateId`

### 7.3 推荐 ID 前缀
* 角色：`char_`
* 卡牌：`card_`
* 敌人：`enemy_`
* 状态：`status_`
* 被动：`passive_`
* 遗物：`relic_`
* 事件：`event_`
* 遭遇：`enc_`
* 奥义：`ult_`

### 7.4 常见数值字段命名
* `BaseCostAP`
* `BaseCostEP`
* `BaseAttack`
* `BaseDefense`
* `BaseVitalShare`
* `BaseStressCap`
* `BaseBreakRate`
* `BaseBreakValue`
* `BaseDamagePower`
* `InitialInitiativeValue`
* `InitiativeResponse`

### 7.5 布尔字段命名
* `bCanCopy`
* `bCanGenerate`
* `bCanUpgrade`
* `bCollapsed`
* `bActedThisRound`

### 7.6 卡牌相关字段命名
* 静态卡牌所属字段统一使用 `OwnerUnitId`
* 运行时单位句柄统一使用 `RuntimeUnitId`
* 运行时卡牌所属字段统一使用 `RuntimeOwnerUnitId`
* 卡牌类型字段统一使用 `CardType`
* 卡牌稀有度字段统一使用 `Rarity`
* 关键词字段统一使用 `Keywords`
* 文本字段统一使用 `RulesText`

**说明**
* `唯一 / 保留 / 消耗` 统一视为规则关键词，写入 `Keywords`
* 不再为同一语义重复保留 `bUnique / bRetain / bConsume`

### 7.7 状态与被动字段命名
* 状态实例所属单位统一使用 `OwnerUnitId`
* 状态来源统一使用 `SourceUnitId`
* 被动所属统一使用 `OwnerUnitId`
* 被动来源统一使用 `SourceUnitId`

### 7.8 命名禁用项
以下旧口径不再作为主命名继续使用：
* `OwnerCharacterId` 作为卡牌静态归属字段
* `bExhaust`
* `Advanced` 作为稀有度值
* 同义重复字段并存，例如 `MinStressChangePerEvent / StressChangeMinPerEvent`

## 8. 共享协议
### 8.1 设计原则
共享协议用于统一定义字段允许使用的枚举值与协议类型。  
先把协议层定稳，再写具体实体，避免每个实体自己发明一套口径。

### 8.2 UnitReference
**用途**  
统一“所属单位 / 来源单位 / 目标单位”在静态模板层与运行时实例层的引用方式。

**组成**
* `TemplateUnitId`：静态模板层使用的单位引用
* `RuntimeUnitId`：运行时实例层使用的单位句柄
* `ReservedTeamUnitId`：用于队伍共享归属的保留值

**建议格式**
* 角色模板：`char_*`
* 敌人模板：`enemy_*`
* 玩家单位实例：`unit_player_*`
* 敌方单位实例：`unit_enemy_*`
* 队伍保留值：`team_player`、`team_enemy`

**说明**
* `CardDefinition.OwnerUnitId` 这类静态字段默认使用 `TemplateUnitId`
* `BattleCharacterState.RuntimeUnitId`、`BattleEnemyState.RuntimeUnitId`、`BattleCardInstance.RuntimeOwnerUnitId`、`BattleStatusInstance.OwnerUnitId / SourceUnitId`、`BattlePassiveInstance.OwnerUnitId / SourceUnitId` 一律使用 `RuntimeUnitId` 或 `ReservedTeamUnitId`
* 通用卡静态模板允许 `OwnerUnitId` 为空，但在入组、生成、复制或被敌人塞入后，必须解析为具体 `RuntimeUnitId`
* 队伍共享状态、共享资源结算与全队公共效果统一使用 `team_player`
* 敌方公共机制、敌方共享效果统一使用 `team_enemy`
* 运行时字段不得直接引用 `char_* / enemy_*` 这类模板 ID

### 8.3 CardType
**用途**  
统一卡牌类型字段 `CardType` 的可选值。

**建议枚举**
* `Attack`
* `Skill`
* `Ability`

### 8.4 Rarity
**用途**  
统一卡牌与遗物的稀有度字段 `Rarity` 的可选值。

**建议枚举**
* `Common`
* `Rare`
* `Epic`
* `Legendary`

**说明**
* 文档口径对应关系为：`普通 / 稀有 / 罕见 / 传说`
* 数据字段中允许继续使用稳定英文枚举值，但必须和文档口径一一对应

### 8.5 LoadoutRole
**用途**  
统一开局卡组条目中 `LoadoutRole` 的可选值。

**建议枚举**
* `BaseAttack`
* `BaseDefense`
* `BaseTactic`
* `InitialSignature`
* `ExtraStartCard`

### 8.6 TargetType
**用途**  
描述交互层如何选择目标。

**建议枚举**
* `None`
* `Self`
* `SingleEnemy`
* `SingleAlly`
* `AllEnemies`
* `AllAllies`
* `Team`

**说明**
* `TargetType` 主要服务 UI 和交互层
* 它回答的是“玩家如何选目标”，不是“实际效果最后打到谁”

### 8.7 UnitTargetRule
**用途**  
描述效果真实结算到哪个单位目标。

**建议枚举**
* `None`
* `Self`
* `OwnerUnit`
* `SingleEnemy`
* `SingleAlly`
* `AllEnemies`
* `AllAllies`
* `Team`
* `RandomEnemy`
* `RandomAlly`
* `LowestHpAlly`
* `HighestHpEnemy`

**说明**
* `UnitTargetRule` 只回答“效果结算到哪个单位目标”
* `UnitTargetRule` 默认按相对施加者阵营解释，而不是按绝对阵营解释

### 8.8 TriggerCondition
**用途**  
统一效果的触发时点。

**建议枚举**
* `OnPlay`
* `OnResolve`
* `OnHit`
* `OnBreak`
* `OnTurnStart`
* `OnTurnEnd`
* `Passive`

### 8.9 ScaleMode
**用途**  
统一 `BattleScalarValue.ScaleMode` 的可选值。

**建议枚举**
* `Flat`
* `BySourceStat`
* `ByTargetStat`
* `BonusPercent`

**说明**
* `Flat`：直接使用固定值
* `BySourceStat`：按来源属性倍率结算
* `ByTargetStat`：按目标属性倍率结算
* `BonusPercent`：作为附加百分比修正值参与结算

### 8.10 SourceStat
**用途**  
统一数值缩放引用的属性来源。

**建议枚举**
* `None`
* `Attack`
* `Defense`
* `VitalShare`
* `BreakRate`
* `BaseDamagePower`

### 8.11 CardZone
**用途**  
统一卡牌进入、离开或被放入的牌区。

**建议枚举**
* `Hand`
* `DrawPile`
* `DiscardPile`
* `OngoingZone`
* `ConsumePile`

**说明**
* 首版正式牌区统一为 `抽牌堆 / 手牌区 / 弃牌堆 / 持续区 / 消耗区`
* `Ability` 牌默认进入 `OngoingZone`
* 带 `消耗` 或 `消灭` 规则的牌，结算后统一进入 `ConsumePile`
* 首版不单独维护 `Graveyard`；若旧文档出现“坟墓区”，统一按 `ConsumePile` 解释

### 8.12 CardSelectionScope
**用途**  
统一从哪个牌区或卡牌集合中选牌。

**建议枚举**
* `None`
* `Hand`
* `DrawPile`
* `DiscardPile`
* `OngoingZone`
* `ConsumePile`
* `BattleDeck`
* `RunDeck`

### 8.13 CardSelectionRule
**用途**  
统一在已确定的选牌范围内如何选牌。

**建议枚举**
* `SourceCard`
* `TargetCard`
* `PlayerChoice`
* `Random`
* `RandomWithTag`
* `FirstMatch`

### 8.14 DurationType
**用途**  
统一状态、被动与临时效果的持续方式。

**建议枚举**
* `Instant`
* `Turn`
* `Round`
* `Battle`
* `Permanent`

### 8.15 ConditionType
**用途**  
统一效果条件条目的条件类型。

**建议枚举**
* `HasStatus`
* `StatusStacksAtLeast`
* `TargetBroken`
* `TeamTookDamageThisRound`
* `ConsumedSpecificCard`
* `HasHandTag`
* `HasUnitTag`
* `HasKeyword`

### 8.16 ConditionScope
**用途**  
统一条件检查作用于谁。

**建议枚举**
* `OwnerUnit`
* `TargetUnit`
* `BattleState`
* `Hand`
* `ConsumedCard`

### 8.17 CompareOp
**用途**  
统一条件数值比较方式。

**建议枚举**
* `Equal`
* `GreaterOrEqual`
* `LessOrEqual`
* `GreaterThan`
* `LessThan`

### 8.18 MatchRule
**用途**  
统一条件集合中多个条件条目的组合关系。

**建议枚举**
* `All`
* `Any`

### 8.19 EnemyTier
**用途**  
统一敌人模板与遭遇层级使用的强度分层口径。

**建议枚举**
* `Normal`
* `Elite`
* `Boss`

**说明**
* `EnemyDefinition.EnemyTier` 直接使用该协议
* `BattleEncounterDefinition.EncounterTier` 默认复用同一套枚举值

### 8.20 IntentType
**用途**  
统一敌人意图的大类标签。

**建议枚举**
* `Attack`
* `Defense`
* `Buff`
* `Debuff`
* `Summon`
* `Charge`
* `Special`

### 8.21 RecommendedStage
**用途**  
统一遭遇或内容节点的推荐出现阶段。

**建议枚举**
* `Early`
* `Mid`
* `Late`
* `Boss`

### 8.22 RunTargetScope
**用途**  
统一事件、商店、节点奖励等局外效果可作用到的目标范围。

**建议枚举**
* `None`
* `Team`
* `AnyCharacter`
* `UncollapsedCharacter`
* `CollapsedCharacter`
* `SpecificUnit`

### 8.23 RunTargetRule
**用途**  
统一局外效果在已确定目标范围内如何选中目标。

**建议枚举**
* `None`
* `PlayerChoice`
* `Random`
* `AllValid`
* `Specific`

**说明**
* `全部角色`：`RunTargetScope=AnyCharacter`，`RunTargetRule=AllValid`
* `随机 1 名未崩溃角色`：`RunTargetScope=UncollapsedCharacter`，`RunTargetRule=Random`
* `选择 1 名角色`：`RunTargetScope=AnyCharacter`，`RunTargetRule=PlayerChoice`
* `指定某名角色`：`RunTargetScope=SpecificUnit`，`RunTargetRule=Specific`

### 8.24 BreakActionCancelRule
**用途**  
统一 Break 触发后对敌方行动的默认处理方式。

**建议枚举**
* `CancelCurrentAction`
* `DelayToNextRound`
* `KeepCurrentAction`

### 8.25 IntentSelectRule
**用途**  
统一敌人意图池的抽取或轮换方式。

**建议枚举**
* `WeightedRandom`
* `Cycle`
* `PhaseSequence`
* `Scripted`

### 8.26 RunRequirementType
**用途**  
统一事件选项、商店条目与节点奖励等局外内容的可选条件类型。

**建议枚举**
* `HasUnit`
* `GoldAtLeast`
* `TeamHpAtLeast`
* `HasCard`
* `HasUpgradeableCard`
* `HasRemovableCard`
* `HasValidTarget`

**说明**
* `队伍中有霍断岳`：`RunRequirementType=HasUnit`，`TargetUnitId=char_huoduanyue`
* `存在可升级的霍断岳攻击牌`：`RunRequirementType=HasUpgradeableCard`，并组合 `TargetUnitId / TargetCardType / CardSelectionScope`
* `随机 1 名未崩溃角色` 这类前置合法性检查：`RunRequirementType=HasValidTarget`，并组合 `RunTargetScope / RunTargetRule`

### 8.27 RunCostType
**用途**  
统一事件选项、商店条目与节点奖励等局外内容的预先支付代价类型。

**建议枚举**
* `Gold`
* `TeamHP`
* `CharacterStress`

**说明**
* `Costs` 只描述“选择该选项前必须支付什么”
* 支付完成后结算的结果、奖励与惩罚仍通过 `RunEffectDefinition` 表达

### 8.28 BattleEffectType
**用途**  
统一战斗内效果定义 `BattleEffectDefinition.EffectType` 的可选值。

**建议枚举**
* `Damage`
* `GainShield`
* `Heal`
* `ApplyStatus`
* `RemoveStatus`
* `DrawCards`
* `GainAP`
* `GainEP`
* `BonusBreak`
* `ApplyPassive`
* `GenerateCard`
* `CopyCard`

**说明**
* `EffectType` 应与具体 `UBattleEffect_*` 子类一一对应
* 若后续新增效果子类，应同步补充 `BattleEffectType`

### 8.29 StatusCategory
**用途**  
统一状态定义 `StatusDefinition.StatusCategory` 的可选值。

**建议枚举**
* `Buff`
* `Debuff`
* `Signature`
* `Mechanic`

**说明**
* 文档口径对应关系为：`增益 / 减益 / 专属 / 机制`
* `刀势 / 药引` 这类角色专属状态建议归为 `Signature`
* `剑阵` 属于衍生牌体系，不应归入 `StatusDefinition`
* `Break 破绽`、首领阶段机制等全局规则状态建议归为 `Mechanic`

### 8.29.1 StatusMergeRule
**用途**  
统一同名状态在运行时是否归并到同一条实例的默认规则。

**建议枚举**
* `ByOwner`
* `ByOwnerAndSource`
* `NeverMerge`

**说明**
* `ByOwner`：同 `StatusId + OwnerUnitId` 的状态默认归并到同一条实例
* `ByOwnerAndSource`：同 `StatusId + OwnerUnitId + SourceUnitId` 的状态才归并，适合持续伤害或明显依赖来源独立结算的状态
* `NeverMerge`：重复获得时总是创建新实例，首版应谨慎使用

### 8.29.2 StatusStackRule
**用途**  
统一状态在命中已有实例时如何处理层数或实例本身。

**建议枚举**
* `AddAndClamp`
* `RefreshOnly`
* `ReplaceExisting`
* `RejectIfExists`

**说明**
* `AddAndClamp`：层数累加后按 `MaxStacks` 截断，首版多数可叠层状态使用此规则
* `RefreshOnly`：不增加层数，只刷新持续值或结算顺序，适合 `MaxStacks=1` 的常见状态
* `ReplaceExisting`：移除旧实例后应用新实例，适合明确要求覆盖旧版本的状态
* `RejectIfExists`：若已有实例则本次获得失败，适合“不可重复获得”类状态

### 8.29.3 StatusRefreshRule
**用途**  
统一重复获得状态时，持续值如何刷新。

**建议枚举**
* `KeepLonger`
* `ResetToNew`
* `NoRefresh`

**说明**
* `KeepLonger`：若新持续值更长则覆盖，否则保持当前值，首版 `Turn / Round` 类状态默认优先使用
* `ResetToNew`：无论当前剩余值多少，都按本次新值重置
* `NoRefresh`：重复获得时不刷新持续值，仅处理层数、覆盖或拒绝获得
* `Battle / Permanent` 类型状态通常使用 `NoRefresh`

### 8.30 PassiveCategory
**用途**  
统一被动定义 `PassiveDefinition.PassiveCategory` 的可选值。

**建议枚举**
* `Ability`
* `Relic`
* `Talent`
* `EnemyMechanic`

**说明**
* 文档口径对应关系为：`能力 / 遗物 / 天赋 / 敌方机制`
* `Ability` 主要用于能力牌带来的持续被动
* `EnemyMechanic` 主要用于首领机制、敌方编组机制或特殊敌方战斗规则

### 8.31 EventCategory
**用途**  
统一事件定义 `EventDefinition.EventCategory` 的可选值。

**建议枚举**
* `Rest`
* `Risk`
* `Growth`
* `Trade`
* `Mystery`

**说明**
* 文档口径对应关系为：`休整 / 风险 / 成长 / 交易 / 神秘`
* `角色成长事件` 默认归入 `Growth`
* 偏赌博、结果高波动但不以稳定资源交换为主的事件优先归入 `Mystery`

### 8.32 TagDesignPrinciple
**用途**  
统一标签字段的职责边界，避免 `CardTags / EnemyTags / RelicTags / EncounterTags` 重新变成万能口袋。

**基本原则**
* 主分类优先使用独立字段或枚举，不用标签代替主分类
* 不同语义分层存放，不把“来源限制”“内容主题”“编组定位”混写在同一个标签字段里
* 标签主要服务筛选、生成池、构筑检索与轻量条件匹配，不承载核心规则真相

**分层建议**
* `RoleTags`：角色或敌人的战斗定位
* `ContentTags / ThemeTags`：内容主题、流派或功能方向
* `AccessTags`：获取限制、来源限制、是否只可生成等
* `StructureTags`：遭遇结构、编组结构、节点结构

**说明**
* 阶段脚本使用的 `PhaseTags` 属于局部调度标签，不并入通用内容标签体系
* 状态与被动上的 `Tags` 仅用于检索与条件匹配，不替代 `StatusCategory / PassiveCategory`

### 8.33 RoleTag
**用途**  
统一角色或敌人“在战斗中负责什么”的定位标签。

**建议枚举**
* `Frontline`
* `Breaker`
* `Support`
* `Healer`
* `Backline`
* `Tank`
* `Summoner`
* `Boss`

### 8.34 CardContentTag
**用途**  
统一卡牌的主题、流派或内容族标签。

**建议枚举**
* `Break`
* `Guard`
* `Heal`
* `Draw`
* `Array`
* `Collapse`
* `UltimateSupport`

**说明**
* `CardContentTags` 回答“这张牌主要服务哪类构筑或主题”
* 不在这里放“只能生成”“起始牌”这类获取限制

### 8.35 CardAccessTag
**用途**  
统一卡牌的获取方式与来源限制标签。

**建议枚举**
* `Starter`
* `GeneratedOnly`
* `Derived`
* `RewardOnly`
* `ShopOnly`

**说明**
* `CardAccessTags` 回答“这张牌如何进入牌组或是否可被常规获得”
* `衍生牌 / 只可生成牌` 优先写在这里，而不是混入 `CardContentTags`

### 8.36 EnemyRoleTag
**用途**  
统一敌人的编组定位标签。

**建议枚举**
* `Frontline`
* `Backline`
* `Tank`
* `Summoner`
* `Support`
* `Leader`
* `Boss`

### 8.37 EnemyMechanicTag
**用途**  
统一敌人的机制主题标签。

**建议枚举**
* `Poison`
* `MultiHit`
* `GuardBreak`
* `CallReinforcement`
* `Counter`
* `InitiativePressure`

**说明**
* `EnemyMechanicTags` 回答“这个敌人的核心机制压力来自哪里”
* 不用它来替代 `EnemyTier`、站位或是否首领

### 8.38 RelicThemeTag
**用途**  
统一遗物支持的构筑主题或资源方向。

**建议枚举**
* `Break`
* `Defense`
* `Heal`
* `AP`
* `EP`
* `Collapse`
* `Array`

### 8.39 EncounterThemeTag
**用途**  
统一遭遇在内容表现与战斗风格上的主题标签。

**建议枚举**
* `Road`
* `Ambush`
* `Poison`
* `Defense`
* `Boss`

### 8.40 EncounterStructureTag
**用途**  
统一遭遇在编组与流程结构上的标签。

**建议枚举**
* `SingleElite`
* `FrontBack`
* `MultiWave`
* `SummonLoop`
* `BossPhase`

### 8.41 ExternalReferenceBoundary
**用途**  
统一本文档中“只保存稳定 ID、但不展开内部结构”的外部引用字段边界。

**分类**
* `PresentationRef`：表现资源引用，例如 `Portrait / Art / Icon / BattlePrefab / AudioProfileId / VfxProfileId / BackgroundId / MusicId`
* `ProgressionRef`：成长系统引用，例如 `SkillTreeId`
* `RewardRef`：奖励与掉落系统引用，例如 `DeathRewardId / RewardProfileId`
* `CombatSubconfigRef`：战斗子配置引用，例如 `PhaseConfigId`

**说明**
* 本文档只要求这些字段提供稳定 ID，不负责展开其内部 schema
* 这些字段不得承载当前战斗主循环成立所必需的基础规则真相
* 若某个外部引用开始承担核心战斗规则，而不只是桥接扩展模块，应在后续版本中升格为独立 `Definition`
* 首版最小落地允许这些字段为空，只要不影响战斗主循环、基础掉落和最小表现

## 9. 静态定义
### 9.1 CharacterDefinition
**用途**  
描述一个可选角色的基础配置。

**推荐载体**  
`PrimaryDataAsset`

**必填字段**
* `CharacterId`：角色系统唯一 ID
* `DisplayName`：角色显示名称
* `RoleTags`：角色定位标签，协议见 `8.33 RoleTag`
* `BaseVitalShare`：角色提供的生命份额
* `BaseStressCap`：角色基础压力上限
* `BaseAttack`：角色基础攻击力
* `BaseDefense`：角色基础防御力
* `BaseBreakRate`：角色基础 Break 系数
* `BaseCritChance`：角色基础暴击率
* `BaseCritDamage`：角色基础暴击伤害
* `EpGainPerAP`：每消耗 `1 AP` 回复的 EP
* `InitialLoadoutCards`：角色开局投入共享牌组的卡牌条目列表
* `CharacterCardPoolIds`：该角色后续可获得的角色卡池
* `UltimateId`：角色绑定的奥义定义
* `SignatureStatusId`：角色专属状态定义

**可选字段**
* `Description`：角色简介文本
* `Portrait`：角色头像资源，外部引用类型见 `8.41 ExternalReferenceBoundary`
* `BattlePrefab`：战斗中使用的角色实体，外部引用类型见 `8.41 ExternalReferenceBoundary`
* `SkillTreeId`：角色技能树定义，外部引用类型见 `8.41 ExternalReferenceBoundary`
* `AudioProfileId`：角色音频配置，外部引用类型见 `8.41 ExternalReferenceBoundary`
* `VfxProfileId`：角色特效配置，外部引用类型见 `8.41 ExternalReferenceBoundary`

**InitialLoadoutCardEntry 最小字段**
* `CardId`
* `Count`
* `LoadoutRole`

**说明**
* `InitialLoadoutCards` 只记录角色开局自带并投入共享牌组的牌
* `CharacterCardPoolIds` 只记录该角色后续可获得的角色卡，不记录通用卡
* 角色专属状态统一挂在 `SignatureStatusId`

**最小示例**
```text
CharacterId: char_huoduanyue
DisplayName: 霍断岳
RoleTags: [Frontline, Breaker]
BaseVitalShare: 72
BaseStressCap: 30
BaseAttack: 24
BaseDefense: 20
BaseBreakRate: 0.14
BaseCritChance: 0.05
BaseCritDamage: 1.50
EpGainPerAP: 1
InitialLoadoutCards:
  - CardId: card_huod_liefeng
    Count: 2
    LoadoutRole: BaseAttack
  - CardId: card_huod_wenjia
    Count: 1
    LoadoutRole: BaseDefense
  - CardId: card_huod_duanyuezhan
    Count: 1
    LoadoutRole: InitialSignature
UltimateId: ult_huod_duanyuejueshi
SignatureStatusId: status_huod_daoshi
```

### 9.2 CardDefinition
**用途**  
描述一张可进入牌组循环的卡牌模板。

**推荐载体**  
`PrimaryDataAsset`

**必填字段**
* `CardId`：卡牌系统唯一 ID
* `DisplayName`：卡牌显示名称
* `OwnerUnitId`：静态所属单位引用；通用卡允许为空
* `CardType`：卡牌类型
* `Rarity`：卡牌稀有度
* `BaseCostAP`：基础 AP 消耗
* `Keywords`：规则关键词列表
* `RulesText`：牌面规则文本
* `Effects`：卡牌效果列表
* `TargetType`：交互层选目标方式

**可选字段**
* `Description`：补充说明文本
* `Art`：卡牌立绘资源，外部引用类型见 `8.41 ExternalReferenceBoundary`
* `CardContentTags`：卡牌主题标签，协议见 `8.34 CardContentTag`
* `CardAccessTags`：卡牌获取限制标签，协议见 `8.35 CardAccessTag`
* `VariantGroupId`：变体分组 ID
* `BaseCardId`：原牌 ID；基础牌默认等于自身
* `bIsVariantCard`：是否为变体卡
* `bCanUpgrade`：是否允许升级
* `bCanGenerate`：是否允许被生成
* `bCanCopy`：是否允许被复制
* `AudioProfileId`：音频配置，外部引用类型见 `8.41 ExternalReferenceBoundary`
* `VfxProfileId`：特效配置，外部引用类型见 `8.41 ExternalReferenceBoundary`

**说明**
* 角色卡默认填写 `OwnerUnitId`
* 静态 `OwnerUnitId` 默认使用 `TemplateUnitId`，协议见 `8.2 UnitReference`
* 通用卡允许 `OwnerUnitId` 为空，但在进入牌组时必须指定运行时所属单位
* 衍生牌也使用同一套 `CardDefinition`
* `CardContentTags` 用于描述流派与内容主题，例如 `Array / Break / Heal`
* `CardAccessTags` 用于描述获取限制，例如 `Derived / GeneratedOnly / Starter`
* `唯一 / 保留 / 消耗` 等规则性质统一写入 `Keywords`，不再为同一语义重复保留布尔字段
* 已在 `CardType / Keywords` 中表达的信息，不重复写入 `RulesText`

**最小示例**
```text
CardId: card_huod_duanyuezhan
DisplayName: 断岳斩
OwnerUnitId: char_huoduanyue
CardType: Attack
Rarity: Rare
BaseCostAP: 1
Keywords: []
RulesText: 对目标造成相当于攻击力 130% 的伤害。额外造成 3 点削韧。若目标处于 Break 状态，伤害再提高 20%。
TargetType: SingleEnemy
Effects: [effect_huod_duanyuezhan_damage, effect_huod_duanyuezhan_bonus_break, effect_huod_duanyuezhan_break_bonus]
bCanUpgrade: true
bCanGenerate: true
bCanCopy: true
```

```text
CardId: card_shen_guopai_jianzhen
DisplayName: 过牌剑阵
OwnerUnitId: char_shenqingxian
CardType: Skill
Rarity: Common
BaseCostAP: 0
Keywords: [Retain, Consume]
RulesText: 抽 1 张牌。
TargetType: None
Effects: [effect_draw_1]
bCanUpgrade: false
bCanGenerate: true
bCanCopy: true
```

### 9.3 BattleEffectDefinition
**用途**  
描述战斗系统中可复用的单个效果对象。  
该定义不再走“大一统字段结构”，而是采用“统一基类 + 条件结构 + 数值结构 + 具体效果子类”的拆分方式。

**推荐载体**  
`Instanced UObject`，挂在 `CardDefinition` / `EnemyIntentDefinition` / `UltimateDefinition` / `RelicDefinition` 内

**目标结构**
* `UBattleEffectDefinition`：所有战斗效果的共同基类
* `FBattleEffectConditionSet`：统一条件结构
* `FBattleEffectConditionEntry`：统一条件条目
* `FBattleScalarValue`：统一数值缩放结构
* `UBattleEffect_*`：具体效果子类

**推荐字段分层**
* 目标规则：统一协议
* 数值缩放：统一协议
* 触发条件：统一协议
* 效果参数：由具体效果子类自己持有

**UBattleEffectDefinition 基类必填字段**
* `EffectId`：效果条目唯一 ID
* `EffectType`：效果类型，协议见 `8.28 BattleEffectType`
* `UnitTargetRule`：效果真实结算目标
* `TriggerCondition`：效果触发条件

**UBattleEffectDefinition 基类可选字段**
* `Conditions`：条件集合
* `Notes`：补充备注

### 9.3.1 BattleEffectConditionSet
**用途**  
统一承载效果触发条件，避免条件字段散落到每个效果子类中。

**建议字段**
* `MatchRule`
* `Entries`

**说明**
* `MatchRule` 建议至少支持 `All / Any`
* `Break` 条件、受击条件、消耗条件优先写入 `Conditions`
* 仅当某类效果存在强专用条件时，才允许在子类中补自己的专用字段

### 9.3.2 BattleEffectConditionEntry
**用途**  
描述单个条件条目，避免 `ConditionSet` 继续长成“大一统字段包”。

**建议字段**
* `ConditionType`
* `ConditionScope`
* `CompareOp`
* `StatusId`
* `RequiredTag`
* `RequiredCardId`
* `ExpectedValue`
* `Notes`

**说明**
* 状态类条件优先使用 `ConditionType + StatusId + ExpectedValue`
* 标签类条件优先使用 `ConditionType + RequiredTag`
* 消耗特定卡牌、手牌中存在某标签卡、目标处于 Break，都通过条目表达，不再为单个需求新增顶层字段
* 当前 Runtime 原型为避免直接引入完整脚本式 `ConditionSet`，已经先落了若干最小专用 requirement 结构：`StatusConsumeRequirement / GeneratedCardConsumeRequirement / HandCardRequirement / TargetStateRequirement`。其中 `TargetStateRequirement` 先挂在 `Damage` effect 上，用于要求实际敌方目标存在、存活且处于 Break；后续若统一条件集合成熟，再收敛到同一 condition 表达

### 9.3.3 BattleScalarValue
**用途**  
统一表达伤害、护盾、治疗、削韧等数值缩放方式。

**建议字段**
* `BaseValue`
* `ScaleMode`
* `SourceStat`
* `FlatBonus`
* `Cap`

**说明**
* `ScaleMode` 用于区分固定值、倍率值、按来源属性结算等模式
* `Damage / Shield / Heal / BonusBreak` 优先复用这一结构

### 9.3.4 推荐效果子类
建议按真实效果行为拆分类，而不是继续向同一结构追加专用字段。

**首批建议子类**
* `UBattleEffect_Damage`
* `UBattleEffect_GainShield`
* `UBattleEffect_Heal`
* `UBattleEffect_ApplyStatus`
* `UBattleEffect_RemoveStatus`
* `UBattleEffect_DrawCards`
* `UBattleEffect_GainAP`
* `UBattleEffect_GainEP`
* `UBattleEffect_BonusBreak`
* `UBattleEffect_ApplyPassive`
* `UBattleEffect_GenerateCard`
* `UBattleEffect_CopyCard`

**各子类最小字段建议**
* `UBattleEffect_Damage`
  * `Scalar`
  * `HitCount`
  * `DamageSegments`
* `UBattleEffect_GainShield`
  * `Scalar`
* `UBattleEffect_Heal`
  * `Scalar`
* `UBattleEffect_ApplyStatus`
  * `StatusId`
  * `Stacks`
  * `DurationOverride`
* `UBattleEffect_RemoveStatus`
  * `StatusId`
  * `Stacks`
* `UBattleEffect_DrawCards`
  * `DrawCount`
* `UBattleEffect_GainAP`
  * `GainValue`
* `UBattleEffect_GainEP`
  * `GainValue`
* `UBattleEffect_BonusBreak`
  * `Scalar`
* `UBattleEffect_ApplyPassive`
  * `PassiveId`
  * `DurationOverride`
* `UBattleEffect_GenerateCard`
  * `GeneratedCardIds`
  * `DestinationCardZone`
  * `bUsePreferredGeneratedCard`
* `UBattleEffect_CopyCard`
  * `CardSelectionScope`
  * `CardSelectionRule`
  * `DestinationCardZone`

**说明**
* `BattleResolver` 的职责应收缩为效果编排，不再继续膨胀成大型字段解释器
* 卡牌、敌人意图、遗物、奥义应尽量共用同一套效果基类族
* 条件优先写入 `FBattleEffectConditionSet`
* 数值优先写入 `FBattleScalarValue`
* 具体效果参数只留在真正需要它的子类里
* 多段攻击优先由 `UBattleEffect_Damage.HitCount / DamageSegments` 承载，`EnemyIntentDefinition` 只负责意图抽取、预览与使用限制

**最小示例**
```text
EffectId: effect_huod_duanyuezhan_damage
EffectClass: UBattleEffect_Damage
EffectType: Damage
UnitTargetRule: SingleEnemy
TriggerCondition: OnPlay
Scalar:
  BaseValue: 130
  ScaleMode: BySourceStat
  SourceStat: Attack
Notes: 对目标造成相当于攻击力 130% 的伤害。
```

```text
EffectId: effect_huod_duanyuezhan_bonus_break
EffectClass: UBattleEffect_BonusBreak
EffectType: BonusBreak
UnitTargetRule: SingleEnemy
TriggerCondition: OnPlay
Scalar:
  BaseValue: 3
  ScaleMode: Flat
Notes: 额外造成 3 点削韧。
```

```text
EffectId: effect_shen_bufeng_generate_array
EffectClass: UBattleEffect_GenerateCard
EffectType: GenerateCard
UnitTargetRule: Self
TriggerCondition: OnPlay
GeneratedCardIds: [card_shen_random_array]
DestinationCardZone: Hand
Notes: 生成 1 张剑阵牌到手牌。
```

```text
EffectId: effect_huod_duanyuezhan_break_bonus
EffectClass: UBattleEffect_Damage
EffectType: Damage
UnitTargetRule: SingleEnemy
TriggerCondition: OnPlay
Conditions:
  MatchRule: All
  Entries:
    - ConditionType: TargetBroken
      ConditionScope: TargetUnit
      CompareOp: Equal
      ExpectedValue: 1
Scalar:
  BaseValue: 20
  ScaleMode: BonusPercent
Notes: 若目标处于 Break 状态，伤害再提高 20%。
```

### 9.4 EnemyDefinition
**用途**  
描述敌人本体模板。  
该定义负责敌人的基础面板、节奏参数与可用意图池，不直接承载每个招式的具体效果。

**推荐载体**  
`PrimaryDataAsset`

**必填字段**
* `EnemyId`：敌人系统唯一 ID
* `DisplayName`：敌人显示名称
* `EnemyTier`：普通 / 精英 / 首领
* `EnemyRoleTags`：敌人定位标签，协议见 `8.36 EnemyRoleTag`
* `EnemyMechanicTags`：敌人机制标签，协议见 `8.37 EnemyMechanicTag`
* `BaseHP`：基础生命值
* `BaseBreakValue`：基础韧性值
* `BaseDamagePower`：敌方输出基准
* `InitialInitiativeValue`：本轮起始先机值
* `InitiativeResponse`：每次先机事件的响应值
* `IntentPoolIds`：该敌人可用意图池
* `IntentSelectRule`：意图选择规则

**可选字段**
* `Description`：敌人说明文本
* `Portrait`：敌人头像资源，外部引用类型见 `8.41 ExternalReferenceBoundary`
* `BattlePrefab`：战斗实体，外部引用类型见 `8.41 ExternalReferenceBoundary`
* `PhaseConfigId`：阶段配置 ID，外部引用类型见 `8.41 ExternalReferenceBoundary`
* `DeathRewardId`：掉落配置 ID，外部引用类型见 `8.41 ExternalReferenceBoundary`
* `AudioProfileId`：音频配置，外部引用类型见 `8.41 ExternalReferenceBoundary`
* `VfxProfileId`：特效配置，外部引用类型见 `8.41 ExternalReferenceBoundary`

**说明**
* 普通敌人与精英敌人优先使用单体模板 + 编组组合，不把整场遭遇写死在敌人模板里
* 首领的阶段变化、召援节奏与特殊循环优先写入 `IntentPoolIds + PhaseConfigId`
* `PhaseConfigId` 当前只作为战斗子配置桥接字段，不在本文档中展开内部结构
* `DeathRewardId` 当前只负责桥接掉落与结算模块，不回写到战斗主循环字段中
* `EnemyRoleTags` 用于节点编组、站位与敌人职责筛选
* `EnemyMechanicTags` 用于机制筛选，例如 `Poison / MultiHit / CallReinforcement`

### 9.5 EnemyIntentDefinition
**用途**  
描述敌人的单个意图模板。  
攻击、护盾、召援、蓄势、施加减益统一收进同一意图结构，通过效果列表区分行为。

**推荐载体**  
`PrimaryDataAsset`

**必填字段**
* `IntentId`：意图系统唯一 ID
* `DisplayName`：意图显示名称
* `IntentType`：攻击 / 防御 / 增益 / 减益 / 召援 / 蓄势 / 特殊
* `PreviewText`：意图预览文本
* `Effects`：意图效果列表
* `Weight`：基础抽取权重

**可选字段**
* `CooldownTurns`：冷却回合
* `UseLimitPerBattle`：本场使用上限
* `PhaseTags`：可用阶段标签
* `RequiredEnemyRoleTags`：要求敌人自身具备的定位标签
* `RequiredEnemyMechanicTags`：要求敌人自身具备的机制标签
* `Notes`：补充备注

**说明**
* 敌方意图效果统一复用 `UBattleEffectDefinition` 基类族，不再单独发明 `EnemyEffectDefinition`
* 多段攻击由 `UBattleEffect_Damage` 子类增加 `HitCount` 或 `DamageSegments` 承载，不在意图层重复拆字段
* 召援、护盾、施加中毒 / 腐蚀、蓄势增伤都通过效果列表表达
* `EnemyIntentDefinition` 只负责“这回合想做什么”，不负责重新发明一套伤害结构

### 9.6 StatusDefinition
**用途**  
描述可在战斗中挂载的状态模板。

**推荐载体**  
`PrimaryDataAsset` 或 `DataTable`

**必填字段**
* `StatusId`：状态系统唯一 ID
* `DisplayName`：状态显示名称
* `StatusCategory`：状态分类，协议见 `8.29 StatusCategory`
* `MaxStacks`：最大层数
* `DurationType`：默认持续方式
* `StatusMergeRule`：同名状态归并规则，协议见 `8.29.1 StatusMergeRule`
* `StatusStackRule`：命中已有实例时的叠层或覆盖规则，协议见 `8.29.2 StatusStackRule`
* `StatusRefreshRule`：重复获得时的持续值刷新规则，协议见 `8.29.3 StatusRefreshRule`
* `SummaryText`：状态摘要文本

**可选字段**
* `Description`：完整说明文本
* `Icon`：状态图标资源，外部引用类型见 `8.41 ExternalReferenceBoundary`
* `OnApplyEffects`：获得时效果
* `OnTickEffects`：结算时效果
* `OnExpireEffects`：失效时效果
* `Tags`：状态标签

**说明**
* `刀势 / 药引` 这类角色专属状态与 `中毒 / 腐蚀 / 易伤 / 免疫` 这类通用状态统一走同一模板
* `剑阵` 属于衍生牌体系，不应通过 `StatusDefinition` 或 `BattleStatusInstance` 承载
* 状态负责“战斗中当前挂着什么”，不直接替代卡牌关键词
* 若状态的主要效果依赖 `SourceUnitId` 独立结算，建议使用 `StatusMergeRule=ByOwnerAndSource`
* 首版常见单层状态建议使用 `StatusStackRule=RefreshOnly`
* 首版常见回合持续状态建议使用 `StatusRefreshRule=KeepLonger`

**最小示例**
```text
StatusId: status_poison
DisplayName: 中毒
StatusCategory: Debuff
MaxStacks: 待补
DurationType: Turn
StatusMergeRule: ByOwnerAndSource
StatusStackRule: AddAndClamp
StatusRefreshRule: KeepLonger
SummaryText: 在玩家结束回合后的敌方行动前窗口结算持续伤害。
OnTickEffects:
  - effect_status_poison_tick
Tags: [DamageOverTime]
```

* `腐蚀` 默认复用与 `中毒` 相同的模板结构，只通过 `StatusId / DisplayName / SummaryText` 与默认归属对象区分。

```text
StatusId: status_dao_shi
DisplayName: 刀势
StatusCategory: Signature
MaxStacks: 待补
DurationType: Battle
StatusMergeRule: ByOwner
StatusStackRule: AddAndClamp
StatusRefreshRule: NoRefresh
SummaryText: 按层数累积；打出攻击牌时默认消耗 1 层，使该牌额外造成 1 点削韧。
Tags: [Breaker, CharacterResource]
```

### 9.7 PassiveDefinition
**用途**  
描述需要持续生效、且通常应在状态栏或日志中可见的被动模板。

**推荐载体**  
`PrimaryDataAsset`

**必填字段**
* `PassiveId`：被动系统唯一 ID
* `DisplayName`：被动显示名称
* `PassiveCategory`：被动分类，协议见 `8.30 PassiveCategory`
* `DurationType`：默认持续方式
* `SummaryText`：摘要文本

**可选字段**
* `Description`：完整说明文本
* `Icon`：图标资源，外部引用类型见 `8.41 ExternalReferenceBoundary`
* `Effects`：被动自带效果列表
* `MaxStacks`：最大层数
* `Tags`：被动标签

**说明**
* 能力牌带来的持续规则、遗物带来的战斗内被动、首领战机制被动，统一走同一模板

### 9.8 RelicDefinition
**用途**  
描述遗物的静态模板与战斗内触发规则。

**推荐载体**  
`PrimaryDataAsset`

**必填字段**
* `RelicId`：遗物系统唯一 ID
* `DisplayName`：遗物显示名称
* `Rarity`：遗物稀有度
* `RulesText`：遗物规则文本
* `Effects`：遗物效果列表

**可选字段**
* `Description`：补充说明文本
* `Icon`：图标资源，外部引用类型见 `8.41 ExternalReferenceBoundary`
* `RelicThemeTags`：遗物主题标签，协议见 `8.38 RelicThemeTag`
* `BattleStartEffects`：兼容字段，表达当前最小 battle-start 数值触发
* `PlayerTurnStartEffects`：兼容字段，表达当前最小玩家回合开始数值触发
* `RuntimeTriggers`：通用遗物触发协议；第一版只落地 `Domain=Battle / Window=PlayerTeamTookHealthDamage / Effect=GainShield`
* `bUnique`：是否唯一
* `AudioProfileId`：音频配置，外部引用类型见 `8.41 ExternalReferenceBoundary`
* `VfxProfileId`：特效配置，外部引用类型见 `8.41 ExternalReferenceBoundary`

**说明**
* 遗物效果统一复用 `UBattleEffectDefinition` 基类族
* `每回合第一次 / 每场第一次` 这类限制不写死在字段名里，由运行时 `BattleRelicRuntimeState` 记录触发次数与轮次标记
* `RuntimeTriggers` 不命名为 `BattleTriggers`，以便后续承载 Run domain；当前 `BattleStartEffects / PlayerTurnStartEffects` 仍保留兼容，后续可逐步收敛
* 当前首发遗物中出现的 `额外削韧 / 获得护盾 / 抽牌 / 获得 AP / 获得 EP / 回复生命 / 降低压力 / Break 后费用修正` 都必须能直接由 `Effects` 表达

### 9.9 EventDefinition
**用途**  
描述事件节点本体与可选项结构。

**推荐载体**  
`PrimaryDataAsset`

**必填字段**
* `EventId`：事件系统唯一 ID
* `DisplayName`：事件显示名称
* `EventCategory`：事件分类，协议见 `8.31 EventCategory`
* `Description`：事件正文文本
* `Options`：事件选项列表

**EventOptionDefinition 必填字段**
* `OptionId`：选项系统唯一 ID
* `DisplayName`：选项标题
* `ResultText`：结果文本
* `Effects`：选项效果列表

**EventOptionDefinition 可选字段**
* `Requirements`：`EventRequirementDefinition` 列表
* `Costs`：`EventCostDefinition` 列表
* `bOneShot`：是否一次性选项

**说明**
* 事件不复用 `UBattleEffectDefinition`，而是单独使用 `URunEffectDefinition`
* 当前首发事件必须至少覆盖：`GainGold / LoseTeamHP / HealTeamHP / HealTeamMissingPercent / AdjustStress / GainRandomRelic / RemoveCard / UpgradeCard / GainSpecificCard / GainRandomCard`
* 角色成长事件通过 `Requirements` 读取队伍中是否包含目标角色
* `Requirements` 用于判断“这个选项能不能出现或能不能被点”
* `Costs` 用于判断“选择该选项前先支付什么”
* 选择后发生的奖励、伤害、回血、加压、删牌、升级等结果统一走 `Effects`

#### 9.9.1 EventRequirementDefinition
**用途**  
描述事件选项的前置条件，避免把“是否包含角色”“是否存在可升级牌”“是否存在可选目标”写成脚本特判。

**建议字段**
* `RequirementType`
* `RunTargetScope`
* `RunTargetRule`
* `TargetUnitId`
* `TargetCardType`
* `TargetTag`
* `CardSelectionScope`
* `CardSelectionRule`
* `CompareOp`
* `ExpectedValue`
* `Notes`

**说明**
* `HasUnit` 优先配合 `TargetUnitId` 使用
* `HasUpgradeableCard / HasRemovableCard / HasCard` 优先配合 `TargetUnitId / TargetCardType / TargetTag / CardSelectionScope / CardSelectionRule`
* `HasValidTarget` 优先配合 `RunTargetScope / RunTargetRule`
* 金币、生命这类资源门槛也可通过 `GoldAtLeast / TeamHpAtLeast + ExpectedValue` 表达

#### 9.9.2 EventCostDefinition
**用途**  
描述事件选项在确认时需要预先支付的代价。

**建议字段**
* `CostType`
* `RunTargetScope`
* `RunTargetRule`
* `TargetUnitId`
* `Amount`
* `Notes`

**说明**
* `Gold` 通常不需要目标范围，直接支付队伍金币
* `TeamHP` 默认作用于队伍共享生命
* `CharacterStress` 可通过 `RunTargetScope / RunTargetRule / TargetUnitId` 指定由谁承担压力代价
* 若某项生命、压力变化属于结果结算而非前置支付，则不写在 `Costs`，而写进 `Effects`

### 9.10 RunEffectDefinition
**用途**  
描述事件、商店、节点奖励等局外流程效果。

**推荐载体**  
`Instanced UObject`

**必填字段**
* `EffectId`：效果条目唯一 ID
* `EffectType`：局外效果类型

**可选字段**
* `FlatValue`：固定数值
* `PercentValue`：百分比数值
* `RunTargetScope`：局外目标范围
* `RunTargetRule`：局外选目标方式
* `TargetUnitId`：目标单位 ID
* `TargetCardId`：目标卡牌 ID
* `TargetCardType`：目标卡牌类型
* `TargetRarity`：目标稀有度
* `TargetPoolId`：目标池 ID
* `TargetTag`：目标标签
* `CardSelectionScope`：选牌范围
* `CardSelectionRule`：选牌规则
* `Count`：数量
* `Notes`：补充备注

**首批建议类型**
* `GainGold`
* `LoseTeamHP`
* `HealTeamHP`
* `HealTeamMissingPercent`
* `AdjustStress`
* `GainRandomRelic`
* `RemoveCard`
* `UpgradeCard`
* `GainSpecificCard`
* `GainRandomCard`

**说明**
* `UpgradeCard / GainSpecificCard / GainRandomCard / RemoveCard` 等效果不应依赖事件特判选牌
* `TargetUnitId` 在 run 层默认使用 `TemplateUnitId` 或 `team_player`，不使用战斗中的 `RuntimeUnitId`
* 需要“升级某角色的一张攻击牌”“获得 1 张随机剑阵牌”这类效果时，优先组合 `TargetUnitId / TargetCardType / TargetTag / TargetPoolId / CardSelectionScope / CardSelectionRule`
* 需要“全部角色”“随机 1 名未崩溃角色”“选择 1 名角色”这类效果时，优先组合 `RunTargetScope / RunTargetRule / Count`

### 9.11 UltimateDefinition
**用途**  
描述角色奥义的静态模板。

**推荐载体**  
`PrimaryDataAsset`

**必填字段**
* `UltimateId`：奥义系统唯一 ID
* `OwnerUnitId`：所属单位 ID
* `DisplayName`：奥义显示名称
* `BaseCostEP`：基础 EP 消耗
* `RulesText`：规则文本
* `Effects`：奥义效果列表

**可选字段**
* `Description`：补充说明文本
* `AudioProfileId`：音频配置，外部引用类型见 `8.41 ExternalReferenceBoundary`
* `VfxProfileId`：特效配置，外部引用类型见 `8.41 ExternalReferenceBoundary`

### 9.12 BattleEncounterDefinition
**用途**  
描述一个可进入章节节点池的战斗遭遇模板。  
该定义只负责“这场战斗刷什么敌人、用什么规则配置、属于哪个节点层级”，不负责记录玩家当前队伍。

**推荐载体**  
`PrimaryDataAsset`

**必填字段**
* `EncounterId`：遭遇系统唯一 ID
* `DisplayName`：遭遇显示名称
* `ChapterId`：所属章节 ID
* `EncounterTier`：普通 / 精英 / 首领
* `RecommendedStage`：推荐出现阶段
* `RuleConfigId`：本场使用的战斗规则配置
* `EnemyRoster`：敌方编组列表

**可选字段**
* `PreviewText`：节点预览文本
* `EncounterThemeTags`：遭遇主题标签，协议见 `8.39 EncounterThemeTag`
* `EncounterStructureTags`：遭遇结构标签，协议见 `8.40 EncounterStructureTag`
* `EncounterWeight`：节点池权重
* `bUniquePerRun`：单局内是否只出现一次
* `BackgroundId`：战斗背景资源，外部引用类型见 `8.41 ExternalReferenceBoundary`
* `MusicId`：战斗音乐资源，外部引用类型见 `8.41 ExternalReferenceBoundary`
* `RewardProfileId`：奖励配置，外部引用类型见 `8.41 ExternalReferenceBoundary`

**EnemyRosterEntry 最小字段**
* `EnemyId`
* `PositionIndex`
* `SpawnWave`
* `EntryTags`：站位或波次辅助标签

**说明**
* `BattleEncounterDefinition` 不再记录 `PartyCharacters`，玩家队伍属于 run 状态，不属于遭遇模板
* 同一个敌人模板应可被多个遭遇复用，区别主要体现在编组、站位、波次与规则配置
* 首领战中需要召援、阶段转换、固定循环时，优先通过首领自己的 `EnemyIntentDefinition / PhaseConfigId` 表达，而不是把整套行为写死在遭遇模板里
* `RecommendedStage` 用于节点流优先分配，不代表硬锁关卡阶段
* `RewardProfileId` 当前只负责桥接战斗结算奖励模块，不在本文档中展开其内部结构

**最小示例**
```text
EncounterId: enc_blackwind_road_01
DisplayName: 山道拦截
ChapterId: chapter_blackwind_road
EncounterTier: Normal
RecommendedStage: Early
RuleConfigId: rule_default_ch1
EnemyRoster:
  - EnemyId: enemy_bandit_blade
    PositionIndex: 1
    SpawnWave: 1
    EntryTags: [Front]
  - EnemyId: enemy_bandit_crossbow
    PositionIndex: 2
    SpawnWave: 1
    EntryTags: [Back]
```

### 9.13 BattleRuleConfig
**用途**  
描述一场战斗使用的核心规则常量与基础判定参数。

**推荐载体**  
`PrimaryDataAsset`

**必填字段**
* `RuleConfigId`：规则配置唯一 ID
* `InitialAP`：战斗初始 AP
* `InitialHandSize`：战斗初始抽牌数
* `HandLimit`：手牌上限
* `MaxEP`：EP 上限
* `EndTurnEpGain`：回合结束获得的 EP
* `OnHitEpGain`：受击后获得的 EP
* `BaseCardEpGain`：普通牌固定基础 EP
* `BreakRewardAP`：触发 Break 后返还的 AP
* `NormalCardInitiativeEventCount`：普通牌默认触发的先机减少事件次数
* `CollapsedCardInitiativeEventCount`：崩溃卡默认触发的先机减少事件次数
* `bUltimateTriggersInitiativeEvent`：奥义是否触发先机减少事件
* `AwakenThresholdByCollapseCount`：按单角色当前累计崩溃次数读取的苏醒阈值表
* `DirectAwakenChanceByRemainingCount`：按距离当前苏醒阈值剩余次数读取的直苏概率表
* `AwakenStressResetValue`：角色苏醒后压力重置值
* `CollapseCardAwakenGain`：每打出 1 张崩溃卡增加的苏醒计数
* `StressHpLossPerPoint`：每损失多少点生命转化为 1 点压力
* `StressHealPerPoint`：每恢复多少点生命转化为 1 点减压
* `MinStressChangePerEvent`：发生有效生命变化时的最小压力变化
* `MaxStressGainPerHit`：单次独立受击事件的最大加压值
* `StressRandomProtectionCount`：连续命中同一角色多少次后优先换人
* `DamageToBreakCap`：伤害转削韧的单段上限

**可选字段**
* `Notes`：补充备注

**说明**
* `BattleRuleConfig` 只保留一套主字段，不再接受 `MinStressChangePerEvent / StressChangeMinPerEvent` 这类同义字段并存
* `DirectAwakenChanceByRemainingCount` 已覆盖“剩余 1 次时 10%”这类概率映射，不再额外保留单独的近阈值字段
* `NormalCardInitiativeEventCount / CollapsedCardInitiativeEventCount / bUltimateTriggersInitiativeEvent` 用于直接承载 [Battle_Rules.md](Battle_Rules.md) 中关于先机减少事件的核心规则
* 压力安全区、危险区这类 UI 或 AI 判断阈值不放进 `BattleRuleConfig` 主结构
* Break 的默认承伤与行动取消逻辑优先直接服从 [Battle_Rules.md](Battle_Rules.md)，不在首版再补一组推断配置字段
* 规则配置应优先描述“全局常量与主判定参数”，不把敌人个体差异混入这一层

**最小示例**
```text
RuleConfigId: rule_default_ch1
InitialAP: 3
InitialHandSize: 5
HandLimit: 10
  MaxEP: 70
  EndTurnEpGain: 3
  OnHitEpGain: 4
  BaseCardEpGain: 1
BreakRewardAP: 1
NormalCardInitiativeEventCount: 1
CollapsedCardInitiativeEventCount: 1
bUltimateTriggersInitiativeEvent: false
AwakenThresholdByCollapseCount:
  1: 3
  2: 5
  3: 7
DirectAwakenChanceByRemainingCount:
  2: 0.0
  1: 0.1
AwakenStressResetValue: 10
CollapseCardAwakenGain: 1
StressHpLossPerPoint: 5
StressHealPerPoint: 8
MinStressChangePerEvent: 1
MaxStressGainPerHit: 3
StressRandomProtectionCount: 2
DamageToBreakCap: 6
```

## 10. 运行时结构
### 10.1 BattleState
**用途**  
描述一场战斗运行中的完整状态容器。

**核心字段**
* `BattleId`：战斗实例唯一 ID
* `EncounterId`：本场对应的遭遇模板 ID
* `RuleConfigId`：本场使用的规则配置
* `CurrentRound`：当前回合数
* `CurrentAP`：当前 AP
* `CurrentEP`：当前 EP
* `TeamCurrentHP`：队伍当前生命
* `TeamMaxHP`：队伍最大生命
* `Characters`：玩家角色状态列表
* `Enemies`：敌人状态列表
* `DeckState`：共享牌组状态
* `StatusInstances`：全场状态实例列表
* `PassiveInstances`：全场被动实例列表
* `RelicRuntimeStates`：遗物运行时状态列表
* `BattleLogEntries`：战斗日志

**说明**
* `StatusInstances` 与 `PassiveInstances` 是运行时唯一主记录
* `EncounterId` 用于回查本场遭遇模板、节点来源与奖励桥接字段
* 角色、敌人、队伍公共状态统一通过实例内的 `OwnerUnitId / SourceUnitId` 区分归属
* `Characters / Enemies` 中的每个运行时条目都必须带自己的 `RuntimeUnitId`
* 不在 `BattleCharacterState / BattleEnemyState` 中重复保存完整状态对象，避免双真相

### 10.2 BattleCharacterState
**用途**  
描述单个角色在本场战斗中的动态状态。

**核心字段**
* `RuntimeUnitId`
* `CharacterId`
* `CurrentStress`
* `bCollapsed`
* `CurrentAwakenCount`
* `CollapseCount`
* `RuntimeAttack`
* `RuntimeDefense`
* `RuntimeBreakRate`

**说明**
* `RuntimeUnitId` 是运行时唯一单位句柄；`CharacterId` 只用于回查角色模板
* 角色自己的专属状态，例如 `刀势 / 药引`，通过 `BattleState.StatusInstances` 中 `OwnerUnitId=该角色 RuntimeUnitId` 的条目读取
* 与队伍共享生命直接相关的状态，例如 `护体 / 易伤 / 免疫`，通过 `BattleState.StatusInstances` 中 `OwnerUnitId=team_player` 的条目读取
* `中毒 / 流血 / 灼烧` 当前只作用于敌方单位，因此在首版不应写入 `OwnerUnitId=team_player` 的玩家侧状态实例
* `腐蚀` 若作用于玩家侧共享生命，首版建议写入 `OwnerUnitId=team_player`

### 10.3 BattleEnemyState
**用途**  
描述单个敌人在本场战斗中的动态状态。

**核心字段**
* `RuntimeUnitId`
* `EnemyId`
* `PositionIndex`
* `SpawnWave`
* `CurrentHP`
* `CurrentBreakValue`
* `CurrentInitiative`
* `CurrentIntentId`
* `bActedThisRound`

**说明**
* `RuntimeUnitId` 是运行时唯一单位句柄；`EnemyId` 只用于回查敌人模板
* `PositionIndex` 表示敌人当前有效站位；若战斗中存在换位效果，应直接更新该值
* `SpawnWave` 表示该敌人进入战斗的波次来源；召援与多波次敌人都应写入实际波次
* 敌人身上的状态与被动统一通过 `BattleState.StatusInstances / PassiveInstances` 中 `OwnerUnitId=该敌人 RuntimeUnitId` 的条目读取
* 同时 Break、同时归零先机或同窗口内需要按站位顺序处理时，统一读取 `PositionIndex`

### 10.4 TeamDeckState
**用途**  
描述共享牌组与各牌区的当前状态。

**核心字段**
* `DrawPileCardInstanceIds`
* `HandCardInstanceIds`
* `DiscardPileCardInstanceIds`
* `OngoingZoneCardInstanceIds`
* `ConsumePileCardInstanceIds`

**说明**
* `持续区` 用于承载已生效的能力牌与其他持续存在的卡牌实例
* `消耗` 与 `消灭` 在首版统一进入 `ConsumePileCardInstanceIds`
* `开战` 关键词导致的置顶处理，应在战斗初始化时直接写入 `DrawPileCardInstanceIds`

### 10.5 BattleCardInstance
**用途**  
描述一张牌在本场战斗中的实例状态。

**核心字段**
* `CardInstanceId`：卡牌实例唯一 ID
* `CardId`：模板 ID
* `RuntimeOwnerUnitId`：当前所属单位
* `RuntimeCostAP`：当前 AP 消耗
* `RuntimeKeywords`：当前实际关键词
* `TempModifiers`：临时修正集合
* `RecycleCount`：`回收X` 当前剩余次数
* `bRetained`：当前回合结束后是否保留

**说明**
* `RuntimeOwnerUnitId` 一律使用 `RuntimeUnitId` 或 `ReservedTeamUnitId`
* 通用卡、衍生牌、敌方塞入牌都通过 `RuntimeOwnerUnitId` 决定按谁的属性与联动结算
* `保留 / 消耗 / 回收X / 主导` 的当前实际状态优先维护在实例层，不回写模板
* 崩溃卡形态不作为实例常驻字段保存；是否以崩溃卡表现，改由 `RuntimeOwnerUnitId` 对应角色是否处于 `bCollapsed`，以及该牌当前是否位于手牌区动态推导
* 表现层若需要临时高亮或替换文案，应基于 `RulesText + RuntimeKeywords + TempModifiers` 动态生成，不单独保存一份 `RuntimeRulesText`

### 10.6 BattleStatusInstance
**用途**  
描述一个状态在战斗中的实际挂载情况。

**核心字段**
* `StatusInstanceId`
* `StatusId`
* `OwnerUnitId`
* `SourceUnitId`
* `CurrentStacks`
* `RemainingDuration`
* `DurationType`
* `AppliedSequence`
* `OutgoingDamagePercentPerStack`
* `IncomingTeamHealthDamageReductionPercentPerStack`
* `bExpireAtPlayerTurnEnd`
* `bConsumeOnSuccessfulOwnerDamage`
* `bConsumeOnPreventedTeamHealthDamage`

### 10.7 BattlePassiveInstance
**用途**  
描述一个被动在战斗中的实际生效情况。

**核心字段**
* `PassiveInstanceId`
* `PassiveId`
* `OwnerUnitId`
* `SourceUnitId`
* `CurrentStacks`
* `RemainingDuration`
* `DurationType`
* `AppliedSequence`

### 10.8 BattleRelicRuntimeState
**用途**  
描述单个遗物在本场战斗中的触发记录与临时计数。

**核心字段**
* `RelicId`
* `DisplayId`
* `DisplayName`
* `RuntimeTriggers`
* `TriggerStates`
* `CustomCounters`（后续复杂遗物再补）

**说明**
* `BattleRelicRuntimeState` 只负责这件遗物的总运行时容器，不直接把多条效果混成一组计数
* `每回合第一次`、`每场第一次`、`Break 后下一张攻击牌费用 -1 AP` 这类遗物效果，都优先记录在按效果拆分的运行时子状态中

### 10.8.1 BattleRelicEffectRuntimeState
**用途**  
描述单条遗物效果在本场战斗中的触发记录。

**核心字段**
* `EffectId`
* `TriggeredCountThisRound`
* `TriggeredCountThisBattle`
* `LastTriggeredRound`
* `PendingConsumeMarks`

### 10.9 RunPersistentCharacterState
**用途**  
描述跨战斗保留的角色状态。

**核心字段**
* `CharacterId`
* `CurrentStress`
* `bCollapsed`
* `CurrentAwakenCount`
* `CollapseCount`

## 11. 首版最小落地范围
首批建议先完成以下 schema：
* `BattleRuleConfig`
* `CharacterDefinition`
* `CardDefinition`
* `BattleEffectDefinition`
* `EnemyDefinition`
* `EnemyIntentDefinition`
* `StatusDefinition`
* `PassiveDefinition`
* `RelicDefinition`
* `EventDefinition`
* `EventRequirementDefinition`
* `EventCostDefinition`
* `RunEffectDefinition`
* `UltimateDefinition`
* `BattleEncounterDefinition`
* `BattleState`
* `BattleCardInstance`
* `BattleStatusInstance`
* `BattleRelicRuntimeState`
