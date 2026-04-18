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
* 当前最小竖切至少支持两类小窗口遗物协议：
* `battle-start`：由 `RelicDefinition` 提供少量 battle-start effect，经 `Run -> Battle` 桥接后，在 `FinalBattle` 初始化阶段真实落地并写入 `BattleEvent`
* `player-turn-start`：由 `RelicDefinition` 提供少量玩家回合开始 effect，在 Battle 初始化后保留到权威状态，并在玩家回合开始窗口真实落地并写入 `BattleEvent`

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
* `FinalApp` 应基于这组公开字段落统一的 BattleEvent presentation/helper 与只读事件账本 UI，服务 HUD、Debug、世界提示与未来 replay-ready 消费，但不承担规则推导
* `FinalBattleResolver` 当前继续作为唯一对外规则入口，但私有实现细节应回收到 `FinalBattle/Private/Systems`
* 当前已开始真实承接实现的 Battle 私有 system 至少包括：
  * `FinalBattleCardService`：手牌/牌堆去向、卡牌实例查找、抽牌与手牌视图构建；同时承接 battle 内衍生牌实例生成、直接入手、以及 `ConsumePile` 去向
  * `FinalBattleResourceService`：AP / EP 初始化、增减与回合资源重置
  * `FinalBattleTurnService`：`EndTurn` 后敌人行动推进、玩家回合开始窗口与遗物触发入口
  * `FinalBattleStatusService`：当前最小状态窗口 tick、状态加层/减层/移除与状态快照整理
* `FinalBattleResolver` 负责 command dispatch、战斗初始化、事件时序与 snapshot orchestration，不继续作为所有战斗细节的单文件实现

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
* 为战斗开始提供当前遗物的最小 battle-start 输入，并通过 `FinalBattleStartRequest -> FFinalBattleInitContext` 显式传入 Battle，但不让 `FinalBattle` 直接读取 `RunState`
* 在战斗结束后消费战斗结果并回写单局状态
* 对外至少提供当前构筑的只读查询面，让 UI 能读取当前牌库条目与遗物条目，而不直接访问 `RunState` 容器真相

优先级：
* `P0`

### 5.2 单局外命令入口
职责：
* 接收事件选项、奖励选择、商店购买、成长分支等单局外命令
* 校验条件、代价与可选项是否合法
* `RunSession` 负责命令分发与事件时序，reward / event / shop / growth 的私有解析细节应回收到 `FinalRun/Private` 下的 resolver，不把 `FinalRunSession.cpp` 继续扩成单文件真相与解析器混合体

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
* prototype 节点图和节点内容流应优先落在 `FinalData` 的 route / node definition 中，由 `FinalRunSession` 读取并接管初始化，不继续由 `FinalApp` 手工拼装 `TArray<FFinalRunNodeDefinition>`
* 对外查询面至少公开 `CurrentChapter / CurrentFloor / 节点展示名或展示标签 / 已访问 / 锁定状态与原因 / 候选节点展示数据`
* 当前节点查询面至少明确区分 `已访问` 与 `已解析`，避免 `FinalApp` 通过 flow stage 反推节点状态

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
* 奖励协议中的“真正授予对象”应使用稳定的 typed payload 标识，例如 `GrantedCardId / GrantedRelicId / RemovedCardId / UpgradeFromCardId / UpgradeToCardId`，不能长期把 `DisplayId` 当作权威玩法身份
* 当前 `RunState` 至少应真实承接：`Gold -> Gold`、`CardGrant -> RunDeck`、`RelicGrant -> Relics`、`RemoveCard -> 从 RunDeck 删除目标卡`、`UpgradeCard -> 用升级结果替换 RunDeck 中的目标卡`
* `CardGrant / RelicGrant` 在真正落地到 `RunState` 前，应通过 `FinalDataRegistry` 校验对应 definition 是否存在；`RelicGrant` 的 `DisplayName / DisplayId` fallback 应优先来自 `RelicDefinition`
* `RemoveCard / UpgradeCard` 落地前应校验必要 payload、对应 card definition、以及 `RunDeck` 中是否存在目标卡；`UpgradeCard` 还应校验升级结果不是无效或自指
* `Growth` 当前阶段可先支持锚定 `RunPersistentCharacterState` 的最小 typed payload，例如 `GrowthTargetCharacterId + GrowthEffectType + Value`，并真实落地到 `CurrentStress / CurrentAwakenCount / CollapseCount`
* `FinalDataRegistry` 运行时应先承担 definition 资产发现/加载主路径，至少覆盖 `BattleRuleConfig / CharacterDefinition / CardDefinition / UltimateDefinition / EnemyDefinition / EnemyIntentDefinition / StatusDefinition / BattleEncounterDefinition / RelicDefinition / RunRouteDefinition / PrototypeBootstrapDefinition`
* prototype content 应优先落成项目里的真实 definition 资产，由 `FinalDataRegistry` 在运行时发现并注册；`FinalApp` 的测试入口只按 stable id 查询这些内容，不再主路径 `NewObject` 创建 definition bundle
* 当前 prototype bundle 推荐落在 `/Game/Prototype/Definitions/...`，并由 Editor 侧的 `FinalPrototypeContentBootstrap` commandlet 负责生成或刷新；运行时若缺少对应 stable id，应返回明确缺失错误，而不是继续让 `FinalApp` 充当主内容源
* 当前已开始录入真实 starter content：`FinalPrototypeContentBootstrap` 会同时刷新 `/Game/Prototype/Definitions/Starter/...` 下的 `prototype.bootstrap.starter.chapter1 / run.route.starter.chapter1`、霍断岳 / 叶半夏 / 沈清弦、每名角色 4 张起始牌与 1 个测试奥义、2 名普通敌人、1 名精英敌人与普通 / 精英遭遇；这些内容仍通过 `FinalDataRegistry` 与 Editor validation 进入现有数据驱动体系，不回写成 `FinalApp` 或规则层硬编码
* starter content 第一版已把霍断岳 `刀势`、叶半夏 `药引` 的第一波 battle-side 机制收回 Runtime：当前 effect 协议已承接 `Heal / ApplyStatus / RemoveStatus / GainAP / BonusBreak`，starter 资产中的 Huo / Ye 相关卡牌与奥义不再只靠文本占位
* starter content 当前已把沈清弦 `剑阵` 第一波收回到 Battle Runtime：`布锋` 随机生成衍生剑阵牌、`引阵` 稳定生成 `过牌剑阵`、`过牌剑阵 / 破阵剑阵` 作为 battle 内衍生牌进入手牌并在打出后进入 `ConsumePile`、`引爆剑阵` 真实消耗 1 张手中的衍生剑阵牌后兑现伤害/抽牌
* Battle 当前已补最小 `HandCardRequirement` 协议，至少支持 `RequiredCardId / RequiredKeyword / MinimumCount / bGeneratedOnly / bRequireInHand`，并由 `FinalBattleCardService` 提供“按当前手牌内容统计/判定是否满足条件”的只读查询
* starter content 当前已用这套协议把 `守阵` 的“若手中有剑阵牌”改成真实规则：基础护盾始终生效，只有当前手牌里存在满足条件的衍生剑阵牌时，后续抽牌收益才会执行
* Battle 当前已补最小“状态驱动的伤害修正”协议：`StatusDefinition` 可配置 `OutgoingDamagePercentPerStack / bExpireAtPlayerTurnEnd / bConsumeOnSuccessfulOwnerDamage / bOnlyAffectAttackCards`，`FinalBattleStatusService` 负责在运行时统计 owner 的总伤害修正，并在成功对敌伤害后按规则消费一层状态
* starter content 当前已把 `锋锐剑阵` 与 `万象归阵` 的第一波战斗真相收回到 Runtime：`锋锐剑阵` 会对自身施加 1 层 `锋锐`，令下一张攻击牌伤害提高 20% 且在成功造成敌方生命伤害后消耗；`万象归阵` 现已改为抽 2 张牌、生成 1 张剑阵牌，并为每名角色施加 1 层 `士气`
* Battle 当前已补最小 `OwnerTookHealthDamage` 触发窗口协议：`CharacterDefinition` 可配置 battle trigger effect list，初始化时镜像到 `FinalBattleCharacterState`，当玩家共享生命实际下降时由 `FinalBattleResolver` 按角色顺序执行；当前 starter 已用它把霍断岳“受压得刀势”收回 Runtime
* starter content 仍保留占位的内容包括：`万象归阵` 的阵牌扩散、免疫、复杂治疗保护、复杂 Break 条件追伤、经济 / 商店 / 未来窗口等；这些内容仍应先补协议与规则服务，再升级为权威效果
* prototype 启动配置也应收回到 `FinalData` 的 bootstrap/profile definition，例如 `PrototypeBootstrapDefinition`，承载 `RuleConfigId / EncounterId / RunRouteId / PartyCharacterIds / StarterDeckCardIds / 初始角色持久状态 / InitialTeamCurrentHP`；`FinalApp` 运行时只查询一个 bootstrap stable id
* 当前最小 `GrowthEffectType` 可先限制在 `ReduceStress / GainAwakenProgress / ReduceCollapseCount`；更大的成长树、奥义解锁与终极天赋仍后置
* 战后奖励查询面至少公开结构化 `RewardEntries`，可扩展到金币、卡牌、遗物、删牌与升级牌
* 在保留 raw `RewardEntries` 的前提下，Run 查询面还应补 `RewardEntryViewData` 一类稳定展示数据，至少能表达 `PrimaryText / SecondaryText / Value` 与必要的只读目标 id，避免 `FinalApp` 自行拼接 reward 文案
* `RewardEntryViewData` 应继续向产品化展示靠拢，最小应补 `PresentationKind / IconId / VisualTier / DetailText` 这类 metadata，并优先通过 `CardDefinition / RelicDefinition / CharacterDefinition` 补全
* `PendingBattleReward / PendingRewardNode / EventOption / ShopOffer` 应同时公开 raw reward 与 reward view data，两者分别服务于规则链和展示层
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
* 对于会产出奖励结果的 `RunEvent`，在保留 raw `RewardEntries` 的前提下，也应补 `RewardEntryViewData` 数组，供 toast、日志和结果反馈直接消费
* 对于会修改角色持久状态的 `RunEvent`（含 Growth 类奖励），应在事件中补 `AffectedCharacterResults` 数组，输出结算后的角色 view data，避免 FinalApp 自行推算角色结果
* `AffectedCharacterResults` 复用现有 `FFinalRunCharacterViewData`，包含 `DisplayName / IconId / StateSummaryText / CurrentStress / bCollapsed / CurrentAwakenCount / CollapseCount`
* 当前只有 `EventNodeResolved / RewardNodeResolved / ShopOfferPurchased / PendingBattleRewardClaimed` 在 reward entries 包含 Growth 时才会填充 `AffectedCharacterResults`
* `RunSnapshot` 当前应至少公开 `CurrentBuild` 这类只读 view data，用于呈现 `RunDeck / Relics` 的聚合条目、展示名与数量
* `RunSnapshot.Characters` 不应只停留在调试数值，应至少补 `DisplayName` 与最小展示辅助字段，并优先通过 `CharacterDefinition` 查询补全，避免 `FinalApp` 自行猜角色名
* `RunSession` 在组装 `BattleStartRequest` 时，应能桥接最小遗物战斗输入，例如 battle-start relic effects，并经由 `FinalBattleStartRequest -> FFinalBattleInitContext` 显式传入 `FinalBattle`；不要把 `RunState.Relics` 私有容器直接暴露给 `FinalBattle`

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

当前已开始落地：
* `FinalEditor` 已建立最小 Editor-only 数据资产校验器
* 第一版优先覆盖 `Card / Character / Enemy / EnemyIntent / Encounter / Relic / RuleConfig / Status / Ultimate` definition
* 当前校验范围已覆盖：稳定主 ID、`DisplayName`、关键数值、直接软引用、效果数组空项与最小 relic battle-start / player-turn-start effect 合法性
* 当前已补一层 Editor-only 全项目扫描/索引，用于检查 `Card / Character / Enemy / EnemyIntent / Encounter / Relic / Status / Ultimate / RuleConfig / RunRoute` 的主 ID 是否重复
* 当前已补第一批跨资产稳定 ID 引用存在性检查：`CharacterDefinition.InitialLoadoutCards[*].CardId`、`CharacterDefinition.CharacterCardPoolIds[*]`、`CharacterDefinition.UltimateId`、`CharacterDefinition.SignatureStatusId`
* 当前已补 `RunRouteDefinition` 内容一致性校验：`RouteId / EntryNodeId`、同 route 内 `NodeDefinitions[*].NodeId` 唯一性、`NextNodeIds[*]` 可达性，以及 battle / reward / event / shop 节点的最小结构合法性
* 当前已补 reward payload typed reference 校验，覆盖 `Gold / CardGrant / RelicGrant / RemoveCard / UpgradeCard / Growth`，并对缺失 stable id、非法 growth effect、自指升级和非正数值给出明确错误
* 全局一致性校验结果仍挂回当前被校验资产，并会报出缺失字段名、缺失稳定 ID 和重复 ID 的冲突资产路径
* 当前已补 prototype vertical slice 的 Editor 自动化冒烟测试，覆盖 `prototype.bootstrap.test / prototype.bootstrap.starter.chapter1` 等 stable id 的发现、bootstrap 核心引用经 `FinalDataRegistry` 解析、bootstrap 启动最小 run、run 进入 battle 并执行最小推进、battle result 回写 run，以及战斗外 `ExportSaveData / RestoreFromSaveData`
* 遗物允许暂时没有 `BattleStartEffects / PlayerTurnStartEffects`，以便录入未来窗口、经济、商店类合法遗物；若数组有条目，则校验 `EffectType != None` 且 `Value > 0`
* 不做自动修复、复杂编辑器 UI、内容资产迁移，也不改变 Runtime 规则语义

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
* `FinalApp` 当前已补 BattleEvent 统一投影 helper，并新增最小只读 Battle event ledger UI；`BattleHUD`、`PrototypeRunDebugScreen`、`BattleDirector` 优先共用这套事件投影，而不是各自散拼

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
* 当前第一版只保存战斗外 Run 外层状态，由 `FinalApp` 协调固定 SaveGame slot，`FinalRun` 通过公开 `FinalRunSaveData` 协议导出与恢复
* 保存稳定 ID、`FFinalRunState`、Run 事件日志、节点配置与访问 / 解析进度、当前 FlowStage、待领奖励上下文
* `FinalRunSaveData` 当前支持 `SaveVersion == 1`，Load 前必须做版本校验与结构合法性检查，坏档应返回明确失败原因并拒绝恢复
* 结构校验当前至少覆盖空 Run 状态、当前节点 / 已访问节点 / 已解析节点是否存在于配置节点中、待领奖励上下文自洽，以及 `LastEventSequence` 是否覆盖 RunLog 最大序号
* 支持战斗外继续读取 `CollapseCount` 等持久字段，恢复后 `RunSnapshot` 应反映恢复后的外层状态
* 当前不保存 active `BattleSession` 内部状态；存在 active battle 时 Save / Load 应拒绝
* 当前不保存 UI 页面栈、Widget 状态、transient UObject definition，也不做自动迁移、async save/load、正式存档菜单或生产级多 slot 管理
* `PrototypeRunDebugScreen` 可显示固定 slot 是否存在、最近 Save/Load 状态与失败原因，并提供原型级 Save / Load 按钮；这不是正式存档 UI

优先级：
* `P1`

### 8.2 调试工具
职责：
* 查看当前战斗状态
* 查看敌人意图
* 查看状态实例、被动实例、遗物触发记录
* 输出战斗事件日志
* 提供只读 Battle 事件账本视图，并与 HUD / 世界桥接共用统一事件投影 helper

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
| Save / Load | `FinalApp` 协调，`FinalRun` 提供 Save DTO / Restore API | `UFinalSaveGameCoordinator` / `UFinalRunSaveGame` / `FFinalRunSaveData` | `FinalBattle` 内部类、active battle 状态、UI 状态 |
| 数据校验 / 编辑器工具 | `FinalEditor` | DataValidation 校验器 / 后续编辑器菜单 | Runtime 模块 |

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
