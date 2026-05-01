# Development Backlog

本文件只记录尚未完成的开发任务。每条任务尽量保持为一段，方便完成后整段移出，并在 `Implementation_Progress.md` 记录完成事实。临时新想法先放入本文件，不直接打断当前主线；每轮开始前优先从 P0 / P1 中选择。

## P0

**Battle HUD 角色 / 敌人面板正式化**：系统为 `FinalApp / UI`。当前默认 HUD 骨架已收口，但角色面板和敌人面板仍偏 fallback 文本风格，下一步需要把角色 Entry 和敌人 Entry 的正式字段、显示层级、fallback 文案统一起来，只显示玩家战斗决策所需信息，不显示 raw debug dump。

**状态系统后续完善收口**：系统为 `FinalData / FinalBattle / FinalApp`。状态系统已经完成 RuntimeModifiers、ProjectedCardModifiers、资源型状态、AppliesTo、DOT 中毒和 legacy 字段清理等主干，但后续仍需要继续补状态显示、DOT 可见性、状态说明与测试覆盖，避免状态规则完成但玩家侧不可读。

## P1

**牌堆详情页**：系统为 `FinalApp / UI`。玩家后续需要点击抽牌堆、弃牌堆、持续区、消耗区等入口查看对应牌区详情；首版应只读展示牌名、所属角色、类型和运行时文本，不在 UI 中修改牌区真相。

**状态 / 被动 / 遗物正式 HUD 可见性**：系统为 `FinalApp / UI`。当前状态、被动、遗物已经有 snapshot/debug/event 可见性，但正式 HUD 还没有稳定的展示规范；后续需要确定哪些显示为状态栏、哪些走事件反馈、哪些只留 Debug。

**敌人意图正式视觉**：系统为 `FinalApp / UI`。当前敌人意图已有数据和文本，但正式 HUD 需要更清晰地展示意图类型、目标、伤害/护盾/状态预告和阶段信息，减少玩家读大段文本的负担。

**Battle HUD Debug overlay 增强**：系统为 `FinalApp / UI`。默认 HUD 已从 Debug 信息中降噪，后续应把牌区明细、运行时状态、被动、遗物、事件账本、snapshot 摘要集中到 Debug overlay，方便开发验证。

## P2

**主菜单 / 设置页基于 Core UI 建立**：系统为 `FinalApp / UI`。Core UI 可继续复用到主菜单、设置页、地图页等大类界面；后续需要按 `Screen / Panel / Entry` 的轻量结构建立非战斗 UI，不复制 Battle HUD 的重型 panel 三件套。

**UI 资产清理**：系统为 `FinalApp / Content`。当前 `Content/UI/BattleHUD` 中存在旧版 WBP、重复贴图和导入源文件；后续应先用 Reference Viewer / Asset Audit 确认引用，再移动到 Deprecated 或删除，避免误删仍被 WBP 使用的资源。

**卡牌大图预览与 Tooltip / Toast 正式化**：系统为 `FinalApp / UI`。当前卡牌可在手牌中显示并点击，但还没有稳定的大图预览、关键词 tooltip、状态 tooltip 和战斗 toast 规范；这类表现等核心 HUD 信息层级稳定后再做。
