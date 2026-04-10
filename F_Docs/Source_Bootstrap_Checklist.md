# 源码启动清单

## 1. 文档定位
本文档用于把当前架构方案落成真正的 Unreal 源码启动顺序。  
它不重新讨论架构对错，只回答：
* 第一批先建哪些模块
* 每个模块先建哪些文件
* 第一版先跑通哪条链路
* 哪些东西先不要建

本文档默认服务于：
* [Code_Function_Requirements.md](Code_Function_Requirements.md)
* [Unreal_Source_Structure.md](Unreal_Source_Structure.md)
* [Combat_Data_Schema_v2.md](Combat_Data_Schema_v2.md)
* [Battle_Rules.md](Battle_Rules.md)

---

## 2. 启动原则
首批启动只追求三件事：
* 模块边界先立住
* 最小战斗闭环先跑通
* 数据定义先能录入并读到运行时

首批不追求：
* 一次把所有 DataAsset 都补齐
* 一次把 Run 外层系统写满
* 一次把 UI 和演出做到完整

---

## 3. 首批模块创建顺序

### 3.1 第一批必须创建
* `FinalCore`
* `FinalData`
* `FinalBattle`
* `FinalRun`
* `FinalApp`

### 3.2 第二批再创建
* `FinalEditor`

说明：
* `FinalRun` 进入首批，但只先做最小 `RunSession + RunState + 战斗前后桥接`
* `FinalEditor` 明确后置，不阻塞首个垂直切片

---

## 4. 首批模块清单

| 模块 | 模块类型 | 直接依赖 | 首批职责 |
| --- | --- | --- | --- |
| `FinalCore` | Runtime | `Core` `CoreUObject` `GameplayTags` | 共用 ID、枚举、日志、Gameplay Tags |
| `FinalData` | Runtime | `FinalCore` `Engine` | 静态定义、规则配置、定义查询 |
| `FinalBattle` | Runtime | `FinalCore` `FinalData` `Engine` | 战斗状态、命令、结算、状态窗口、敌人行动 |
| `FinalRun` | Runtime | `FinalCore` `FinalData` `Engine` | 单局持久状态、Run 命令入口、战斗前后桥接 |
| `FinalApp` | Runtime | `FinalCore` `FinalData` `FinalBattle` `FinalRun` `UMG` `Slate` `SlateCore` `EnhancedInput` | 世界桥接、输入转命令、基础 UI 接口 |

---

## 5. 第一批必须创建的文件

### 5.1 FinalCore

#### 必建文件
* `Source/FinalCore/FinalCore.Build.cs`
* `Source/FinalCore/Private/FinalCoreModule.cpp`
* `Source/FinalCore/Public/Ids/FinalIds.h`
* `Source/FinalCore/Public/Types/FinalCoreTypes.h`
* `Source/FinalCore/Public/Tags/FinalGameplayTags.h`
* `Source/FinalCore/Public/Logging/FinalLogChannels.h`

#### 首批目标
* 定义 `CardId / EnemyId / StatusId / EventId / RelicId / EncounterId`
* 建立最小共用枚举
* 建立 Gameplay Tags 注册入口
* 建立日志分类

#### 暂不创建
* 大量通用工具库
* 网络相关类型
* 编辑器专用工具

### 5.2 FinalData

#### 必建文件
* `Source/FinalData/FinalData.Build.cs`
* `Source/FinalData/Private/FinalDataModule.cpp`
* `Source/FinalData/Public/Battle/Definitions/FinalBattleRuleConfig.h`
* `Source/FinalData/Public/Battle/Definitions/FinalCharacterDefinition.h`
* `Source/FinalData/Public/Battle/Definitions/FinalCardDefinition.h`
* `Source/FinalData/Public/Battle/Definitions/FinalUltimateDefinition.h`
* `Source/FinalData/Public/Battle/Definitions/FinalEnemyDefinition.h`
* `Source/FinalData/Public/Battle/Definitions/FinalEnemyIntentDefinition.h`
* `Source/FinalData/Public/Battle/Definitions/FinalStatusDefinition.h`
* `Source/FinalData/Public/Battle/Definitions/FinalBattleEncounterDefinition.h`
* `Source/FinalData/Public/Battle/Effects/FinalBattleEffectDefinition.h`
* `Source/FinalData/Public/Run/Definitions/FinalRunNodeDefinition.h`
* `Source/FinalData/Public/Run/Definitions/FinalRunNodeContentDefinition.h`
* `Source/FinalData/Public/Run/Rewards/FinalRunRewardTypes.h`
* `Source/FinalData/Public/Queries/FinalDataRegistry.h`

#### 首批目标
* 先把最小战斗闭环需要的定义录进去
* 建立统一 ID 查询入口
* 不在 DataAsset 内写战斗逻辑

#### 暂不创建
* `RelicDefinition`
* `EventDefinition`
* `RunEffectDefinition`

说明：
* 这些类第二批补，不阻塞第一场战斗跑通

### 5.3 FinalBattle

#### 必建文件
* `Source/FinalBattle/FinalBattle.Build.cs`
* `Source/FinalBattle/Private/FinalBattleModule.cpp`
* `Source/FinalBattle/Public/Commands/FinalBattleCommand.h`
* `Source/FinalBattle/Public/Events/FinalBattleEvent.h`
* `Source/FinalBattle/Public/Queries/FinalBattleSnapshot.h`
* `Source/FinalBattle/Public/Queries/FinalBattleQueryTypes.h`
* `Source/FinalBattle/Public/Facade/FinalBattleSession.h`
* `Source/FinalBattle/Private/Runtime/FinalBattleState.h`
* `Source/FinalBattle/Private/Runtime/FinalBattleCharacterState.h`
* `Source/FinalBattle/Private/Runtime/FinalBattleEnemyState.h`
* `Source/FinalBattle/Private/Runtime/FinalTeamDeckState.h`
* `Source/FinalBattle/Private/Runtime/FinalBattleCardInstance.h`
* `Source/FinalBattle/Private/Runtime/FinalBattleStatusInstance.h`
* `Source/FinalBattle/Private/Resolver/FinalBattleResolver.h`
* `Source/FinalBattle/Private/Resolver/FinalBattleResolver.cpp`
* `Source/FinalBattle/Private/Systems/FinalBattleCardService.h`
* `Source/FinalBattle/Private/Systems/FinalBattleResourceService.h`
* `Source/FinalBattle/Private/Systems/FinalBattleStatusService.h`
* `Source/FinalBattle/Private/Systems/FinalBattleTurnService.h`
* `Source/FinalBattle/Private/Systems/FinalEnemyIntentService.h`

#### 首批目标
* 跑通战斗初始化
* 跑通 `PlayCard` 命令
* 跑通 `PlayUltimate` 命令
* 跑通伤害、削韧、Break、先机减少事件
* 跑通敌人行动
* 跑通回合开始与回合结束

#### 暂不创建
* 遗物运行时状态
* 被动运行时状态
* 完整战斗日志回放
* 多阶段首领专用服务

### 5.4 FinalApp

#### 必建文件
* `Source/FinalApp/FinalApp.Build.cs`
* `Source/FinalApp/Private/FinalAppModule.cpp`
* `Source/FinalApp/Public/Subsystems/FinalGameFlowSubsystem.h`
* `Source/FinalApp/Public/Subsystems/FinalBattleFlowSubsystem.h`
* `Source/FinalApp/Public/World/FinalBattleGameMode.h`
* `Source/FinalApp/Public/World/FinalBattlePlayerController.h`
* `Source/FinalApp/Public/World/FinalBattleDirector.h`
* `Source/FinalApp/Public/ViewModels/FinalBattleHUDViewModel.h`
* `Source/FinalApp/Public/Controllers/FinalBattleWidgetController.h`

#### 建议同步预留的 UI 基座（允许空壳）
* `Source/FinalApp/Public/Subsystems/UI/FinalUISubsystem.h`
* `Source/FinalApp/Public/UI/Core/FinalUITypes.h`
* `Source/FinalApp/Public/UI/Root/FinalUIRootLayout.h`
* `Source/FinalApp/Public/UI/Screens/FinalScreenBase.h`
* `Source/FinalApp/Public/UI/Panels/FinalPanelWidgetBase.h`
* `Source/FinalApp/Public/UI/Controllers/FinalWidgetControllerBase.h`
* `Source/FinalApp/Public/UI/ViewModels/FinalViewModelBase.h`

说明：
* 首批允许这些类先做空壳，不要求一开始就把 Battle HUD 拆成大量 Panel
* 现有 `FinalBattleWidgetController + FinalBattleHUDViewModel` 可先作为聚合入口
* 若首批仍沿用 `Public/Controllers`、`Public/ViewModels` 旧路径，第二批开始再迁入 `Public/UI/...` 目标目录即可

#### 首批目标
* 把输入转成 `PlayCard` / `EndTurn` 命令
* 创建并持有 `BattleSession`
* 生成最小 HUD 可读状态
* 驱动一场战斗的开始与结束

#### 暂不创建
* Save / Load 协调器
* 大量细分 ViewModel

### 5.5 FinalRun

#### 必建文件
* `Source/FinalRun/FinalRun.Build.cs`
* `Source/FinalRun/Private/FinalRunModule.cpp`
* `Source/FinalRun/Public/Runtime/FinalRunState.h`
* `Source/FinalRun/Public/Runtime/FinalRunPersistentCharacterState.h`
* `Source/FinalRun/Public/Commands/FinalRunCommand.h`
* `Source/FinalRun/Public/Facade/FinalRunSession.h`
* `Source/FinalRun/Public/Requests/FinalBattleStartRequest.h`
* `Source/FinalRun/Public/Requests/FinalBattleResult.h`

#### 首批目标
* 维护最小单局持久状态
* 作为进入战斗前的唯一队伍与牌组输入来源
* 接收战斗结果并回写最小单局状态
* 提供事件 / 奖励 / 商店等单局外命令的统一入口
* 在 `RunSnapshot` 中公开 `PendingBattleReward / PendingRewardNode / PendingEventNode / PendingShopNode / Progression` 的最小结构化内容

#### 暂不创建
* 完整事件解析器
* 完整奖励解析器
* 商店解析器
* 成长解析器

---

## 6. 第二批补充文件

### 6.1 FinalData
* `FinalPassiveDefinition.h`
* `FinalRelicDefinition.h`
* `FinalUltimateDefinition.h`
* `FinalEventDefinition.h`
* `FinalRunEffectDefinition.h`

### 6.2 FinalBattle
* `FinalBattlePassiveInstance.h`
* `FinalBattleRelicRuntimeState.h`
* `FinalCollapseAwakenService.h`
* `FinalBattleLogService.h`

### 6.3 FinalRun
* `Source/FinalRun/Private/Events/FinalRunEventResolver.h`
* `Source/FinalRun/Private/Rewards/FinalRewardResolver.h`
* `Source/FinalRun/Private/Shops/FinalShopResolver.h`
* `Source/FinalRun/Private/Growth/FinalGrowthResolver.h`

### 6.4 FinalApp
* `Source/FinalApp/Public/UI/Screens/FinalOverlayScreenBase.h`
* `Source/FinalApp/Public/UI/Screens/FinalModalScreenBase.h`
* `Source/FinalApp/Public/UI/Screens/Battle/FinalBattleRootScreen.h`
* `Source/FinalApp/Public/UI/Panels/Battle/FinalBattleTopBarPanel.h`
* `Source/FinalApp/Public/UI/Panels/Battle/FinalBattlePartyPanel.h`
* `Source/FinalApp/Public/UI/Panels/Battle/FinalBattleEnemyListPanel.h`
* `Source/FinalApp/Public/UI/Panels/Battle/FinalBattleHandPanel.h`
* `Source/FinalApp/Public/UI/Panels/Battle/FinalBattleLogPanel.h`
* `Source/FinalApp/Public/UI/Panels/Battle/FinalBattleUltimateBarPanel.h`

---

## 7. 第三批补充文件

### 7.1 FinalApp
* `FinalRunFlowSubsystem.h`
* `FinalSaveGameCoordinator.h`
* `FinalBattleRewardScreen.h`
* `FinalBattleEventScreen.h`
* `FinalBattleShopScreen.h`
* `FinalBattleNodeSelectScreen.h`
* `FinalToastWidgetBase.h`
* `FinalTooltipWidgetBase.h`
* 更多 HUD / Hand / Enemy 细分 `WidgetController / ViewModel`

### 7.2 FinalEditor
* `Source/FinalEditor/FinalEditor.Build.cs`
* `Source/FinalEditor/Private/FinalEditorModule.cpp`
* 数据校验器
* 资源检查菜单
* 调试面板

---

## 8. 第一批必须跑通的链路

### 8.1 最小链路
1. 游戏进入一个普通战入口
2. `FinalGameFlowSubsystem` 从 `FinalRunSession` 触发 `FinalBattleStartRequest` 组装
3. `FinalBattleFlowSubsystem::CreateBattleSessionFromStartRequest` 根据 `FinalBattleStartRequest` 创建 `FinalBattleSession`
4. `FinalBattleSession` 根据 `BattleEncounterDefinition + BattleRuleConfig + 当前队伍配置` 初始化 `BattleState`
5. UI 能显示：
   * 队伍生命
   * AP / EP
   * 手牌
   * 敌人生命 / 韧性 / 当前意图 / 当前先机
6. 玩家打出 1 张攻击牌
7. 结算伤害、削韧、Break、先机变化
8. 若敌人先机归零，则进入敌方行动
9. 玩家结束回合
10. 下一回合开始
11. 至少能释放 1 次测试奥义
12. 敌人死亡后判定胜利
13. `FinalGameFlowSubsystem::CompleteBattleAndApplyResult` 驱动 `FinalRunSession` 消费 `FinalBattleResult` 并回写单局状态
14. `FinalRunSession` 生成待领取的战后奖励状态
15. 外层领取奖励
16. 外层推进到下一节点

### 8.2 第一批验收标准
* 没有硬编码 `CardId == xxx` 的规则分支
* 伤害与削韧从定义数据读
* 先机与 Break 的顺序符合 [Battle_Rules.md](Battle_Rules.md)
* UI 通过 ViewModel 读状态，而不是直接改规则状态
* UI 页面层级与输入模式由 `FinalApp` 集中管理，不由单个 Widget 各自切换
* Run 外层查询面至少公开结构化奖励条目和节点展示字段，不要求第一批就完成事件 / 商店 / 成长解析器

### 8.3 当前测试入口
* `FinalGameInstance` 负责注册一组瞬时测试资产，并构造最小 `RunSession`
* `FinalGameInstance::StartTestBattle()` 会串起 `BootstrapNewRun -> ConfigureBattleStartState -> StartBattleFromRunSession`
* `FinalBattlePlayerController::StartTestBattle()` 可供地图按钮直接调用
* 控制台命令 `FinalStartTestBattle` 可在测试地图内直接起一场战斗
* 控制台命令 `FinalDumpBattleSnapshot / FinalPlayFirstHandCard / FinalEndTurnCommand / FinalCompleteResolvedBattle` 可直接用日志验证战斗推进与回写

---

## 9. 第一批建议先录入的资产

### 9.1 角色
* 霍断岳
* 叶半夏
* 沈清弦

### 9.2 卡牌
每名角色先录入：
* 1 张基础攻击牌
* 1 张基础技能牌
* 1 张基础战术牌或功能牌
* 1 张初始特有卡
* 1 个测试奥义定义

### 9.3 敌人
* 2 名普通敌人
* 1 名精英敌人

### 9.4 规则与状态
* 1 份 `BattleRuleConfig`
* 中毒
* 腐蚀
* 护体
* 士气
* 易伤
* 虚弱
* 免疫

### 9.5 遭遇
* 1 个普通遭遇模板
* 1 个精英遭遇模板

---

## 10. 暂不创建的东西
第一批明确不要提前建：
* `FinalEditor` 校验工具
* 多阶段首领专用系统
* 战斗回放系统
* GAS 接入层
* 网络同步层
* 复杂 SaveGame 结构

---

## 11. 当前执行建议
如果现在正式开工程，推荐执行顺序：
1. 建 5 个首批模块：`FinalCore / FinalData / FinalBattle / FinalRun / FinalApp`
2. 先写 5 个 `.Build.cs`
3. 再写 `FinalCore` 的 ID、Tags、Log
4. 再写 `FinalData` 的最小定义资产
5. 再写 `FinalRunSession + FinalRunState + FinalBattleStartRequest`
6. 再写 `FinalBattleSession + FinalBattleState + FinalBattleResolver`
7. 最后用 `FinalApp` 把输入、HUD、战斗入口接起来

做到这里，再开始补 `FinalRun` 的事件、奖励、商店和成长链，成本会最低。
