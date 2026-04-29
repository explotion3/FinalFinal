# Unreal 源码架构方案

## 1. 文档定位

本文档用于约束 Unreal 工程中的模块拆分、依赖方向、核心类职责和第一版竖切落点。

本文档不记录具体开发流水账，也不展开具体卡牌、遗物、敌人和 UI 布局细节。  
具体规则以 `Battle_Rules.md`、`Combat_Data_Schema_v2.md`、`Card_Design_Guide.md`、`Code_Function_Requirements.md` 为准。

---

## 2. 模块目标结论

项目按职责拆分为：

```text
FinalCore
FinalData
FinalBattle
FinalRun
FinalApp
FinalEditor
```

推荐依赖方向：

```text
FinalCore
  ↓
FinalData
  ↓
FinalBattle / FinalRun
  ↓
FinalApp
  ↓
FinalEditor
```

核心原则：

- `FinalCore` 只放通用基础类型和工具。
- `FinalData` 只放静态定义、配置、Schema 和数据校验辅助。
- `FinalBattle` 负责单场战斗权威规则。
- `FinalRun` 负责一次 Run 的持久状态、路线、奖励和成长。
- `FinalApp` 负责运行时流程编排、UI 桥接和世界表现。
- `FinalEditor` 负责编辑器工具、导入、生成和校验。
- `FinalBattle` 与 `FinalRun` 不应互相直接依赖。
- 战斗与 Run 的协作通过数据定义、快照、命令、结果和事实记录完成。

---

## 3. 模块依赖图

推荐依赖关系：

```text
FinalCore
  └─ 基础枚举、Id、通用 Result、随机工具、日志分类

FinalData
  └─ 依赖 FinalCore
  └─ 卡牌、角色、敌人、状态、遗物、成长、进化、数值配置

FinalBattle
  └─ 依赖 FinalCore / FinalData
  └─ 单场战斗规则、BattleState、出牌结算、压力临界状态机

FinalRun
  └─ 依赖 FinalCore / FinalData
  └─ RunPersistentState、路线、奖励、角色升级、卡牌实例进化

FinalApp
  └─ 依赖 FinalCore / FinalData / FinalBattle / FinalRun
  └─ GameInstanceSubsystem、Flow、UI、World Presentation、SaveGame 协调

FinalEditor
  └─ Editor-only
  └─ 依赖运行时模块
  └─ 数据导入、Commandlet、Validation、编辑器菜单
```

禁止方向：

```text
FinalCore -> FinalData / FinalBattle / FinalRun / FinalApp
FinalData -> FinalBattle / FinalRun / FinalApp
FinalBattle -> FinalRun
FinalRun -> FinalBattle
运行时模块 -> FinalEditor
```

---

## 4. 模块职责

### 4.1 FinalCore

职责：

- 项目通用枚举、轻量值类型和 Id 类型。
- 通用 Result / Error Code / 日志分类。
- 通用随机接口和种子封装。
- 不依赖游戏业务模块。

示例内容：

```text
FCardId
FCharacterId
FEnemyId
FStatusId
FRunSeed
FFinalResult
```

不应放入：

- 卡牌规则。
- 战斗状态。
- Run 状态。
- UI 类型。
- DataAsset 业务定义。

---

### 4.2 FinalData

职责：

- 静态数据定义。
- 数据表 / DataAsset Schema。
- 数值配置。
- 数据校验辅助。
- Runtime 只读数据查询接口。

第一版重点定义：

```text
CardDefinition
CharacterDefinition
EnemyDefinition
StatusDefinition
RelicDefinition

CharacterGrowthConfig
GrowthChoiceDefinition
CardEvolutionDefinition
StressRuleConfig
```

后续扩展定义：

```text
CardGemDefinition
GemSlotRuleDefinition
CardUltimateEvolutionDefinition
```

原则：

- `FinalData` 不持有 Run 内实例状态。
- `FinalData` 不结算战斗。
- `FinalData` 不决定奖励结果。
- `FinalData` 只提供可配置的静态定义和查询能力。

---

### 4.3 FinalBattle

职责：

- 单场战斗权威状态。
- 出牌、抽牌、弃牌、消耗、生成和复制规则。
- 伤害、治疗、护盾、状态、Break、先机等结算。
- 压力 `Normal / Critical / Collapse` 状态机。
- 崩溃卡、苏醒流程。
- 战斗事实记录。

第一版重点：

```text
BattleState
BattleCharacterState
BattleEnemyState
BattleCardInstance
BattleEvent
BattleFact
BattleResult
```

压力临界职责：

```text
StressValue
StressCap
CriticalThreshold = ceil(StressCap * CriticalRatio)
StressState = Normal / Critical / Collapse
EnterCritical
EnterCollapse
Awaken
```

`FinalBattle` 可以记录：

```text
某角色打出所属牌
某角色造成 Break
某角色击杀敌人
某角色承受压力
某角色进入 Critical
某角色进入 Collapse
```

`FinalBattle` 不负责：

- 角色升级。
- 成长三选一候选生成。
- 属性成长应用。
- 永久替换 `RunCardInstance.CurrentCardId`。
- Run 路线、奖励、商店、事件。
- 保存游戏。

---

### 4.4 FinalRun

职责：

- 一次 Run 的持久状态。
- 地图路线、节点、奖励、商店、事件。
- 角色突破值、角色升级和成长三选一。
- 属性成长应用。
- `RunCardInstance` 管理。
- 卡牌进化和绝学化应用。
- 战斗结果消费。

第一版重点：

```text
RunState
RunPersistentCharacterState
RunCardInstance
PendingGrowthChoice
GrowthChoiceInstance
```

角色成长职责：

```text
BreakthroughValue
BreakthroughRequiredValue
CharacterLevel
RootBone
Insight
KillingIntent
```

卡牌实例进化职责：

```text
InstanceId
BaseCardId
CurrentCardId
EvolutionStage
OwnerCharacterId
```

原则：

- 卡牌永久成长发生在 `FinalRun`。
- `BaseCardId` 表示原始身份。
- `CurrentCardId` 表示当前规则版本。
- `FinalRun` 可以根据 `BattleResult / BattleFact` 增加成长进度。
- `FinalRun` 不直接执行战斗结算。
- `FinalRun` 不直接操控 Widget。

---

### 4.5 FinalApp

职责：

- GameInstanceSubsystem。
- 游戏流程编排。
- Battle / Run Session 生命周期管理。
- UI ViewModel / WidgetController。
- 世界表现桥接。
- SaveGame 协调。

示例系统：

```text
UFinalGameInstanceSubsystem
UFinalBattleFlowSubsystem
UFinalRunFlowSubsystem
UFinalUISubsystem
UFinalSaveGameCoordinator
UFinalRunGrowthChoiceOverlayScreen
```

原则：

- `FinalApp` 不做权威规则。
- `FinalApp` 不直接修改 `BattleState` 和 `RunPersistentState` 私有结构。
- UI 只通过 Snapshot / ViewModel / Command 与规则层交互。
- World Actor 只表现规则结果，不反向决定规则。
- 独立 Growth overlay 也只是 `RunSnapshot.PendingGrowthChoice` 的只读投影；真正的成长应用仍然只发生在 `FinalRun`

---

### 4.6 FinalEditor

职责：

- 编辑器工具。
- Commandlet。
- 数据导入。
- 数据生成。
- Data Validation。
- Prototype 内容生成辅助。
- 编辑器菜单和开发工具。

原则：

- `FinalEditor` 只在 Editor 构建中启用。
- `FinalEditor` 不参与运行时规则真相。
- 导入工具应输出 `FinalData` 可读取的静态定义。
- 复杂编辑器流程不应泄露到 Runtime 模块。

---

## 5. 新成长方向下的模块边界

### 5.1 临界状态机边界

`FinalBattle` 负责：

```text
Normal / Critical / Collapse
临界阈值计算
首次越过临界保护
临界期间继续受压的崩溃判定
压力降低后的临界退出
崩溃和苏醒流程
```

`FinalRun` 不负责临界即时结算。

第一版不默认实现：

```text
角色专属临界收益
临界牌
临界流派遗物
临界专属事件
```

这些属于后续内容扩展。

---

### 5.2 角色升级成长三选一边界

`FinalRun` 负责：

```text
读取角色突破值
判断角色是否升级
生成成长三选一
应用被选择的成长
清除 PendingGrowthChoice
```

当前 Step 4 已落地的最小口径：

```text
AddBreakthroughValue()
-> 累计 BreakthroughValue
-> 若当前没有 PendingGrowthChoice 且达到阈值，则只触发一次升级
-> 生成 3 个 deterministic 候选
-> 写入 RunState.PendingGrowthChoice
```

这一步仍未接入：

```text
RunCommand
RunEvent
UI 提交
```

Step 5 当前已补：

```text
SelectGrowthChoice (RunCommand)
-> 通过 ChoiceInstanceId 选择 pending growth
-> 应用属性成长或 RunCardInstance 进化
-> 清空 PendingGrowthChoice
-> 输出 GrowthChoiceApplied 事件
```

当前仍保持：

```text
不会因为剩余突破值足够而自动连锁生成下一组候选
```

成长候选类型：

```text
属性成长：根骨 / 悟性 / 杀意 +1
卡牌进化：基础卡 -> 进化卡
后续扩展：进化卡 -> 绝学卡
```

`FinalBattle` 只提供战斗事实，不直接触发升级 UI。

`FinalApp` 负责展示成长选择 UI，并把玩家选择提交给 `FinalRun`。

当前 `FinalApp` 落地口径：

```text
RunFlowSubsystem
-> 统一读取 RunSnapshot / RunEvent
-> 若存在 PendingGrowthChoice，则优先呈现 FinalRunGrowthChoiceOverlayScreen
-> 否则继续走统一 FinalRunFlowOverlayScreen
```

---

### 5.3 RunCardInstance 进化边界

卡牌实例成长归 `FinalRun` 管理。

```text
BaseCardId    = 原始卡牌身份
CurrentCardId = 当前规则版本
```

当卡牌进化时：

```text
FinalRun 更新 RunCardInstance.CurrentCardId
FinalApp 作为桥接层定位目标 RunCardInstanceId
若当前存在 active battle，则通知 FinalBattle 原地刷新直接来源 BattleCardInstance
若当前没有 active battle，则下次建立 BattleCardInstance 时读取新的 CurrentCardId
```

当前首版兼容桥接仍保留：

```text
BuildBattleStartRequest()
-> 从 RunDeck 中每张实例派生 DeckEntries
-> DeckEntries 至少包含 SourceRunCardInstanceId / EffectiveCardId / OwnerCharacterId
-> 同时继续兼容派生 DeckCardIds
```

当前首版战斗中进化已支持：

```text
当前 hand / draw / discard / consume / ongoing 中直接来源于目标 RunCardInstanceId 的 BattleCardInstance 原地刷新
已经完成的历史结算不回滚
generated / temporary / copied cards 默认不联动
刷新只重建基础定义字段，不承诺保留未来 temp modifiers
```

---

### 5.4 强化珠与强化槽边界

强化珠是后续扩展，不进入第一版必须实现。

预留边界：

```text
FinalData:
- CardGemDefinition
- GemSlotRuleDefinition

FinalRun:
- EquippedGems
- GemSlots
- 珠子镶嵌、替换、合成

FinalBattle:
- 只读取最终合成后的卡牌效果或运行时修正
```

第一版代码可以先只实现 `RunCardInstance.BaseCardId / CurrentCardId / EvolutionStage`。

---

### 5.5 UI / 表现层边界

`FinalApp` 负责：

```text
成长三选一界面
卡牌进化结果展示
临界状态提示
手牌刷新表现
战斗日志展示
```

`FinalApp` 不负责：

```text
成长候选生成规则
压力临界判定
卡牌永久进化规则
```

---

## 6. Public / Private 边界

建议原则：

- Public 暴露稳定 API、只读 Snapshot、命令入口和必要结构声明。
- Private 持有状态机细节、结算细节和内部工具。
- 不让 UI 层直接访问内部可变状态。
- 不让 DataAsset 持有运行时状态。
- 不让 Editor 工具依赖运行时 Private 细节。

推荐接口形态：

```text
Command
Snapshot
Event
Result
Query
```

避免：

```text
外部直接改 BattleState
外部直接改 RunPersistentState
UI 直接写卡牌实例字段
Editor 工具调用 Runtime Private 逻辑
```

---

## 7. Build.cs 依赖建议

### FinalCore

```text
PublicDependencyModuleNames:
- Core

PrivateDependencyModuleNames:
- CoreUObject
```

### FinalData

```text
PublicDependencyModuleNames:
- Core
- CoreUObject
- Engine
- FinalCore
```

### FinalBattle

```text
PublicDependencyModuleNames:
- Core
- CoreUObject
- Engine
- FinalCore
- FinalData
```

### FinalRun

```text
PublicDependencyModuleNames:
- Core
- CoreUObject
- Engine
- FinalCore
- FinalData
```

### FinalApp

```text
PublicDependencyModuleNames:
- Core
- CoreUObject
- Engine
- UMG
- FinalCore
- FinalData
- FinalBattle
- FinalRun
```

### FinalEditor

```text
PrivateDependencyModuleNames:
- Core
- CoreUObject
- Engine
- UnrealEd
- AssetTools
- FinalCore
- FinalData
- FinalBattle
- FinalRun
- FinalApp
```

实际 `Build.cs` 可根据 UE 版本和使用的子系统再精简。

---

## 8. 推荐目录结构

示例：

```text
Source/
  FinalCore/
    Public/
    Private/

  FinalData/
    Public/
      Definitions/
      Schema/
      Query/
    Private/

  FinalBattle/
    Public/
      Session/
      Snapshot/
      Commands/
      Events/
    Private/
      State/
      Rules/
      Resolution/

  FinalRun/
    Public/
      Session/
      Snapshot/
      Commands/
      Events/
    Private/
      State/
      Growth/
      Rewards/
      Map/

  FinalApp/
    Public/
      Subsystems/
      UI/
      Presentation/
    Private/
      Flow/
      ViewModels/
      Widgets/
      World/

  FinalEditor/
    Public/
    Private/
      Commandlets/
      Validation/
      Import/
      Menus/
```

---

## 9. 第一版竖切落点

第一版竖切建议只落这些内容：

### FinalData

```text
CharacterGrowthConfig
GrowthChoiceDefinition
CardEvolutionDefinition
StressRuleConfig
```

### FinalBattle

```text
压力 Normal / Critical / Collapse
BattleFact 记录
BattleCardInstance 从 CurrentCardId 建立
```

### FinalRun

```text
RunPersistentCharacterState
RunCardInstance
角色突破值
角色升级成长三选一
属性成长应用
卡牌进化应用
```

### FinalApp

```text
成长三选一 UI
卡牌进化结果展示
临界状态提示
Battle / Run Flow 桥接
```

### FinalEditor

```text
成长与进化定义的数据校验
基础 DataAsset 生成或导入辅助
```

---

## 10. 暂不放入架构文档的内容

以下内容暂时不在本文档展开：

```text
具体 HUD 布局
具体卡牌数值
具体敌人行为
具体遗物触发窗口
具体强化珠数值表
具体 Boss 机制
具体 DataValidation 检查全集
具体 DebugScreen 实现细节
```

这些内容应放在对应的设计文档、数值文档、UI 文档、编辑器工具文档或开发任务中。

---

## 11. 维护规则

修改架构文档时，应优先回答：

```text
这个职责属于哪个模块？
这个模块是否应该依赖另一个模块？
这是静态定义、Run 持久状态，还是战斗临时状态？
这个信息是规则真相，还是表现层缓存？
```

如果无法回答，应先补充 `Code_Function_Requirements.md` 或 `Combat_Data_Schema_v2.md`，再修改源码结构文档。
