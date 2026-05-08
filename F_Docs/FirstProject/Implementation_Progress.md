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
- `FirstBattle EndTurn + 部位行动刷新 v0.1` 已建立：敌方部位拥有首版 `IntentSequence`，部位行动会记录 `EnemyPartActed`、推进到下一意图并重置先机。
- `EndTurn` 现在可执行：所有存活部位按 `PositionIndex` 行动一次，随后 `CurrentRound +1`；本玩家回合内已经因先机归零行动过的部位，结束回合仍会再次行动。
- `PlayCard` 触发先机归零时已从占位事件升级为真实部位行动刷新；本轮仍不执行敌人意图效果、不伤害玩家、不抽牌、不接 UI / Run / DataAsset。
- `FirstBattle Enemy Intent Effects v0.1` 已建立：玩家侧拥有最小 `PlayerMaxHP / PlayerCurrentHP` runtime 与 snapshot 字段，敌方部位意图可 authoring 最小 `Damage` effect。
- 部位行动现在会执行当前意图的 `Damage` effect，对玩家 HP 造成伤害并记录 `PlayerDamaged`；玩家 HP 归零时记录 `BattleLost`，设置战斗结束且玩家失败。
- `PlayCard` 先机归零行动队列与 `EndTurn` 部位行动都已接入失败中断：某个部位行动击败玩家后，后续部位不再行动，`EndTurn` 不推进回合。
- `FirstBattle Hand Zones v0.1` 已建立：First 卡牌实例新增 `HandRole`，snapshot 中每张手牌会投影 `HandIndex / HandRole / HandZone`。
- 手牌三区首版按当前手牌顺序计算：左手牌左侧为左手区，右手牌右侧为右手区，左右手牌之间为双手区；缺少任一锚点时双手区不存在。
- 左手 / 右手核心牌仍按普通卡牌打出并进入弃牌堆；它们离开手牌后，其他手牌不移动，只在 snapshot 中自然失去对应区域。
- `FirstBattle Player Turn Draw v0.1` 已建立：First runtime 新增抽牌堆，snapshot 输出 `DrawPileCount / DiscardPileCount`，`EndTurn` 在敌方部位行动结算且战斗未结束后推进回合并进入下一玩家回合抽牌。
- 回合开始抽牌首版会强制找回缺失的左手 / 右手核心牌，再从抽牌堆顶部补足本回合最多 5 张抽入牌；弃牌堆本轮只用于找回核心牌，不做洗牌回收。
- First kernel 现在使用 `RandomSeed + FRandomStream` 做可复现随机落区：保留手牌和本回合抽入的普通牌会被随机分配到左手区 / 双手区 / 右手区，再重建手牌顺序供 snapshot 投影。
- `FirstBattle Zone Condition Rules v0.1` 已建立：First 卡牌实例可声明 `RequiredHandZone`，`PlayCard` 会在移除手牌前按当前 `HandZone` 校验区域出牌限制。
- 区域限制失败会稳定拒绝命令并返回 `first.command.rejected.hand_zone_requirement_not_met`，不会移动手牌、不会改 HP / 先机、不会追加事件。
- 卡牌可声明“指定区域 + 本次触发完美释放时跳过先机扣减”：该规则不同于 `First.Keyword.Swift`，仍会正常触发 `PerfectReleaseTriggered`，只跳过后续敌方部位先机扣减。
- `FirstBattle Hand Move v0.1` 已建立：First 卡牌 effect 新增 `MoveHandCard`，可从当前手牌中随机选择普通牌并腾挪到左手区、双手区或右手区的随机位置。
- 腾挪首版支持 `RandomValidZone / RandomOtherThanSourceZone / FixedZone` 三种目标策略，可选限制来源区域；左手 / 右手核心锚点不会被移动，缺少有效目标区域时效果 no-op 但出牌仍成功。
- 腾挪后会记录 `HandCardMoved` 事件，并由 snapshot 重新投影 `HandIndex / HandZone`；本轮不实现 Cost 转移、连击自移动、DataAsset 或 UI。
- `FirstBattle Draw Loop v0.2` 已建立：抽牌堆耗尽且本次仍需要抽牌时，First kernel 会使用 `RandomStream` 将弃牌堆洗入抽牌堆，并继续补抽本回合最多 5 张。
- 回合开始、抽牌和洗牌现在有正式事件可见性：`PlayerTurnStarted / CardDrawn / DrawPileShuffled`；`CardDrawn` 会记录强制核心牌、普通抽牌和洗后抽牌等来源。
- 本轮仍不引入手牌上限：保留手牌不会被弃掉，本回合抽入最多 5 张可以和原手牌共存；手牌容量规则留后续单独处理。
