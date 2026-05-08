# FirstProject Implementation Progress

本文件只记录 First 新项目已经完成的事实。旧三角色项目的历史进度继续保留在 `F_Docs/Implementation_Progress.md`，不迁入本文件。

## 2026-05-08

- First 新方向已确认：单人第一人称 HD-2D 卡牌冒险，战斗核心从旧三角色小队、AP/EP、单体敌人单位迁移到单人、敌人多部位、Cost 先机、完美释放、手牌三区和背包。
- `AGENTS.md` 已切换到 First 新项目协作口径：旧三角色、AP/EP、旧 Battle HUD、旧线性 Run、旧 EnemyPanel、旧 Break 均标记为 legacy。
- First 专用文档目录已建立：`F_Docs/FirstProject/`。
- First 玩法文档已迁入：`F_Docs/FirstProject/FirstPerson_HD2D_Card_Variant.md`。
- First 专用进度与待办文档已重开：`Implementation_Progress.md` 与 `Development_Backlog.md`。
- `FirstBattle Kernel Skeleton v0.1` 已建立：`FinalBattle` 模块内新增 First 专用 `Session / Command / Snapshot / Kernel / State` 骨架，固定 `Initialize -> SubmitCommand -> GetSnapshot` 入口。
- First 卡牌实例首版采用 `RuntimeCost + Keywords + Effects` 形状，避免把伤害直接写成卡牌字段；当前只定义最小 `Damage` effect 类型，尚未执行效果结算。
- 新增 `Final.Battle.First.Kernel.*` 自动化测试覆盖初始化、snapshot 复制隔离、未实现命令拒绝和未初始化安全 snapshot。
- `FirstBattle PlayCard Core v0.1` 已建立：`PlayCard` 现在可执行目标部位 `Damage`、移动手牌到弃牌堆、按出牌前先机触发完美释放事件、按 `RuntimeCost` 扣减所有存活部位先机、为归零部位记录行动占位事件，并在全部部位破坏后判定胜利。
- `First.Keyword.Swift` 已注册为 First 专用 native gameplay tag；迅捷牌跳过先机扣减和完美释放，但仍执行自身其他效果。
