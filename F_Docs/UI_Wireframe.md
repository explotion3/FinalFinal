# 战斗 UI 线框

## 0. 当前实现状态（2026-04-11）
* 当前 `FinalApp` 已补上首轮 `UI` 基座：
  * `UISubsystem`
  * `UIRootLayout`
  * `Screen / Panel / Widget / WidgetController / ViewModel` 基类
  * 代码生成的 `BattleHUDScreen`
  * 常驻 `HUD / Overlay / Modal / Tooltip / Toast` 分层
* 当前 Battle HUD 通过 `FinalBattleWidgetController` 订阅 `FinalBattleFlowSubsystem`
* 当前 Battle HUD 已打通：
  * `Snapshot / Event -> WidgetController -> ViewModel -> HUD`
  * 敌人目标选择
  * 点击手牌出牌
  * 点击奥义按钮转发 `PlayUltimate`
  * `1~6` 快捷出牌
  * 点击 / `Enter / Space` 结束回合
* 当前 Battle HUD 已开始消费 Battle / Run 公开查询字段，但仍只做只读展示与命令转发，不承载规则结算
* 当前 `UISubsystem` 已补齐外层流程承接能力：
  * `OpenOverlayScreen / CloseOverlayScreen`
  * `OpenModalScreen / CloseModalScreen`
  * `FinalOverlayScreenBase`
  * `FinalModalScreenBase`
  * `FinalRunStageOverlayScreenBase`
  * `FinalRunRewardOverlayScreen`（战后奖励页）
  * `FinalRunNodeOverlayScreen`（节点选择页）
  * `FinalRunRewardNodeOverlayScreen`（奖励节点页）
  * `FinalRunEventNodeOverlayScreen`（事件节点页）
  * `FinalRunShopNodeOverlayScreen`（商店节点页）
  * `FinalPlaceholderModalScreen`（确认类模态占位）
* 当前 `FinalApp` 已新增 `RunFlowSubsystem`：
  * 读取 `RunSession`
  * 增量消费 `RunEvent`
  * 根据 `RunSnapshot.Progression / PendingBattleReward` 自动协调战后奖励页、节点选择页、奖励节点页、事件节点页、商店节点页与常驻 HUD
* 当前 `FinalGameInstance::PrepareTestBattleRun()` 已不再只配置裸 `BattleStartState`：
  * 会构建一个瞬时原型 Run 节点图，串起 `Battle -> Reward -> Event -> Shop -> Battle`
  * 便于在同一套测试 bootstrap 里实际走通 Run 外层页
* 当前 `HUD Layer` 已新增一个常驻 `PrototypeRunDebugScreen`：
  * 只读显示当前 `Run FlowStage`、节点摘要、`Gold / DeckCount / RelicCount`、最近流程反馈与 `ActiveBattleSession`
  * 可复用现有 `FinalApp` 测试入口快速重启 prototype run，或在战斗已结束时调用 `CompleteResolvedBattle`

## 1. 当前最小布局
* 当前战斗界面已进入 `UMG` 过渡阶段，由根界面统一承载主 HUD 与覆盖面板；旧 `Canvas HUD` 仅保留兜底
* 左上：回合、遭遇名、`AP`、`EP`、队伍生命、护盾、金币、遗物数、战斗反馈
* 右上角常驻调试摘要窗：
  * 当前 `Run FlowStage`
  * 当前节点显示名、章节、楼层
  * `Gold / DeckCount / RelicCount`
  * `LastFlowMessage` 或最近流程反馈
  * 当前是否存在 `ActiveBattleSession`
  * 调试动作：重启 prototype run、在战斗已结束时调用 `CompleteResolvedBattle`
* 顶部上下文区：当前目标、牌堆计数、团队状态
* 左中：三名角色状态
  * 角色名
  * 压力 `Current / Cap`
  * 生命份额
  * 崩溃次数
  * 苏醒计数 / 阈值
  * 角色私有状态
  * 角色奥义名、`EP` 消耗、可释放状态
  * 交互反馈中的结构化拒绝原因
* 右侧：敌人面板
  * 名称
  * 站位
  * `HP / Shield / Break`
  * `Init`
  * 当前意图
  * 阶段进度
  * 是否已行动
  * 敌方状态
* 底部左侧：最近事件日志
* 底部中部：手牌区
  * 卡牌类型
  * 当前消耗
  * 所属单位
  * 关键词 / 规则文本
* 底部右侧：结束回合按钮
* 右下：三名角色奥义快捷按钮
* 中央覆盖层：
  * 当前已落地 `Overlay` 原型：战后奖励页、节点选择页、奖励节点页、事件节点页、商店节点页
  * 当前已落地 `Modal` 骨架：流程确认占位
  * 未来继续承接：事件、商店、变体、章节完成/失败

## 2. 当前输入映射
* `1~6`：按手牌序号出牌
* `Enter / Space`：结束回合
* `F1 / F2 / F3`：预留为霍断岳 / 叶半夏 / 沈清弦奥义
* 点击敌人：切换当前目标
* 点击手牌：打出该牌
* 点击奥义按钮：转发 `PlayUltimate`

## 2.1 RootLayout 分层口径
* `HUD Layer`：常驻 Battle HUD，只在 `UISubsystem` 初始化时建立，不由外层页替换生命周期
* `Overlay Layer`：承接战后奖励、节点选择、奖励节点、事件节点、商店节点这类整页流程界面；当前同一时刻只显示栈顶页
* `Modal Layer`：承接确认、放弃、二次确认等阻断交互；优先级高于 `Overlay`
* `Tooltip / Toast Layer`：当前保留为后续扩展挂点
* 输入优先级：
  * `Modal > Overlay > Battle HUD`
  * Overlay / Modal 关闭后恢复到常驻 HUD 输入模式

## 2.2 Run 外层流程编排口径
* `RunFlowSubsystem` 是当前 Run 外层流程的集中编排入口
* Battle 结果回写到 `RunSession` 后，`RunFlowSubsystem` 会按最新 `RunSnapshot / RunEvent` 自动决定：
  * 进入 `PendingBattleReward` 时打开战后奖励页
  * 进入 `AwaitingNodeAdvance` 时切到节点选择页
  * 进入 `PendingRewardNode` 时切到奖励节点页
  * 进入 `PendingEventNode` 时切到事件节点页
  * 进入 `PendingShopNode` 时切到商店节点页
  * 进入 `PreparingBattle / None / RunEnded` 时关闭不该停留的外层页
* 当 `RunSession` 进入 `PreparingBattle`，且：
  * `HasValidBattleStartState == true`
  * 当前没有 `ActiveBattleSession`
  * `RunFlowSubsystem` 会委托 `FinalGameFlowSubsystem` 自动调用 `StartBattleFromRunSession()`
* 自动开战后不保留 Run overlay；屏幕恢复为常驻 Battle HUD 输入模式
* `UISubsystem` 中保留的 `ShowBattleRewardOverlayPlaceholder / ShowNodeProgressOverlayPlaceholder / ShowNodeSelectOverlayPlaceholder / ShowRewardNodeOverlayPlaceholder / ShowEventNodeOverlayPlaceholder / ShowShopNodeOverlayPlaceholder` 现在属于显式调用 / 调试入口，不再是主流程驱动点

## 3. 当前已桥接字段
### 3.1 Battle Snapshot 已可直接驱动
* 回合数
* 当前 `AP`
* 当前 `EP`
* 当前目标单位 `CurrentTargetUnitId`
* 牌堆计数 `DeckState`
* 队伍当前生命 / 最大生命
* 队伍护盾
* 角色运行时单位 `RuntimeUnitId`
* 角色 `CharacterId`
* 角色当前压力
* 角色是否崩溃
* 角色苏醒计数 / 阈值
* 角色崩溃次数
* 角色生命份额
* 敌人名称
* 敌人站位
* 敌人当前生命 / 护盾 / Break / 先机
* 敌人当前意图文案
* 敌人是否本回合已行动
* 团队状态列表 `TeamStatuses`
* 角色状态列表 `CharacterStatuses`
* 敌方状态列表 `Statuses`
* 手牌实例 `CardInstanceId`
* 手牌 `CardId`
* 手牌所属单位 `RuntimeOwnerUnitId`
* 手牌当前消耗 `AP`
* 手牌类型 / 运行时关键词 / 保留 / 崩溃牌标记
* 角色奥义列表 `CharacterUltimates`
* 奥义 `EP` 消耗
* 奥义是否可释放
* 奥义是否被崩溃阻塞
* 奥义定义是否就绪
* 奥义是否本战已释放 `bUsedThisBattle`
* 敌人阶段进度 `EnemyView.PhaseProgress`
* 最近战斗日志文本
* 交互拒绝原因 `BattleEvent.RejectReason`
* 交互拒绝标签 `BattleEvent.ReasonTag`

### 3.2 FinalApp 通过 Data / Run 补出的展示字段
* 遭遇名
* 当前金币
* 当前遗物数量
* Run 牌库数量
* `EP` 上限
* 角色显示名
* 角色压力上限
* 卡牌显示名
* 卡牌规则文本
* 状态显示名
* 奥义显示名
* 手牌所属单位显示名

## 4. 当前缺口字段 / 需要 Battle 或 Run 提供的接口
### 4.1 Battle HUD
当前这轮 Battle HUD 主链路所需的新增公开字段已全部接入，当前没有新增的阻塞性接口缺口。

当前实现口径：
* 团队状态直接来自 `FFinalBattleSnapshot.TeamStatuses`
* 角色状态直接来自 `FFinalBattleSnapshot.CharacterStatuses`
* 敌方状态继续来自总 `FFinalBattleSnapshot.Statuses`
* 阶段进度来自 `FFinalBattleEnemyViewData.PhaseProgress`
* 奥义“本战已释放”来自 `FFinalBattleUltimateViewData.bUsedThisBattle`
* 结构化交互反馈来自 `FFinalBattleEvent.RejectReason / ReasonTag`

### 4.2 Run 外层流程页
当前 `FinalApp` 已经具备承接战后奖励页 / 节点选择页 / 奖励节点页 / 事件节点页 / 商店节点页的 `Overlay / Modal` 生命周期，并且已开始真实消费 `PendingBattleReward`、`Progression`、`PendingRewardNode`、`PendingEventNode` 与 `PendingShopNode`。

当前已接入字段：
* `PendingBattleReward.bHasPendingReward`
* `PendingBattleReward.SourceNodeId`
* `PendingBattleReward.SourceNodeType`
* `PendingBattleReward.SourceNodeDisplayName`
* `PendingBattleReward.SourceNodeDisplayLabel`
* `PendingBattleReward.SourceEncounterId`
* `PendingBattleReward.SourceBattleOutcome`
* `PendingBattleReward.RewardGold`
* `PendingBattleReward.bCanClaim`
* `PendingBattleReward.RewardEntries[*].RewardType`
* `PendingBattleReward.RewardEntries[*].DisplayName`
* `PendingBattleReward.RewardEntries[*].Value`
* `PendingBattleReward.RewardEntries[*].bCanClaim`
* `PendingBattleReward.RewardEntries[*].bClaimed`
* `Progression.FlowStage`
* `Progression.CurrentNodeId`
* `Progression.CurrentNodeType`
* `Progression.CurrentNodeDisplayName`
* `Progression.CurrentNodeDisplayLabel`
* `Progression.CurrentChapter`
* `Progression.CurrentFloor`
* `Progression.bCurrentNodeVisited`
* `Progression.bCurrentNodeNeedsResolution`
* `Progression.bCurrentNodeHasImplementedResolver`
* `Progression.CurrentNodeStateMessage`
* `Progression.bCanClaimPendingBattleReward`
* `Progression.bCanAdvanceToNextNode`
* `Progression.AvailableNextNodes[*].DisplayName`
* `Progression.AvailableNextNodes[*].DisplayLabel`
* `Progression.AvailableNextNodes[*].ChapterIndex`
* `Progression.AvailableNextNodes[*].FloorIndex`
* `Progression.AvailableNextNodes[*].bVisited`
* `Progression.AvailableNextNodes[*].bLocked`
* `Progression.AvailableNextNodes[*].AvailabilityMessage`
* `Progression.AvailableNextNodes[*].bHasImplementedResolver`
* `PendingRewardNode.Title`
* `PendingRewardNode.Summary`
* `PendingRewardNode.bCanResolve`
* `PendingRewardNode.bResolved`
* `PendingRewardNode.RewardEntries[*].RewardType`
* `PendingRewardNode.RewardEntries[*].DisplayName`
* `PendingRewardNode.RewardEntries[*].Value`
* `PendingRewardNode.RewardEntries[*].bCanClaim`
* `PendingRewardNode.RewardEntries[*].bClaimed`
* `PendingEventNode.Title`
* `PendingEventNode.Summary`
* `PendingEventNode.bCanResolve`
* `PendingEventNode.bResolved`
* `PendingEventNode.Options[*].OptionId`
* `PendingEventNode.Options[*].DisplayText`
* `PendingEventNode.Options[*].OutcomeSummary`
* `PendingEventNode.Options[*].bSelectable`
* `PendingEventNode.Options[*].AvailabilityMessage`
* `PendingEventNode.Options[*].RewardEntries`
* `PendingShopNode.Title`
* `PendingShopNode.Summary`
* `PendingShopNode.bCanResolve`
* `PendingShopNode.bResolved`
* `PendingShopNode.Offers[*].OfferId`
* `PendingShopNode.Offers[*].DisplayName`
* `PendingShopNode.Offers[*].Description`
* `PendingShopNode.Offers[*].Price`
* `PendingShopNode.Offers[*].bPurchasable`
* `PendingShopNode.Offers[*].bPurchased`
* `PendingShopNode.Offers[*].AvailabilityMessage`
* `PendingShopNode.Offers[*].RewardEntries`

战后奖励页仍缺：
* 奖励项的图标、稀有度、描述、来源说明等 richer 展示元数据
* 多奖励选择、替换、跳过等 richer reward flow 的结构化状态
* 奖励条目分组、卡片化布局、二次确认交互

节点选择页仍缺：
* 节点图标、章节路线布局、地图空间关系
* 更完整的节点类型扩展与对应专用展示

奖励节点 / 事件节点 / 商店节点页仍缺：
* richer 布局、图标、长文本滚动、分页和更细的视觉层次
* 二次确认、离开节点、刷新商店等更完整的次级交互
* 多奖励、多选项、多商品下更细的卡片化表现与焦点管理

当前实现口径：
* 战后奖励页当前以 `PendingBattleReward.RewardEntries` 为主展示口径，`RewardGold` 只保留为聚合摘要，并把“领取奖励”意图转发给 `RunFlowSubsystem`
* 节点选择页当前以 `Progression.AvailableNextNodes` 和当前节点展示字段为主展示口径，并把“推进节点”意图转发给 `RunFlowSubsystem`
* 奖励节点页当前真实消费 `PendingRewardNode.Title / Summary / bCanResolve / bResolved / RewardEntries`，并把“确认奖励节点”意图经 `RunFlowSubsystem` 转发为 `ResolveReward`
* 事件节点页当前真实消费 `PendingEventNode.Title / Summary / bCanResolve / bResolved / Options[*]`，并把当前选中的 `OptionId` 经 `RunFlowSubsystem` 转发为 `ResolveEvent`
* 商店节点页当前真实消费 `PendingShopNode.Title / Summary / bCanResolve / bResolved / Offers[*]`，并把当前选中的 `OfferId` 经 `RunFlowSubsystem` 转发为 `ResolveShop`
* `RunFlowSubsystem` 再统一调用 `RunSession` 并决定是否切页、关页、恢复常驻 HUD 输入
* `PendingRewardNode / PendingEventNode / PendingShopNode` 已经是 Run 的真实流程阶段；当前 `FinalApp` 会分别路由到对应的专用页，而不是继续挤在节点选择页
* `FinalApp` 不自行推导奖励结算，也不自行伪造节点合法性

## 5. 必须显示的信息
* `EP` 必须独立显示当前值与上限
* 当 `EP` 足以释放某角色奥义时，该角色奥义按钮必须显示为可用
* 顶部必须保留一条短反馈，用于显示 `EP 不足 / 角色崩溃 / 无合法目标 / 奥义已释放 / 战斗未初始化 / 命令不支持` 这类即时结果
* `Team Status` 只显示团队状态
* `Character Status` 只显示角色私有状态
* 奖励 / 事件 / 商店 / 节点选择使用 `Overlay Layer` 承接，不直接销毁常驻 `Battle HUD`
* 阻断确认类交互使用 `Modal Layer`，优先级高于 `Overlay`
* 节点选择中的战斗类节点至少区分 `Battle / Elite / Boss`
