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
- `FirstBattle Cost Transfer v0.1` 已建立：`MoveHandCard` 可在腾挪成功后修改被腾挪牌 `RuntimeCost`，并把实际降低的费用转移到本次打出的来源牌上。
- 费用转移只修改 `RuntimeCost`，不改 `BaseCost`；来源牌费用承接写回弃牌堆中的同一 `CardInstanceId`，因此后续回手后仍保留转移后的费用。
- 卡牌运行时费用变化现在通过 `CardRuntimeCostChanged` 事件可见，事件记录被改费牌以及改费前后的 `RuntimeCost`。
- `First Card Data Skeleton v0.1` 已建立：`FinalData` 模块新增 `UFirstCardDefinition`，用于 First 专用卡牌 authoring，不复用旧 `UFinalCardDefinition` 的 AP、三角色卡牌类型或旧 effect 对象。
- `UFirstCardDefinition` 首版覆盖当前 First kernel 已支持的最小字段：`CardId / DisplayName / BaseCost / Keywords / HandRole / 区域出牌限制 / 完美释放区域跳过先机 / Damage / MoveHandCard`。
- `FinalDataRegistry` 已支持 `RegisterFirstCardDefinition()` 与 `FindFirstCardDefinition(CardId)`，First 与 legacy 卡牌定义双轨索引。
- `FinalBattle` 已新增 `FFirstCardDefinitionCompiler`，可将 `UFirstCardDefinition` 编译为 `FFirstCardInstance`，其中 `RuntimeCost` 初始等于 `BaseCost`，后续战斗改费仍只作用于 runtime instance。
- `FinalEditor` Validator 已接入 First card 基础校验与 `CardId` 唯一性检查；本轮不生成 starter 资产，也不接 First session 初始化、Run、UI 或背包。
- `FirstBattle Session Data 接入 v0.1` 已建立：`FFirstBattleStartParams` 新增 `InitialHandCardDefinitions / InitialDrawPileCardDefinitions`，可用 `CardId + Count` 引用 First card definitions。
- `FFirstBattleSession::InitializeFromDefinitions()` 已接入 `UFinalDataRegistry + FFirstCardDefinitionCompiler`，会把 First card definitions 编译并追加到 runtime `InitialHand / InitialDrawPile` 后再初始化 kernel。
- FirstBattle 初始化现在对缺失 `CardId` 和非法 `Count` 做结构化失败返回；失败时不会覆盖当前 session runtime 状态。
- 现有低层 `Initialize(const FFirstBattleStartParams&)` 保持不变，仍可直接接收手写 `FFirstCardInstance`，供 kernel 单元测试和特殊 runtime 场景使用。
- `First Card Content Bootstrap v0.1` 已建立：`FinalPrototypeContentBootstrapCommandlet` 现在会通过 First 专用 builder 生成 `/Game/Prototype/FirstProject/Cards/` 下的首批 `UFirstCardDefinition` 资产。
- 首批 First 内容只覆盖当前 kernel 已支持的规则：左手、右手、朝光暮蝶、赤腹工蚁和烁光蝶；拂晓飞蛾、暮蛉、暮色引虫灯等仍等待手牌回收、状态/冻结、任务/耐久/容量 schema 后再 authoring。
- 新增 First 内容验证覆盖真实资产加载、DataRegistry 查询、compiler 字段保留，以及 `InitializeFromDefinitions()` 使用真实 First card assets 初始化并执行右手伤害链。
- `FirstBattle Card Entry Stats v0.1` 已建立：`UFirstCardDefinition / FFirstCardInstance / FFirstCardViewData` 新增 `PlayerMaxHPBonusOnEnterBattle`，用于表达卡牌入战时提高玩家生命上限。
- First kernel 初始化时会对 `InitialHand / InitialDrawPile` 中的入战生命加成统一结算一次：玩家 `PlayerMaxHP` 与 `PlayerCurrentHP` 同步增加，后续抽牌、回手、腾挪、出牌、弃牌和洗牌不会重复触发。
- 卡牌入战生命上限变化通过 `PlayerMaxHPChanged` 事件可见；首批 First 内容中，朝光暮蝶和赤腹工蚁提供 `+1` 生命上限，烁光蝶提供 `+6` 生命上限，左手 / 右手暂不提供该加成。
- `FirstBattle Combo / Self Move v0.1` 已建立：`UFirstCardDefinition / FFirstCardInstance / FFirstCardViewData` 新增 `PlayDestination`，首版支持 `DiscardPile` 与 `ReturnToHandRandomZone`。
- First kernel 出牌后会在完整结算结束时处理打出后去向；普通牌维持进入弃牌堆，`ReturnToHandRandomZone` 牌会从弃牌堆取回并插入随机有效手牌区域，缺少有效区域时安全回到手牌且区域为 `None`。
- 回手不会重复触发 `PlayerMaxHPBonusOnEnterBattle`，并通过 `CardReturnedToHand` 事件可见；首批 First 内容中，烁光蝶已设置为打出后回手随机落区，左手、右手、朝光暮蝶、赤腹工蚁仍进入弃牌堆。
