# FinalFinal

FinalFinal 是一个基于 Unreal Engine 5.6 的多角色卡牌 Roguelite 原型工程。项目当前重点是验证首章竖切：稳定的战斗规则、Run 外层流程、数据驱动内容、HUD 表现和自动化测试闭环。

工程采用多模块拆分，把静态数据、战斗规则、Run 规则、应用桥接、编辑器工具分离。核心原则是：规则真相只存在于 `FinalBattle` / `FinalRun` / `FinalData`，`FinalApp`、Actor、Widget Blueprint 只负责表现、输入转发和流程桥接。

## 项目概述

当前原型包含：

- `RunSession -> BattleSession -> BattleResult -> RunSession` 的最小闭环。
- 多角色队伍、共享生命、压力 / 临界 / 崩溃、AP / EP、奥义等战斗资源。
- 卡牌出牌、抽牌、弃牌、消耗区、持续区、手牌投影和卡牌 modifier。
- 状态、被动、遗物、敌人意图、Break、先机、DOT 等战斗系统。
- 战后奖励、路线推进、事件、商店、成长选择和 RunEnded。
- Battle HUD、手牌拖拽出牌、场中点击选目标、敌人 OverHead、敌人 / 角色详情、牌区详情、队伍面板。
- Editor 数据校验、Starter 内容 bootstrap 和自动化烟测。

## 环境与入口

- Unreal Engine：`5.6`
- 工程文件：`FinalFinal.uproject`
- 主要插件：
  - `PaperZD`
  - `ModelingToolsEditorMode`（Editor only）
- Visual Studio：项目使用 UE C++ 模块构建，推荐 VS2022。

常用编译命令：

```powershell
D:\UE_5.6\Engine\Build\BatchFiles\Build.bat FinalFinalEditor Win64 Development -Project=D:\UE_Project\5.6\FinalFinal\FinalFinal.uproject -NoHotReload
```

常用自动化：

```powershell
D:\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe D:\UE_Project\5.6\FinalFinal\FinalFinal.uproject -ExecCmds="Automation RunTests Final.Editor.PrototypeSmoke; Quit" -DDC-ForceMemoryCache -unattended -nop4 -nosplash -NullRHI
```

内容重建入口：

```powershell
D:\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe D:\UE_Project\5.6\FinalFinal\FinalFinal.uproject -run=FinalPrototypeContentBootstrap -DDC-ForceMemoryCache -unattended -nop4 -nosplash
```

## 目录结构

```text
FinalFinal/
  Config/                         UE 配置
  Content/                        资产、WBP、DataAsset、表现资源
  F_Docs/                         设计、规则、架构、进度与待办文档
  Source/
    FinalCore/                    通用基础类型、Id、GameplayTags
    FinalData/                    静态定义、DataAsset schema、数据注册表
    FinalBattle/                  单场战斗权威规则
    FinalRun/                     Run 权威状态、路线、奖励、事件、商店、成长
    FinalApp/                     GameInstance、Subsystem、UI、World 表现桥接
    FinalEditor/                  编辑器校验、Commandlet、测试工具
  FinalFinal.uproject
```

## 模块分析

### FinalCore

`FinalCore` 是最底层运行时模块，放项目通用基础能力：

- Id 类型、轻量值类型和通用枚举。
- Native Gameplay Tags。
- 通用日志、结果类型和跨系统薄协议。

该模块不依赖其他业务模块，也不包含具体战斗、Run 或 UI 规则。

### FinalData

`FinalData` 负责静态内容定义和只读查询：

- Card / Character / Enemy / Status / Passive / Relic / Encounter 等 DataAsset schema。
- `FinalDataRegistry` 数据查询入口。
- 数据校验辅助与 authoring 约束。

`FinalData` 不保存运行时状态，不结算战斗，也不推进 Run。它只描述“内容是什么”和“内容是否合法”。

### FinalBattle

`FinalBattle` 是单场战斗的权威规则模块：

- BattleState、DeckState、CharacterState、EnemyState、StatusInstance、PassiveInstance 等战斗内真相。
- Battle command：出牌、选择目标、结束回合、释放奥义等。
- 卡牌效果执行、伤害 / 治疗 / 护盾、AP / EP、压力、Break、先机、敌人行动。
- 状态、被动、遗物的触发窗口和 runtime 结算。
- Battle snapshot、battle event、battle result。

`FinalBattle` 不依赖 `FinalRun`，也不处理永久成长、路线、商店或奖励选择。

### FinalRun

`FinalRun` 是单局外层流程的权威模块：

- RunState、路线节点、当前阶段、Run snapshot。
- 战后奖励、卡牌候选、金币、事件、商店、成长选择。
- 角色成长、卡牌进化、RunCardInstance 管理。
- `RouteOverview` 和 `AvailableFlowActions`，供 UI / 世界表现消费。

`FinalRun` 不执行战斗内卡牌结算，不直接 include `FinalBattle`。

### FinalApp

`FinalApp` 是运行时应用层桥接模块：

- GameInstance / Subsystem / SaveGame 协调。
- Battle 和 Run 的流程编排。
- HUD、Overlay、Widget Controller、ViewModel。
- World Presentation Actor、BattleDirector、TargetInteractor。
- 输入、点击、拖拽和表现事件转发。

`FinalApp` 可以依赖 `FinalBattle` 和 `FinalRun`，但不保存玩法真相，不直接结算数值规则。

### FinalEditor

`FinalEditor` 只在编辑器环境中使用：

- Starter 内容 bootstrap commandlet。
- 数据资产校验。
- 自动化测试和编辑器辅助工具。

运行时模块不能依赖 `FinalEditor`。

## 系统架构

### 分层依赖

```text
FinalCore
  ↓
FinalData
  ↓
FinalBattle      FinalRun
      \          /
       \        /
        FinalApp
           ↓
       FinalEditor
```

关键边界：

- `FinalBattle` 与 `FinalRun` 互不依赖。
- 静态定义属于 `FinalData`。
- 战斗内真相属于 `FinalBattle`。
- Run 外层真相属于 `FinalRun`.
- UI 和 World 表现属于 `FinalApp`，只消费 snapshot 和转发 command。

### 数据流

典型战斗流程：

```text
FinalRun
  生成 BattleStartRequest
    ↓
FinalApp
  创建 / 持有 BattleSession
    ↓
FinalBattle
  初始化 BattleState
  执行 BattleCommand
  输出 BattleSnapshot / BattleEvent / BattleResult
    ↓
FinalApp
  刷新 HUD / World Presentation
    ↓
FinalRun
  消费 BattleResult，进入奖励 / 成长 / 路线推进
```

典型 UI 输入流程：

```text
WBP / Widget
  点击、拖拽、按钮
    ↓
FinalApp WidgetController / Subsystem
  组装 command
    ↓
FinalBattle 或 FinalRun
  权威校验与结算
    ↓
Snapshot / Event
    ↓
FinalApp
  刷新 ViewData 和表现
```

## 主要系统

### Battle 系统

战斗系统围绕 command、state、service、snapshot 组织：

- `PlayCard`：从手牌解析目标、费用、效果和牌区去向。
- `EndTurn`：处理回合结束、敌人行动、状态过期和牌区清理。
- `Card Projection`：把卡牌定义与 runtime modifier 合成实际显示 / 结算数据。
- `EffectExecution`：执行伤害、护盾、治疗、抽牌、状态、被动、资源消费等效果。
- `TriggerService`：统一处理被动、遗物等 runtime trigger。
- `StatusService`：状态叠层、资源型状态、DOT、RuntimeModifiers、ProjectedCardModifiers。
- `PassiveService`：被动实例生命周期、ApplyPassive、移除和 snapshot 投影。
- `InitiativeService`：敌人先机、行动队列、Break 行动覆盖与恢复。

### Run 系统

Run 系统围绕当前阶段和可执行动作组织：

- `RouteOverview`：整条路线、节点状态、当前节点、可达性。
- `AvailableFlowActions`：当前阶段可执行动作列表。
- `PendingBattleReward`：战后卡牌候选和跳过奖励。
- `PendingEventNode`：事件选项。
- `PendingShopNode`：商店商品和离开商店。
- `PendingGrowthChoice`：角色成长候选。
- `AwaitingNodeAdvance`：路线推进。

UI 不自行推断这些阶段，只消费 `FinalRunSnapshot`。

### UI / World 表现系统

Battle HUD 当前由 C++ 父类和 WBP 组合：

- RootLayout / BattleHUDScreen。
- TeamPanel：共享生命、护盾、角色简化 Entry、队伍 / 角色状态。
- HandPanel：扇形手牌、AP 不足不可用表现、拖卡出牌。
- EnemyOverhead：敌人头顶 HP / Shield / Break / Intent / Initiative / 状态。
- EnemyDetail / CharacterDetail。
- CardZoneDetail：抽牌堆、手牌、弃牌堆、持续区、消耗区只读详情。
- Run Overlay：RunFlow、Reward、Event、Shop、Growth 专用页。

World 表现层使用 `FinalBattleDirector` 同步 Battle snapshot 到 PresentationActor。场中单位 Actor 不拼接 debug 字符串；正式 UI 通过结构化 ViewData 显示。

## 文档导航

主要文档位于 `F_Docs/`：

- `GDD4.0.md`：玩法方向。
- `Battle_Rules.md`：战斗规则。
- `Status_System_Guide.md`：状态系统。
- `Card_Design_Guide.md`：卡牌设计和文本规范。
- `Combat_Data_Schema_v2.md`：数据 schema。
- `Code_Function_Requirements.md`：代码功能边界。
- `Unreal_Source_Structure.md`：模块与源码架构。
- `UI_Wireframe.md`：UI 结构与绑定约定。
- `Implementation_Progress.md`：已完成内容。
- `Development_Backlog.md`：待办内容。
- `Source_Bootstrap_Checklist.md`：内容生成与 bootstrap 检查。


## 当前状态

FinalFinal 当前已经具备可运行的战斗 / Run 原型闭环，并持续向首章竖切推进。工程重点不在“临时做出一个演示”，而在把 Battle、Run、Data、App/UI 的边界稳定下来，让后续内容扩展可以继续沿用同一套规则协议和测试链路。
