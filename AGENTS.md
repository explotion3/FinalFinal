# AGENTS.md

## 1. 当前项目状态
本项目正在从旧“三角色小队首章竖切”迁移到新的 **First 单人第一人称 HD-2D 卡牌冒险**方向。

旧系统仍可作为工程底座复用，但不再是玩法主线。旧三角色小队、AP/EP、旧 Battle HUD、旧线性 Run 流程、旧 EnemyPanel、旧 Break 口径都视为 legacy。后续可以逐步删除或替换，但修改时仍应保持工程编译链可控。

当前首要目标：
1. 建立 `FirstBattle` 规则核。
2. 再逐步迁移 `FinalData / FinalApp / FinalRun`。
3. 保持规则真相正确。
4. 保持 `FinalBattle / FinalRun / FinalApp` 边界清楚。
5. 稳定协议、权威规则或模块边界变化必须同步回写 `F_Docs`。

---

## 2. 模块边界
项目模块名暂时保持不变：
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

新规则文件、类型和目录使用 `First` 前缀，例如：
* `FFirstBattleSession`
* `FFirstBattleState`
* `FFirstCardInstance`
* `Source/FinalBattle/Public/First`
* `Source/FinalBattle/Private/First`

不要在当前阶段做全项目 `Final` 到 `First` 的模块级重命名。UE 导出宏仍使用现有模块宏，例如 `FINALBATTLE_API`。

默认不要新增跨模块依赖。确实需要共享协议时，优先放到 `FinalCore` 或 `FinalData` 的薄结构中。

---

## 3. 权威状态归属
战斗内权威状态属于 `FinalBattle`。单局外权威状态属于 `FinalRun`。静态内容定义属于 `FinalData`。UI、Actor、Widget Blueprint、动画、特效只做表现和命令转发。

First 战斗核心包括：
* 单人战斗
* 敌人多部位
* 卡牌 `Cost` 驱动敌方部位先机
* 完美释放
* 左手区 / 双手区 / 右手区
* 左手牌 / 右手牌
* 背包
* 部位掉落
* 击倒事件
* 状态归属

这些规则只能在 `FinalBattle / FinalRun / FinalData` 中实现。

不允许在 `FinalApp`、Widget Blueprint、Actor 中结算：
* 伤害 / 治疗 / 护盾
* 敌方部位先机
* 完美释放
* 回合推进
* 部位破坏 / 敌人击倒
* 状态叠加和触发窗口
* 敌人 Intent 选择
* 卡牌进入 / 离开手牌区的规则真相
* 背包容量、战斗带入、战斗回流
* 奖励、商店、事件、Run 进度真相

---

## 4. 当前可复用工程能力
旧项目已经具备一批可复用底座：
* Unreal 多模块源码结构。
* `FinalData / FinalBattle / FinalRun / FinalApp / FinalEditor` 分层。
* Command / Snapshot / Event 风格的规则与 UI 桥接方式。
* DataRegistry、DataAsset、Validator、Starter bootstrap。
* Battle HUD、World Presentation、Run Overlay、Widget Class Settings 等 UI 基础设施。
* 自动化测试、PrototypeSmoke、内容生成工具。

这些能力可以复用，但不能为了兼容旧系统而扭曲 First 规则。

---

## 5. 下一阶段优先级
当前优先级：
1. `FirstBattle Core`：敌人部位、Cost 先机、完美释放、部位行动。
2. `First Hand Zones`：左手区 / 双手区 / 右手区、左手牌 / 右手牌、腾挪。
3. `First Card Data`：新卡牌 schema、Cost、迅捷、完美释放、区域条件。
4. `First Enemy Parts`：多部位敌人、部位意图、部位掉落、击倒事件。
5. `First Backpack / Run`：背包、战斗带入、战斗回流、自由探索 Run。
6. `First App/UI`：第一人称 HD-2D 表现、手牌三区 UI、敌人部位 UI。

如果任务会同时碰到多个系统，优先保证 `FinalBattle` 和 `FinalRun` 的权威边界不被打穿。

---

## 6. 文档真相优先级
First 玩法冲突时按以下顺序：
1. `F_Docs/FirstProject/FirstPerson_HD2D_Card_Variant.md`
2. 后续新增的 First 专用规则文档
3. 本文件 `AGENTS.md`

旧文档仅作为历史参考，除非明确迁移到 First 文档：
* `F_Docs/GDD4.0.md`
* `F_Docs/Battle_Rules.md`
* `F_Docs/Card_Design_Guide.md`
* `F_Docs/Status_System_Guide.md`
* `F_Docs/Combat_Data_Schema_v2.md`
* `F_Docs/Numbers_FirstPass.md`
* `F_Docs/Starter_Characters.md`
* `F_Docs/Starter_Enemies.md`
* `F_Docs/Starter_Relics.md`
* `F_Docs/Starter_Events.md`

工程冲突时按以下顺序：
1. `AGENTS.md`
2. `F_Docs/Code_Function_Requirements.md`
3. `F_Docs/Unreal_Source_Structure.md`
4. `F_Docs/Source_Bootstrap_Checklist.md`
5. `F_Docs/UI_Wireframe.md`

发现冲突时不要静默折中。先标出冲突，再按优先级执行。

---

## 7. C++ 与 Blueprint 分工
C++ 必须负责：
* Battle / Run 权威状态
* 命令合法性校验
* 规则结算
* Intent 选择
* 手牌区规则
* 背包、奖励、商店、事件、Run 进度
* 数据校验与自动化测试

Blueprint / UMG 可以负责：
* HUD 布局、控件层级、字体、材质、动画
* 第一人称 HD-2D 表现
* 纯表现 Actor
* 摄像机、特效、音频
* 点击、悬停、拖拽等输入表现，但最终必须转成 C++ 命令

一旦 Blueprint 开始改变数值、卡牌去向、手牌区归属、部位状态、背包容量或 Run 真相，应回收到 C++。

---

## 8. 数据驱动原则
默认通过 `FinalData` 定义资产和协议结构表达内容。

禁止把玩法写成：
* `if CardId == ...`
* `if EnemyId == ...`
* `if 当前是某个具体资产 then 特判`

如果现有 schema 表达不了 First 需求：
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

First 新规则初期可以在 `FinalBattle/Public/First` 暴露最小 command / snapshot / session facade，在 `FinalBattle/Private/First` 放规则实现。

---

## 10. 测试与验证
改规则、协议、流程时至少考虑：
* 编译：`Build.bat FinalFinalEditor Win64 Development -Project=D:\UE_Project\5.6\FinalFinal\FinalFinal.uproject -NoHotReload`
* First 战斗规则：`Final.Battle.First.*`
* 旧主链路安全烟测：`Final.Editor.PrototypeSmoke`

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
* 静默沿用旧文档口径覆盖 `F_Docs/FirstProject/FirstPerson_HD2D_Card_Variant.md`。
* 为了兼容旧 HUD 扭曲 First 规则。
* 把旧 AP/EP、三角色队伍、Break、旧 EnemyPanel 当新功能基础继续扩。

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

如果任务涉及 First 玩法，必须先读取：
* `F_Docs/FirstProject/FirstPerson_HD2D_Card_Variant.md`
* `AGENTS.md`

First 专用进度与待办记录写入：
* `F_Docs/FirstProject/Implementation_Progress.md`
* `F_Docs/FirstProject/Development_Backlog.md`

根目录旧 `F_Docs/Implementation_Progress.md` 与 `F_Docs/Development_Backlog.md` 保留为 legacy 项目历史记录，不再作为 First 主线记录入口。

---

## 13. 每次提交前
检查：
* 是否破坏模块依赖方向。
* 是否把规则写进 UI / Blueprint。
* 是否新增硬编码资产个例。
* 是否需要同步 `F_Docs`。
* 是否跑过与改动范围匹配的测试。
* 是否有内容资产或自动生成资产的预期变更。
* 是否误把 legacy 旧系统当作 First 新规则继续扩展。
