# 状态系统规范

## 1. 文档定位
本文档用于统一状态系统的设计口径，重点说明状态分类、归属规则、结算窗口、共享血条边界与首版状态清单。

文档分工如下：
* [GDD4.0.md](GDD4.0.md)：负责说明状态系统的定位、分类与设计原则
* [Battle_Rules.md](Battle_Rules.md)：负责说明状态的结算窗口、优先级、共享血条边界与战斗判定
* [Combat_Data_Schema_v2.md](Combat_Data_Schema_v2.md)：负责说明状态模板、实例字段与运行时挂载方式

本文档不负责：
* 卡牌关键词定义
* 遗物、事件、敌方意图的完整规则
* 具体数值测试参数

## 2. 状态系统总览
状态用于承载战斗中的持续修正、持续伤害、条件收益与角色专属机制。

当前已确认的总口径：
* 状态系统分为通用正面状态、通用负面状态、角色专属状态三类
* 敌人与角色都使用状态栏
* 规则关键词与状态名严格分层，不混写
* 状态结算应统一挂接到少量固定窗口，不为单个状态临时发明新时点
* 当前工程实现已明确分为三类运行时语义：
  * 直接规则型状态：直接参与 battle 数值结算
  * 卡牌投影型状态：把修正投影到当前手牌中的 `BattleCard`
  * 资源型状态：只保存层数与持续信息，由牌或被动显式读取和消耗
* 当前迁移状态：
  * `士气`、`生命免疫` 已迁到新 `RuntimeModifiers`
  * `锋锐` 已迁移到 `ProjectedCardModifiers`
  * `刀势 / 药引` 已归位为正式资源型状态：获得继续走 `ApplyStatus`，消费改走 `ConsumeStatusResource`
  * `易伤 / 虚弱 / 腐蚀 / 中毒 / 流血` 仍待后续迁移

## 3. 状态分类
### 3.1 设计口径
首版状态系统按玩法设计分为三类：
* 通用正面状态
* 通用负面状态
* 角色专属状态

### 3.2 数据口径映射
按 [Combat_Data_Schema_v2.md](Combat_Data_Schema_v2.md) 的字段约定，状态分类映射如下：
* `Buff`：增益
* `Debuff`：减益
* `Signature`：专属
* `Mechanic`：机制

补充说明：
* `刀势 / 药引` 这类角色专属状态，默认归入 `Signature`
* `剑阵` 属于衍生牌体系，不纳入状态分类
* `Break 破绽`、首领阶段机制等全局规则状态，默认归入 `Mechanic`

## 4. 状态归属规则
### 4.1 主归属
状态默认只挂在一个主对象上结算。

默认规则：
* 队伍共享状态默认挂在 `team_player`
* 角色个人状态默认挂在对应角色的运行时单位
* 敌方状态默认挂在对应敌方运行时单位

### 4.2 来源与归属
状态至少要区分两件事：
* `OwnerUnitId`：这条状态当前挂在哪个对象上
* `SourceUnitId`：这条状态由谁施加或创建

默认规则：
* 归属决定这条状态影响谁
* 来源决定持续伤害、条件收益、来源联动等规则按谁结算

### 4.3 文本未写明时的默认判断
若文本没有明确写出状态挂载对象，首版默认按以下方式判断：
* 若文本关注“队伍受到伤害 / 队伍回复 / 全队获得 / 共享生命变化”，默认挂到 `team_player`
* 若文本关注“该角色打出 / 该角色受到压力 / 该角色崩溃 / 该角色苏醒 / 该角色的牌”，默认挂到对应角色
* 若文本同时包含两层影响，则优先将状态挂在触发主体上，实际影响结果再按文本结算到共享对象或角色对象

## 5. 状态叠加与刷新规则
首版状态叠加与刷新默认采用“先归并，再叠层或刷新”的处理方式。

### 5.1 基本原则
* 每个状态模板都应定义 `MaxStacks` 与 `DurationType`
* 状态实例层需要记录当前层数、持续方式、剩余持续值与添加顺序
* 同一窗口内若多个同类状态同时存在，默认按站位顺序与状态添加顺序处理
* 同一状态默认先判断是否与已有实例归并，再决定是叠层、刷新、覆盖还是拒绝获得

### 5.2 同名状态的默认归并规则
首版默认先按以下顺序判断同名状态是否归并到同一条实例：

* 若 `StatusId` 不同，则一定不归并
* 若 `OwnerUnitId` 不同，则一定不归并
* 若该状态的主要效果依赖 `SourceUnitId` 独立结算，则不同来源默认不归并
* 若该状态的主要效果不依赖来源独立结算，则同名状态默认归并到同一条实例

补充说明：
* 持续伤害、来源联动明显依赖施加者的状态，默认优先按不同 `SourceUnitId` 分开保存
* 纯修正类状态、纯层数类状态，默认优先按 `StatusId + OwnerUnitId` 归并

### 5.3 获得已存在状态时的默认处理
#### 5.3.1 可叠层状态
若状态允许叠层，即 `MaxStacks > 1`，则默认按以下顺序处理：

1. 将本次新增层数加入当前层数
2. 若超过 `MaxStacks`，则截断到上限
3. 按刷新规则更新持续值
4. 更新 `AppliedSequence`

默认说明：
* 若文本未写明新增层数，默认按 `+1` 层处理
* 到达上限后仍可刷新持续值，但不会继续无限叠层

#### 5.3.2 单层状态
若状态最大层数为 `1`，则默认不创建第二条同归属实例，而是刷新现有实例。

默认说明：
* 重复获得单层状态时，默认刷新持续值
* 若该状态没有持续值概念，则默认只更新 `AppliedSequence`
* 若文本明确写出“覆盖为新版本”或“替换旧状态”，则按文本处理

### 5.4 持续值刷新规则
#### 5.4.1 Turn / Round
对 `Turn` 或 `Round` 类型状态，重复获得时默认刷新其 `RemainingDuration`。

默认规则：
* 若新持续值大于当前剩余值，则以新持续值覆盖
* 若新持续值小于或等于当前剩余值，则默认保持当前剩余值不被缩短
* 若文本明确写出“重置为固定回合数”，则按文本重置

#### 5.4.2 Battle / Permanent
对 `Battle` 或 `Permanent` 类型状态，默认没有常规剩余回合刷新问题。

默认规则：
* `Battle` 类型状态在本场战斗内持续存在，直到被移除或战斗结束
* `Permanent` 类型状态不因常规战斗窗口自然减少
* 重复获得时默认只处理层数、归并和 `AppliedSequence`

#### 5.4.3 Instant
对 `Instant` 类型状态，默认不形成可持续维护的常驻实例。

默认规则：
* `Instant` 更接近一次性结算结果
* 若某效果只需要“获得时生效一次”，默认不走持续刷新逻辑

### 5.5 超出最大层数时的处理
当状态层数超过 `MaxStacks` 时，首版默认按以下方式处理：

* 当前层数截断到 `MaxStacks`
* 超出的层数默认直接丢弃
* 即使层数被截断，仍可继续刷新持续值
* 若文本明确写出“溢出转化为其他收益”或“满层时触发额外效果”，则按文本处理

### 5.6 覆盖、互斥与拒绝获得
首版默认规则如下：

* 不同 `StatusId` 的状态默认可以同时存在，除非文本明确写出互斥或替换关系
* 同名状态若符合归并规则，默认不创建重复实例
* 状态若已存在且文本明确写出“不可重复获得”，则本次获得失败
* 状态若要求替换旧状态，则默认先移除旧状态，再创建或刷新新状态

### 5.7 结算顺序与实例记录
状态实例层至少应维护以下信息：
* 当前层数
* 持续方式
* 剩余持续值
* 添加顺序
* 来源单位

默认规则：
* 需要比较先后顺序时，优先读取 `AppliedSequence`
* 同一窗口内多个状态效果同时触发时，先按归属对象与站位顺序确定结算顺位，再按 `AppliedSequence` 处理同类状态

## 6. 状态结算窗口
### 6.1 固定窗口
首版优先使用以下几类固定窗口：
* 回合开始窗口
* 玩家结束回合后的敌方行动前窗口
* 单位行动后窗口
* 条件触发窗口

### 6.2 条件触发窗口
条件触发窗口主要用于承载非持续跳伤类效果，例如：
* 受击时
* 打牌时
* Break 时
* 崩溃时
* 苏醒时

当前 Runtime 已落地的最小 battle trigger window：
* `OwnerTookHealthDamage`：当玩家共享生命实际下降时触发；当前用于霍断岳 innate passive“受压得刀势”，通过 `BattlePassiveInstance.TriggerStates` 触发 `ApplyStatus(刀势 +1)`。
* `PlayerCardResolved`：当玩家打出的卡牌完整结算后触发；当前用于霍断岳能力牌赋予的 passive“压势追刀”，在每回合第一次打出攻击牌后为当前手牌中的攻击牌投影 `-1 AP / +20% 伤害`。

### 6.3 首版默认状态时点表
* `灼烧`：当前只作用于敌方单位；在敌方回合开始窗口结算
* `中毒`：当前只作用于敌方单位；在玩家结束回合后的敌方行动前窗口结算
* `腐蚀`：当前主要作用于玩家侧；效果与中毒相同；在玩家结束回合后的敌方行动前窗口结算
* `流血`：当前只作用于敌方单位；在敌方单位行动后窗口结算
* `护体`：常驻修正状态，在受击时参与减伤计算
* `士气`：常驻修正状态，在造成伤害时参与攻击修正
* `易伤`：常驻修正状态，在目标受到伤害时参与伤害放大
* `虚弱`：常驻修正状态，在目标造成伤害时参与伤害降低
* `破绽`：条件收益状态，在满足 Break、追击、处决等文本条件时参与判定
* `迟滞`：先机干预状态，在先机变化或敌方行动检查时参与判定
* `专属状态`：默认优先挂接到条件触发窗口；若为持续型状态，再按各自文本接入固定窗口

### 6.4 同窗口默认结算顺序
当同一窗口内需要同时结算多个状态时，默认按以下顺序处理：
1. 先结算高危负面持续效果
2. 再结算一般持续效果
3. 最后结算由状态引发的附加收益、转化或移除

## 7. 共享血条下的状态边界
### 7.1 队伍共享状态
以下类型的玩家侧状态，默认按队伍共享状态处理：
* 直接作用于队伍共享生命的持续伤害、持续治疗、减伤、增伤承受或免疫效果
* 直接修正队伍共享生命变化结果的状态
* 明确作用于 `全队`、`队伍`、`共享生命` 或其他队伍公共对象的状态

首版默认示例：
* `护体 / 易伤 / 免疫`
* `腐蚀`：若作用于玩家侧共享生命，默认挂在 `team_player`
* `中毒 / 灼烧 / 流血` 当前不作用于玩家侧，因此不纳入队伍共享状态示例

### 7.2 角色个人状态
以下类型的玩家侧状态，默认按角色个人状态处理：
* 只影响某个角色出牌、伤害、治疗、削韧或其他主动行为的状态
* 只影响某个角色压力、崩溃、苏醒、受击承压、专属机制或奥义判定的状态
* 只作用于该角色所属牌、该角色专属机制或该角色专属资源的状态

首版默认示例：
* `士气 / 虚弱`
* `刀势 / 药引`

### 7.3 默认结算顺序
当一次效果同时涉及共享血条与角色个人状态时，首版默认按以下原则处理：
1. 先确定这次效果影响的是共享对象还是角色个人对象
2. 先结算共享生命相关状态修正，再得到本次实际生命变化
3. 再根据实际生命变化结算压力、崩溃、苏醒等角色个人结果
4. 只影响角色主动行为的状态，不回头改写已经完成的共享生命结算结果

## 8. 首版通用状态清单
### 8.1 通用正面状态
* `护体`：受击时参与减伤计算；玩家侧通常按队伍共享状态处理
* `士气`：造成伤害时参与攻击修正；玩家侧通常按角色个人状态处理
* `免疫`：用于抵消或忽略特定负面效果；玩家侧通常按队伍共享状态处理
  * `生命免疫`：当前 Runtime 已落地的免疫子类，用于抵消下一次穿透护盾的玩家共享生命 HP damage

### 8.2 通用负面状态
* `中毒`：当前只作用于敌方单位；在玩家结束回合后的敌方行动前窗口结算
* `腐蚀`：当前主要作用于玩家侧；效果与中毒相同；在玩家结束回合后的敌方行动前窗口结算
* `灼烧`：当前只作用于敌方单位；在敌方回合开始窗口结算
* `流血`：当前只作用于敌方单位；在敌方单位行动后窗口结算
* `易伤`：目标受到伤害时参与伤害放大；玩家侧通常按队伍共享状态处理
* `虚弱`：目标造成伤害时参与伤害降低；玩家侧通常按角色个人状态处理
* `迟滞`：先机变化或敌方行动检查时参与判定
* `破绽`：Break、追击、处决等条件收益判定状态

## 9. 首版角色专属状态清单
### 9.1 霍断岳
* `刀势`：霍断岳专属状态；默认按层数累积；starter Runtime 当前通过霍断岳 innate passive 的 `OwnerTookHealthDamage` 窗口获得，并由明确配置的攻击牌通过 `ConsumeStatusResource` 消耗来追加削韧
* `压势追刀`：霍断岳能力牌“受压蓄势”授予的被动；在 `PlayerCardResolved` 的 `ResolvedCard(Attack) + OncePerPlayerTurn` 窗口下，为当前手牌中的攻击牌投影 `-1 AP / +20% 伤害`，持续到打出或玩家回合结束

### 9.2 叶半夏
* `药引`：叶半夏专属状态；默认按层数累积；不自动生效，只有被牌通过 `ConsumeStatusResource` 明确消耗时才结算收益

### 9.3 沈清弦
* 沈清弦当前不以专属状态承载核心机制；`剑阵` 相关内容属于衍生牌体系，不纳入状态系统

### 9.4 后续扩展
* `猎印`：已在 [GDD4.0.md](GDD4.0.md) 中作为专属状态方向出现，具体规则待补

## 10. 首版状态模板样例
### 10.1 使用说明
* 以下样例用于验证 `StatusMergeRule / StatusStackRule / StatusRefreshRule` 在首版状态中的落地方式
* 已被其他文档定死的内容直接采用现有口径
* 尚未在其他文档定死的数值型字段，统一标注为 `首版建议`

### 10.2 中毒
* `StatusId`：`status_poison`
* `StatusCategory`：`Debuff`
* `默认归属`：目标敌方单位
* `DurationType`：`Turn`（首版建议）
* `StatusMergeRule`：`ByOwnerAndSource`
* `StatusStackRule`：`AddAndClamp`
* `StatusRefreshRule`：`KeepLonger`
* `摘要文本`：在玩家结束回合后的敌方行动前窗口结算持续伤害。
* `适用理由`：持续伤害默认依赖施加者来源结算，不同 `SourceUnitId` 分开保存更稳。
* `补充说明`：`腐蚀` 默认复用与 `中毒` 相同的模板结构，只通过 `StatusId / DisplayName / 摘要文本 / 默认归属` 区分。

### 10.3 护体
* `StatusId`：`status_guard`
* `StatusCategory`：`Buff`
* `默认归属`：玩家侧通常为 `team_player`
* `DurationType`：`Turn`（首版建议）
* `StatusMergeRule`：`ByOwner`
* `StatusStackRule`：`RefreshOnly`
* `StatusRefreshRule`：`KeepLonger`
* `摘要文本`：在受击时参与减伤计算。
* `适用理由`：护体当前更像单层防护修正，首版默认优先做单实例刷新，不急着做复杂叠层。

### 10.4 士气
* `StatusId`：`status_morale`
* `StatusCategory`：`Buff`
* `默认归属`：实际造成效果的角色运行时单位
* `DurationType`：`Turn`（首版建议）
* `StatusMergeRule`：`ByOwner`
* `StatusStackRule`：`RefreshOnly`
* `StatusRefreshRule`：`KeepLonger`
* `摘要文本`：在造成伤害时参与攻击修正。
* `适用理由`：士气当前更像角色个人输出修正，首版默认以单层刷新模型更容易控数值。
* `当前 authoring / runtime 口径`：
  * authoring：`StatusDefinition.RuntimeModifiers`
  * runtime：`BattleStatusInstance.RuntimeModifiers`
  * 结算：`FinalBattleStatusService.GetOutgoingDamageModifierPercent()`
* `starter 第一波 RuntimeModifiers 落点`：
  * `OutgoingDamagePercentPerStack = 20`
  * `bConsumeOnSuccessfulOwnerDamage = false`
  * `bOnlyAffectAttackCards = false`
  * `DurationType = PlayerTurns`
  * `ExpireWindow = PlayerTurnEnd`

### 10.4.1 锋锐
* `StatusId`：`status_shen_feng_rui`
* `StatusCategory`：`Buff`
* `默认归属`：沈清弦当前实际出牌的角色运行时单位
* `摘要文本`：下一张攻击牌伤害提高 20%；若本回合未触发，则在回合结束时移除。
* `starter 第一波 ProjectedCardModifiers 落点`：
  * `TargetSource = CurrentOwnedHandCards`
  * `RequiredCardType = Attack`
  * `OutgoingDamagePercentPerStack = 20`
  * `LifetimePolicy = WhileStatusActive`
  * `bExpireAtPlayerTurnEnd = true`
* `保留的状态生命周期字段`：
  * `bExpireAtPlayerTurnEnd = true`
  * `bConsumeOnSuccessfulOwnerDamage = true`
  * `bOnlyAffectAttackCards = true`
* `当前 Battle 规则口径`：
  * `锋锐` 不再走通用状态伤害修正路径。
  * 当前由 `FinalBattle` 基于 `ProjectedCardModifiers` 把 `锋锐` 同步为手牌攻击牌上的 derived `BattleCard` modifier。
  * 首版只作用于当前手牌中的攻击牌；抽到手或生成进手的新攻击牌，只要 `锋锐` 仍在，也会立即获得同样投影。
  * 弃牌堆、抽牌堆、消耗区、持续区中的牌默认不带 `锋锐` 投影。
  * 成功造成一次伤害后仍只消耗 1 层；若本回合未触发，则在玩家回合结束时移除。

### 10.5 刀势
* `StatusId`：`status_dao_shi`
* `StatusCategory`：`Signature`
* `默认归属`：霍断岳
* `DurationType`：`Battle`（首版建议）
* `StatusMergeRule`：`ByOwner`
* `StatusStackRule`：`AddAndClamp`
* `StatusRefreshRule`：`NoRefresh`
* `摘要文本`：按层数累积；starter Runtime 已支持已配置牌获得、受队伍生命伤害触发获得，以及由明确配置的攻击牌消耗来追加削韧。
* `适用理由`：刀势是典型层数资源，重点在累积与消耗，不需要按回合刷新持续值；完整“所有攻击默认消耗”的通用挂钩留待后续深化。
* `当前资源协议口径`：
  * 获得：`ApplyStatus`
  * 消费：`ConsumeStatusResource`
  * 资源状态默认不参与 `RuntimeModifiers / ProjectedCardModifiers / RuntimeTriggers`

### 10.6 药引
* `StatusId`：`status_yao_yin`
* `StatusCategory`：`Signature`
* `默认归属`：叶半夏
* `DurationType`：`Battle`（首版建议）
* `StatusMergeRule`：`ByOwner`
* `StatusStackRule`：`AddAndClamp`
* `StatusRefreshRule`：`NoRefresh`
* `摘要文本`：按层数累积；不自动生效，只有被牌明确消耗时才结算收益。
* `适用理由`：药引本质上是专属储备资源，首版默认按战斗内累积资源处理。
* `当前资源协议口径`：
  * 获得：`ApplyStatus`
  * 消费：`ConsumeStatusResource`
  * 典型消费牌：`化引 / 回春散`

### 10.7 免疫
* `StatusId`：`status_immunity`
* `StatusCategory`：`Buff`
* `默认归属`：玩家侧通常为 `team_player`
* `DurationType`：`Turn`（首版建议）
* `StatusMergeRule`：`ByOwner`
* `StatusStackRule`：`RefreshOnly`
* `StatusRefreshRule`：`KeepLonger`
* `摘要文本`：用于抵消或忽略特定负面效果。
* `适用理由`：免疫当前更像单层防护状态，首版默认优先采用单实例刷新模型。
* `当前 Runtime 子类`：`生命免疫`
  * `StatusId`：`status.starter.ye.mianyi`
  * `DisplayName`：`生命免疫`
  * `摘要文本`：抵消下一次穿透护盾的玩家共享生命 HP damage；触发后消耗，若到玩家回合结束仍未触发则过期。
* `生命免疫 RuntimeModifiers / 持续配置`：
  * `RuntimeModifiers.IncomingTeamHealthDamageReductionPercentPerStack = 100`
  * `RuntimeModifiers.bConsumeOnPreventedTeamHealthDamage = true`
  * `DurationType = PlayerTurns`
  * `ExpireWindow = PlayerTurnEnd`
* `生命免疫结算顺序`：先由护盾抵消总伤害，再由生命免疫抵消剩余 pending Team HP damage，保护后的实际 HP damage 才扣 `TeamCurrentHP`。
* `生命免疫触发边界`：若生命免疫完全抵消本次 HP damage，则不触发 `OwnerTookHealthDamage`。
* `未落地范围`：免疫中毒、免疫控制、免疫压力、免疫崩溃等更泛化负面效果免疫，仍需要后续补独立协议。

## 11. 状态文本规范
状态文案默认遵守以下原则：
* 状态名只表示状态本身，不承担关键词语义
* 摘要文本优先描述“这条状态当前会造成什么结果”
* 完整说明文本优先描述“何时触发、影响谁、如何结束”
* 不把状态名、资源名、关键词名混写成同一层规则

推荐写法：
* 先写触发时点
* 再写主要效果
* 最后写层数、持续或移除条件

## 12. 首版制作范围
### 12.1 首版必须明确的内容
* 通用状态与专属状态的完整词表
* 每个状态的归属对象
* 每个状态的默认结算窗口
* 每个状态的最大层数、持续方式与摘要文本
* 共享血条下玩家侧状态的结算边界

### 12.2 仍待补充的内容
* 首领专属机制状态与多阶段状态切换规则
* `猎印` 等后续角色专属状态的详细定义
* 通用状态的正式 `MaxStacks`、持续回合数与数值倍率
