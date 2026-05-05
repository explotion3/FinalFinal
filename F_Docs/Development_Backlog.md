# Development Backlog

本文件只记录尚未完成的开发任务。每条任务尽量保持为一段，方便完成后整段移出，并在 `Implementation_Progress.md` 记录完成事实。临时新想法先放入本文件，不直接打断当前主线；每轮开始前优先从 P0 / P1 中选择。

## P0

**Run 外层流程页面可读化**：系统为 `FinalRun / FinalApp / UI`。Run 主流程已经能走通战后奖励、节点推进、事件、商店和成长选择，但页面仍偏原型列表；后续应统一整理 RunFlowOverlay、Reward、Event、Shop、Growth 的标题、候选卡片、反馈、关闭/恢复入口，让首章竖切从战斗结束到下一节点都可读。

**拖卡到场中目标 v0.1**：系统为 `FinalApp / World / UI / FinalBattle command bridge`。场中点击敌人选择目标已经由 `BattleTargetInteractor` 接管；下一步可在手牌 Card Entry 拖拽开始 / 移动 / 松手时复用同一套 `TargetHitBox` trace，完成拖卡悬停目标反馈与松手出牌请求。

## P1

**敌人意图正式视觉**：系统为 `FinalData / FinalBattle / FinalApp / UI`。当前敌人意图已有 DisplayName、Preview 文本和 icon key，但正式 HUD 还需要清晰展示意图类型、目标、伤害 / 护盾 / 状态预告和阶段信息；这项应在敌人 OverHead / Detail 的信息分工稳定后推进。

**状态 / 被动 / 遗物正式 HUD 可见性**：系统为 `FinalApp / UI`。状态、被动、遗物已经有 snapshot/debug/event 可见性，但正式 HUD 还没有统一解释层；后续可从状态行 / 状态图标 / tooltip 的基础数据与显示层级开始，再决定哪些信息常驻、哪些进入详情、哪些只留 Debug。

**Battle HUD Debug overlay 增强**：系统为 `FinalApp / UI`。默认 HUD 已从 Debug 信息中降噪，后续应把牌区明细、运行时状态、被动、遗物、事件账本、snapshot 摘要集中到 Debug overlay，方便开发验证，不再依赖正式 HUD 的临时文本区。

**Card Zone Detail WBP 视觉收口**：系统为 `FinalApp / UI / Content`。牌区详情已有 snapshot 明细、UI 查看状态和 C++ fallback Tab 面板；后续需要制作正式 `WBP_BattleCardZoneDetailPanel / WBP_BattleCardZoneEntry`，让抽牌堆、弃牌堆、持续区、消耗区的只读查看成为正式 HUD 能力。

**角色 / 队伍状态图标资源与归属表现**：系统为 `FinalApp / UI / Content`。TeamPanel、角色详情、队伍状态详情已经能显示状态数据；后续需要状态图标资源、owner 角标、正负面色彩和简短标签规范，但暂不做复杂 tooltip 也不改变状态规则。

## P2

**主菜单 / 设置页基于 Core UI 建立**：系统为 `FinalApp / UI`。Core UI 可继续复用到主菜单、设置页、地图页等大类界面；后续需要按 `Screen / Panel / Entry` 的轻量结构建立非战斗 UI，不复制 Battle HUD 的重型 panel 三件套。

**卡牌大图预览与 Tooltip / Toast 正式化**：系统为 `FinalApp / UI`。当前卡牌可在手牌中显示并点击，但还没有稳定的大图预览、关键词 tooltip、状态 tooltip 和战斗 toast 规范；这类表现等核心 HUD 信息层级稳定后再做。

**UI Class Set / Theme 配置收口**：系统为 `FinalApp / UI / Content`。当前 `FinalUIWidgetClassSettings` 作为开发期默认 Widget Class 注册表继续使用；等 Battle HUD、敌人详情、角色详情、牌区详情和 Tooltip 稳定后，应新增 `UI Class Set` 或等价 DataAsset，把一套主题 / 模式内的具体 WBP class 从全局 settings 中迁出。`FinalUIWidgetClassSettings` 长期只保留 `RootLayoutClass`、默认 screen class 和少量全局入口，不继续承载布局、颜色、图标、动画或 Tooltip 样式。

**UI 资产清理**：系统为 `FinalApp / Content`。当前 `Content/UI/BattleHUD` 中存在旧版 WBP、重复贴图和导入源文件；后续应先用 Reference Viewer / Asset Audit 确认引用，再移动到 Deprecated 或删除，避免误删仍被 WBP 使用的资源。
