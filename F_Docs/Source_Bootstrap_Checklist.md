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
* `FinalEditor` 原本后置，不阻塞首个垂直切片；当前已先建立最小 Editor-only 校验模块，用于提前拦截基础数据资产问题

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
* `Source/FinalData/Public/Run/Definitions/FinalRelicDefinition.h`
* `Source/FinalData/Public/Run/Definitions/FinalRelicBattleTypes.h`
* `Source/FinalData/Public/Run/Definitions/FinalRelicRuntimeTriggerTypes.h`
* `Source/FinalData/Public/Run/Bridge/FinalBattleRelicPayload.h`
* `Source/FinalData/Public/Run/Rewards/FinalRunRewardTypes.h`
* `Source/FinalData/Public/Queries/FinalDataRegistry.h`

#### 首批目标
* 先把最小战斗闭环需要的定义录进去
* 建立统一 ID 查询入口
* 不在 DataAsset 内写战斗逻辑

#### 暂不创建
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
* `Source/FinalBattle/Private/Runtime/FinalBattleRelicRuntimeState.h`
* `Source/FinalBattle/Private/Runtime/FinalBattleStatusInstance.h`
* `Source/FinalBattle/Private/Resolver/FinalBattleResolver.h`
* `Source/FinalBattle/Private/Resolver/FinalBattleResolver.cpp`
* `Source/FinalBattle/Private/Systems/FinalBattleInitializationService.h`
* `Source/FinalBattle/Private/Systems/FinalBattleInitializationService.cpp`
* `Source/FinalBattle/Private/Systems/FinalBattleCardService.h`
* `Source/FinalBattle/Private/Systems/FinalBattleCardService.cpp`
* `Source/FinalBattle/Private/Systems/FinalBattleResourceService.h`
* `Source/FinalBattle/Private/Systems/FinalBattleResourceService.cpp`
* `Source/FinalBattle/Private/Systems/FinalBattleRelicService.h`
* `Source/FinalBattle/Private/Systems/FinalBattleRelicService.cpp`
* `Source/FinalBattle/Private/Systems/FinalBattleStatusService.h`
* `Source/FinalBattle/Private/Systems/FinalBattleStatusService.cpp`
* `Source/FinalBattle/Private/Systems/FinalBattleTurnService.h`
* `Source/FinalBattle/Private/Systems/FinalBattleTurnService.cpp`
* `Source/FinalBattle/Private/Systems/FinalBattleEnemyActionService.h`
* `Source/FinalBattle/Private/Systems/FinalBattleEnemyActionService.cpp`
* `Source/FinalBattle/Private/Systems/FinalEnemyIntentService.h`

#### 首批目标
* 跑通战斗初始化
* 跑通 `PlayCard` 命令
* 跑通 `PlayUltimate` 命令
* 跑通伤害、削韧、Break、先机减少事件
* 跑通敌人行动
* 跑通回合开始与回合结束
* 暴露稳定的 `BattleSnapshot / BattleEvent / EventsSince` 公开查询面，供 `FinalApp` 只读接入
* 第二批可补最小 relic 触发深化：由 `RunSession -> FinalBattleStartRequest -> FinalBattleInitContext` 显式传入遗物输入
* 先支持 `battle-start` 的 AP / Shield 修正
* 再支持 `player-turn-start` 的 AP / Shield 修正，并在 Battle 权威状态里保留对应输入，供玩家回合开始窗口使用
* 当前已新增通用 `RuntimeTriggers` 第一版并继续扩到第二个 Battle 窗口：`PlayerTeamTookHealthDamage -> GainShield` 用于护心铜镜，`PlayerCardResolved + CardCondition(RuntimeCostAP 等于指定值，可选 CardType / Keyword) -> DrawCards` 用于阵门木签

#### 当前口径
* `FinalBattleResolver` 继续作为唯一对外 facade / orchestrator
* `FinalBattleInitializationService` 当前承接遭遇 / 规则 / init context 到初始 BattleState 的展开，包括角色、敌人、初始手牌、初始 intent、默认目标、`SessionStarted` 与 battle-start relic 初始化
* `PlayCard / PlayUltimate / EndTurn` 的卡牌区变更、资源调整、回合推进细节已开始迁入 `Private/Systems`
* `FinalBattleCardService` 当前承接手牌 / 牌堆去向、卡牌实例查找、抽牌与手牌视图构建，并已补最小衍生牌生成/入手/ConsumePile 通路
* `FinalBattleResourceService` 当前承接 AP / EP 初始化、打牌 / 回合结束 EP 增减与玩家回合开始 AP 重置
* `FinalBattleTurnService` 当前承接 `EndTurn` 后敌人行动 orchestration 与玩家回合开始窗口衔接
* `FinalBattleEnemyActionService` 当前承接单个敌人的 intent effect / fallback 普攻解析，`TurnService` 只编排回合窗口与敌人行动事件聚合
* `FinalBattleRelicService` 当前承接 battle-start / player-turn-start relic 数值触发、runtime trigger 计数重置、`PlayerTeamTookHealthDamage` / `PlayerCardResolved` 窗口与 `RelicTriggered` 事件生成
* `FinalBattleStatusService` 当前承接最小状态 tick 占位、状态加层/减层/移除，以及 `TeamStatuses / CharacterStatuses / Statuses` 快照整理
* `FinalEnemyIntentService` 继续独立承接 phase / intent 选择与推进

#### 暂不创建
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
* `Source/FinalApp/Public/Save/FinalRunSaveGame.h`
* `Source/FinalApp/Public/Save/FinalSaveGameCoordinator.h`

#### 当前已落地的 UI 基座
* `Source/FinalApp/Public/Subsystems/UI/FinalUISubsystem.h`
* `Source/FinalApp/Public/UI/Core/FinalUITypes.h`
* `Source/FinalApp/Public/UI/Root/FinalUIRootLayout.h`
* `Source/FinalApp/Public/UI/Screens/FinalScreenBase.h`
* `Source/FinalApp/Public/UI/Screens/Battle/FinalBattleHUDScreen.h`
* `Source/FinalApp/Public/UI/Panels/FinalPanelWidgetBase.h`
* `Source/FinalApp/Public/UI/Controllers/FinalWidgetControllerBase.h`
* `Source/FinalApp/Public/UI/ViewModels/FinalViewModelBase.h`

说明：
* 当前这批类已不再只是预留空壳，已经能承接首轮 Battle HUD
* 现有 `FinalBattleWidgetController + FinalBattleHUDViewModel` 仍作为聚合入口
* 旧 `Public/Controllers`、`Public/ViewModels` 与新 `Public/UI/...` 当前并存，后续再逐步迁移

#### 首批目标
* 把输入转成 `PlayCard` / `EndTurn` 命令
* 创建并持有 `BattleSession`
* 生成最小 HUD 可读状态
* 驱动一场战斗的开始与结束
* 通过 `UFinalSaveGameCoordinator` 调用 `UFinalRunSession::ExportSaveData / RestoreFromSaveData`，完成第一版 Run 外层 Save / Load
* Save / Load 当前只覆盖战斗外 Run 状态、RunLog、节点进度与待领奖励上下文；active battle、UI 页面栈和 Widget 状态不进入存档
* Save / Load 当前固定 `SaveVersion == 1`，Load 前会拒绝 slot 不存在、SaveGame 类型不对、版本不支持和结构不合法的坏档
* `PrototypeRunDebugScreen` 当前显示 save slot 存在性、最近 Save/Load 状态 / 失败原因，并提供 `Save Prototype Run / Load Prototype Run` 两个原型按钮；不代表正式存档菜单

#### 暂不创建
* 大量细分 ViewModel

### 5.5 FinalRun

#### 必建文件
* `Source/FinalRun/FinalRun.Build.cs`
* `Source/FinalRun/Private/FinalRunModule.cpp`
* `Source/FinalRun/Public/Events/FinalRunEvent.h`
* `Source/FinalRun/Public/Runtime/FinalRunState.h`
* `Source/FinalRun/Public/Runtime/FinalRunPersistentCharacterState.h`
* `Source/FinalRun/Public/Commands/FinalRunCommand.h`
* `Source/FinalRun/Public/Facade/FinalRunSession.h`
* `Source/FinalRun/Public/Queries/FinalRunQueryTypes.h`
* `Source/FinalRun/Public/Queries/FinalRunSnapshot.h`
* `Source/FinalRun/Public/Requests/FinalBattleStartRequest.h`
* `Source/FinalRun/Public/Requests/FinalBattleResult.h`
* `Source/FinalRun/Public/Save/FinalRunSaveData.h`

#### 首批目标
* 维护最小单局持久状态
* 作为进入战斗前的唯一队伍与牌组输入来源
* 接收战斗结果并回写最小单局状态
* 提供事件 / 奖励 / 商店等单局外命令的统一入口
* 在 `RunSnapshot` 中公开 `PendingBattleReward / PendingRewardNode / PendingEventNode / PendingShopNode / Progression` 的最小结构化内容
* 在 `RunSnapshot` 中公开 `CurrentBuild` 只读构筑视图，至少能表达当前牌库条目与遗物条目
* 在 `RunSnapshot.Characters` 中补最小可展示的持久角色 view data，至少包含角色显示名与少量展示辅助字段，不把 `RunPersistentCharacterState` 私有结构原样抬进 `Public`
* 在保留 raw `RewardEntries` 的同时，补一层 `RewardEntryViewData` 只读展示查询，供 `PendingBattleReward / PendingRewardNode / EventOption / ShopOffer` 直接输出稳定 reward 展示语义
* `RewardEntryViewData` 至少应能补出 `PresentationKind / IconId / VisualTier / DetailText` 这类元数据，避免 `FinalApp` 自己推断奖励图标、类别与描述
* `FinalApp` 的奖励页、奖励节点页、事件页、商店页和 prototype debug 优先直接消费 `RewardEntryViewData` 及其 `PresentationKind / IconId / VisualTier / DetailText`；raw `RewardEntries` 只保留为回退与调试底稿
* 对产出奖励结果的 `RunEvent`，也应同步输出 `RewardEntryViewData` 数组，避免后续结果 toast / 日志回到 raw reward 文本拼接
* `FinalApp` 当前的结果反馈主路径也应优先消费 `RunEvent.RewardEntryViews`，让最近反馈、调试摘要与外层页反馈口径保持一致；raw `RewardEntries` 只作回退
* 对包含 Growth 类奖励的 `RunEvent`，应在事件中补 `AffectedCharacterResults` 数组，复用现有 `FFinalRunCharacterViewData`，输出结算后的角色持久状态 view data，避免 `FinalApp` 自行推算角色结果
* `AffectedCharacterResults` 只在 `EventNodeResolved / RewardNodeResolved / ShopOfferPurchased / PendingBattleRewardClaimed` 的 reward entries 包含 Growth 时填充，不扩大到所有事件
* 在 `BuildBattleStartRequest()` 中桥接当前遗物的最小 battle-start payload，供 `FinalBattle` 初始化阶段消费
* 暴露 `FinalRunSaveData`、`ExportSaveData()`、`RestoreFromSaveData()`，仅用于 Run 外层状态导出 / 恢复，不开放 `RunSession` 私有字段给 `FinalApp` 直接写入
* `FinalRunSaveData` 当前定义 `CurrentSaveVersion = 1`，并提供 `IsSupportedVersion / IsStructurallyValid` 供 `FinalApp` 在 Load 前拒绝坏档
* 奖励协议使用 typed payload 标识授予对象，当前至少支持 `Gold / CardGrant / RelicGrant / RemoveCard / UpgradeCard / 最小 Growth` 落地到 `RunState`
* `CardGrant / RelicGrant` 落地前通过 `FinalDataRegistry` 校验 definition；`RelicGrant` 的展示 fallback 优先取 `FinalRelicDefinition`
* `RemoveCard / UpgradeCard` 使用稳定 payload，例如 `RemovedCardId / UpgradeFromCardId / UpgradeToCardId`，并在落地前校验必要 payload、card definition 和 `RunDeck` 中的目标
* `Growth` 使用最小 typed payload，例如 `GrowthTargetCharacterId / GrowthEffectType / Value`，并只落地到现有 `RunPersistentCharacterState` 字段
* `Growth` 当前最小 effect 范围控制在 `ReduceStress / GainAwakenProgress / ReduceCollapseCount`，不提前扩成完整成长树
* 暴露稳定的 `RunSnapshot / RunEvent / EventsSince` 公开查询面，供 `FinalApp` 与调试读取

#### 暂不创建
* 完整事件解析器
* 完整奖励解析器
* 商店解析器
* 成长解析器
* `Growth` 的完整成长树、奥义解锁与复杂分支结算器

---

## 6. 第二批补充文件

### 6.1 FinalData
* `FinalPassiveDefinition.h`
* `FinalUltimateDefinition.h`
* `FinalEventDefinition.h`
* `FinalRunEffectDefinition.h`

### 6.2 FinalBattle
* `FinalBattlePassiveInstance.h`
* `FinalCollapseAwakenService.h`
* `FinalBattleLogService.h`

### 6.3 FinalRun
已落地的私有解析器：
* `Source/FinalRun/Private/Events/FinalRunEventResolver.h`
* `Source/FinalRun/Private/Rewards/FinalRewardResolver.h`
* `Source/FinalRun/Private/Shops/FinalShopResolver.h`
* `Source/FinalRun/Private/Growth/FinalGrowthResolver.h`

当前口径：
* `UFinalRunSession` 继续作为唯一对外 facade / orchestrator
* reward 校验与应用、reward view data、affected character results 已迁入 `FinalRewardResolver`
* event option 查找与解析已迁入 `FinalRunEventResolver`
* shop offer 查找、价格校验与购买解析已迁入 `FinalShopResolver`
* 现有最小 `Growth` 校验与应用已迁入 `FinalGrowthResolver`

### 6.4 FinalApp
* `Source/FinalApp/Public/UI/Screens/FinalOverlayScreenBase.h`
* `Source/FinalApp/Public/UI/Screens/FinalModalScreenBase.h`
* `Source/FinalApp/Public/UI/Root/FinalUIRootLayout.h` 的通用 Overlay / Modal 容器能力
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
* `FinalBattleRewardScreen.h`
* `FinalBattleShopScreen.h`
* `FinalBattleNodeSelectScreen.h`
* `FinalToastWidgetBase.h`
* `FinalTooltipWidgetBase.h`
* 更多 HUD / Hand / Enemy 细分 `WidgetController / ViewModel`

### 7.2 FinalEditor
* `Source/FinalEditor/FinalEditor.Build.cs`
* `Source/FinalEditor/Private/FinalEditorModule.cpp`
* `Source/FinalEditor/Private/Validation/FinalDataAssetValidator.h`
* `Source/FinalEditor/Private/Validation/FinalDataAssetValidator.cpp`
* `Source/FinalEditor/Private/Validation/FinalDataValidationProjectIndex.h`
* `Source/FinalEditor/Private/Validation/FinalDataValidationProjectIndex.cpp`
* `Source/FinalEditor/Private/Tests/FinalPrototypeSmokeTests.cpp`
* 数据校验器已覆盖 `Card / Character / Enemy / EnemyIntent / Encounter / Relic / RuleConfig / Status / Ultimate / RunRoute`，并在 Editor 内补全局主 ID 重复扫描
* 第一批跨资产稳定 ID 引用存在性检查已覆盖 `Character.InitialLoadoutCards[*].CardId / CharacterCardPoolIds[*] / UltimateId / SignatureStatusId`
* 当前已补 `RunRouteDefinition` 校验，覆盖 `RouteId / EntryNodeId`、route 内节点唯一性、`NextNodeIds` 引用、battle 节点 `EncounterId / RuleConfigId`、以及 reward / event / shop 节点的最小结构合法性
* 当前已补 reward payload typed reference 校验，覆盖 `Gold / CardGrant / RelicGrant / RemoveCard / UpgradeCard / Growth`
* 当前已补 relic `RuntimeTriggers` 校验：有条目时要求 `Domain / Window` 非空、`Effects` 非空、`EffectType != None` 且 `Value > 0`；启用卡牌费用条件时要求费用非负；不要求每个 relic 都带 runtime trigger
* 当前已补 prototype vertical slice 自动化冒烟测试，覆盖 bootstrap 发现、registry 引用解析、最小 run 启动、进入 battle 后最小推进、battle result 回写 run，以及战斗外 save/load 恢复
* 资源检查菜单和调试面板仍后置

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
* `FinalApp` 只消费 Battle / Run 的公开查询与事件，不直接访问规则层私有状态
* Run 外层查询面至少公开结构化奖励条目和节点展示字段，不要求第一批就完成事件 / 商店 / 成长解析器

### 8.3 当前测试入口
* `FinalDataRegistry` 当前已开始承担运行时 definition 发现/加载：初始化时会扫描并注册项目中的 `BattleRuleConfig / CharacterDefinition / CardDefinition / UltimateDefinition / EnemyDefinition / EnemyIntentDefinition / StatusDefinition / BattleEncounterDefinition / RelicDefinition / RunRouteDefinition / PrototypeBootstrapDefinition`
* 当前 prototype bundle 已以真实资产落地在 `/Game/Prototype/Definitions/...`，覆盖 `prototype.bootstrap.test / rule.test.bootstrap / encounter.test.bootstrap / character.test.guardian / character.test.support / card.test.guardian.strike / card.test.guardian.guard / card.test.support.shot / card.test.support.focus / relic.test.charm / relic.test.repair_kit / run.route.test.prototype`
* 当前 starter bundle 也已以真实资产落地在 `/Game/Prototype/Definitions/Starter/...`，覆盖 `prototype.bootstrap.starter.chapter1 / run.route.starter.chapter1`、霍断岳 / 叶半夏 / 沈清弦、每名角色 4 张起始牌与 1 个测试奥义、2 名普通敌人、1 名精英敌人、1 个普通遭遇与 1 个精英遭遇
* starter bundle 继续复用 `FinalPrototypeContentBootstrap` commandlet、`FinalDataRegistry` 与 `FinalEditor` validation；当前已把霍断岳 `刀势`、叶半夏 `药引` 的第一波 battle-side 机制录入成真实可运行数据，starter 资产不再只靠文案描述这些资源
* starter bundle 当前已把沈清弦 `剑阵` 第一波接回 Runtime：`布锋` 随机生成 `过牌剑阵 / 破阵剑阵`、`引阵` 稳定生成 `过牌剑阵`、`过牌剑阵 / 破阵剑阵` 作为衍生牌进入手牌并在打出后进入 `ConsumePile`、`引爆剑阵` 真实消耗 1 张手中的衍生剑阵牌
* Battle 当前已补最小 `HandCardRequirement` 协议，并把它接到现有 `GainShield / DrawCards` 等效果上；`FinalBattleCardService` 负责统计当前手牌中满足条件的卡数量，并供 Battle resolver 做 gated effect 判定
* starter bundle 当前已把 `守阵` 的“若手中有剑阵牌”改成真实规则：护盾部分无条件结算，抽牌部分只有在当前手牌里存在满足 `SwordArray + GeneratedOnly` 条件的衍生剑阵牌时才会执行
* Battle 当前已补最小“状态驱动的伤害修正”协议：`FinalBattleStatusService` 负责在运行时统计 owner 的总伤害修正百分比、在一次成功对敌伤害后消费带 `bConsumeOnSuccessfulOwnerDamage` 的状态 1 层，并在玩家结束回合进入敌方行动前统一递减 `bExpireAtPlayerTurnEnd` 状态
* starter bundle 当前已把 `锋锐剑阵` 接回 Runtime：该衍生牌现在会为自身施加 1 层 `锋锐` 状态，使下一张攻击牌伤害提高 20%，若本回合内至少一次成功对敌生命伤害则消耗，否则在玩家回合结束时过期
* starter bundle 当前已把 `万象归阵` 改成真实规则：抽 2 张牌、生成 1 张剑阵牌到手牌，并为每名角色施加 1 层 `士气`；不再用团队护盾近似团队增益
* starter bundle 当前已补最小 `OwnerTookHealthDamage` 触发窗口：霍断岳角色定义挂接一组 battle trigger effects，玩家共享生命实际受损时按角色顺序触发，霍断岳因此获得 1 层 `刀势`
* starter bundle 当前已补最小 `TargetStateRequirement`：`Damage` effect 可按实际敌方目标是否处于 Break 做 gated 执行；霍断岳 `断岳绝式` 已追加一段“目标 Break 时额外攻击倍率伤害”的真实效果
* starter bundle 当前已补最小 incoming team HP damage protection：`生命免疫` 状态挂在 `team_player`，护盾后抵消下一次会扣共享生命的 HP damage，触发后消耗，未触发则在玩家回合结束时过期；叶半夏 `回天续脉` 已施加该状态。`免疫` 保持为上位状态概念，不被这条首版保护协议完全替代
* starter bundle 仍保留占位的内容包括：`万象归阵` 的阵牌扩散、复杂治疗保护、更复杂 Break 条件追伤链、经济 / 商店 / 未来窗口效果；这些内容仍以后续协议与规则服务深化为前提
* `FinalGameInstance` 当前集中持有 prototype bootstrap profile 的单点真相：默认 stable id 已切到 `prototype.bootstrap.starter.chapter1`，同时保留 `prototype.bootstrap.test` 作为调试回切入口；运行时按当前选中的 bootstrap stable id 查询 `FinalDataRegistry` 并构造最小 `RunSession`
* prototype Run 图当前已收回到 `FinalData` 的 `RunRouteDefinition`；`FinalGameInstance` 不再主路径 `NewObject` 拼整套节点图，也不再运行时生成整包 prototype definition
* prototype 启动配置当前也已从 `FinalGameInstance` 收回到 `PrototypeBootstrapDefinition`，统一承载 `RuleConfigId / EncounterId / RunRouteId / PartyCharacterIds / StarterDeckCardIds / InitialCharacterStates / InitialTeamCurrentHP`
* prototype bootstrap 调试切换当前由 `FinalGameInstance` 单点收口：`FinalBattlePlayerController` 的 exec 命令和 `PrototypeRunDebugScreen` 的 starter/test 切换按钮都只透传到这一个入口，不再各自硬编码 bootstrap id
* bootstrap 切换当前只允许在无 `ActiveBattleSession` 时生效；切换成功后会立即重新初始化最小 prototype run，并刷新当前 Run flow / UI 页面
* 当前 prototype Run 图里的事件节点选项已加入最小 `Growth` 奖励示例，直接使用 `GrowthTargetCharacterId / GrowthEffectType / Value`，用于验证 `RunPersistentCharacterState -> RunSnapshot.Characters` 的真实回写
* 当前 prototype Run 图已重新接回 `RemoveCard / UpgradeCard` 奖励示例，直接使用 `RemovedCardId / UpgradeFromCardId / UpgradeToCardId`，用于验证 `RunDeck -> RunSnapshot.CurrentBuild` 的真实构筑修正
* `FinalGameInstance::StartTestBattle()` 当前会串起 `BootstrapNewRun -> ConfigureBattleStartState + ConfigureRunRouteById -> RefreshRunFlow`
* `RunFlowSubsystem` 会在 `PreparingBattle + HasValidBattleStartState + 无 ActiveBattleSession` 时委托 `FinalGameFlowSubsystem::StartBattleFromRunSession()` 自动开战
* `UISubsystem` 当前会常驻挂一个 `PrototypeRunDebugScreen`，用于快速查看当前 bootstrap id、默认 bootstrap id、bootstrap route id、Run 阶段、当前 node id / 节点摘要、资源摘要、当前构筑、角色持久状态摘要、战斗期 `ActiveRelics`、最近一条 `RelicTriggered` 与战斗是否已激活；角色摘要优先直接消费 `RunSnapshot.Characters.DisplayName / IconId / StateSummaryText`
* `UISubsystem` 当前还会常驻挂一个只读 `FinalBattleEventScreen`，用于按事件序号查看最近 BattleEvent；它通过 `FinalBattleFlowSubsystem` 转发的 `GetBattleLogEntries / GetBattleEventsSince / GetLatestBattleEventSequence` 驱动刷新
* `FinalApp` 当前已补 BattleEvent 统一投影 helper，`BattleHUD` 顶部反馈、`BattleHUD` 日志区、`PrototypeRunDebugScreen` 的最新 Battle 事件摘要、`FinalBattleEventScreen` 账本文本、`BattleDirector` 的世界提示都优先共用这套 helper
* `FinalEditor` 当前提供 `FinalPrototypeContentBootstrap` commandlet，用于生成或刷新这批 prototype definition 资产；运行时如果缺少 `prototype.bootstrap.test` 或其引用的 stable id，应返回明确缺失错误并提示执行 commandlet，而不是继续由 `FinalApp` 瞬时造数
* `FinalBattleGameMode` 当前会确保存在一个 `FinalBattleDirector`，用于把 `BattleSnapshot / BattleEvent` 桥接到世界层占位表现对象
* `FinalBattleDirector` 当前会按 `Snapshot.Characters / Snapshot.Enemies / CurrentTargetUnitId` 维护最小 presentation roster，并在事件到来时刷新最近反馈；世界提示直接复用 BattleEvent 统一投影 helper，不替代 HUD / Debug 明细
* `FinalBattlePlayerController::StartTestBattle()` 可供地图按钮直接调用，并会按 `FinalGameInstance` 当前选中的 bootstrap profile 重启 prototype run
* 控制台命令 `FinalStartTestBattle` 可在测试地图内直接按当前 bootstrap profile 起一场战斗；`FinalSetPrototypeBootstrap <BootstrapId>` 可在非 active battle 时切换到指定 bootstrap 并重新初始化 prototype run
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
* 复杂 `FinalEditor` 校验工具、自动修复、内容迁移和批量 UI
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
