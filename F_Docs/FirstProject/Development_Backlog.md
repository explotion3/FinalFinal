# FirstProject Development Backlog

本文件只记录 First 新项目尚未完成的开发任务。旧三角色项目的待办继续保留在 `F_Docs/Development_Backlog.md`，不迁入本文件。每条任务尽量保持为一段，方便完成后移动到 `Implementation_Progress.md`。

## P0

FirstBattle EndTurn + 部位行动刷新：实现 `EndTurn` 和正式部位行动刷新，玩家主动结束回合时所有存活部位按顺序行动；本玩家回合中已经因先机归零行动过的部位，在结束回合时仍然会再次行动。补齐部位行动后的 intent 刷新与 initiative 重置，替换当前 `EnemyPartActed` 占位事件。

First Hand Zones：实现左手区 / 双手区 / 右手区，左手牌 / 右手牌作为可打出和可删除的核心卡牌，回合开始强制抽回缺失的左右手牌，再补抽到 5 张；打出任一手牌后双手区立即失效。

First Card Data：建立 First 卡牌 schema，明确 Cost、Runtime Cost、迅捷、完美释放、区域条件、腾挪、连击、耐久、容量、任务和升级等字段如何 authoring。

## P1

First Enemy Parts：建立多部位敌人 authoring 与 runtime，支持部位 HP、部位 intent、部位 initiative、部位破坏、敌人击倒事件、部位掉落和全部部位破坏胜利。

First Backpack / Run：建立背包、战斗带入、战斗结束回流、掉落永久获得、背包容量和自由探索 Run 方向；旧线性节点 Run 只作为工程参考，不作为新主线。

First App/UI：重做第一人称 HD-2D 战斗表现、手牌三区 UI、敌人部位 UI、部位意图和先机显示；UI 只消费 snapshot 和转发 command，不承载规则真相。

## P2

First 状态与关键词深化：正式化中毒、减速、暮气、冻结、连击、突袭等规则，并明确它们可挂载到玩家、卡牌、敌人整体、敌人部位或敌人意图的口径。

First 内容验证：用已记录的初始卡牌样例搭建第一批可测试内容，包括左手、右手、朝光暮蝶、拂晓飞蛾、赤腹工蚁、烁光蝶、暮蛉和暮色引虫灯。
