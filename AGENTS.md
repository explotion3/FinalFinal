# AGENTS.md

## 1. 适用范围
本文件用于当前项目的开发阶段。  
当前目录中的玩法文档、数据文档、架构文档、启动清单，以及后续新建的 Unreal 源码，都应遵守本文件。

如果后续真正开始写 Unreal 工程代码，并且代码目录不在 `F_Docs` 下，应将本文件同步到项目根目录。

---

## 2. 当前阶段目标
当前阶段的目标不是继续扩写文档，而是基于现有设计文档，稳定启动一套：
* 可扩展
* 可复用
* 数据驱动
* 模块边界清楚
* 能持续补内容而不频繁返工底层

开发优先级：
1. 规则真相正确
2. 模块边界正确
3. 数据结构稳定
4. 最小闭环可跑通
5. 表现层后接

---

## 3. 真相来源优先级

### 3.1 玩法真相
多个文档重叠或冲突时，按以下顺序执行：
1. `GDD4.0.md`
2. `Battle_Rules.md`
3. `Status_System_Guide.md`
4. `Card_Design_Guide.md`
5. `Combat_Data_Schema_v2.md`
6. `Numbers_FirstPass.md`
7. `Starter_Characters.md`
8. `Starter_Enemies.md`
9. `Starter_Relics.md`
10. `Starter_Events.md`

### 3.2 工程真相
工程落地按以下顺序执行：
1. `Code_Function_Requirements.md`
2. `Unreal_Source_Structure.md`
3. `Source_Bootstrap_Checklist.md`

规则：
* 玩法文档定义“做什么”
* Schema 文档定义“数据长什么样”
* 工程文档定义“代码怎么拆”
* 启动清单定义“先做哪一批”

---

## 4. 已定工程结论
以下内容已定，不允许在开发中自行改回旧口径：

### 4.1 模块
统一采用：
* `FinalCore`
* `FinalData`
* `FinalBattle`
* `FinalRun`
* `FinalApp`
* `FinalEditor`

### 4.2 依赖方向
必须满足：
* `FinalData` 只依赖 `FinalCore`
* `FinalBattle` 只依赖 `FinalCore + FinalData`
* `FinalRun` 只依赖 `FinalCore + FinalData`
* `FinalBattle` 不依赖 `FinalRun`
* `FinalRun` 不依赖 `FinalBattle`
* `FinalApp` 负责桥接 Battle、Run、世界、UI

### 4.3 权威状态
* 战斗权威状态属于 `FinalBattle`
* 单局外权威状态属于 `FinalRun`
* UI、Actor、Widget、Blueprint 都不持有规则真相

### 4.4 首批闭环
首批最小闭环必须经过：
`RunSession -> FinalBattleStartRequest -> BattleSession -> FinalBattleResult -> RunSession`

首批必须支持：
* 普通战初始化
* `PlayCard`
* `PlayUltimate`
* `EndTurn`
* 伤害 / 削韧 / Break / 先机减少事件
* 敌人行动
* 战斗胜利
* 战后回写 Run 状态

### 4.5 命令边界
必须明确区分：
* `BattleCommand`
* `RunCommand`

规则：
* `BattleCommand` 只承载战斗内动作
* `RunCommand` 只承载事件、奖励、商店、成长、节点推进等单局外动作

### 4.6 Public / Private
默认规则：
* `Public` 只暴露跨模块必须使用的接口
* `Private` 放权威运行时结构和内部实现
* `FinalBattle/Public` 优先暴露：`Session / Command / Event / Snapshot / QueryTypes`
* `BattleState`、`BattleCharacterState`、`BattleEnemyState`、`TeamDeckState` 默认放 `Private/Runtime`

### 4.7 数据驱动
默认必须走定义资产和协议结构。  
不允许把完整玩法规则写成：
* `CardId == xxx`
* `EnemyId == xxx`
* `if 某张牌 then 特殊处理`

如果结构承载不了：
1. 先记录缺口
2. 再决定补协议、补定义还是补服务层
3. 不允许先绕过数据层硬写个例

---

## 5. 当前阶段禁止事项
当前默认禁止：
* 用 GAS 作为核心规则框架
* 用 Tick 驱动战斗主逻辑
* 用行为树驱动战斗时序
* 在 `FinalApp` 里写权威战斗规则
* 在 Blueprint 中结算伤害、Break、先机、崩溃、苏醒、状态窗口
* 为了方便把 `Private` 结构抬进 `Public`
* 为了方便让 `FinalBattle` 和 `FinalRun` 互相 include
* 静默修改玩法真相但不回写文档
* 靠记忆补规则

---

## 6. Blueprint 与 C++ 分工

### 6.1 必须放在 C++
* 战斗权威状态
* 命令合法性校验
* 伤害、治疗、压力
* Break 与先机
* 状态叠加、刷新、结算窗口
* 崩溃与苏醒
* 敌人意图选择
* 事件条件与代价判定
* 奖励、成长、商店的核心结算

### 6.2 可以放在 Blueprint
* 场景摆放
* UI 布局
* 动画、特效、音频
* 表现层 Actor
* 摄像机与纯展示逻辑

### 6.3 原则
一旦 Blueprint 开始改：
* 数值
* 卡牌去向
* 行动顺序
* 状态真相

就应回收到 C++。

---

## 7. 开发流程
每次开始一个任务时，按以下顺序执行：
1. 先确认任务属于 `Battle / Run / Data / App / Editor`
2. 回读相关真相文档
3. 检查是否涉及模块边界变化
4. 先做最小实现，不顺手扩写无关系统
5. 新增了稳定接口、共享协议、权威结构或规则后，必须同步回写文档

若发现文档冲突：
* 不要静默折中
* 先标出冲突
* 默认按第 3 节优先级执行

---

## 8. 从零开工的默认顺序
默认执行顺序：
1. 建立首批模块
   * `FinalCore`
   * `FinalData`
   * `FinalBattle`
   * `FinalRun`
   * `FinalApp`
2. 建立首批 `.Build.cs`
3. 先写 `FinalCore`
4. 再写 `FinalData`
5. 再写 `FinalRun` 的最小桥接状态与请求结构
6. 再写 `FinalBattle` 的最小战斗闭环
7. 最后由 `FinalApp` 接入输入、HUD、世界桥接
8. `FinalEditor` 后置

---

## 9. 第一批必须具备的能力

### 9.1 最小数据资产
第一批必须有：
* `BattleRuleConfig`
* `CharacterDefinition`
* `CardDefinition`
* `UltimateDefinition`
* `EnemyDefinition`
* `EnemyIntentDefinition`
* `StatusDefinition`
* `BattleEncounterDefinition`

### 9.2 FinalApp 首批职责
首批 `FinalApp` 必须能：
* 转发 `PlayCard`
* 转发 `PlayUltimate`
* 转发 `EndTurn`
* 把单局外输入转成 `RunCommand`
* 创建并持有 `BattleSession`
* 提供最小 HUD 可读状态

### 9.3 第二批与第三批边界
第二批：
* 状态、被动、遗物触发深化
* `FinalRun` 的事件、奖励、商店、成长链

第三批：
* Save / Load 深化
* `FinalEditor` 校验与工具
* 完整战斗日志 / 回放
* 复杂首领阶段与后置增强内容

不允许把第二批已经覆盖的 Run 外层系统，再重复写进第三批。

---

## 10. 允许的实现策略
允许：
* 用 `TODO`
* 用 `待补`
* 用最小空壳接口占位
* 用 `Snapshot / Query / Request / Result` 隔离内部状态
* 在结构未定死前先做最小竖切，但必须保留模块边界

不允许：
* 先写一套临时实现，后面不回收
* 用 UI 或 Blueprint 暂时代替权威规则并长期保留
* 通过复制状态真相解决跨模块读取

---

## 11. 文档更新规则
如果开发过程中发生以下变化，必须同步更新文档：
* 新增稳定模块
* 新增共享协议
* 新增权威运行时结构
* 修改命令边界
* 修改战斗结算规则
* 修改首批最小闭环范围

更新顺序：
1. 先改真相文档
2. 再改支撑文档
3. 不要只改其中一份

---

## 12. 停机条件
出现以下任一情况时，不要继续写代码，先停下来检查：
* 不确定该逻辑属于 `FinalBattle` 还是 `FinalRun`
* 需要让 `FinalBattle` 直接依赖 `FinalRun`
* 需要把权威运行时结构暴露到 `Public`
* 需要用 Blueprint 改写规则真相
* 需要靠 `CardId / EnemyId` 写死玩法
* 上位文档之间出现冲突
* 首批闭环里的关键链路被绕开

---

## 13. 每次提交前检查
至少检查：
* 是否读过相关真相文档
* 是否违反 `Battle / Run` 边界
* 是否把权威状态泄露到 `Public`
* 是否把规则写进了 UI / Actor / Blueprint
* 是否引入了新的硬编码个例
* 是否影响了首批闭环
* 是否需要同步更新文档

只要有一项答案不明确，就先停下来检查，不直接继续扩写代码。
