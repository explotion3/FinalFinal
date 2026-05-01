# 战斗 UI 线框

## 0. 当前实现状态（2026-04-25）
* 当前 `FinalApp` 已补上首轮 `UI` 基座：
  * `UISubsystem`
  * `UIRootLayout`
  * `Screen / Panel / Widget / WidgetController / ViewModel` 基类
  * 可由 Blueprint 子类换肤的 `BattleHUDScreen`
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
* 当前 Battle HUD 已新增角色突破槽投影，直接从 `RunSnapshot.Characters` 读取：
  * `Level`
  * `BreakthroughValue / BreakthroughRequiredValue`
  * `BreakthroughFillNormalized`
  * `bBreakthroughReady`
  * 满槽时允许 Blueprint 或 C++ fallback 做高亮、描边和轻量动效
* 当前 Battle HUD 已进入第一版水墨 16:9 桌面重构：
  * `UFinalUIWidgetClassSettings` 在 `Project Settings > Final > UI` 暴露 HUD screen、panel、entry widget 的 `TSoftClassPtr`
  * `UFinalUISubsystem` 创建 Battle HUD 时优先读取配置的 Widget Blueprint class，未配置或加载失败时回退 C++ `StaticClass()`
  * `BattleHUDScreen` 保留 C++ panel 装配和 fallback Canvas 布局，Blueprint 子类只负责外观、槽位和容器排布
  * Battle HUD panel / entry widget 通过 `BindWidgetOptional` 绑定 Blueprint 内控件；缺少绑定控件时继续走 C++ fallback 文本展示
  * `BattleResourcePanel` 已从手牌区独立出来，作为底部资源展示容器；它只消费 `BattleSnapshot.CurrentEP / MaxEP` 生成左下 EP 气圈与 7 个 QIPip 点亮状态，EP 满值时允许 Blueprint 换色提示
  * 第一版占位视觉资源放在 `/Game/UI/BattleHUD/InkPrototype/`，只作为表现资源，不新增规则字段
* 当前 `UISubsystem` 已补齐外层流程承接能力：
  * `OpenOverlayScreen / CloseOverlayScreen`
  * `OpenModalScreen / CloseModalScreen`
  * `FinalOverlayScreenBase`
  * `FinalModalScreenBase`
  * `FinalRunStageOverlayScreenBase`
  * `FinalRunFlowOverlayScreen`（Run 外层统一主流程页）
  * `FinalRunGrowthChoiceOverlayScreen`（角色成长选择页）
  * `FinalRunRewardOverlayScreen`（战后奖励页）
  * `FinalRunNodeOverlayScreen`（节点选择页）
  * `FinalRunRewardNodeOverlayScreen`（奖励节点页）
  * `FinalRunEventNodeOverlayScreen`（事件节点页）
  * `FinalRunShopNodeOverlayScreen`（商店节点页）
  * `FinalPlaceholderModalScreen`（确认类模态占位）
* 当前 `FinalApp` 已新增 `RunFlowSubsystem`：
  * 读取 `RunSession`
  * 增量消费 `RunEvent`
  * 根据 `RunSnapshot.Progression / PendingBattleReward / PendingGrowthChoice` 自动路由外层页
  * 当存在 `PendingGrowthChoice` 时，优先打开独立 `FinalRunGrowthChoiceOverlayScreen`
  * 其他外层阶段仍由统一 `FinalRunFlowOverlayScreen` 承接战后奖励、节点推进、奖励节点、事件节点、商店节点与本局结束
  * 对 `PendingBattleRewardGenerated / PendingBattleRewardClaimed / PendingBattleRewardSkipped / BattleResultApplied / RewardNodeResolved / EventNodeResolved / ShopOfferPurchased` 这类奖励结果事件，当前反馈主路径优先直接消费 `RunEvent.RewardEntryViews`，raw `RewardEntries` 只作回退
  * 当 `RunEvent` 带有 `AffectedCharacterResults` 时，当前反馈主路径与 prototype debug 会直接消费这些角色结果 view data，而不再根据 Growth reward 自行推断角色变化
* 当前 `FinalGameFlowSubsystem` 已承担 BattleGrowthFact 桥接职责：
  * 监听 battle snapshot 更新并拉取新增 `BattleGrowthFactBatch`
  * 使用 `CharacterGrowthConfig` 把 facts 转成突破值
  * 调用 `RunSession.AddBreakthroughValue()`
  * 仅在玩家主动命令结算后首次满槽时，立即刷新并展示 Growth overlay
  * 敌方阶段、被动链或战斗胜利导致的 pending growth 则延后到安全窗口展示
* 当前 `FinalGameInstance::PrepareTestBattleRun()` 已不再只配置裸 `BattleStartState`：
  * 会构建一个瞬时原型 Run 节点图，串起 `Battle -> Reward -> Event -> Shop -> Battle`
  * 便于在同一套测试 bootstrap 里实际走通 Run 外层页
  * 当前原型节点内容已重新覆盖 `RemoveCard / UpgradeCard`，便于验证 `RunDeck -> CurrentBuild` 的真实构筑修正
* 当前 `PrototypeRunDebugScreen` 已改为按需打开的 overlay：
  * 只读显示当前 `Run FlowStage`、节点摘要、`Gold / DeckCount / RelicCount`、最近流程反馈与 `ActiveBattleSession`
  * 构筑观察区直接消费 `RunSnapshot.CurrentBuild.DeckEntries / RelicEntries`，作为当前牌库与遗物真相的主展示
  * 角色持久状态摘要直接消费 `RunSnapshot.Characters.DisplayName / IconId / StateSummaryText`，并保留 `CharacterId / CurrentStress / CurrentAwakenCount / CollapseCount` 作为补充验证信息
  * 最近事件角色结果区直接消费 `RunEvent.AffectedCharacterResults`，显示 `DisplayName / IconId / StateSummaryText`
  * `PendingBattleReward / PendingRewardNode / PendingEventNode / PendingShopNode` 里的奖励条目只保留为“当前可见变动候选”附加调试区，不再冒充当前构筑真相
  * 当候选奖励里出现 `Growth / RemoveCard / UpgradeCard` 时，会额外显示 typed payload，例如目标角色、移除目标卡、升级路径，而不是只显示奖励类型名
  * 战斗期间额外显示 `BattleSnapshot.ActiveRelics` 与最近一条 `RelicTriggered` 事件，作为开场遗物生效的只读调试观察入口
  * 当前 battle relic 调试摘要已按 `RuntimeTriggers` 的 `BattleStart / PlayerTurnStart / 其他 battle windows` 汇总展示，并继续直接显示 `RelicTriggered` 的原始事件反馈
  * 可复用现有 `FinalApp` 测试入口快速重启 prototype run，或在战斗已结束时调用 `CompleteResolvedBattle`

## 1. 当前最小布局
* 当前战斗界面已进入 `UMG` + Blueprint 外观层阶段，由根界面统一承载主 HUD 与覆盖面板；C++ fallback HUD 继续作为未配置 Blueprint 时的兜底
* 当前主验收布局固定为 16:9 桌面：
  * 左侧：我方队伍三名角色状态、等级、突破槽、压力、生命份额、状态摘要与奥义入口
  * 顶部：敌方信息、生命 / 护盾 / Break / 先机、意图与阶段进度
  * 右上：目标、当前目标、团队状态、遗物摘要与战斗进度信息
  * 底部：手牌区，保留点击出牌与快捷键出牌
  * 底部：独立 `BattleResourcePanel` 占满底部 HUD 区域，当前主要承载左下 EP 气圈
  * 右侧中下：`RunFlowPromptPanel`，当 Run 外层存在待处理流程时显示短提示，点击后重新打开统一 `RunFlowOverlay`
  * 左下：抽牌堆、手牌 / 弃牌 / 消耗计数、AP 资源摘要
  * 右下：弃牌堆 / 消耗计数、结束回合、Debug 与账本入口
* Fallback 16:9 Canvas 参考锚点：
  * 左队伍 `0.015,0.02 -> 0.23,0.48`
  * 上敌人 `0.30,0.02 -> 0.82,0.20`
  * 右目标 `0.83,0.02 -> 0.985,0.28`
  * 底手牌 `0.16,0.56 -> 0.82,0.985`
  * 底部资源容器 `0.0,0.56 -> 1.0,1.0`
  * Run 流程恢复入口 `0.72,0.48 -> 0.92,0.56`
  * 左下资源 / 奥义 `0.015,0.49 -> 0.18,0.98`
  * 右下行动 `0.825,0.63 -> 0.985,0.975`
* 原顶部资源区现在作为 fallback 文本摘要保留：回合、遭遇名、`AP`、`EP`、队伍生命、护盾、金币、遗物数、战斗反馈
  * 当收到 `BattleEvent.EventType == RelicTriggered` 时，顶部反馈显示触发 relic 名称与 Battle 侧原始 `Message`
* 通过 Battle HUD 按钮打开的调试摘要窗：
  * 当前 `Run FlowStage`
  * 当前节点显示名、章节、楼层
  * `Gold / DeckCount / RelicCount`
  * `LastFlowMessage` 或最近流程反馈
  * 当前是否存在 `ActiveBattleSession`
  * 调试动作：重启 prototype run、在战斗已结束时调用 `CompleteResolvedBattle`
* 顶部上下文区：当前目标、牌堆计数、团队状态
  * 当前已接入一行精简 `ActiveRelics` 摘要，直接区分本场遗物的 `battle-start` 与 `player-turn-start` 能力
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
  * AP 不足时保持可点击，但费用使用 bad 色、卡牌整体半透明并在扇形布局中下沉
  * AP 不足的卡牌仍可被鼠标命中与点击，但不触发 hover 抬升、放大或置顶表现；点击后继续走统一出牌请求路径，由 Battle 返回失败反馈
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
* `PrototypeRunDebugScreen / FinalBattleEventScreen`：按需打开的 `Overlay Layer` 调试工具，不再和 Battle HUD 常驻同层
* `Overlay Layer`：默认由 `FinalRunFlowOverlayScreen` 承接 Run 外层主流程；当前 C++ fallback 是右侧紧凑流程面板，不再使用全屏遮罩，面板外区域应尽量保持对 Battle HUD 的点击可达；旧的战后奖励、节点选择、奖励节点、事件节点、商店节点专用页保留为显式调试 / 后续详情页入口
* `Overlay Layer`：当 `PendingGrowthChoice` 存在时，优先由独立 `FinalRunGrowthChoiceOverlayScreen` 占用；成长处理完成后再回到普通 RunFlow overlay 路径
* `Modal Layer`：承接确认、放弃、二次确认等阻断交互；优先级高于 `Overlay`
* `Tooltip / Toast Layer`：当前保留为后续扩展挂点
* 输入优先级：
  * `Modal > Overlay > Battle HUD`
  * Overlay / Modal 关闭后恢复到常驻 HUD 输入模式

## 2.2 Run 外层流程编排口径
* `RunFlowSubsystem` 是当前 Run 外层流程的集中编排入口
* `FinalGameFlowSubsystem` 是当前战斗内增长桥接与安全窗口判断入口
* Battle 结果回写到 `RunSession` 后，`RunFlowSubsystem` 会按最新 `RunSnapshot / RunEvent` 自动决定：
  * 若存在 `PendingGrowthChoice`，优先打开独立 Growth overlay
  * 进入 `PendingBattleReward` 时打开统一 RunFlow 页，并显示战后卡牌候选与跳过按钮
  * 进入 `AwaitingNodeAdvance` 时在统一 RunFlow 页显示可前往下一节点
  * 进入 `PendingRewardNode` 时在统一 RunFlow 页显示奖励节点确认
  * 进入 `PendingEventNode` 时在统一 RunFlow 页显示事件选项选择
  * 进入 `PendingShopNode` 时在统一 RunFlow 页显示商店商品选择
  * 进入 `RunEnded` 时在统一 RunFlow 页显示本局结束摘要
  * 进入 `PreparingBattle / None` 时关闭不该停留的外层页
* 当 `RunSession` 进入 `PreparingBattle`，且：
  * `HasValidBattleStartState == true`
  * 当前没有 `ActiveBattleSession`
  * `RunFlowSubsystem` 会委托 `FinalGameFlowSubsystem` 自动调用 `StartBattleFromRunSession()`
* 自动开战后不保留 Run overlay；屏幕恢复为常驻 Battle HUD 输入模式
* `RunFlowSubsystem.RefreshRunFlow(true)` 是当前自动流程主入口；它会根据最新 snapshot 决定打开 Growth overlay 还是普通 RunFlow overlay
* `FinalRunFlowOverlayScreen` 的关闭按钮只关闭 overlay 显示，不修改 `RunSession`、奖励候选、当前节点或流程阶段；后续 `RefreshRunFlow(true)` 或流程阶段变化仍可按 `RunSnapshot` 重新打开统一页
* `FinalRunGrowthChoiceOverlayScreen` 当前是独立 screen，只消费 `RunSnapshot.PendingGrowthChoice` 与角色 view data；它不保存成长真相，只负责选择并转发 `SelectGrowthChoice`
* 若突破值在玩家命令结算后首次达到阈值，Growth overlay 会立即成为当前外层页；若来自敌方阶段、被动链或战斗胜利，则等下一个安全窗口再展示
* `BattleHUDScreen` 当前提供独立 `RunFlowPromptPanel` 作为可恢复入口：当流程处于成长选择、战后奖励、节点推进、奖励节点、事件节点、商店节点或 `RunEnded` 时显示；它只触发 `RunFlowSubsystem.RefreshRunFlow(true)`，不直接执行任何 RunCommand
* `FinalRunFlowOverlayScreen` 当前主操作区采用列表按钮：战后卡牌候选、下一节点、事件选项、商店商品都直接显示为按钮；点击后分别转发 `ClaimPendingBattleRewardById / AdvanceToNode / ResolveEventOption / ResolveShopOffer`；节点显示优先使用 `DisplayName`，缺失时回退到中文节点类型，裸 `NodeId` 只作为最后 fallback
* `Final > UI` 的 Widget Class 设置支持替换 `RunFlowOverlayScreenClass` 与 `RunFlowOptionButtonClass`；未配置时继续使用 C++ fallback
* 旧的上一个 / 下一个 / 执行当前操作按钮保留为字段与奖励节点 fallback，但不再是战后奖励、节点推进、事件、商店的主流程交互路径
* `UISubsystem` 中保留的 `ShowBattleRewardOverlayPlaceholder / ShowNodeProgressOverlayPlaceholder / ShowNodeSelectOverlayPlaceholder / ShowRewardNodeOverlayPlaceholder / ShowEventNodeOverlayPlaceholder / ShowShopNodeOverlayPlaceholder` 现在属于显式调用 / 调试入口，不再是主流程驱动点
* `RunFlowSubsystem.GetLastFlowMessage()` 当前对奖励结果事件优先拼接 `RunEvent.RewardEntryViews` 与 `AffectedCharacterResults`，因此 Reward 页、节点页和 `PrototypeRunDebugScreen` 的最近反馈不会再以 raw `RunEvent.RewardEntries` 或本地 Growth 推断为主路径

### 2.2.1 RunFlowOverlay WBP 绑定建议
推荐 `WBP_RunFlowOverlay` 父类使用 `FinalRunFlowOverlayScreen`。根节点保持全屏 `Overlay` 或 `CanvasPanel`，空白区域设为 `Self Hit Test Invisible`，右侧内容面板承载可交互控件。

可绑定控件名：
* `TitleText`
* `SummaryText`
* `CurrentNodeText`
* `StageDetailText`
* `FeedbackText`
* `RewardOptionListBox`
* `NextNodeListBox`
* `EventOptionListBox`
* `ShopOfferListBox`
* `SecondaryActionButton / SecondaryActionButtonText`
* `CloseButton / CloseButtonText`

推荐 `WBP_RunFlowOptionButton` 父类使用 `FinalRunFlowOptionButton`。条目只负责展示与点击，不保存 Run 状态，不直接提交 `RunCommand`。

可绑定控件名：
* `OptionButton`：实际点击区域
* `TitleText`：主标题，例如卡牌名、`前往：节点名`、事件选项、商品名
* `SubtitleText`：类型说明，例如奖励类型、节点类型、结果摘要、价格
* `MetaText`：详情、章节楼层、奖励摘要或商品说明
* `StateText`：可领取 / 可前往 / 可选择 / 可购买，或不可用原因
* `OptionLabel`：旧版兼容字段；如果 WBP 只绑定这个控件，C++ 会写入合并后的多行文本

条目数据来源仍是 `RunSnapshot` 展示数据，点击后由 `FinalRunFlowOverlayScreen` 统一转发到 `RunFlowSubsystem`。

### 2.2.2 RunGrowthChoiceOverlay WBP 绑定建议
推荐 `WBP_RunGrowthChoiceOverlay` 父类使用 `FinalRunGrowthChoiceOverlayScreen`。它是独立 overlay，不与统一 `RunFlowOverlay` 混用。

可绑定控件名：
* `TitleText`
* `SummaryText`
* `CharacterSummaryText`
* `SelectionSummaryText`
* `GrowthChoiceListBox`
* `PrimaryActionButton / PrimaryActionButtonText`
* `CloseButton / CloseButtonText`
* `FeedbackText`

推荐继续复用 `FinalRunFlowOptionButton` 作为候选条目控件。条目只负责展示和回传被点击的 `ChoiceInstanceId / PayloadIndex`，不直接执行 Run 规则。

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
* 手牌 UI 可用性提示 `bCanPlayHint / UnplayableHintText`，当前只由 FinalApp 根据 `CurrentAP / RuntimeCostAP` 计算 AP 不足表现，不作为 Battle 规则真相
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
* 角色 / 敌人 / 卡牌展示用 `IconId / ArtId`，仅作为 UI 资源映射键，不参与规则判定

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
* AP、HP、Stress、Break、Initiative、Deck counts 仍来自现有 Snapshot；水墨 HUD 不新增 Battle 规则字段
* 角色突破槽来自 `RunSnapshot.Characters[*].BreakthroughValue / BreakthroughRequiredValue / Level` 的只读投影，不扩展 `BattleSnapshot` 保存成长真相
* Hand Panel 当前负责 AP 不足牌的扇形布局表现：中间不可用牌下沉接近 `UnplayableDropMax`，两侧不可用牌下沉接近 `UnplayableDropMin`；Card Entry 只消费提示结果并更新费用颜色 / 透明度，不禁用按钮

### 4.2 Run 外层流程页
当前 `FinalApp` 已经具备承接战后奖励 / 节点推进 / 奖励节点 / 事件节点 / 商店节点 / RunEnded 的统一 `RunFlowOverlay` 生命周期，并且已开始真实消费 `PendingBattleReward`、`Progression`、`PendingRewardNode`、`PendingEventNode` 与 `PendingShopNode`。

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
* `PendingBattleReward.RewardEntryViews[*].RewardType`
* `PendingBattleReward.RewardEntryViews[*].PrimaryText`
* `PendingBattleReward.RewardEntryViews[*].SecondaryText`
* `PendingBattleReward.RewardEntryViews[*].Value`
* `PendingBattleReward.RewardEntryViews[*].PresentationKind`
* `PendingBattleReward.RewardEntryViews[*].IconId`
* `PendingBattleReward.RewardEntryViews[*].VisualTier`
* `PendingBattleReward.RewardEntryViews[*].DetailText`
* `PendingBattleReward.RewardEntries[*]`（raw 回退）
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
* `PendingRewardNode.RewardEntryViews[*].RewardType`
* `PendingRewardNode.RewardEntryViews[*].PrimaryText`
* `PendingRewardNode.RewardEntryViews[*].SecondaryText`
* `PendingRewardNode.RewardEntryViews[*].Value`
* `PendingRewardNode.RewardEntryViews[*].PresentationKind`
* `PendingRewardNode.RewardEntryViews[*].IconId`
* `PendingRewardNode.RewardEntryViews[*].VisualTier`
* `PendingRewardNode.RewardEntryViews[*].DetailText`
* `PendingRewardNode.RewardEntries[*]`（raw 回退）
* `PendingEventNode.Title`
* `PendingEventNode.Summary`
* `PendingEventNode.bCanResolve`
* `PendingEventNode.bResolved`
* `PendingEventNode.Options[*].OptionId`
* `PendingEventNode.Options[*].DisplayText`
* `PendingEventNode.Options[*].OutcomeSummary`
* `PendingEventNode.Options[*].bSelectable`
* `PendingEventNode.Options[*].AvailabilityMessage`
* `PendingEventNode.Options[*].RewardEntryViews`
* `PendingEventNode.Options[*].RewardEntryViews[*].PresentationKind / IconId / VisualTier / DetailText`
* `PendingEventNode.Options[*].RewardEntries`（raw 回退）
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
* `PendingShopNode.Offers[*].RewardEntryViews`
* `PendingShopNode.Offers[*].RewardEntryViews[*].PresentationKind / IconId / VisualTier / DetailText`
* `PendingShopNode.Offers[*].RewardEntries`（raw 回退）

战后奖励页当前第一版已落地：
* 胜利金币自动入账，页面只展示战后卡牌候选
* 最多 3 个卡牌候选按钮，点击后通过 `RunFlowSubsystem.ClaimPendingBattleRewardById(RewardId)` 转发给 `FinalRun`
* 跳过按钮通过 `RunFlowSubsystem.SkipPendingBattleReward()` 转发给 `FinalRun`

统一 RunFlow 页当前第一版已落地：
* 同一页面根据 `FlowStage` 切换当前可执行操作，不再自动跳转到多个专用页
* 战后卡牌候选、下一节点选择、事件选项、商店商品都在同一页内用列表按钮直接提交
* `RunEnded` 会保留在统一页内显示结束摘要，而不是关闭外层页

战后奖励页仍缺：
* 真实图标资源、来源说明、卡片化布局与更细的视觉层次
* 奖励条目分组、卡片化布局、二次确认交互

节点选择页仍缺：
* 节点图标、章节路线布局、地图空间关系
* 更完整的节点类型扩展与对应专用展示

奖励节点 / 事件节点 / 商店节点页仍缺：
* richer 布局、图标、长文本滚动、分页和更细的视觉层次
* 二次确认、离开节点、刷新商店等更完整的次级交互
* 多奖励、多选项、多商品下更细的卡片化表现与焦点管理

当前实现口径：
* `PendingGrowthChoice` 当前已经成为正式 Run 外层页的一部分；若它与其他外层流程并存，Growth overlay 优先
* 自动流程主入口是 `FinalRunFlowOverlayScreen`；旧专用页继续编译并保留为显式调试 / 后续详情页能力
* `FinalRunGrowthChoiceOverlayScreen` 是自动流程主入口之一，但只承接成长选择；战后奖励、节点推进、事件、商店、RunEnded 仍走统一 `FinalRunFlowOverlayScreen`
* 战后奖励页当前优先以 `PendingBattleReward.RewardEntryViews` 为主展示口径，并实际消费 `PresentationKind / IconId / VisualTier / DetailText`；raw `RewardEntries` 只作回退；`RewardGold / LastBattleRewardGold` 只保留为金币自动入账摘要，并把“选择/跳过卡牌奖励”意图转发给 `RunFlowSubsystem`
* 节点选择页当前以 `Progression.AvailableNextNodes` 和当前节点展示字段为主展示口径，并把“推进节点”意图转发给 `RunFlowSubsystem`
* 奖励节点页当前真实消费 `PendingRewardNode.Title / Summary / bCanResolve / bResolved / RewardEntryViews`，并实际显示 `PresentationKind / IconId / VisualTier / DetailText`；raw `RewardEntries` 只作回退，并把“确认奖励节点”意图经 `RunFlowSubsystem` 转发为 `ResolveReward`
* 事件节点页当前真实消费 `PendingEventNode.Title / Summary / bCanResolve / bResolved / Options[*].RewardEntryViews`，并实际显示 `PresentationKind / IconId / VisualTier / DetailText`；raw `RewardEntries` 只作回退，并把当前选中的 `OptionId` 经 `RunFlowSubsystem` 转发为 `ResolveEvent`
* 商店节点页当前真实消费 `PendingShopNode.Title / Summary / bCanResolve / bResolved / Offers[*].RewardEntryViews`，并实际显示 `PresentationKind / IconId / VisualTier / DetailText`；raw `RewardEntries` 只作回退，并把当前选中的 `OfferId` 经 `RunFlowSubsystem` 转发为 `ResolveShop`
* `RunFlowSubsystem` 再统一调用 `RunSession` 并决定是否切页、关页、恢复常驻 HUD 输入
* `PendingRewardNode / PendingEventNode / PendingShopNode` 已经是 Run 的真实流程阶段；当前 `FinalApp` 自动路由到统一 RunFlow 页，专用页不再作为自动流程主入口
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
