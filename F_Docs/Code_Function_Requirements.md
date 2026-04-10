# 代码功能需求

## 1. 文档定位
本文档用于把当前设计文档中的玩法需求，拆解成 Unreal 项目真正需要落地的代码功能块。  
它不负责规定目录结构，也不负责定义字段细节，而是回答两个问题：
* 代码需要实现哪些系统
* 每个系统负责什么输入、输出与边界

本文档默认服务于：
* [GDD4.0.md](GDD4.0.md)
* [Battle_Rules.md](Battle_Rules.md)
* [Status_System_Guide.md](Status_System_Guide.md)
* [Card_Design_Guide.md](Card_Design_Guide.md)
* [Combat_Data_Schema_v2.md](Combat_Data_Schema_v2.md)

---

## 2. 首版代码目标
首版代码需要优先满足：
* 战斗规则按固定时序稳定结算
* 静态内容可由 DataAsset 驱动，不靠硬编码卡牌与敌人
* 战斗内状态与单局外状态分层清楚
* UI 与表现层不直接改写规则真相
* 后续新增角色、敌人、遗物、事件时尽量少改底层

首版不追求：
* 过早插件化
* 过度抽象的一套万能框架
* 依赖 Tick 的实时驱动
* 把全部玩法都塞进 Blueprint

---

## 3. 总体分层
代码侧至少应拆成五层：
* 内容定义层：静态定义、DataAsset、共享协议
* 运行时状态层：战斗内与单局外的权威状态
* 规则执行层：命令校验、效果解析、结算顺序、窗口处理
* 外层编排层：地图、节点、奖励、事件、进入战斗、战后结算
* 表现接入层：UI、Actor、动画、特效、音频、调试界面

硬性边界：
* 表现层不能直接改写权威状态
* 战斗规则层不能直接依赖 Widget 与场景 Actor
* 单局外系统不能直接操作单张牌的战斗结算
* 静态定义层不承担运行时逻辑

---

## 4. 战斗内核心功能

### 4.1 战斗初始化
职责：
* 根据遭遇模板、规则配置与当前单局状态建立一场战斗
* 初始化敌人、队伍、抽牌堆、手牌区、持续区、消耗区
* 处理开战关键词、跨战斗保留崩溃、开战初始状态

输入：
* `BattleEncounterDefinition`
* `BattleRuleConfig`
* `RunPersistentState`

输出：
* `BattleState`
* 初始 `BattleEvent` 日志

优先级：
* `P0`

### 4.2 战斗命令入口
职责：
* 接收玩家或系统发出的战斗命令
* 校验命令是否合法
* 把合法命令交给规则结算层

典型命令：
* 打牌
* 释放奥义
* 结束回合
* 选择目标

说明：
* `BattleCommand` 只承载战斗内命令
* 奖励选择、事件选项、商店购买、成长分支等都属于 `RunCommand`

优先级：
* `P0`

### 4.3 卡牌与牌区循环
职责：
* 维护抽牌堆、手牌区、弃牌堆、持续区、消耗区
* 处理抽牌、弃牌、生成、复制、回收、进入持续区
* 处理关键词对牌区去向的改写

重点规则来源：
* [Battle_Rules.md](Battle_Rules.md)
* [Card_Design_Guide.md](Card_Design_Guide.md)

优先级：
* `P0`

### 4.4 资源系统
职责：
* 维护 AP、EP、Break 奖励 AP、受击 EP、普通牌基础 EP
* 处理奥义消耗与 EP 获取公式
* 处理回合开始、回合结束、受击、打牌等窗口的资源变化

优先级：
* `P0`

### 4.5 伤害、治疗、压力
职责：
* 计算实际伤害、实际生命损失、实际回复量
* 处理共享血条与角色压力的转化
* 统一暴击、护盾、减伤、易伤、士气等常见修正

优先级：
* `P0`

### 4.6 Break 与先机
职责：
* 处理攻击转削韧
* 处理中途 Break 检查
* 处理先机减少事件
* 处理敌人插队时机与同窗口优先级

优先级：
* `P0`

### 4.7 状态系统
职责：
* 处理状态归属
* 处理叠层、刷新、覆盖、拒绝获得
* 按固定窗口结算通用状态与专属状态
* 处理共享血条下 `team_player` 与角色个人状态边界

优先级：
* `P0`

### 4.8 被动与遗物触发
职责：
* 处理战斗内被动
* 处理遗物触发窗口
* 记录每回合、每战斗、每效果的触发次数
* 不允许遗物直接绕过命令与规则结算层改状态

优先级：
* `P1`

### 4.9 崩溃与苏醒
职责：
* 处理角色崩溃
* 处理崩溃卡转换
* 处理苏醒计数、直苏概率、苏醒后恢复
* 处理跨战斗保留的崩溃状态

优先级：
* `P0`

### 4.10 敌人意图与行动
职责：
* 选择敌方当前意图
* 处理敌方多段攻击、召援、蓄力、强化
* 处理敌方已行动状态和行动顺序
* 按运行时站位处理同窗口优先级

优先级：
* `P0`

### 4.11 事件日志与回放基础
职责：
* 记录规则层发生了什么
* 为 UI、表现层、调试工具提供统一事件流
* 为后续回放、战斗日志、QA 检查保留基础

当前稳定公开面：
* `BattleEvent` 已承载 `EventSequence`
* 事件已可携带来源 / 目标单位、关联卡牌 / 奥义 / 状态、关键数值、战斗结果
* `BattleSession` 已提供全量读取与按序号增量读取，供 `FinalApp/UI` 做事件驱动刷新

优先级：
* `P1`

---

## 5. 单局外核心功能

### 5.1 单局持久状态
职责：
* 维护当前角色 roster
* 维护牌组、遗物、金币、事件结果
* 维护跨战斗保留状态，例如 `CollapseCount`
* 为战斗开始提供当前队伍、当前牌组和遭遇输入
* 在战斗结束后消费战斗结果并回写单局状态

优先级：
* `P0`

### 5.2 单局外命令入口
职责：
* 接收事件选项、奖励选择、商店购买、成长分支等单局外命令
* 校验条件、代价与可选项是否合法
* 把合法命令交给 Run 层解析器处理

典型命令：
* 进入节点
* 确认事件选项
* 确认战后奖励
* 购买商店内容
* 选择成长分支

优先级：
* `P0`

### 5.3 地图与节点推进
职责：
* 维护当前章节、节点池、节点选择结果
* 组织普通战、精英战、商店、事件、休整、首领战入口
* 对外查询面至少公开 `CurrentChapter / CurrentFloor / 节点展示名或展示标签 / 已访问 / 锁定状态与原因 / 候选节点展示数据`

优先级：
* `P1`

### 5.4 事件系统
职责：
* 呈现事件选项
* 校验选项条件与代价
* 结算事件奖励、删牌、加牌、成长分支、压力变化

优先级：
* `P1`

### 5.5 奖励与商店
职责：
* 生成战后奖励
* 管理删牌、购牌、买遗物、恢复、重铸等行为
* 维护单局构筑修正路径
* 奖励协议中的“真正授予对象”应使用稳定的 typed payload 标识，例如 `GrantedCardId / GrantedRelicId`，不能长期把 `DisplayId` 当作权威玩法身份
* 当前 `RunState` 至少应真实承接：`Gold -> Gold`、`CardGrant -> RunDeck`、`RelicGrant -> Relics`
* `RemoveCard / UpgradeCard / Growth` 若协议仍不足，当前阶段应明确拒绝而不是伪造成功
* 战后奖励查询面至少公开结构化 `RewardEntries`，可扩展到金币、卡牌、遗物、删牌与升级牌
* 非战斗节点查询面至少公开 `PendingRewardNode / PendingEventNode / PendingShopNode` 的最小结构化内容，供 UI 读取标题、简介、选项、商品与可执行状态

优先级：
* `P1`

### 5.6 角色成长入口
职责：
* 发放特有卡
* 升级或分支变体
* 解锁奥义与技能树节点

优先级：
* `P1`

### 5.7 Run 查询与事件流
职责：
* 为 `FinalApp` 与调试工具提供 `RunSnapshot`
* 提供结构化 `RunEvent`，记录初始化、战前桥接、RunCommand、战后回写等关键外层流程
* 提供全量读取与按序号增量读取，避免 UI 只能猜当前 Run 状态

优先级：
* `P1`

---

## 6. 内容与数据功能

### 6.1 定义资产加载
职责：
* 加载角色、卡牌、敌人、状态、遗物、事件、遭遇、规则配置等定义
* 通过稳定 ID 提供查询入口

优先级：
* `P0`

### 6.2 资源校验
职责：
* 校验 ID 是否重复
* 校验外部引用是否缺失
* 校验效果字段是否与协议匹配
* 校验文案、关键词、类型是否符合规范

优先级：
* `P1`

### 6.3 数据查询与索引
职责：
* 按 `CardId / EnemyId / EventId / RelicId` 查询
* 按标签、章节、稀有度、角色归属、推荐阶段做筛选
* 为 Battle / Run / UI 提供 `Enemy / EnemyIntent / Status / Ultimate` 等静态定义查询

优先级：
* `P1`

---

## 7. 表现与外层接入

### 7.1 UI 编排与视图模型
职责：
* 在 `FinalApp` 中维护运行时 UI 根布局与页面层级
* 采用“常驻 HUD + Overlay + Modal + Tooltip + Toast”分层
* 把权威状态转成 UI 可读数据
* 不在 Widget 中做规则推导
* 优先事件驱动刷新，不依赖 Tick 或 Blueprint Binding 轮询权威状态

最低要求：
* `UISubsystem` 负责根布局、页面栈、输入模式与焦点恢复
* `WidgetController` 负责订阅 `BattleSnapshot / BattleEvent / RunQuery` 并组装 `ViewModel`
* `ViewModel` 不绑定某一版具体布局
* `Widget` 只读 `ViewModel`，不直接访问 `BattleState / RunState` 私有结构

当前已落地：
* `UISubsystem + UIRootLayout + BattleHUDScreen`
* `FinalBattleWidgetController` 已可把 `Snapshot / Event` 转成首轮 `HUD Presentation`
* `FinalApp` 可结合 `FinalData / RunSession` 补齐遭遇名、金币、`EP` 上限、角色名、卡牌名等展示字段

优先级：
* `P0`

### 7.2 世界桥接
职责：
* 在场景中生成和维护角色 / 敌人表现 Actor
* 接收战斗事件并驱动表现
* 不在 Actor 中改写战斗真相

优先级：
* `P0`

### 7.3 输入与交互
职责：
* 承接战斗内的手牌点击、目标选择、结束回合输入
* 把战斗内输入转换成 `BattleCommand`
* 把事件、奖励、商店、成长等单局外输入转换成 `RunCommand`
* 明确 `GameOnly / GameAndUI / UIOnly` 的切换时机
* 在覆盖页、模态页打开与关闭时恢复焦点，不让单个 Widget 各自持有输入模式真相

优先级：
* `P0`

### 7.4 音画反馈
职责：
* 根据战斗事件播放动画、特效、音频、浮字
* 不参与最终数值判定

优先级：
* `P1`

---

## 8. 保存、调试与测试支撑

### 8.1 Save / Load
职责：
* 保存单局外状态
* 支持中断恢复
* 支持战斗外继续读取 `CollapseCount` 等持久字段

优先级：
* `P1`

### 8.2 调试工具
职责：
* 查看当前战斗状态
* 查看敌人意图
* 查看状态实例、被动实例、遗物触发记录
* 输出战斗事件日志

优先级：
* `P1`

### 8.3 自动化校验
职责：
* 基础规则回归
* 数据资产合法性检查
* 关键战斗链路冒烟测试

优先级：
* `P2`

---

## 9. 首版优先级建议

### 9.1 P0 必做
* 内容定义加载
* 单局持久状态
* 单局外命令入口
* 战斗初始化
* 战斗命令入口
* 卡牌与牌区循环
* 资源系统
* 伤害、治疗、压力
* Break 与先机
* 状态系统
* 崩溃与苏醒
* 敌人意图与行动
* UI 视图模型
* 世界桥接
* 输入与交互

### 9.2 P1 应做
* 被动与遗物触发
* 事件系统
* 奖励与商店
* 角色成长入口
* 数据查询与索引
* 资源校验
* 战斗事件日志
* Save / Load
* 音画反馈
* 调试工具

### 9.3 P2 后续补
* 自动化校验
* 完整回放
* 编辑器增强工具

---

## 10. 功能归属总表

| 功能块 | 主归属模块 | 对外入口 | 不应放入 |
| --- | --- | --- | --- |
| 定义资产加载 | `FinalData` | 数据查询服务 / 资源注册表 | `FinalBattle`、`FinalApp` |
| 战斗初始化 | `FinalBattle` | `BattleSession` | `BattleGameMode` |
| 战斗命令入口 | `FinalBattle` | `SubmitCommand` | Widget、Actor |
| 卡牌与牌区循环 | `FinalBattle` | 卡牌服务 / 结算器 | `FinalRun` |
| 资源系统 | `FinalBattle` | 资源服务 | UI |
| 伤害、治疗、压力 | `FinalBattle` | 结算器 / 原子操作 | Blueprint |
| Break 与先机 | `FinalBattle` | Break 服务 / 回合服务 | `FinalRun` |
| 状态系统 | `FinalBattle` | 状态服务 | Widget |
| Battle 只读查询与事件流 | `FinalBattle` | `BattleSnapshot / BattleEvent / EventsSince` | `FinalApp` 内部私有缓存真相 |
| 崩溃与苏醒 | `FinalBattle` | 崩溃苏醒服务 | `FinalApp` |
| 敌人意图与行动 | `FinalBattle` | 意图服务 / 回合服务 | 世界表现 Actor |
| 单局持久状态 | `FinalRun` | `RunSession` | `FinalBattle` |
| 单局外命令入口 | `FinalRun` | `SubmitRunCommand` | `FinalBattle` |
| 地图与节点推进 | `FinalRun` | 节点解析器 | `FinalBattle` |
| 事件系统 | `FinalRun` | 事件解析器 | Widget |
| 奖励与商店 | `FinalRun` | 奖励 / 商店解析器 | `FinalBattle` |
| 角色成长入口 | `FinalRun` | 成长解析器 | `FinalBattle` |
| Run 只读查询与事件流 | `FinalRun` | `RunSnapshot / RunEvent / EventsSince` | `FinalApp` 内部私有缓存真相 |
| UI 页面栈与根布局 | `FinalApp` | `UISubsystem / RootLayout` | `FinalBattle`、`FinalRun` |
| UI 视图模型 | `FinalApp` | Widget Controller / ViewModel / HUDScreen | `FinalBattle` 内部服务 |
| 世界桥接 | `FinalApp` | Flow Subsystem / Director | `FinalBattle` |
| Save / Load | `FinalApp` | Save 协调器 | `FinalBattle` 内部类 |
| 数据校验 / 编辑器工具 | `FinalEditor` | 编辑器菜单 / 校验器 | Runtime 模块 |

说明：
* `FinalBattle` 与 `FinalRun` 都不应直接依赖彼此
* 二者通过 `FinalData` 的静态定义与 `FinalApp` 的桥接流程协作
* UI 只读权威状态，不直接写入战斗或单局状态

---

## 11. 首批最小可玩闭环

### 11.1 目标链路
首批代码至少需要跑通下面这条链路：
1. 进入一个普通战节点
2. `FinalGameFlowSubsystem` 从 `RunSession` 发起战斗输入组装
3. `FinalBattleFlowSubsystem` 创建 `BattleSession`
4. 完成战斗开始初始化
5. 玩家打出 1 张牌
6. 结算伤害、削韧、Break、先机变化
7. 敌人行动
8. 回合结束并进入下一回合
9. 战斗胜利
10. `FinalGameFlowSubsystem` 提交 `BattleResult` 并回写 `RunSession`
11. `RunSession` 进入明确的 `PendingBattleReward` 外层状态
12. 外层领取战后奖励
13. 外层显式推进到下一节点

### 11.2 首批必须有的公开接口
* `BootstrapNewRun()`
* `ConfigureBattleStartState(...)`
* `StartBattleFromRunSession()`
* `SubmitBattleCommand(Command)`
* `SubmitRunCommand(Command)`
* `GetCurrentBattleSnapshot()`
* `GetBattleLogEntries()`
* `GetBattleEventsSince(Sequence)`
* `GetRunSnapshot()`
* `GetRunLogEntries()`
* `GetRunEventsSince(Sequence)`
* `CompleteBattleAndApplyResult(Result)`
* `ClaimPendingBattleReward()`
* `AdvanceToNextNode(NodeId)`

### 11.2.1 首批必须有的 Run 只读查询面
* `RunSnapshot.PendingBattleReward`
* `RunSnapshot.PendingRewardNode`
* `RunSnapshot.PendingEventNode`
* `RunSnapshot.PendingShopNode`
* `RunSnapshot.Progression`

### 11.3 首批必须有的最小状态
* `RunState`
* `BattleState`
* `BattleCharacterState`
* `BattleEnemyState`
* `TeamDeckState`
* `BattleCardInstance`
* `BattleStatusInstance`

### 11.4 首批必须可录入的最小资产
* `BattleRuleConfig`
* `CharacterDefinition`
* `CardDefinition`
* `UltimateDefinition`
* `EnemyDefinition`
* `EnemyIntentDefinition`
* `StatusDefinition`
* `BattleEncounterDefinition`

### 11.5 首批明确不进闭环的内容
* 商店
* 复杂事件链
* 技能树
* 多阶段首领
* 完整存档恢复
* 编辑器批量工具

---

## 12. 首版必须预留的扩展点

### 12.1 战斗侧
* 效果系统必须允许继续增加新的 `BattleEffectDefinition` 子类
* 状态系统必须允许新增状态类别、叠加规则和刷新规则
* 敌人行动系统必须允许加入召援、多阶段、阶段切换与特殊意图
* 牌区系统必须预留持续区、消耗区、生成牌与复制牌的扩展空间

### 12.2 单局外侧
* `RunSession` 必须允许扩展事件、商店、遗物、角色成长
* 奖励系统必须允许后续加入“删牌、加牌、升级牌、角色成长分支”
* 已落地的奖励类型需要在 `RunState` 内真实反映，而不是只停留在 Snapshot 展示层
* 节点推进必须允许后续加入特殊节点类型

### 12.3 表现层
* ViewModel 不绑定某一版 UI 布局
* Root HUD 与 `Overlay / Modal` 分层必须可扩展，不因新增页面而改动 Battle / Run 规则层
* UI 页面栈、输入模式、焦点恢复必须集中由 `FinalApp` 管理
* 战斗事件日志必须能继续服务动画、音效、调试面板和回放
* 世界桥接层必须允许替换表现 Actor 而不影响规则层
* 首轮代码生成的 `BattleHUDScreen` 只是承接层，后续替换成 Widget Blueprint 不应改变 `WidgetController / ViewModel` 合约

### 12.4 工程侧
* 模块间不通过 include 私有实现偷引用
* 公共 API 尽量以 Session、Query、Request、Result 这类稳定接口暴露
* 不提前引入 GAS、行为树或 Tick 驱动，除非后续明确某个系统无法承载

---

## 13. 与源码架构的关系
本文档只负责把功能拆开。  
实际 Unreal 模块、目录、Public / Private 边界、Build.cs 依赖，统一由 [Unreal_Source_Structure.md](Unreal_Source_Structure.md) 定义。
