# 嵌入式塔防游戏 (Embedded Tower Defense)

> 基于 STM32H743 + FreeRTOS 的 LCD 触摸屏塔防游戏

## 项目简介

本项目在 STM32H743IIT6 微控制器上实现了一款完整的塔防游戏。游戏通过 800×480 RGB LCD 屏幕显示画面，支持电容触摸屏交互。玩家可以在地图上建造不同类型的防御塔（箭塔、炮塔、冰塔）来阻止敌人前进，包含 5 波敌人、金币系统、计分系统、防御塔升级/出售等完整游戏机制。

## 硬件平台

| 模块     | 型号/参数                  |
| -------- | -------------------------- |
| MCU      | STM32H743IIT6 (开发板: FK743M2-IIT6) |
| 显示屏   | 800×480 RGB 接口 LCD       |
| 触摸     | I2C 电容触摸屏 (GT9147)    |
| 存储     | QSPI W25Q64 Flash, SDRAM   |
| 调试     | J-Link / ST-Link           |

## 软件架构

```
┌──────────────────────────────────────────┐
│              游戏任务层                    │
│  game_tasks  ┌──────┬──────┬──────┐      │
│              │ Game │ Render│  UI  │      │
│              └──────┴──────┴──────┘      │
├──────────────────────────────────────────┤
│          游戏逻辑层                        │
│  game_state / game_enemy / game_tower    │
│  game_map / game_score / game_ui         │
├──────────────────────────────────────────┤
│          驱动抽象层                        │
│  lcd_rgb / lcd_pwm / touch_800x480       │
│  qspi_w25q64 / sdram / usart / led       │
├──────────────────────────────────────────┤
│          FreeRTOS (多任务调度)            │
├──────────────────────────────────────────┤
│          STM32H7xx HAL 库                 │
├──────────────────────────────────────────┤
│          CMSIS (ARM 内核支持)             │
└──────────────────────────────────────────┘
```

### 游戏系统

- **地图系统**: 16×10 网格，640×400 像素地图区域，支持路径编辑
- **防御塔**: 箭塔（高速单体）、炮塔（范围溅射）、冰塔（减速效果），可升级
- **敌人**: 普通怪、快速怪、坦克怪，5 波递增难度
- **经济系统**: 初始 200 金币，击杀获得奖励
- **生命系统**: 初始 20 生命值，漏怪扣除
- **UI 系统**: 主菜单、暂停、波次间隔、胜利/失败界面

## 目录结构

```
塔防游戏/
├── Core/                          # 主程序入口
│   ├── Inc/                       # main.h, FreeRTOSConfig.h, stm32h7xx_it.h
│   └── Src/                       # main.c, 中断服务, 系统初始化
├── Drivers/
│   ├── CMSIS/                     # ARM Cortex-M 核心支持文件
│   ├── STM32H7xx_HAL_Driver/      # STM32 HAL 库
│   └── User/                      # 用户代码
│       ├── Inc/                   # 头文件
│       │   ├── game_defs.h        # 游戏数据结构与常量定义
│       │   ├── game_*.h           # 各游戏子系统头文件
│       │   ├── lcd_*.h            # LCD 驱动头文件
│       │   └── touch_*.h          # 触摸驱动头文件
│       └── Src/                   # 源文件
│           ├── game_enemy.c       # 敌人逻辑
│           ├── game_tower.c       # 防御塔逻辑
│           ├── game_map.c         # 地图与寻路
│           ├── game_state.c       # 游戏主状态机
│           ├── game_render.c      # 渲染引擎
│           ├── game_ui.c          # UI 界面
│           ├── game_score.c       # 计分与存储
│           ├── game_tasks.c       # FreeRTOS 任务
│           └── lcd_*.c            # LCD/触摸驱动
├── Middlewares/
│   └── FreeRTOS/                  # FreeRTOS 实时操作系统
├── fonts/                         # GB2312 字库文件
├── tools/                         # Python 辅助脚本
│   ├── make_subset_from_text.py   # 从文本抽取字模
│   └── gen_e08_fon_*.py          # 字模生成工具
├── MDK-ARM/                       # Keil 工程目录
│   ├── FK743M2.uvprojx            # Keil 工程文件
│   ├── FK743M2.uvoptx             # Keil 工程选项
│   ├── startup_stm32h743xx.s      # 启动文件
│   └── DebugConfig/               # 调试配置
├── keilkilll.bat                   # 清理编译产物脚本
└── FONT_WORKFLOW.md               # 中文字库使用说明
```

## 开发环境

- **IDE**: Keil MDK-ARM 5.x
- **编译器**: ARM Compiler 5 / 6
- **RTOS**: FreeRTOS v10.x
- **下载调试**: J-Link / ST-Link

## 快速开始

1. 用 Keil MDK-ARM 打开 `MDK-ARM/FK743M2.uvprojx`
2. 选择目标配置 (FK743M1 或 FK743M2)
3. 编译工程 (`Project` → `Build Target` 或按 F7)
4. 通过 J-Link/ST-Link 下载到开发板
5. 上电自动运行，触摸屏操作

### 添加中文字符

```bash
python tools/make_subset_from_text.py --text "你要显示的中文"
```

详见 `FONT_WORKFLOW.md`。

## 游戏操作

- 点击地图空地 → 选择建造防御塔
- 点击已有防御塔 → 升级 / 出售
- 每一波敌人清完后进入波次间隔，可继续建造
- 全部 5 波通关即为胜利

## 截图

![游戏截图](屏幕截图%202026-05-26%20165049.png)

---

*嵌入式系统综合设计实验课程项目*
