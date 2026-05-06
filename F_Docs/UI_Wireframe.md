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
  * 独立敌人详情查看状态 `InspectedEnemyUnitId`
  * 独立牌区详情查看状态 `InspectCardZone / SelectedCardZone`
  * 按住手牌进入预选态，拖动释放出牌
  * 拖卡出牌：无目标牌拖出手牌区释放；敌方目标牌拖到场中敌人身体命中框释放
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
  * `UFinalUIWidgetClassSettings` 在 `Project Settings > Final > UI` 暴露 RootLayout、HUD screen、panel、entry widget 的 `TSoftClassPtr`；当前它作为默认 Widget Class 注册表，不承担规则真相
  * `UFinalUISubsystem` 创建 RootLayout / Battle HUD 时优先读取配置的 Widget Blueprint class，未配置或加载失败时回退 C++ `StaticClass()`
  * `BattleHUDScreen` 保留 C++ panel 装配、Slot 注入和 fallback Canvas 布局，Blueprint 子类只负责外观、槽位和容器排布
  * Battle HUD panel / entry widget 通过 `BindWidgetOptional` 绑定 Blueprint 内控件；缺少绑定控件时继续走 C++ fallback 文本展示
  * `BattleResourcePanel` 已从手牌区独立出来，作为底部资源展示容器；它只消费 `BattleSnapshot.CurrentEP / MaxEP` 生成左下 EP 气圈与 7 个 QIPip 点亮状态，EP 满值时允许 Blueprint 换色提示
  * `CardZoneDetailPanel` 是统一牌区详情面板，内部 Tab 切换 `抽牌堆 / 手牌 / 弃牌堆 / 持续区 / 消耗区`；它只读消费 `BattleSnapshot.CardZones`，不修改牌区真相，不复用手牌出牌 Entry
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

## 0.1 UI Class Tiers

当前 UI 类按长期职责分为以下层级，后续新增界面优先沿用这个分级，不再把正式 HUD、Debug 信息和 fallback dump 混在同一默认屏幕。

### Core UI
长期 UI 基础设施，所有主菜单、战斗、Run 外层、设置、地图等大类界面都继续复用。

* `UFinalUISubsystem`
* `UFinalUIRootLayout`
* `UFinalWidgetBase`
* `UFinalScreenBase`
* `UFinalPanelWidgetBase`
* `UFinalOverlayScreenBase`
* `UFinalModalScreenBase`
* `UFinalWidgetControllerBase`
* `UFinalViewModelBase`
* `UFinalUIWidgetClassSettings`

### Battle Formal HUD
玩家默认战斗界面，只承载战斗决策所需信息和命令入口。

* `UFinalBattleHUDScreen`
* `UFinalBattleWidgetController`
* `UFinalBattleCharacterPanel / UFinalBattleCharacterEntryWidget`
* `UFinalBattleEnemyPanel / UFinalBattleEnemyEntryWidget`
* `UFinalBattleEnemyDetailPanel / UFinalBattleEnemyDetailWidget`
* `UFinalBattleHandPanel / UFinalBattleCardEntryWidget`
* `UFinalBattleResourcePanel`
* `UFinalBattleFeedbackPanel`
* `UFinalBattleUltimatePanel / UFinalBattleUltimateEntryWidget`
* `UFinalBattleActionPanel` 中的 `EndTurnButton`

### Battle Debug UI
开发验证和账本观察入口，不应默认常驻在正式 HUD 中。

* `UFinalPrototypeRunDebugScreen`
* `UFinalBattleEventScreen`
* `UFinalBattleRecentEventPanel / UFinalBattleLogEntryWidget`
* `UFinalBattleTopBarPanel`
* `UFinalBattleContextPanel`
* `UFinalBattleActionPanel` 中的 `OpenDebugButton / OpenEventLedgerButton`

### Run Overlay UI
Run 外层流程界面，使用 Overlay 层承接，不直接销毁 Battle HUD。

* `UFinalRunFlowOverlayScreen`
* `UFinalRunFlowOptionButton`
* `UFinalRunGrowthChoiceOverlayScreen`
* `UFinalRunRewardOverlayScreen`
* `UFinalRunNodeOverlayScreen`
* `UFinalRunRewardNodeOverlayScreen`
* `UFinalRunEventNodeOverlayScreen`
* `UFinalRunShopNodeOverlayScreen`
* `UFinalRunStageOverlayScreenBase`
* `UFinalRunFlowPromptPanel`

### Legacy / Fallback UI
旧原型和 C++ fallback 仍保留作为兜底与参考，但不再作为默认正式 HUD 的信息来源。

* `/Game/UI/BattleHUD/WBP_BattleHUD_Ink` 当前作为旧原型 HUD 资产保留，不再作为默认 PIE HUD screen。
* C++ fallback 文本只用于未绑定 WBP 控件时的最低限度可运行展示；Debug dump 应进入 Debug overlay。
* `TopBarPanel / RecentEventPanel` 暂不删除，但从默认 C++ HUD 骨架中移出；`ContextPanel` 当前作为 Legacy / Debug 区域保留，用于牌区详情 fallback 入口。

## 0.2 UI Widget Class 配置口径

`UFinalUIWidgetClassSettings` 当前定位为开发期默认 Widget Class 注册表，用来把 C++ 父类和可替换的 WBP 类接起来。它适合保存全局默认入口，例如 `RootLayoutClass / BattleHUDScreenClass / RunFlowOverlayScreenClass`，以及当前仍在快速迭代的 Battle panel / entry 默认类。它不承载规则真相，也不应保存布局参数、颜色参数、动画参数、图标映射或状态 / 关键词资源表。

当前继续使用 `FinalUIWidgetClassSettings` 的原因：
* 可以在 `Project Settings > Final > UI` 里快速替换 WBP，不需要改 C++。
* C++ fallback 和自动化测试可以继续在未配置 WBP 时运行。
* 避免每个 UI 系统各自硬编码 `/Game/UI/...` 资产路径。
* 适合当前 UI 仍在快速成型、WBP 结构还会频繁调整的阶段。

后续收口方向：
* `FinalUIWidgetClassSettings` 长期只保留全局默认入口和少量开发期默认类。
* 当 Battle HUD、敌人详情、角色详情、牌区详情、Tooltip 等 UI 稳定后，新增 `UI Class Set` 或等价 `DataAsset` 承接一套主题 / 模式内的具体 Widget Class，例如水墨 Battle HUD、Debug Battle HUD、手柄版 HUD。
* `RootLayout / Screen` 负责实际 Slot 挂载、层级、打开关闭和输入模式；`WidgetClassSettings` 只回答“默认用哪个类”，不回答“放在哪、何时显示、谁挡输入”。
* 不要把 UI 皮肤、图标表、状态图标、Tooltip 样式、数值颜色、动画曲线继续塞进 `FinalUIWidgetClassSettings`。这些应进入 WBP、样式资源、DataTable 或后续 UI Theme / UI Class Set。
* 如果某个关卡、战斗模式或平台需要不同 HUD，不应通过临时改全局 settings 解决；应走 Screen / ClassSet / Theme 的显式切换。

## 1. 当前最小布局
* 当前战斗界面已进入 `UMG` + Blueprint 外观层阶段，由根界面统一承载主 HUD 与覆盖面板；C++ fallback HUD 继续作为未配置 Blueprint 时的兜底
* 当前默认 PIE HUD 已切回 `UFinalBattleHUDScreen` 的正式 C++ 骨架，并继续复用已配置的 `WBP_BattleHandPanel_Ink / WBP_BattleCardEntry_Ink1 / WBP_BattleResourcePanel`
* 默认 C++ HUD 骨架不再装配 `TopBarPanel / RecentEventPanel`；`ContextPanel` 作为 Legacy / Debug 入口保留，当前同时提供牌区详情的 fallback 打开按钮
* 当前主验收布局固定为 16:9 桌面：
  * 左侧：`BattleTeamPanel` 显示我方队伍整体生存信息，包括共享生命条、护盾框、三名角色简化 Entry、队伍/角色状态快捷图标；点击角色简化 Entry 打开角色详情，Vital、苏醒、崩溃次数、属性和状态说明进入角色详情或队伍状态详情面板
  * 顶部：敌方信息、生命 / 护盾 / Break / 先机、意图与阶段进度
  * 右侧：敌人详情面板，按 `InspectedEnemyUnitId` 只读展示敌人详细信息
  * 中央覆盖层：牌区详情面板，按 `SelectedCardZone` 只读展示抽牌堆、手牌、弃牌堆、持续区、消耗区
  * 底部：手牌区，保留点击出牌与快捷键出牌
  * 底部：独立 `BattleResourcePanel` 占满底部 HUD 区域，当前主要承载左下 EP 气圈
  * 右侧中下：`RunFlowPromptPanel`，当 Run 外层存在待处理流程时显示短提示，点击后重新打开统一 `RunFlowOverlay`
  * 右下：结束回合、Debug 与账本入口
* Fallback 16:9 Canvas 参考锚点：
  * 左队伍 `0.015,0.02 -> 0.23,0.48`
  * 上敌人 `0.30,0.02 -> 0.82,0.20`
  * 底手牌 `0.16,0.56 -> 0.82,0.985`
  * 底部资源容器 `0.0,0.56 -> 1.0,1.0`
  * Run 流程恢复入口 `0.72,0.48 -> 0.92,0.56`
  * 敌人详情 `0.70,0.18 -> 0.985,0.60`
  * 牌区详情 `0.24,0.14 -> 0.76,0.78`
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
* 左中：`BattleTeamPanel`
  * 队伍共享生命 `TeamCurrentHP / TeamMaxHP` 与 `TeamHealthPercent`
  * 队伍护盾 `TeamShield` 与 `TeamShieldFramePercent`
  * 三名角色简化 Entry：头像或立绘映射键 `IconId / ArtId`、压力 `Current / Cap / StressPercent`、突破 `Current / Required / BreakthroughPercent`、崩溃与突破可用提示
  * 点击角色简化 Entry 会调用 `InspectCharacterByUnitId(RuntimeUnitId)` 打开角色详情；该交互只修改 HUD 查看状态，不提交 BattleCommand，不改变当前敌人目标
  * 状态快捷区聚合 `team_player` 状态与全部角色状态，主 HUD 默认显示 5 个状态入口，超出显示 `+N`
  * 点击状态快捷区打开 `TeamStatusDetailPanel`；该面板只读显示全部队伍/角色状态，不提交 BattleCommand，不改变当前敌人目标
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
* 点击敌人场中身体命中框：切换当前目标
* 点击手牌：只触发按住预选态，松开不出牌；手牌出牌统一走拖动释放路径
* 拖动 `TargetRequirement = None` 的手牌离开手牌区域并释放：打出该牌；仍在手牌区域内释放则取消拖拽
* 拖动 `TargetRequirement = Enemy` 的手牌到敌人场中身体命中框并释放：对该敌人出牌；空白处、非敌人区域或未命中活着敌人时释放则取消拖拽
* 拖动敌方目标牌命中敌人时，卡牌回到拖拽前 hover 位置，敌人表现 Actor 进入 drop preview；离开命中框后卡牌重新跟随鼠标并清除 preview
* AP 不足牌不进入拖卡出牌状态，仍保留不可用表现；点击仍走统一请求路径并由 Battle 返回失败反馈
* 点击奥义按钮：转发 `PlayUltimate`
* 点击敌人头顶 UI：调用 `InspectEnemyByUnitId(RuntimeUnitId)` 打开 / 刷新敌人详情面板，不切换当前战斗目标

## 2.1 RootLayout 分层口径
* `UFinalUIRootLayout` 是全局 UI Layer 容器，可在 `Final > UI` 中通过 `RootLayoutClass` 替换成 WBP。推荐 WBP 父类仍为 `UFinalUIRootLayout`，并绑定 `HUDLayer / OverlayLayer / ModalLayer / TooltipLayer / ToastLayer` 五个 `Overlay` 控件。
* 未配置 `RootLayoutClass` 或 WBP 未绑定任何 Layer 时，C++ 会继续生成当前 fallback 全屏 Layer，保证 PIE 和自动化不依赖 WBP 制作进度。
* `HUD Layer`：常驻 Battle HUD，只在 `UISubsystem` 初始化时建立，不由外层页替换生命周期
* `PrototypeRunDebugScreen / FinalBattleEventScreen`：按需打开的 `Overlay Layer` 调试工具，不再和 Battle HUD 常驻同层
* `Overlay Layer`：默认由 `FinalRunFlowOverlayScreen` 承接 Run 外层主流程；当前 C++ fallback 是右侧紧凑流程面板，不再使用全屏遮罩，面板外区域应尽量保持对 Battle HUD 的点击可达；旧的战后奖励、节点选择、奖励节点、事件节点、商店节点专用页保留为显式调试 / 后续详情页入口
* `Overlay Layer`：当 `PendingGrowthChoice` 存在时，优先由独立 `FinalRunGrowthChoiceOverlayScreen` 占用；成长处理完成后再回到普通 RunFlow overlay 路径
* `Modal Layer`：承接确认、放弃、二次确认等阻断交互；优先级高于 `Overlay`
* `Tooltip / Toast Layer`：当前保留为后续扩展挂点
* 输入优先级：
  * `Modal > Overlay > Battle HUD`
  * Overlay / Modal 关闭后恢复到常驻 HUD 输入模式

## 2.1.0 BattleHUDScreen Slot 化口径
* `UFinalBattleHUDScreen` 是战斗 HUD 内部 Slot 容器；C++ 仍负责创建具体 panel、绑定 controller / view model 和刷新数据，WBP 只负责 Slot 的位置、尺寸、层级和动画。
* 推荐 `WBP_BattleHUDScreen` 父类使用 `UFinalBattleHUDScreen`，并按需绑定以下 `Overlay` Slot：`TopBarSlot / ResourceSlot / RunFlowPromptSlot / FeedbackSlot / ContextSlot / TeamPanelSlot / TeamStatusDetailSlot / CharacterPanelSlot / LegacyEnemyPanelSlot / EnemyDetailSlot / CharacterDetailSlot / CardZoneDetailSlot / UltimateSlot / HandSlot / RecentEventSlot / ActionSlot`。
* `TeamPanelSlot` 是新的左侧队伍入口；`CharacterPanelSlot` 当前仅作为兼容别名，若 WBP 尚未提供 `TeamPanelSlot`，C++ 会把 `BattleTeamPanel` 注入旧 `CharacterPanelSlot`。旧 `CharacterPanel` 不再作为 Battle HUD 常驻角色信息入口。
* `LegacyEnemyPanelSlot` 当前只作为旧敌人列表 debug / 兼容入口保留；正式 HUD 不再默认向该 Slot 注入 `EnemyPanel`，敌人目标选择改由场中表现 Actor 的命中区承担。
* 如果 WBP 直接绑定具体 panel 控件，例如 `HandPanel / EnemyDetailPanel`，C++ 继续初始化这些控件；如果 WBP 只绑定 Slot，C++ 会创建配置的 panel class 并放入对应 Slot。
* Slot 缺失时不视为错误；缺失区域继续走当前 C++ fallback Canvas 布局。这样可以先只制作 `EnemyDetailSlot / CharacterDetailSlot / RunFlowPromptSlot / FeedbackSlot / RecentEventSlot`，手牌、资源和角色面板等复杂区域后续再迁。
* Slot 根层和非交互装饰层默认应使用 `SelfHitTestInvisible` 或 `HitTestInvisible`，只让按钮、卡牌 Entry、列表项等真实交互控件参与命中，避免遮挡场中 OverHeadWidget。

## 2.1.1 Battle World 表现层口径
* `AFinalBattleDirector` 只负责把 `FinalBattleFlowSubsystem` 的 Snapshot / Event 同步到场中表现 Actor，不承载规则真相。
* 推荐通过 `BP_BattleDirector : AFinalBattleDirector` 保存战斗表现默认配置，例如 `DefaultPlayerPresentationClass / DefaultEnemyPresentationClass / EnemyPresentationClassMappings`；关卡中放置 BP 子类，避免每次直接改 C++ Actor 实例。
* `AFinalBattlePresentationActor` 只消费 `FFinalBattlePresentationUnitViewData` 并播放选中、攻击、受击、击败等表现；不再拼接或渲染单位 debug 文本。
* `AFinalBattlePresentationActor.TargetHitBox` 是场中目标选择与拖卡投放命中区，首版使用 `Visibility` trace。WBP / 蓝图子类可在视口中调整命中框位置和尺寸；点击活着的敌人命中框会经 `UFinalBattleTargetInteractorComponent` 转发到 `SelectEnemyByUnitId()`，拖卡命中则只提供目标查询并由 HandPanel 在松手时提交出牌请求，不在 Actor 或 WBP 内修改战斗真相。
* 敌人头顶 UI 当前由 `AFinalBattlePresentationActor.EnemyOverheadWidgetComponent` 挂载，Widget 父类为 `UFinalBattleEnemyOverheadWidget`。C++ 负责投影 `FFinalBattleEnemyOverheadViewData`，包括 HP / Shield / Break 百分比、先机、短意图名、意图图标 key、敌人 rank tag 与状态数组；WBP 只负责视觉绑定、图标映射和动画。OverHead 属于轻量战场信息：HP 文本只显示当前血量，意图只显示 `EnemyView.CurrentIntent.DisplayName` 与一个图标，完整意图说明使用 `EnemyView.CurrentIntent.PreviewText` 放在敌人详情面板。
* `UFinalBattleEnemyOverheadWidget` 会自动刷新一组可选绑定控件：`NameText / HPText / HealthBar / ShieldFrameBar / BreakBar / InitiativeText / IntentText / IntentIconImage / StatusBox / TargetedVisual / DefeatedVisual`。WBP 中存在同名控件即可自动接线；缺失控件不会报错。`IntentIconImage` 的资源映射由 widget 父类上的 `IntentIconBrushes` 读取，key 为 snapshot 提供的 `IntentIconId`；缺少 brush 时可隐藏图标或显示默认 brush。
* `UFinalBattleEnemyOverheadWidget` 支持可选 `InspectButton` 绑定。WBP 推荐放置一个覆盖头顶 UI 根区域的透明 Button，命名为 `InspectButton` 并勾选 Is Variable；点击只打开详情，不选择目标。若没有该按钮，C++ 保留左键点击 fallback，但正式表现应优先使用 Button 控制命中范围。
* `EnemyOverheadWidgetComponent` 使用 `UFinalBattleOverheadWidgetComponent` 由 C++ 开启硬件输入，OverHead 根 Widget 在允许点击时保持 hit-testable。WBP 中 `InspectButton` 本身必须是 `Visible`，装饰性图片、进度条和文本可设为 `Hit Test Invisible`，避免遮挡透明按钮。`WidgetComponent` 的 `Begin Cursor Over` 不是 Screen-space UMG 点击链路的可靠验证方式，验证时应以 `InspectButton.OnClicked` 或敌人详情面板打开为准。
* Battle HUD 常驻根层、HUD Layer 和 BattleHUDScreen 默认使用 `SelfHitTestInvisible`，只让具体按钮、卡牌 Entry、列表项等交互控件参与命中。不要在 WBP 根 Canvas 或全屏装饰层上保持 `Visible` 命中状态，否则会挡住场中 Screen-space OverHeadWidget。
* 玩家单位本轮不显示敌人头顶组件；后续玩家头顶 UI 需要单独定义 ViewData 或复用更通用的 UnitOverhead 结构。
* Battle 场景内不再使用 `TextRenderComponent` 显示 BattleDirector summary 或单位详情；调试信息统一放到 Debug Screen、Battle Log、HUD feedback。
* 后续正式场中头顶 UI 应基于结构化 ViewData 构建 `WidgetComponent / UnitOverheadWidget`，不要解析中文详情字符串，也不要在 WBP 中回查战斗规则真相。

## 2.1.2 Battle Enemy Detail Panel 口径
* `SelectedTargetUnitId / CurrentTargetUnitId` 是战斗目标状态，负责出牌目标与目标高亮；`InspectedEnemyUnitId` 是 HUD 只读查看状态，负责敌人详情面板。两者长期分离，可以查看 A 敌人同时保持 B 敌人为当前战斗目标。
* `UFinalBattleWidgetController.InspectEnemyByUnitId()` 只修改 `InspectedEnemyUnitId` 并刷新 HUD，不提交 `SelectTarget`，不改变 `CurrentTargetUnitId`，不触发任何 battle command。
* `UFinalBattleEnemyDetailPanelController` 从 `CachedSnapshot + DataRegistry + InspectedEnemyUnitId` 构建 `FFinalBattleHUDEnemyDetailData`，包括名称、rank tag、HP / Shield / Break、先机、意图名、意图详情、阶段文本、是否当前目标、是否存活与状态详情。意图名 / 意图详情优先消费 `FFinalBattleIntentViewData`，不再为了显示意图短名反查 `EnemyIntentDefinition`。
* `UFinalBattleEnemyDetailPanel / UFinalBattleEnemyDetailWidget / UFinalBattleEnemyDetailStatusLineWidget` 都可以在 `Final > UI` 的 `FinalUIWidgetClassSettings` 中配置。没有完整 Battle HUD WBP 时，C++ fallback `EnemyDetailPanel` 会优先使用配置的 `BattleEnemyDetailWidgetClass` 承接详情表现；若未配置，则回退到面板内置文本兜底。
* `UFinalBattleEnemyDetailWidget` 是 WBP 父类。C++ 提供可选绑定控件与 `OnEnemyDetailViewApplied(ViewData)`，WBP 只负责布局、图标、状态行样式和关闭按钮表现。推荐绑定名：`ContentRoot / EmptyText / CloseButton / TitleText / RankText / TargetStateText / HPText / HealthBar / ShieldText / ShieldFrameBar / BreakText / BreakBar / InitiativeText / IntentIconImage / IntentNameText / IntentDetailText / StatusBox / EmptyStatusText`。旧 `IntentText` 仍作为完整意图说明兼容绑定；`IntentIconImage` 与 OverHead 使用同样的 `IntentIconId -> Brush` 映射口径；`CloseButton` 点击后自动调用 `ClearInspectedEnemy()`；未配置正式 WBP 时，C++ fallback 面板也必须提供关闭按钮。
* 敌人详情状态行使用 `UFinalBattleEnemyDetailStatusLineWidget` 作为 WBP 父类。推荐绑定名：`StatusNameText / StackText / DurationText / SummaryText`。`UFinalBattleEnemyDetailWidget.StatusLineWidgetClass` 可指定状态行 WBP；未指定时优先使用 `FinalUIWidgetClassSettings.BattleEnemyDetailStatusLineWidgetClass`，再回退到 C++ 状态行。
* 若 inspected 敌人不存在，详情面板自动清空并隐藏。敌人死亡但仍存在于 snapshot 时，ViewData 通过 `bIsAlive=false` 交给 WBP 表现死亡态。
* 敌人 OverHeadWidget 点击当前已接入 `InspectEnemyByUnitId()`，不要通过旧 `EnemyPanel` 绕一层，也不要把详情打开逻辑和目标选择逻辑绑定在一起。
* 旧 `EnemyPanel` 已退出正式 HUD 目标选择入口；相关类保留为 legacy/debug 兼容，拖卡目标选择复用场中 `TargetHitBox` 链路。

## 2.1.3 Battle Character Detail Panel 口径
* `InspectedCharacterUnitId` 是 HUD 只读查看状态，负责角色详情面板；它不提交 BattleCommand，不改变 `SelectedTargetUnitId / CurrentTargetUnitId`，也不影响敌人目标选择。
* `InspectedCharacterUnitId` 与 `InspectedEnemyUnitId` 当前互斥：查看角色时清空敌人详情，查看敌人时清空角色详情。右侧详情区域首版只显示一个详情对象，避免角色详情和敌人详情叠在同一屏幕区域。
* `UFinalBattleWidgetController.InspectCharacterByUnitId()` 只验证 snapshot 中存在该角色并刷新 HUD；如果角色不存在或切换战斗后不再存在，详情面板自动清空并隐藏。
* `UFinalBattleCharacterDetailPanelController` 从 `CachedSnapshot + RunSnapshot + DataRegistry + InspectedCharacterUnitId` 构建 `FFinalBattleHUDCharacterDetailData`，包括名称、等级、定位标签、压力、VitalShare、突破、苏醒计数、崩溃次数、成长属性、运行时属性、状态、被动和奥义详情。
* 角色运行时属性由 `FinalBattle` snapshot 直接提供：`RuntimeAttack / RuntimeDefense / RuntimeBreakRate / RuntimeCritChance / RuntimeCritDamage`。WBP 只显示 ViewData，不回查 battle runtime，也不自行计算规则真相。
* `UFinalBattleCharacterDetailPanel / UFinalBattleCharacterDetailWidget / UFinalBattleCharacterDetailStatusLineWidget / UFinalBattleCharacterDetailPassiveLineWidget` 都可以在 `Final > UI` 的 `FinalUIWidgetClassSettings` 中配置。没有正式 WBP 时，C++ fallback `CharacterDetailPanel` 会显示可读文本并提供关闭按钮。
* `UFinalBattleCharacterDetailWidget` 是 WBP 父类。C++ 提供可选绑定控件与 `OnCharacterDetailViewApplied(ViewData)`，WBP 只负责布局、头像映射、条形图、状态行、被动行和关闭按钮表现。推荐绑定名：`ContentRoot / EmptyText / CloseButton / TitleText / RoleText / StateText / StressText / StressBar / BreakthroughText / BreakthroughBar / VitalText / AwakenText / CollapseText / GrowthText / RuntimeStatsText / StatusBox / EmptyStatusText / PassiveBox / EmptyPassiveText / UltimateNameText / UltimateCostText / UltimateStateText / UltimateRulesText`。
* 角色详情状态行使用 `UFinalBattleCharacterDetailStatusLineWidget` 作为 WBP 父类。推荐绑定名：`StatusNameText / StackText / DurationText / SummaryText`。`UFinalBattleCharacterDetailWidget.StatusLineWidgetClass` 可指定状态行 WBP；未指定时优先使用 `FinalUIWidgetClassSettings.BattleCharacterDetailStatusLineWidgetClass`，再回退到 C++ 状态行。
* 角色详情被动行使用 `UFinalBattleCharacterDetailPassiveLineWidget` 作为 WBP 父类。推荐绑定名：`PassiveNameText / StackText / DurationText / SummaryText`。`UFinalBattleCharacterDetailWidget.PassiveLineWidgetClass` 可指定被动行 WBP；未指定时优先使用 `FinalUIWidgetClassSettings.BattleCharacterDetailPassiveLineWidgetClass`，再回退到 C++ 被动行。
* `UFinalBattleCharacterEntryWidget` 与 `UFinalBattleTeamCharacterEntryWidget` 当前都支持整块点击打开角色详情，也支持可选 `InspectButton`。该按钮只负责打开详情，不负责角色选择、奥义释放或任何规则结算。后续如果加入玩家场中头顶 UI，应复用 `InspectCharacterByUnitId()`，不要把角色详情打开逻辑写进 Battle 规则层。

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
* 场中单位表现用 `FFinalBattlePresentationUnitViewData`，由 `FinalApp/World` 从 Battle Snapshot 投影生成，服务 2D 精灵和后续头顶 UI，不作为战斗规则真相

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
