# 战斗 UI 线框

## 0. 当前实现状态（2026-04-10）
* 当前 `FinalApp` 已补上首轮 `UI` 基座：
  * `UISubsystem`
  * `UIRootLayout`
  * `Screen / Panel / Widget / WidgetController / ViewModel` 基类
  * 代码生成的 `BattleHUDScreen`
* 当前 Battle HUD 通过 `FinalBattleWidgetController` 订阅 `FinalBattleFlowSubsystem`
* 当前 Battle HUD 已打通：
  * `Snapshot / Event -> WidgetController -> ViewModel -> HUD`
  * 敌人目标选择
  * 点击手牌出牌
  * 点击奥义按钮转发 `PlayUltimate`
  * `1~6` 快捷出牌
  * 点击 / `Enter / Space` 结束回合
* 当前 Battle HUD 已开始消费 Battle / Run 公开查询字段，但仍只做只读展示与命令转发，不承载规则结算

## 1. 当前最小布局
* 当前战斗界面已进入 `UMG` 过渡阶段，由根界面统一承载主 HUD 与覆盖面板；旧 `Canvas HUD` 仅保留兜底
* 左上：回合、遭遇名、`AP`、`EP`、队伍生命、护盾、金币、遗物数、战斗反馈
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
* 中央覆盖层：奖励、事件、商店、变体、节点选择、章节完成/失败

## 2. 当前输入映射
* `1~6`：按手牌序号出牌
* `Enter / Space`：结束回合
* `F1 / F2 / F3`：预留为霍断岳 / 叶半夏 / 沈清弦奥义
* 点击敌人：切换当前目标
* 点击手牌：打出该牌
* 点击奥义按钮：转发 `PlayUltimate`

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
当前这轮 Battle HUD 主链路所需的新增公开字段已全部接入，当前没有新增的阻塞性接口缺口。

当前实现口径：
* 团队状态直接来自 `FFinalBattleSnapshot.TeamStatuses`
* 角色状态直接来自 `FFinalBattleSnapshot.CharacterStatuses`
* 敌方状态继续来自总 `FFinalBattleSnapshot.Statuses`
* 阶段进度来自 `FFinalBattleEnemyViewData.PhaseProgress`
* 奥义“本战已释放”来自 `FFinalBattleUltimateViewData.bUsedThisBattle`
* 结构化交互反馈来自 `FFinalBattleEvent.RejectReason / ReasonTag`
## 5. 必须显示的信息
* `EP` 必须独立显示当前值与上限
* 当 `EP` 足以释放某角色奥义时，该角色奥义按钮必须显示为可用
* 顶部必须保留一条短反馈，用于显示 `EP 不足 / 角色崩溃 / 无合法目标 / 奥义已释放 / 战斗未初始化 / 命令不支持` 这类即时结果
* `Team Status` 只显示团队状态
* `Character Status` 只显示角色私有状态
* 奖励 / 事件 / 商店 / 节点选择使用全屏覆盖面板，不隐藏顶部关键资源信息
* 节点选择中的战斗类节点至少区分 `Battle / Elite / Boss`
