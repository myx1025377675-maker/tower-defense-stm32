#ifndef E08_LAB_H
#define E08_LAB_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void E08_Init(void);

/** 绘制背景 + 信息卡片与文字（仅调用一次）。 */
void E08_DrawInformationStatic(void);

/** 下半屏装饰动画（周期性调用）。tick_ms 单调递增毫秒时间戳。 */
void E08_DecorAnimationTick(uint32_t tick_ms);

void E08_DisplayLxyInformation(void);
void E08_RunAll(void);
void E08_RunLyrics(void);

#ifdef __cplusplus
}
#endif

#endif
