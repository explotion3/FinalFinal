# 代码功能需求

## 1. 文档定位

本文档用于把玩法方向拆解成 Unreal 项目需要落地的代码功能块。

本文档不定义具体字段细节，也不规定最终目录结构；它只回答：

- 哪些系统需要实现
- 系统之间的职责边界是什么
- 首版竖切应该优先实现什么

本文档默认服务于：

- [Power_Fantasy_Direction.md](Power_Fantasy_Direction.md)
- [Battle_Rules.md](Battle_Rules.md)
- [Card_Design_Guide.md](Card_Design_Guide.md)
- [Combat_Data_Schema_v2.md](Combat_Data_Schema_v2.md)
- [Unreal_Source_Structure.md](Unreal_Source_Structure.md)

---

## 2. 首版代码目标

首版代码优先验证新的成长爽游方向：

- 战斗规则按固定时序稳定结算
- 压力进入 `Normal / Critical / Collapse` 状态机
- 单局内角色可以获得成长进度并升级
- 角色升级触发一次成长三选一
- 成长候选混合属性成长与卡牌进化
- 卡牌进化作用于 `RunCardInstance`，保留 `BaseCardId / CurrentCardId`
- UI 与表现层只读权威状态，不直接改写规则真相

首版不追求：

- 完整技能树
- 完整绝学树
- 强化珠与强化槽完整实现
- 珠子背包、拆卸、合成树、套装
- 复杂临界流派与角色专属临界收益
- 过度抽象的万能框架

---

## 3. 总体分层

代码侧至少拆成五层：

- 内容定义层：静态定义、DataAsset、共享协议
- 运行时状态层：战斗内与单局外的权威状态
- 规则执行层：命令校验、效果解析、结算顺序、窗口处理
- 外层编排层：地图、节点、奖励、事件、进入战斗、战后结算、成长选择
- 表现接入层：UI、Actor、动画、特效、音频、调试界面

硬性边界：

- `FinalBattle` 不直接处理角色升级、成长候选、卡牌进化选择
- `FinalRun` 不直接执行单张卡牌的战斗结算
- `FinalData` 不承担运行时规则逻辑
- `FinalApp` 不保存玩法真相，只负责查询、展示和流程桥接

---

## 4. FinalBattle：战斗内核心功能

### 4.1 战斗初始化

职责：

- 根据遭遇模板、规则配置与当前单局状态建立一场战斗
- 初始化敌人、队伍、抽牌堆、手牌区、弃牌堆、消耗区
- 从 `RunCardInstance.CurrentCardId` 解析战斗中实际使用的卡牌定义
- 处理开战关键词、开战状态、跨战斗保留的角色状态投影

输入：

- `BattleEncounterDefinition`
- `BattleRuleConfig`
- `FinalBattleStartRequest`

输出：

- `BattleState`
- 初始 `BattleEvent` 日志

优先级：`P0`

### 4.2 战斗命令入口

职责：

- 接收玩家或系统发出的战斗命令
- 校验命令是否合法
- 把合法命令交给规则结算层

典型命令：

- 打牌
- 释放奥义
- 结束回合
- 选择目标

说明：

- `BattleCommand` 只承载战斗内命令
- 奖励选择、事件选项、商店购买、成长三选一属于 `RunCommand`

优先级：`P0`

### 4.3 卡牌与牌区循环

职责：

- 维护抽牌堆、手牌区、弃牌堆、持续区、消耗区
- 处理抽牌、弃牌、生成、复制、回收、消耗
- 统一解释卡牌关键词对牌区去向的改写
- 维护 `BattleCardInstance` 与其来源 `RunCardInstanceId` 的关联
- 当 Run 层确认某张卡牌实例进化时，支持刷新对应战斗手牌展示与后续结算定义

优先级：`P0`

### 4.4 资源系统

职责：

- 维护 AP、EP、Break 奖励 AP、受击 EP、普通牌基础 EP
- 处理奥义消耗与 EP 获取公式
- 处理回合开始、回合结束、受击、打牌等窗口的资源变化

优先级：`P0`

### 4.5 伤害、治疗、压力与临界状态机

职责：

- 计算实际伤害、实际生命损失、实际回复量
- 处理共享血条与角色压力的转化
- 维护角色压力状态：`Normal / Critical / Collapse`
- 按 `BattleRuleConfig` 计算临界阈值与崩溃阈值
- 处理首次越过临界阈值保护
- 处理临界期间继续受压后的崩溃判定

边界：

- `FinalBattle` 负责压力状态机的战斗内权威判断
- 角色专属临界收益、临界牌、临界流派第一版暂不实现
- 角色升级与成长选择不在 `FinalBattle` 中处理

优先级：`P0`

### 4.6 Break 与先机

职责：

- 处理攻击转削韧
- 处理中途 Break 检查
- 处理先机减少事件
- 处理敌人插队时机与同窗口优先级

优先级：`P0`

### 4.7 状态系统

职责：

- 处理状态归属、叠层、刷新、覆盖、拒绝获得
- 按固定窗口结算通用状态与专属状态
- 处理共享血条下 `team_player` 与角色个人状态边界
- 支持状态驱动的伤害修正、生命保护、触发条件判断

优先级：`P0`

### 4.8 被动、角色触发与遗物触发

职责：

- 处理战斗内被动与遗物触发窗口
- 统一使用 `RuntimeTriggerDefinition` 记录触发域、窗口、限制、条件与效果
- 记录每回合、每战斗、每效果的触发次数
- 不允许遗物或角色触发绕过命令与规则结算层直接改状态

优先级：`P1`

### 4.9 敌人意图与行动

职责：

- 选择敌方当前意图
- 处理敌方多段攻击、召援、蓄力、强化
- 处理敌方已行动状态和行动顺序
- 按运行时站位处理同窗口优先级

优先级：`P0`

### 4.10 事件日志与战斗查询

职责：

- 记录规则层发生了什么
- 为 UI、表现层、调试工具提供统一事件流
- 提供 `BattleSnapshot / BattleEvent / EventsSince`
- 为后续回放、战斗日志、QA 检查保留基础

优先级：`P1`

---

## 5. FinalRun：单局外核心功能

### 5.1 单局持久状态

职责：

- 维护当前角色 roster
- 维护 `RunDeck`、遗物、金币、事件结果、节点进度
- 维护角色持久成长状态：等级、成长进度、根骨、悟性、杀意、跨战斗压力相关字段
- 维护卡牌实例：`RunCardInstanceId / BaseCardId / CurrentCardId`
- 为战斗开始提供队伍、牌组、遗物与遭遇输入
- 在战斗结束后消费 `BattleResult` 并回写单局状态

优先级：`P0`

### 5.2 单局外命令入口

职责：

- 接收事件选项、奖励选择、商店购买、成长选择等单局外命令
- 校验条件、代价与可选项是否合法
- 将 reward / event / shop / growth 的私有解析细节收口到 `FinalRun/Private` 的 resolver 或 service

典型命令：

- 进入节点
- 确认事件选项
- 确认战后奖励
- 购买商店内容
- 选择成长候选

优先级：`P0`

### 5.3 地图与节点推进

职责：

- 维护当前章节、楼层、节点池、节点选择结果
- 组织普通战、精英战、商店、事件、休整、首领战入口
- 基于 `RunRouteDefinition` 推进节点，不让 `FinalApp` 手工拼装主流程

优先级：`P1`

### 5.4 事件、奖励与商店

职责：

- 生成战后奖励、事件选项、商店商品
- 校验选项条件与代价
- 结算金币、加牌、删牌、遗物、恢复、压力变化等外层奖励
- 提供结构化 `RewardEntry` 与 `RewardEntryViewData`

说明：

- 常规奖励不直接负责角色升级
- 如果奖励会影响角色成长，应通过成长相关 payload 或成长服务落地

优先级：`P1`

### 5.5 战斗结果回写与成长进度

职责：

- 接收 `BattleResult`
- 回写金币、节点状态、胜负结果、跨战斗角色状态
- 根据战斗事实增加角色成长进度
- 当角色满足升级条件时，进入 `PendingGrowthChoice` 状态

说明：

- 战斗事实可以包括出牌、Break、击杀、承压、进入临界等记录
- 第一版可以先用简单规则增加成长进度，后续再细化来源权重
- 当前 Step 4 运行时已提供 `UFinalRunSession::AddBreakthroughValue()` 作为最小成长入口；它会先累计突破值，再在没有待处理成长选择时尝试触发一次升级

优先级：`P0`

### 5.6 角色升级与成长三选一

职责：

- 维护角色等级与升级条件
- 角色升级时生成 3 个成长候选
- 候选项混合属性成长与卡牌进化
- 应用玩家选择的成长候选

候选类型：

- 属性成长：根骨 / 悟性 / 杀意 +1
- 卡牌进化：基础卡 -> 进化卡
- 高阶卡牌进化：进化卡 -> 绝学卡，第一版可暂不实现

边界：

- 成长三选一属于 `FinalRun`
- `FinalBattle` 只记录战斗事实，不决定升级奖励
- `FinalApp` 只展示候选并提交选择，不自行推导候选结果

当前阶段补充：

- Step 4 已落地“突破值累积 -> 升级触发 -> 候选生成 -> 写入 `PendingGrowthChoice`”。
- 当前同一时刻只允许一个 `PendingGrowthChoice`；若已有待处理候选，新的突破值仍会累计，但不会再次触发升级。
- 当前候选生成是 deterministic：默认 `RootBone +1`、`Insight +1`，第 3 个候选在“可进化卡牌”和 `KillingIntent +1` 之间二选一。
- Step 5 已落地 `SelectGrowthChoice` RunCommand：选择成功后会应用属性成长或卡牌进化，并清空当前 `PendingGrowthChoice`。
- 当前不会在应用成长选择后自动连锁触发下一次升级；剩余突破值会保留到后续再次显式触发成长入口时再处理。

优先级：`P0`

### 5.7 卡牌实例进化

职责：

- 管理 `RunCardInstance` 的 `BaseCardId / CurrentCardId`
- 应用卡牌进化结果
- 校验进化来源与目标是否合法
- 保证进化后卡牌仍能追溯原始基础卡
- 为 UI 查询提供当前卡牌展示定义

说明：

- 第一版进化可以通过替换 `CurrentCardId` 实现
- 强化珠、强化槽、同名珠合成等系统先只预留，不进入第一版实现

优先级：`P0`

### 5.8 Run 查询与事件流

职责：

- 提供 `RunSnapshot / RunEvent / EventsSince`
- 公开当前节点、奖励、成长选择、角色状态、牌库、遗物等只读视图
- 对会修改角色或卡牌实例的事件输出结构化结果，避免 UI 自行推算

优先级：`P1`

---

## 6. FinalData：内容与数据功能

### 6.1 定义资产加载

职责：

- 加载角色、卡牌、敌人、状态、遗物、事件、遭遇、规则配置等定义
- 通过稳定 ID 提供查询入口
- 启动期建立 stable id 到 soft object path 的索引，并按需加载 definition

优先级：`P0`

### 6.2 资源校验

职责：

- 校验 ID 是否重复
- 校验外部引用是否缺失
- 校验效果字段是否与协议匹配
- 校验文案、关键词、类型是否符合规范
- 校验成长候选、卡牌进化、临界配置的基础合法性

优先级：`P1`

### 6.3 数据查询与索引

职责：

- 按稳定 ID 查询卡牌、敌人、状态、遗物、事件、遭遇、规则配置
- 按标签、章节、稀有度、角色归属、推荐阶段做筛选
- 为 Battle / Run / UI 提供静态定义查询
- 对需要生成升级候选的系统，提供可枚举的卡牌进化定义查询入口

优先级：`P1`

### 6.4 成长与进化定义

职责：

- 定义角色成长参数，例如升级阈值、根骨、悟性、杀意的首版效果
- 定义成长候选展示字段
- 定义卡牌进化关系，例如 `FromCardId -> ToCardId`
- 定义进化候选过滤条件，例如角色归属、进化阶段、卡牌标签

首版最小定义：

- `CharacterGrowthConfig`
- `GrowthChoiceDefinition`
- `CardEvolutionDefinition`

优先级：`P0`

### 6.5 强化珠与强化槽定义预留

职责：

- 预留强化珠、珠阶、珠级、强化槽槽级的定义入口
- 第一版不要求实现强化珠运行时逻辑

优先级：`P2`

---

## 7. FinalApp：表现与外层接入

### 7.1 UI 编排与视图模型

职责：

- 维护 UI 根布局与页面层级
- 把权威状态转成 UI 可读数据
- 不在 Widget 中做规则推导
- 优先事件驱动刷新，不依赖 Tick 或 Blueprint Binding 轮询权威状态

首版新增要求：

- 展示角色成长三选一
- 展示属性成长结果
- 展示卡牌进化结果
- 展示压力 `Normal / Critical / Collapse` 状态

优先级：`P0`

### 7.2 世界桥接

职责：

- 在场景中生成和维护角色 / 敌人表现 Actor
- 接收战斗事件并驱动表现
- 不在 Actor 中改写战斗真相

优先级：`P0`

### 7.3 输入与交互

职责：

- 把战斗内输入转换成 `BattleCommand`
- 把事件、奖励、商店、成长选择等单局外输入转换成 `RunCommand`
- 统一处理输入模式、焦点恢复、覆盖页与模态页

优先级：`P0`

### 7.4 音画反馈

职责：

- 根据战斗事件播放动画、特效、音频、浮字
- 根据成长事件播放升级、进化、临界提示
- 不参与最终数值判定

优先级：`P1`

---

## 8. 保存、调试与测试支撑

### 8.1 Save / Load

职责：

- 保存战斗外 Run 状态
- 保存角色等级、成长进度、三大成长属性
- 保存 `RunCardInstance` 的 `BaseCardId / CurrentCardId`
- 不保存 active `BattleSession` 内部状态

优先级：`P1`

### 8.2 调试工具

职责：

- 查看当前战斗状态、压力状态、敌人意图、状态实例
- 查看 Run 状态、角色成长状态、待处理成长选择
- 查看卡牌实例的 Base / Current 关系
- 输出战斗事件与 Run 事件日志

优先级：`P1`

### 8.3 自动化校验

职责：

- 基础规则回归
- 数据资产合法性检查
- 角色升级与卡牌进化链路冒烟测试
- 临界状态机冒烟测试

当前最小覆盖应至少包含：

- 无可进化卡时，角色升级后生成 3 个属性成长候选
- 有可进化卡时，角色升级后生成包含 `CardEvolution` 的候选集
- 已有 `PendingGrowthChoice` 时再次获得突破值，只累计 `BreakthroughValue`，不重复升级

优先级：`P2`

---

## 9. 首版优先级建议

### 9.1 P0 必做

- 内容定义加载
- 单局持久状态
- 单局外命令入口
- 战斗初始化
- 战斗命令入口
- 卡牌与牌区循环
- AP / EP 资源系统
- 伤害、治疗、压力与临界状态机
- Break 与先机
- 状态系统
- 敌人意图与行动
- 角色升级成长三选一
- 卡牌实例进化
- UI 视图模型
- 输入与交互

### 9.2 P1 应做

- 被动、角色触发与遗物触发
- 事件、奖励与商店
- 地图与节点推进
- Run 查询与事件流
- Battle 查询与事件日志
- 资源校验
- Save / Load
- 音画反馈
- 调试工具

### 9.3 P2 后续补

- 强化珠与强化槽完整运行时
- 完整绝学化
- 角色专属临界收益
- 完整回放
- 编辑器增强工具

---

## 10. 功能归属总表

| 功能块 | 主归属模块 | 对外入口 | 不应放入 |
| --- | --- | --- | --- |
| 定义资产加载 | `FinalData` | 数据查询服务 / 资源注册表 | `FinalBattle`、`FinalApp` |
| 成长与进化定义 | `FinalData` | 成长配置 / 进化定义查询 | `FinalBattle` |
| 战斗初始化 | `FinalBattle` | `BattleSession` | `BattleGameMode` |
| 战斗命令入口 | `FinalBattle` | `SubmitCommand` | Widget、Actor |
| 卡牌与牌区循环 | `FinalBattle` | 卡牌服务 / 结算器 | `FinalRun` |
| 压力临界状态机 | `FinalBattle` | 压力服务 / 战斗结算 | `FinalRun`、Widget |
| Break 与先机 | `FinalBattle` | Break 服务 / 回合服务 | `FinalRun` |
| 状态系统 | `FinalBattle` | 状态服务 | Widget |
| 敌人意图与行动 | `FinalBattle` | 意图服务 / 回合服务 | 世界表现 Actor |
| 单局持久状态 | `FinalRun` | `RunSession` | `FinalBattle` |
| 单局外命令入口 | `FinalRun` | `SubmitRunCommand` | `FinalBattle` |
| 角色升级成长三选一 | `FinalRun` | 成长服务 / 成长解析器 | `FinalBattle`、Widget |
| 卡牌实例进化 | `FinalRun` | 卡牌实例服务 | `FinalBattle` 规则细节 |
| 事件、奖励与商店 | `FinalRun` | 事件 / 奖励 / 商店解析器 | Widget |
| Run 查询与事件流 | `FinalRun` | `RunSnapshot / RunEvent / EventsSince` | `FinalApp` 私有缓存真相 |
| UI 页面与视图模型 | `FinalApp` | `UISubsystem / ViewModel` | `FinalBattle`、`FinalRun` |
| 世界桥接 | `FinalApp` | Director / Presentation Actor | `FinalBattle` |
| Save / Load | `FinalApp` 协调，`FinalRun` 提供 Save DTO / Restore API | Save coordinator | active battle 状态、UI 状态 |
| 数据校验 / 编辑器工具 | `FinalEditor` | DataValidation / Commandlet | Runtime 模块 |

---

## 11. 首批最小可玩闭环

首批代码至少需要跑通下面链路：

1. 启动一局 Run
2. 进入普通战节点
3. `FinalRun` 组装战斗输入
4. `FinalBattle` 创建战斗并初始化牌堆
5. 玩家打出 1 张牌
6. 结算伤害、削韧、Break、先机与压力
7. 压力可进入 `Critical`，并在继续受压时进入崩溃判定
8. 敌人行动
9. 战斗胜利
10. `FinalRun` 消费 `BattleResult`
11. 某名角色获得成长进度并升级
12. 进入 `PendingGrowthChoice`
13. UI 展示成长三选一
14. 玩家选择属性成长或卡牌进化
15. 如果选择卡牌进化，`RunCardInstance.CurrentCardId` 被替换
16. 下一场战斗使用进化后的卡牌定义

---

## 12. 首版暂不实现

- 完整技能树
- 完整绝学树
- 强化珠即时镶嵌
- 强化槽槽级限制珠阶
- 同卡同名珠三合一升级
- 珠子背包、拆卸、合成树、套装
- 角色专属临界收益
- 临界牌
- 临界流派遗物
- 生产级多 slot 存档
- 完整回放系统

---

## 13. 与源码架构的关系

本文档只负责把功能拆开。

实际 Unreal 模块、目录、Public / Private 边界、Build.cs 依赖，统一由 [Unreal_Source_Structure.md](Unreal_Source_Structure.md) 定义。
