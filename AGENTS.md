# AGENTS.md

## 1. 当前项目状态
本项目已经不再处于“从零启动文档阶段”。当前已有 Unreal 多模块源码、Starter 原型内容、战斗 HUD 原型、Run 外层流程与自动化测试。

当前开发目标是继续把项目推进成可玩的首章竖切：
1. 保持规则真相正确。
2. 保持 `FinalBattle / FinalRun / FinalApp` 边界清楚。
3. 优先补齐主线可玩流程，再扩复杂表现。
4. 任何稳定协议或规则变化都同步回写 `F_Docs`。

---

## 2. 模块边界
项目模块固定为：
* `FinalCore`
* `FinalData`
* `FinalBattle`
* `FinalRun`
* `FinalApp`
* `FinalEditor`

依赖方向固定：
* `FinalData` 只依赖 `FinalCore`
* `FinalBattle` 只依赖 `FinalCore + FinalData`
* `FinalRun` 只依赖 `FinalCore + FinalData`
* `FinalBattle` 不依赖 `FinalRun`
* `FinalRun` 不依赖 `FinalBattle`
* `FinalApp` 桥接 Battle、Run、World、UI、Save
* `FinalEditor` 只做编辑器校验、内容生成与测试工具

默认不要新增跨模块依赖。确实需要共享协议时，优先放到 `FinalCore` 或 `FinalData` 的薄结构中。

---

## 3. 权威状态归属
* 战斗内权威状态属于 `FinalBattle`。
* 单局外权威状态属于 `FinalRun`。
* 静态内容定义属于 `FinalData`。
* UI、Actor、Widget Blueprint、动画、特效只做表现和命令转发。

不允许在 `FinalApp`、Widget Blueprint、Actor 中结算：
* 伤害 / 治疗 / 护盾
* Break / 先机 / 回合推进
* 状态叠加和触发窗口
* 敌人 Intent 选择
* 奖励、商店、事件、节点推进真相

---

## 4. 当前已落地能力
当前项目已经具备：
* `RunSession -> BattleSession -> BattleResult -> RunSession` 最小闭环。
* `PlayCard / PlayUltimate / EndTurn` 战斗命令。
* 战斗资源、状态、敌人行动、事件账本与 Snapshot 投影。
* 敌人 Intent 的 `Cycle / WeightedRandom / PhaseSequence / Scripted` 第一版。
* Starter 敌人、遭遇、首领雏形与 Intent 内容验证。
* 战后金币自动入账、最多 3 张卡牌候选、选择或跳过奖励。
* 线性 Starter 路线到 Boss 后 `RunEnded` 的规则测试。
* Battle HUD 的可配置 Widget Class、手牌 Canvas 扇形布局、hover/进出动画。
* 独立 `BattleResourcePanel`，用于底部资源区域与左下 EP 气圈。
* `FinalEditor` 数据校验、Starter 内容 bootstrap、PrototypeSmoke 测试。

因此后续任务应默认在这些基础上增量开发，不要重新搭一套临时流程。

---

## 5. 下一阶段优先级
当前优先级：
1. 完善 `FinalRun` 主线可玩 UI：节点页、战后奖励页、事件页、商店页、Boss 后结束页。
2. 深化 Starter 首章内容：敌人、卡牌、事件、商店、奖励节奏。
3. 补齐 Run 外层规则：节点奖励、商店购买/跳过、事件选项、牌库成长。
4. 战斗表现继续迭代：敌人意图 UI、目标选择、卡牌拖拽、选中态、战斗反馈。
5. 最后再扩遗物、复杂被动、正式存档、完整回放与更复杂地图。

如果任务会同时碰到多个系统，优先保证 `FinalRun` 和 `FinalBattle` 的权威边界不被打穿。

---

## 6. 文档真相优先级
玩法冲突时按以下顺序：
1. `F_Docs/GDD4.0.md`
2. `F_Docs/Battle_Rules.md`
3. `F_Docs/Status_System_Guide.md`
4. `F_Docs/Card_Design_Guide.md`
5. `F_Docs/Combat_Data_Schema_v2.md`
6. `F_Docs/Numbers_FirstPass.md`
7. `F_Docs/Starter_Characters.md`
8. `F_Docs/Starter_Enemies.md`
9. `F_Docs/Starter_Relics.md`
10. `F_Docs/Starter_Events.md`

工程冲突时按以下顺序：
1. `F_Docs/Code_Function_Requirements.md`
2. `F_Docs/Unreal_Source_Structure.md`
3. `F_Docs/Source_Bootstrap_Checklist.md`
4. `F_Docs/UI_Wireframe.md`

发现冲突时不要静默折中。先标出冲突，再按优先级执行。

---

## 7. C++ 与 Blueprint 分工
C++ 必须负责：
* Battle / Run 权威状态
* 命令合法性校验
* 规则结算
* Intent 选择
* 奖励、商店、事件、节点推进
* 数据校验与自动化测试

Blueprint / UMG 可以负责：
* HUD 布局、控件层级、字体、材质、动画
* 纯表现 Actor
* 摄像机、特效、音频
* 点击、悬停、拖拽等输入表现，但最终必须转成 C++ 命令

一旦 Blueprint 开始改变数值、卡牌去向、节点状态或战斗真相，应回收到 C++。

---

## 8. 数据驱动原则
默认通过 `FinalData` 定义资产和协议结构表达内容。

禁止把玩法写成：
* `if CardId == ...`
* `if EnemyId == ...`
* `if 当前是某个具体资产 then 特判`

如果现有 schema 表达不了需求：
1. 记录缺口。
2. 判断应补 `FinalData` 协议、`FinalBattle` 服务还是 `FinalRun` 服务。
3. 补测试。
4. 再更新文档。

---

## 9. Public / Private 规则
默认：
* `Public` 只放跨模块必须访问的接口、Snapshot、Command、Event、定义类型。
* `Private` 放运行时状态、服务类、helper、解析器和内部算法。
* 不要为了方便把 `FinalBattle` 或 `FinalRun` 的权威运行时结构抬到 `Public`。
* Header 中优先前置声明，具体 include 放到 `.cpp`。

---

## 10. 测试与验证
改规则、协议、流程时至少考虑：
* 编译：`Build.bat FinalFinalEditor Win64 Development -Project=D:\UE_Project\5.6\FinalFinal\FinalFinal.uproject -NoHotReload`
* 战斗规则：`Final.Battle.*`
* Starter 内容：`Final.Editor.StarterIntentContent`
* 主链路：`Final.Editor.PrototypeSmoke`

涉及内容资产生成时，运行 `FinalPrototypeContentBootstrapCommandlet` 后要确认资产变更是预期的。

---

## 11. 禁止事项
当前仍然禁止：
* 用 GAS 作为核心规则框架。
* 用 Tick 驱动战斗主规则。
* 用行为树驱动卡牌战斗时序。
* 让 `FinalBattle` 和 `FinalRun` 互相 include。
* 在 UI / Actor / Blueprint 中长期保留临时规则真相。
* 复制一份状态给 UI 当真相。
* 静默改玩法口径但不改文档。
* 重写已有闭环而不说明迁移原因。

---

## 12. 每次任务开始前
先判断任务属于：
* `Battle`
* `Run`
* `Data`
* `App/UI`
* `Editor/Validation`
* `Docs`

然后只读取对应文档和代码，不做无关重构。实现应保持最小范围。

---

## 13. 每次提交前
检查：
* 是否破坏模块依赖方向。
* 是否把规则写进 UI / Blueprint。
* 是否新增硬编码资产个例。
* 是否需要同步 `F_Docs`。
* 是否跑过与改动范围匹配的测试。
* 是否有内容资产或自动生成资产的预期变更。
