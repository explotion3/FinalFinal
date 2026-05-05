# Development Backlog

本文件只记录尚未完成的开发任务。每条任务尽量保持为一段，方便完成后整段移出，并在 `Implementation_Progress.md` 记录完成事实。临时新想法先放入本文件，不直接打断当前主线；每轮开始前优先从 P0 / P1 中选择。

## P0

**Battle HUD 敌人信息正式化收口**：系统为 `FinalApp / UI / World`。敌人 OverHead 和敌人详情已经有 C++ ViewData 与 WBP 父类，但旧 `EnemyPanel` 仍承担目标选择和 fallback 信息展示；后续应明确场中 OverHead 显示短信息、详情面板显示完整信息、旧 EnemyPanel 逐步降级为临时目标列表，避免三套敌人信息重复。

**状态 / 被动 / 遗物正式 HUD 可见性**：系统为 `FinalApp / UI`。状态、被动、遗物已经有 snapshot/debug/event 可见性，但正式 HUD 还没有稳定展示规范；后续需要先统一状态行 / 状态图标 / tooltip 的基础数据与显示层级，再决定哪些信息常驻、哪些进入详情、哪些只留 Debug。

## P1

**BattleTeamPanel WBP 视觉与状态图标**：系统为 `FinalApp / UI / Content`。队伍整体面板的 C++ ViewData、Panel、角色简化 Entry 和状态详情骨架已经建立；后续需要制作 `WBP_BattleTeamPanel / WBP_BattleTeamCharacterEntry / WBP_BattleTeamStatusIcon / WBP_BattleTeamStatusDetailPanel`，补状态图标资源、归属角标和正式点击反馈。

**敌人意图正式视觉**：系统为 `FinalData / FinalBattle / FinalApp / UI`。当前敌人意图已有 DisplayName、Preview 文本和 icon key，但正式 HUD 还需要更清晰地展示意图类型、目标、伤害 / 护盾 / 状态预告和阶段信息；这项应在敌人 OverHead / Detail 的信息分工稳定后推进。

**Battle HUD Debug overlay 增强**：系统为 `FinalApp / UI`。默认 HUD 已从 Debug 信息中降噪，后续应把牌区明细、运行时状态、被动、遗物、事件账本、snapshot 摘要集中到 Debug overlay，方便开发验证。

**Run 外层流程页面可读化**：系统为 `FinalRun / FinalApp / UI`。Run 主流程已经能走通战后奖励、节点推进、事件、商店和成长选择，但页面仍偏原型列表；后续应在 Battle HUD 主决策信息稳定后，统一整理 RunFlowOverlay、Reward、Event、Shop、Growth 的标题、候选卡片、反馈和关闭/恢复入口。

## P2

**主菜单 / 设置页基于 Core UI 建立**：系统为 `FinalApp / UI`。Core UI 可继续复用到主菜单、设置页、地图页等大类界面；后续需要按 `Screen / Panel / Entry` 的轻量结构建立非战斗 UI，不复制 Battle HUD 的重型 panel 三件套。

**卡牌大图预览与 Tooltip / Toast 正式化**：系统为 `FinalApp / UI`。当前卡牌可在手牌中显示并点击，但还没有稳定的大图预览、关键词 tooltip、状态 tooltip 和战斗 toast 规范；这类表现等核心 HUD 信息层级稳定后再做。

**UI Class Set / Theme 配置收口**：系统为 `FinalApp / UI / Content`。当前 `FinalUIWidgetClassSettings` 作为开发期默认 Widget Class 注册表继续使用；等 Battle HUD、敌人详情、角色详情、牌区详情和 Tooltip 稳定后，应新增 `UI Class Set` 或等价 DataAsset，把一套主题 / 模式内的具体 WBP class 从全局 settings 中迁出。`FinalUIWidgetClassSettings` 长期只保留 `RootLayoutClass`、默认 screen class 和少量全局入口，不继续承载布局、颜色、图标、动画或 Tooltip 样式。

**UI 资产清理**：系统为 `FinalApp / Content`。当前 `Content/UI/BattleHUD` 中存在旧版 WBP、重复贴图和导入源文件；后续应先用 Reference Viewer / Asset Audit 确认引用，再移动到 Deprecated 或删除，避免误删仍被 WBP 使用的资源。
