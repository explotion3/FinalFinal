# 战斗 UI 线框

## 0. 当前实现状态（2026-04-10）
* 当前 `FinalApp` 已补上首轮 `UI` 基座：
  * `UISubsystem`
  * `UIRootLayout`
  * `Screen / Panel / Widget / WidgetController / ViewModel` 基类
  * 代码生成的 `BattleHUDScreen`
* 当前 Battle HUD 通过 `FinalBattleWidgetController` 订阅 `FinalBattleFlowSubsystem`
* 当前 Battle HUD 已打通：
  * `Snapshot -> ViewModel -> HUD`
  * 敌人目标选择
  * 点击手牌出牌
  * `1~6` 快捷出牌
  * 点击 / `Enter / Space` 结束回合
* 当前 Battle HUD 仍属于首轮桥接，不代表规则层字段已经齐全

## 1. 当前最小布局
* 当前战斗界面已进入 `UMG` 过渡阶段，由根界面统一承载主 HUD 与覆盖面板；旧 `Canvas HUD` 仅保留兜底
* 左上：回合、遭遇名、阶段进度、`AP`、`EP`、队伍生命、护盾、金币、牌堆计数、战斗反馈
* 左中：三名角色状态
  * 角色名
  * 压力 `Current / Cap`
  * 崩溃 / 苏醒进度
  * 角色私有状态
  * 角色奥义名、`EP` 消耗、可释放状态
* 右侧：敌人面板
  * 名称
  * `HP / Shield / Break`
  * `Init`
  * 当前意图
  * 敌方状态
* 底部左侧：最近事件日志
* 底部中部：手牌区
* 底部右侧：结束回合按钮
* 右下：三名角色奥义快捷按钮
* 中央覆盖层：奖励、事件、商店、变体、节点选择、章节完成/失败

## 2. 当前输入映射
* `1~6`：按手牌序号出牌
* `Enter / Space`：结束回合
* `F1 / F2 / F3`：预留为霍断岳 / 叶半夏 / 沈清弦奥义
* 点击敌人：切换当前目标
* 点击手牌：打出该牌
* 点击奥义按钮：当前仅显示占位，不在首轮启用

## 3. 当前已桥接字段
### 3.1 Battle Snapshot 已可直接驱动
* 回合数
* 当前 `AP`
* 当前 `EP`
* `EP` 上限
* 队伍当前生命 / 最大生命
* 队伍护盾
* 当前目标 `RuntimeUnitId`
* 牌堆 / 手牌 / 弃牌 / 持续区 / 消耗区计数
* 角色运行时单位 `RuntimeUnitId`
* 角色 `CharacterId`
* 角色显示名
* 角色当前压力
* 角色压力上限
* 角色是否崩溃
* 角色当前苏醒计数 / 苏醒阈值
* 角色崩溃次数
* 角色生命份额
* 角色奥义 `UltimateId / DisplayName / CostEP / CanActivate`
* 敌人名称
* 敌人站位
* 敌人当前生命 / 护盾 / Break / 先机
* 敌人当前意图 `Id`
* 敌人当前意图文案
* 手牌实例 `CardInstanceId`
* 手牌 `CardId`
* 手牌所属单位 `RuntimeOwnerUnitId`
* 手牌显示名
* 手牌当前消耗 `AP`
* 手牌关键词 / 是否保留 / 是否崩溃牌
* 状态列表 `StatusId / Owner / Stacks / Duration`
* 最近战斗日志文本

### 3.2 FinalApp 通过 Data / Run 补出的展示字段
* 遭遇名
* 当前金币
* 卡牌显示名
* 卡牌规则文本

## 4. 当前 HUD 还未消费的已公开字段
以下字段 Battle 侧已经公开，但当前首轮 `BattleHUDScreen` 仍未全部接入展示：
* `Statuses` 状态列表
* 角色苏醒计数 / 阈值
* 角色崩溃次数
* 角色生命份额
* 奥义当前消耗 / 可用态 / 定义就绪态
* 抽牌堆 / 弃牌堆 / 消耗区计数
* 当前目标 `RuntimeUnitId`
* 手牌所属单位 `RuntimeOwnerUnitId`

## 5. 当前仍缺口字段 / 需要 Battle 或 Run 继续补的接口
以下字段当前仍不能由 `FinalApp/UI` 安全推导，不能在 UI 层硬补：
* `Team Status` 列表
* `Character Status` 列表
* 奥义是否本战已释放
* 当前阶段进度的公开查询值
* 更细粒度的即时反馈类型（如 `EP 不足 / 无合法目标 / 奥义已释放` 的结构化原因）

建议后续由 Battle / Run 公开以下稳定查询结构，而不是让 UI 猜：
* `BattleSnapshot.TeamStatuses`
* `BattleSnapshot.CharacterStatuses` 的团队 / 角色归属约束
* `BattleSnapshot.UltimateEntries.bUsedThisBattle`
* `BattleSnapshot.PhaseProgress`
* `BattleCommandResult.ReasonTag` 或同等可枚举失败原因
## 6. 必须显示的信息
* `EP` 必须独立显示当前值与上限
* 当 `EP` 足以释放某角色奥义时，该角色奥义按钮必须显示为可用
* 顶部必须保留一条短反馈，用于显示 `EP 不足 / 角色崩溃 / 无合法目标 / 奥义已释放` 这类即时结果
* `Team Status` 只显示团队状态
* `Character Status` 只显示角色私有状态
* 奖励 / 事件 / 商店 / 节点选择使用全屏覆盖面板，不隐藏顶部关键资源信息
* 节点选择中的战斗类节点至少区分 `Battle / Elite / Boss`
