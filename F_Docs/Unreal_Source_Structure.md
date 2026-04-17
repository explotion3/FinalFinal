# Unreal 源码架构方案

## 1. 文档定位
本文档用于定义 Unreal 项目的模块划分、目录结构、依赖方向与核心类职责。  
当前项目准备从零开始，因此本文件不再延续旧的 `Final` 单模块方案，而是直接给出一套更适合扩展和复用的多模块起步架构。

本文档默认服务于：
* [Code_Function_Requirements.md](Code_Function_Requirements.md)
* [GDD4.0.md](GDD4.0.md)
* [Battle_Rules.md](Battle_Rules.md)
* [Combat_Data_Schema_v2.md](Combat_Data_Schema_v2.md)
* [Status_System_Guide.md](Status_System_Guide.md)

本文档只回答：
* Unreal 模块怎么拆
* 哪类代码放哪一层
* 哪些类应该公开，哪些应该留在私有实现
* Build.cs 依赖如何保持单向

---

## 2. 目标结论
对于一个准备从零开始、并且后续会持续扩角色、敌人、遗物、事件的项目，推荐直接采用：
* `FinalCore`
* `FinalData`
* `FinalBattle`
* `FinalRun`
* `FinalApp`
* `FinalEditor`（可延后）

说明：
* 若首版一开始不想立刻建 `FinalEditor`，可以先不创建，但文档中先保留它的边界

不再推荐：
* 把全部代码塞进一个 Runtime 模块
* 在 `Battle` 内同时放 DataAsset、ViewModel、World Actor
* 让 Run、Battle、UI 互相横向引用

---

## 3. 模块图

```text
FinalCore
    ↓
FinalData
    ↓
FinalBattle      FinalRun
      \             /
       \           /
         FinalApp
             ↓
        FinalEditor
```

依赖规则：
* `FinalCore` 不依赖业务模块
* `FinalData` 只依赖 `FinalCore`
* `FinalBattle` 只依赖 `FinalCore + FinalData`
* `FinalRun` 只依赖 `FinalCore + FinalData`
* `FinalApp` 负责把 Battle、Run、UI、World 接起来
* `FinalEditor` 只服务编辑器校验与工具，不参与运行时规则真相

---

## 4. 模块职责

### 4.1 FinalCore
职责：
* 全项目共用 ID、轻量枚举、通用类型、日志分类、Gameplay Tags 注册
* 通用接口、薄协议、少量工具函数

不放：
* 卡牌效果逻辑
* DataAsset 定义
* 战斗状态
* Run 进度
* Widget 或 Actor

适合内容：
* `FCardId / FEnemyId / FEventId`
* 日志 Channel
* 共用 `UENUM`
* 基础错误码与调试标签

### 4.2 FinalData
职责：
* 承载全部静态定义资产
* 承载共享协议与数据结构映射
* 提供内容查询、数据索引、基础校验入口

放入：
* `CharacterDefinition`
* `CardDefinition`
* `BattleEffectDefinition`
* `EnemyDefinition`
* `EnemyIntentDefinition`
* `StatusDefinition`
* `PassiveDefinition`
* `RelicDefinition`
* `EventDefinition`
* `RunEffectDefinition`
* `UltimateDefinition`
* `BattleEncounterDefinition`
* `BattleRuleConfig`

不放：
* 战斗运行时实例
* 结算流程
* UI 显示逻辑

### 4.3 FinalBattle
职责：
* 承载战斗内权威状态
* 处理命令校验、规则解析、原子结算、状态窗口、敌人行动、崩溃与苏醒
* 对外只暴露“提交命令、查询状态、读取日志、订阅事件”的接口

放入：
* `BattleState`
* `BattleCharacterState`
* `BattleEnemyState`
* `TeamDeckState`
* `BattleCardInstance`
* `BattleStatusInstance`
* `BattlePassiveInstance`
* `BattleRelicRuntimeState`
* `BattleSession`
* `BattleCommandProcessor`
* `BattleResolver`
* `BattleEffectExecutor`
* `BattleStatusService`
* `BattleTurnService`
* `EnemyIntentService`
* `BattleLogService`

不放：
* Widget Controller
* 角色表现 Actor
* 地图节点推进
* 存档文件读写

### 4.4 FinalRun
职责：
* 承载单局外权威状态
* 管理节点推进、奖励、商店、事件、角色成长、构筑修正
* 在进入战斗前组装战斗输入，在战后消费战斗结果
* 对外只暴露 `RunSession` 与少量桥接请求 / 结果结构

放入：
* `RunState`
* `RunPersistentCharacterState`
* `RosterState`
* `DeckBuildState`
* `RelicInventoryState`
* `RunSession`
* `RunNodeResolver`
* `RunEventResolver`
* `RewardResolver`
* `ShopResolver`
* `GrowthResolver`

不放：
* 单张牌伤害结算
* Break 与先机规则
* UI 控件逻辑

### 4.5 FinalApp
职责：
* 连接 Unreal 世界与运行时系统
* 持有 Subsystem、UISubsystem、GameMode、PlayerController、Root Layout、Widget Controller、ViewModel
* 组织输入、场景生成、UI 刷新、页面层级、表现桥接、存档调用
* 作为 `FinalBattle / FinalRun -> UI` 的唯一桥接层

放入：
* `GameInstanceSubsystem`
* `BattleFlowSubsystem`
* `RunFlowSubsystem`
* `UISubsystem`
* `UIRootLayout`
* `SaveGameCoordinator`
* `BattleGameMode`
* `BattlePlayerController`
* `BattleDirector`
* `BattleHUDScreen`
* `ScreenBase`
* `PanelWidgetBase`
* `WidgetBase`
* `WidgetControllerBase`
* `ViewModelBase`
* `WidgetController`
* `BattleHUDViewModel`
* `HandPanelViewModel`
* `EnemyPanelViewModel`
* 表现层桥接 Actor / Component

不放：
* 权威战斗规则
* DataAsset 定义
* 直接访问 `BattleState / RunState` 私有结构的 Widget

运行时 UI 默认结论：
* 运行时主 UI 默认采用 `UMG`
* `Slate` 只用于 `UMG` 难以承载的少量自定义控件或编辑器工具
* 默认不把 `CommonUI` 作为首版基础框架
* 首批允许先保留 `BattleHUDViewModel + BattleWidgetController` 作为聚合入口，后续再拆成 Panel 级 `WidgetController / ViewModel`
* 当前代码已落地 `UISubsystem + UIRootLayout + BattleHUDScreen + Overlay / Modal` 通用容器
* 当前代码已补上 `RunFlowSubsystem`，用于根据 `RunSnapshot / RunEvent` 协调战后奖励页、节点选择页、奖励节点页、事件节点页、商店节点页与常驻 HUD 的切换
* 当前 runtime content bootstrap 已开始从 `FinalApp` 回收到 `FinalDataRegistry`：运行时优先扫描并注册项目中的 definition 资产，再由 `FinalGameInstance` 按 stable prototype id 查询所需内容
* 当前 prototype content 已开始以真实资产落地在 `/Game/Prototype/Definitions/...`；`FinalGameInstance` 的测试入口不再分别硬编码查询 rule / encounter / route / character / card，而是先查询单一 `PrototypeBootstrapDefinition`，再按其字段驱动最小 `RunSession` 启动
* 当前 prototype Run 图已改成 `RunRouteDefinition` 驱动：节点图和节点内容流优先挂在 `FinalData` 的 route / node definition 上，由 `FinalRunSession` 通过 route id 接管初始化；`FinalGameInstance::PrepareTestBattleRun()` 不再直接拼 `TArray<FFinalRunNodeDefinition>`
* 当前代码已补上常驻 `PrototypeRunDebugScreen`，作为原型闭环的观察入口；它直接消费 `RunSnapshot.CurrentBuild.DeckEntries / RelicEntries` 展示当前构筑真相，并优先消费 `RunSnapshot.Characters.DisplayName / IconId / StateSummaryText` 展示角色持久状态摘要，同时把 pending reward 条目降为附加候选调试信息，并只调用 `FinalApp` 现有测试入口和原型级 Save / Load 调试入口
* 当前 Save / Load 调试入口只显示固定 slot 状态、最近 Save/Load 状态 / 失败原因，并提供原型按钮；不保存 UI 页面栈，不代表正式存档菜单
* 当前代码已补上 `FinalBattleEventPresentationUtils` 与 `FinalBattleEventScreen`：
  * 前者把 `FFinalBattleEvent` 统一投影成标题、摘要、细节、世界提示与账本文本
  * 后者挂在 `HUD Layer`，只读消费 `GetBattleLogEntries / GetBattleEventsSince / GetLatestBattleEventSequence`
  * 这套消费面服务 HUD、Debug、世界提示与未来 replay-ready 表现，但本轮不等于完整 replay 系统
* 当前代码已补上 `BattleDirector` 的最小世界桥接骨架：
  * 只读订阅 `FinalBattleFlowSubsystem` 的 `BattleSnapshot / BattleEvent`
  * 在世界层维护一份玩家侧 / 敌方侧的 presentation roster 映射
  * 用代码生成的文本占位 actor 承接单位名、目标高亮、敌人意图、存活/崩溃状态与最近事件反馈；世界提示当前直接复用 BattleEvent 统一投影 helper
  * 不替代 Battle HUD，也不访问 `FinalBattle` 私有运行时结构
* 当前 Battle 期 relic 反馈分层口径：
  * `BattleHUDScreen` 显示精简 `ActiveRelics` 摘要，并把 `RelicTriggered` 作为顶部交互反馈的一部分
  * `PrototypeRunDebugScreen` 显示详细只读调试信息，包括 `CurrentBuild.RelicEntries`、区分 `BattleStartEffects / PlayerTurnStartEffects` 的 `BattleSnapshot.ActiveRelics` 和最近一条 `RelicTriggered`
  * `BattleDirector` 只保留简短的世界层 relic 提示，不重复堆叠完整列表
* 当前 `ActiveRelics` 允许承载少量 battle-start / player-turn-start 遗物输入；窗口深化继续留在 `FinalBattle`，不回流到 `FinalApp`

#### 4.5.1 FinalApp/UI 推荐分层
* `UISubsystem` 当前负责根布局、Battle HUD 创建、页面栈、输入模式与焦点切换
* `RunFlowSubsystem` 负责读取 `RunSession`，并根据 `RunSnapshot / RunEvent` 决定当前应显示战后奖励页、节点选择页、奖励节点页、事件节点页、商店节点页还是关闭外层页
* `RunFlowSubsystem` 当前还负责把奖励结果类 `RunEvent` 的最近反馈收口成 UI 可读文本，并优先直接消费 `RunEvent.RewardEntryViews` 与 `AffectedCharacterResults`；raw `RewardEntries` 只保留为回退
* 当 `RunSession` 进入 `PreparingBattle`、`HasValidBattleStartState == true` 且当前没有 `ActiveBattleSession` 时，`RunFlowSubsystem` 会委托 `FinalGameFlowSubsystem` 自动调用 `StartBattleFromRunSession()`，不把开战逻辑散在单个页面里
* `RootScreen` / `UIRootLayout` 承载常驻 HUD
* `BattleHUDScreen` 是当前首轮已落地的战斗 HUD 容器
* `PrototypeRunDebugScreen` 挂在 `HUD Layer`，作为不打断主流程的小型调试摘要窗
* `FinalBattleEventScreen` 挂在 `HUD Layer`，作为 Battle 事件账本只读窗，优先服务调试、QA 与后续 replay-ready 消费验证
* `OverlayScreen` 用于奖励、事件、商店、节点选择等覆盖层，不替换顶部关键 HUD
* `ModalScreen` 处理确认类阻断交互
* `FinalRunStageOverlayScreenBase` 可承接 Run 外层页的共用标题区、摘要区、反馈区与按钮布局 helper
* `PanelWidget` 用于 `TopBar / Party / Enemy / Hand / Log / UltimateBar` 这类 HUD 区块复用
* `Widget` 用于卡牌、状态 Chip、资源条、敌人意图等原子控件
* `WidgetController` 负责订阅 `Snapshot / Event / Query` 并组装 `ViewModel`
* Battle 期 `BattleHUDScreen` 顶部反馈、`PrototypeRunDebugScreen` 的 Battle 事件摘要、`FinalBattleEventScreen` 的账本文本、`AFinalBattleDirector` 的世界提示，当前都应优先共用同一套 BattleEvent projection helper
* `ViewModel` 只保存展示数据，不做权威结算，也不承担命令合法性判定

### 4.6 FinalEditor
职责：
* 资源校验
* 数据批量检查
* 调试工具
* 编辑器菜单和导入器

当前已开始落地：
* `FinalEditor` 作为 Editor-only 模块挂入 `FinalFinalEditor.Target.cs` 与 `.uproject`
* 当前 `DataValidation` 原生校验器除了字段级检查，还会在 Editor 内扫描全项目 `FinalData` definition，检查 `Card / Character / Enemy / EnemyIntent / Encounter / Relic / Status / Ultimate / RuleConfig` 的稳定主 ID 是否重复
* 当前已补第一批跨资产稳定 ID 引用存在性检查：角色的初始卡组、角色卡池、奥义 ID、招牌状态 ID
* 遗物允许暂时没有 `BattleStartEffects / PlayerTurnStartEffects`，以免未来窗口、经济、商店类合法遗物被当前最小战斗窗口误拦截
* 当前 `FinalEditor` 还提供 `FinalPrototypeContentBootstrap` commandlet，用于把 prototype rule / encounter / route / bootstrap / character / card / enemy / relic bundle 生成或刷新为真实 Content 资产，避免运行时继续依赖 `FinalApp` 瞬时造数
* `FinalGameInstance` 当前集中持有 prototype bootstrap profile 的运行时单点真相：默认使用 `prototype.bootstrap.starter.chapter1`，并保留 `prototype.bootstrap.test` 作为调试回切入口；`FinalBattlePlayerController` 和 `PrototypeRunDebugScreen` 只透传切换请求，不各自缓存 bootstrap 状态
* 当前已补 prototype vertical slice 自动化冒烟测试，走 `FinalDataRegistry / FinalRunSession / FinalBattleSession / FinalGameFlowSubsystem / FinalBattleFlowSubsystem / RunSnapshot / BattleSnapshot` 的公开面，锁住 `bootstrap -> run -> battle -> battle result -> save/load` 主线
* 当前不参与运行时规则真相，也不提供自动修复、批量迁移或批量改资产工具；Editor 自动化测试只消费现有 facade / subsystem / query API，不把 Battle / Run 私有运行时结构抬进 `Public`

后续再补：
* 数据批量检查入口
* 编辑器菜单
* 调试面板和导入器

---

## 5. Public / Private 边界

### 5.1 总规则
* `Public` 只放真正需要跨模块引用的头文件
* `Private` 放具体实现、内部工具、局部 helper
* 头文件优先前向声明，具体 include 放进 `.cpp`
* 不把大型 USTRUCT、工具类、执行器全都暴露到 `Public`

### 5.2 各模块公开面建议

#### FinalCore/Public
只暴露：
* 共用 ID 类型
* 共用枚举
* Gameplay Tags 注册入口
* 核心日志定义

#### FinalData/Public
只暴露：
* 可被其他模块读取的定义类
* 查询接口
* 少量必需协议类型

不暴露：
* 数据校验器的内部实现
* 编辑器工具

#### FinalBattle/Public
只暴露：
* `BattleSession`
* `BattleSnapshot`
* `BattleQueryTypes`
* `BattleCommand`
* `BattleEvent`
* `GetBattleLogEntries / GetBattleEventsSince / GetLatestBattleEventSequence`
* 少量供 UI / App 读取的查询结构

不暴露：
* Resolver 内部细节
* StatusService 内部流程
* 敌人 AI 选择实现
* `BattleState`、`BattleCharacterState`、`BattleEnemyState` 等权威运行时结构

#### FinalRun/Public
只暴露：
* `RunSession`
* `RunSnapshot`
* `RunQueryTypes`
* `RunEvent`
* `RunCommand`
* `FinalRunSaveData`
* 进入战斗与战后结算的桥接请求结构
* `GetRunLogEntries / GetRunEventsSince / GetLatestRunEventSequence`
* `ExportSaveData / RestoreFromSaveData`，只作为 Run 外层状态保存与恢复协议，不允许 `FinalApp` 直接写 `RunSession` 私有字段
* `FinalRunSaveData::CurrentSaveVersion / IsSupportedVersion / IsStructurallyValid`，用于 SaveVersion 校验与坏档拒绝，不暴露 `RunSession` 私有 runtime 容器
* 如需让 `FinalBattle` 感知本场 Run 输入，应优先通过共享 request / init context payload 显式传递，例如 `FinalBattleStartRequest -> FFinalBattleInitContext`，而不是让 `FinalBattle` 直接依赖 `FinalRun`

#### FinalApp/Public
只暴露：
* 游戏入口 Subsystem
* `UISubsystem`
* `ScreenBase / PanelWidgetBase / WidgetBase`
* `WidgetControllerBase`
* `ViewModelBase`
* `WidgetController`
* 对 Blueprint 必须开放的类

不暴露：
* `ScreenStack` 内部实现
* Widget 工厂与焦点恢复 helper
* 任何跨层临时缓存的权威状态副本
* active battle 内部状态与 UI 页面栈的存档真相

---

## 6. Build.cs 依赖建议

### 6.1 FinalCore
`PublicDependencyModuleNames`
* `Core`
* `CoreUObject`
* `GameplayTags`

### 6.2 FinalData
`PublicDependencyModuleNames`
* `Core`
* `CoreUObject`
* `Engine`
* `GameplayTags`
* `FinalCore`

### 6.3 FinalBattle
`PublicDependencyModuleNames`
* `Core`
* `CoreUObject`
* `Engine`
* `GameplayTags`
* `FinalCore`
* `FinalData`

### 6.4 FinalRun
`PublicDependencyModuleNames`
* `Core`
* `CoreUObject`
* `Engine`
* `GameplayTags`
* `FinalCore`
* `FinalData`

### 6.5 FinalApp
`PublicDependencyModuleNames`
* `Core`
* `CoreUObject`
* `Engine`
* `InputCore`
* `EnhancedInput`
* `UMG`
* `Slate`
* `SlateCore`
* `GameplayTags`
* `FinalCore`
* `FinalData`
* `FinalBattle`
* `FinalRun`

### 6.6 FinalEditor
`PrivateDependencyModuleNames`
* `Core`
* `CoreUObject`
* `Engine`
* `UnrealEd`
* `DataValidation`
* `FinalCore`
* `FinalData`

规则：
* `FinalBattle` 不依赖 `FinalRun`
* `FinalRun` 不依赖 `FinalBattle`
* 两者只通过 `FinalData` 中的定义和 `FinalApp` 中的桥接层协作
* `FinalEditor` 当前只依赖 `FinalCore / FinalData` 与编辑器校验模块；除非后续工具确实需要，不引入 `FinalBattle / FinalRun / FinalApp`

---

## 7. 推荐目录结构

```text
Source
├─ FinalCore
│  ├─ Public
│  │  ├─ Ids
│  │  ├─ Types
│  │  ├─ Tags
│  │  └─ Logging
│  └─ Private
├─ FinalData
│  ├─ Public
│  │  ├─ Battle
│  │  │  ├─ Definitions
│  │  │  ├─ Effects
│  │  │  └─ Rules
│  │  ├─ Run
│  │  │  ├─ Definitions
│  │  │  ├─ Bridge
│  │  │  └─ Rewards
│  │  └─ Queries
│  └─ Private
│     ├─ Registry
│     └─ Validation
├─ FinalBattle
│  ├─ Public
│  │  ├─ Commands
│  │  ├─ Events
│  │  ├─ Queries
│  │  └─ Facade
│  └─ Private
│     ├─ Runtime
│     ├─ Resolver
│     ├─ Ops
│     ├─ Systems
│     ├─ Services
│     └─ Queries
├─ FinalRun
│  ├─ Public
│  │  ├─ Commands
│  │  ├─ Runtime
│  │  ├─ Requests
│  │  ├─ Save
│  │  └─ Facade
│  └─ Private
│     ├─ NodeFlow
│     ├─ Events
│     ├─ Rewards
│     ├─ Shops
│     └─ Growth
├─ FinalApp
│  ├─ Public
│  │  ├─ Subsystems
│  │  │  └─ UI
│  │  ├─ UI
│  │  │  ├─ Core
│  │  │  ├─ Root
│  │  │  ├─ Screens
│  │  │  ├─ Panels
│  │  │  ├─ Widgets
│  │  │  ├─ Controllers
│  │  │  └─ ViewModels
│  │  ├─ World
│  │  └─ Save
│  └─ Private
│     ├─ UI
│     │  ├─ Internal
│     │  ├─ Core
│     │  ├─ Root
│     │  ├─ Screens
│     │  ├─ Panels
│     │  ├─ Widgets
│     │  ├─ Controllers
│     │  └─ ViewModels
│     ├─ BattleBridge
│     └─ RunBridge
└─ FinalEditor
   ├─ Public
   └─ Private
      ├─ Validation
      ├─ Tools
      └─ Menus
```

说明：
* 上述是 `FinalApp/UI` 的目标目录形态
* 首批已经存在的 `Public/Controllers`、`Public/ViewModels` 可作为过渡目录暂存
* 当 `UISubsystem / RootLayout / ScreenBase` 基座落地后，再逐步迁移到 `Public/UI/Controllers` 与 `Public/UI/ViewModels`
* 当前代码已经落地 `Public/UI/...` 与旧 `Public/Controllers`、`Public/ViewModels` 并存的过渡形态

---

## 8. 关键类建议

### 8.1 Battle 入口
推荐：
* `UFinalBattleSession`
* `UFinalBattleCommandProcessor`
* `FFinalBattleResolver`

边界：
* `BattleSession` 是唯一战斗入口
* 外层系统不直接调用内部 `Service`

### 8.2 Battle 子系统
推荐：
* `FFinalBattleCardService`
* `FFinalBattleResourceService`
* `FFinalBattleStatusService`
* `UFinalBattleBreakService`
* `FFinalBattleTurnService`
* `FFinalEnemyIntentService`
* `UFinalCollapseAwakenService`
* `UFinalBattleLogService`

规则：
* Service 可以互相协作，但由 Resolver 或 Session 统一编排
* 不允许多个 Service 同时持有彼此的状态真相副本
* 当前代码口径中，`FinalBattleCardService / ResourceService / TurnService / StatusService / EnemyIntentService` 都是 `Private` 下的轻量 `F*` helper，不作为跨模块 UObject 暴露

### 8.3 Run 入口
推荐：
* `UFinalRunSession`
* `UFinalRunNodeResolver`
* `Private/Events/FinalRunEventResolver`
* `Private/Rewards/FinalRewardResolver`
* `Private/Shops/FinalShopResolver`
* `Private/Growth/FinalGrowthResolver`

规则：
* `UFinalRunSession` 是唯一对外 facade
* resolver 只放 `Private`，承接 reward / event / shop / growth 的内部解析与 view data 组装
* 不把 `RunState` 私有容器或 resolver 内部实现抬进 `Public`

当前稳定公开面：
* `RunSnapshot`
* `RunEvent`
* `RunQueryTypes`
* `GetRunLogEntries / GetRunEventsSince / GetLatestRunEventSequence`
* `FinalRunSaveData / ExportSaveData / RestoreFromSaveData`
* `FinalRunSaveData::IsSupportedVersion / IsStructurallyValid`

### 8.4 App 层桥接
推荐：
* `UFinalGameFlowSubsystem`
* `UFinalBattleFlowSubsystem`
* `UFinalRunFlowSubsystem`
* `UFinalSaveGameCoordinator`
* `UFinalRunSaveGame`
* `AFinalBattleDirector`
* `AFinalBattlePlayerController`
* `UFinalBattleWidgetController`
* `UFinalBattleHUDScreen`
* `UFinalBattleEventScreen`

Save / Load 当前边界：
* `FinalApp` 负责固定 slot 读写、SaveGame 类型检查、active battle 拒绝和 DebugScreen 状态展示
* `FinalRun` 负责 Save DTO 的版本与结构校验，并通过 `RunSession` 公开恢复协议重建 Run 外层状态
* 当前不支持 active battle 保存、多 slot、async save/load、迁移系统或正式存档 UI

### 8.5 UI 基类
推荐：
* `UFinalUISubsystem`
* `UFinalUIRootLayout`
* `UFinalScreenBase`
* `UFinalOverlayScreenBase`
* `UFinalModalScreenBase`
* `UFinalPanelWidgetBase`
* `UFinalWidgetBase`
* `UFinalWidgetControllerBase`
* `UFinalViewModelBase`

规则：
* `Screen` 只能通过 `UISubsystem` 进入和退出页面栈
* `Panel` 不直接控制输入模式和页面栈
* `Widget` 只做展示与轻交互，不直接接触权威状态
* `WidgetController` 负责把 `Snapshot / Event` 变成 `ViewModel`，并把 UI Intent 变成 `BattleCommand / RunCommand`
* `RunFlowSubsystem` 负责 Run 外层页面的自动切换，不把全局流程判断散在单个 Widget 中；战后奖励页优先以 `RewardEntryViews` 为主展示并实际消费 `PresentationKind / IconId / VisualTier / DetailText`、raw `RewardEntries` 只作回退，节点选择页以 `Progression.AvailableNextNodes` 为主展示，奖励/事件/商店节点页分别真实消费 `PendingRewardNode / PendingEventNode / PendingShopNode` 上的 `RewardEntryViews` 及其 metadata，并只转发对应的 `Resolve*` 命令
* `FinalGameFlowSubsystem` 负责 Run/Battle 的实际桥接收口：创建 `RunSession`、启动/完成战斗，并提供 `PreparingBattle -> StartBattleFromRunSession()` 的自动开战入口
* `ViewModel` 不保存权威运行时结构副本
* 当前首轮已落地 `BattleHUDScreen`，后续再把更多 HUD 区块拆成 `Panel / Widget`
* `AFinalBattleDirector` 当前承担最小世界表现桥接：读取 `BattleSnapshot / BattleEvent`，生成并刷新世界层占位表现对象；复杂演出、镜头与美术资源仍后置

---

## 9. C++ 与 Blueprint 分工

### 9.1 必须放在 C++
* 战斗权威状态
* 命令合法性校验
* 伤害、治疗、压力
* Break 与先机
* 状态结算窗口
* 崩溃与苏醒
* 敌人意图选择
* 事件条件与代价判定
* 奖励与成长的核心结算

### 9.2 可以交给 Blueprint
* 角色表现 Actor
* 场景摆放
* UI 布局
* 动画播放
* 相机、特效、音频、浮字
* 节点地图的纯展示部分

### 9.3 谨慎交给 Blueprint
* 事件文本分支的少量展示逻辑
* 首领演出触发
* 非权威调试按钮

原则：
* 一旦 Blueprint 开始改变数值、状态、卡牌去向、行动顺序，就应回收至 C++

---

## 10. 首版落地顺序

### 10.1 第一批
* 建立 `FinalCore / FinalData / FinalBattle / FinalRun / FinalApp`
* 跑通 DataAsset 加载与查询
* 跑通 `RunSession -> BattleSession` 输入桥接
* 跑通 BattleSession 最小链路
* 跑通一场普通战：打牌、伤害、削韧、Break、先机、敌人行动、奥义释放

### 10.2 第二批
* 在现有 `UISubsystem / UIRootLayout` 基础上补通用 `ScreenStack`
* 补 `Overlay / Modal / Tooltip / Toast` 通用容器与生命周期
* 补状态、被动、遗物触发
* 补崩溃与苏醒
* 补 FinalRun 的事件、奖励、商店与成长链

### 10.3 第三批
* 拆更多 Battle `Panel / Screen / WidgetController / ViewModel`
* 补事件、商店、角色成长
* 补 Save / Load
* 深化调试工具与编辑器校验；当前最小数据资产校验器已先行落地

---

## 11. 首版不做
首版明确不做：
* GAS 作为规则核心
* 依赖 Tick 的战斗规则驱动
* 让 UI 直接读写 BattleState
* 默认引入 `CommonUI` 作为首版基础框架
* 用 `Slate` 重写整套运行时主 HUD
* 单模块塞满所有运行时代码
* 把 DataAsset 定义散落在 Battle、Run、UI 各层
* 用 `CardId == xxx` 大量硬编码特例

---

## 12. 当前执行建议
当前项目从零开始时，推荐按以下顺序启动：
1. 先按本文件建立模块和目录
2. 先写 `FinalCore` 与 `FinalData`
3. 再写 `FinalRun` 的最小桥接状态与请求结构
4. 再写 `FinalBattle` 的最小战斗闭环
5. 然后接 `FinalApp` 跑通世界与 UI 桥接
6. 当前 `FinalEditor` 已先补最小数据资产校验；后续只继续扩编辑器工具，不回写 Runtime 规则语义

这样做的好处是：
* 一开始就把数据、规则、外层编排分开
* 后面新增内容时不需要反复搬类
* UI 和表现层可以并行开发
* Run 层系统可以在战斗闭环稳定后再接入
