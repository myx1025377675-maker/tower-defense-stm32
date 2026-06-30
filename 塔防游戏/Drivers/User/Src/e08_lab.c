                                                                                                                                                                                                              /***
 * E08：实验展示页 — 三段式：压缩双列中文资料｜单行放大英文歌词｜英文以下为和风极简意象动画（P5 斜线星芒 + 鬼灭水纹等）。
 ***/

#include "e08_lab.h"
#include "e08_fon_subset.h"
#include "lcd_rgb.h"
#include "lcd_fonts.h"
#include "main.h"
#include "core_cm7.h"

#include <math.h>
#include <string.h>

static uint16_t E08_g_card_bottom_y;

static uint16_t E08_ArgbToRgb565(uint32_t argb)
{
	uint8_t r = (uint8_t)((argb >> 16) & 0xFFU);
	uint8_t g = (uint8_t)((argb >> 8) & 0xFFU);
	uint8_t b = (uint8_t)(argb & 0xFFU);
	return (uint16_t)(((uint16_t)(r & 0xF8U) << 8) | ((uint16_t)(g & 0xFCU) << 3) | (uint16_t)(b >> 3));
}

static void E08_DrawCh32(uint16_t x, uint16_t y, const uint8_t *g, uint32_t fg, uint32_t bg)
{
	uint16_t Xaddress = x;
	uint16_t yy      = y;
	uint16_t idx;
	uint32_t fg5 = E08_ArgbToRgb565(fg);
	uint32_t bg5 = E08_ArgbToRgb565(bg);

	for (idx = 0; idx < 128U; idx++) {
		uint8_t  charData = g[idx];
		uint16_t counter;

		for (counter = 0; counter < 8U; counter++) {
			LCD_DrawPoint(Xaddress, yy, (charData & 0x01U) ? fg5 : bg5);
			charData >>= 1;
			Xaddress++;

			if ((uint16_t)(Xaddress - x) == 32U) {
				Xaddress = x;
				yy++;
				break;
			}
		}
	}
}

static void E08_DrawLine32(uint16_t x, uint16_t y, const uint16_t *codes, uint16_t count, uint32_t fg, uint32_t bg)
{
	uint16_t i;
	uint16_t cx = x;

	for (i = 0; i < count; i++) {
		const uint8_t *bm = E08_FindGlyph32(codes[i]);

		if (bm != 0) {
			E08_DrawCh32(cx, y, bm, fg, bg);
		}
		cx = (uint16_t)(cx + 32U);
	}
}

/* advance ≥32：略大于字宽，模拟参考图汉字区的疏朗字距（嵌入式仅有 GB2312 点阵字模，无法换成真正的宋体文件）。 */
static void E08_DrawLine32Tracked(uint16_t x, uint16_t y, const uint16_t *codes, uint16_t count, uint16_t advance,
				  uint32_t fg, uint32_t bg)
{
	uint16_t i;
	uint16_t cx = x;

	for (i = 0; i < count; i++) {
		const uint8_t *bm = E08_FindGlyph32(codes[i]);

		if (bm != 0) {
			E08_DrawCh32(cx, y, bm, fg, bg);
		}
		cx = (uint16_t)(cx + advance);
	}
}

/* 简约和风夜景基调：深藏青 → 墨紫黑（单次刷屏即可，动画带仅铺同色加深沉浸）。 */
static void E08_DrawMinimalAnimeBackdrop(uint16_t y_top, uint16_t y_bottom)
{
	uint16_t y;
	uint16_t span = (uint16_t)(y_bottom - y_top);

	if (span == 0U) {
		return;
	}

	for (y = y_top; y < y_bottom; y += 2U) {
		uint32_t u = ((uint32_t)(y - y_top) * 255U) / (uint32_t)span;
		uint32_t r = 14U + (u * 12U) / 255U;
		uint32_t g = 12U + (u * 18U) / 255U;
		uint32_t b = 32U + (u * 28U) / 255U;

		LCD_SetColor(0xFF000000U | (r << 16) | (g << 8) | b);
		LCD_FillRect(0, y, LCD_Width, 2U);
	}
}

static void E08_DecorClearBand(uint16_t y0, uint16_t y1)
{
	LCD_SetColor(0xFF100E1AU);
	LCD_FillRect(0, y0, LCD_Width, (uint16_t)(y1 - y0 + 1U));
}

static void E08_DecorHorizDots(uint16_t x0, uint16_t x1, uint16_t y0, uint16_t y1, uint32_t t)
{
	uint16_t yy;

	for (yy = y0; yy <= y1; yy += 26U) {
		uint16_t xx;
		uint16_t phase = (uint16_t)((yy / 26U + (t / 70U)) & 3U);

		for (xx = x0; xx <= x1; xx += 16U) {
			if ((((xx / 16U) + phase) & 1U) != 0U) {
				continue;
			}
			LCD_DrawPoint(xx, yy, E08_ArgbToRgb565(0x5598A8D8U));
		}
	}
}

static void E08_PersonaBurstAt(uint16_t cx, uint16_t cy, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t t, float radius, uint16_t rays)
{
	float ph = (float)t * 0.0023f;
	uint16_t i;

	LCD_SetColor(0xFFFFFFFFU);

	for (i = 0; i < rays; i++) {
		float a = ph + (float)i * (6.2831853f / (float)rays);
		int32_t xe = (int32_t)cx + (int32_t)(cosf(a) * radius);
		int32_t ye = (int32_t)cy + (int32_t)(sinf(a) * radius);

		if (xe < (int32_t)x0) {
			xe = x0;
		}
		if (xe > (int32_t)x1) {
			xe = x1;
		}
		if (ye < (int32_t)y0) {
			ye = y0;
		}
		if (ye > (int32_t)y1) {
			ye = y1;
		}

		LCD_DrawLine(cx, cy, (uint16_t)xe, (uint16_t)ye);
	}
}

static void E08_PersonaScrollingDiagonals(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t t)
{
	int32_t w = (int32_t)(x1 - x0 + 1U);
	int32_t h = (int32_t)(y1 - y0 + 1U);
	int32_t off = (int32_t)(t % 120U);
	uint16_t k;

	LCD_SetColor(0x99FFFFFFU);

	for (k = 0; k < 32U; k++) {
		int32_t b = -h + (int32_t)(k * 38) + off * 3;

		LCD_DrawLine((uint16_t)x0, (uint16_t)(y0 + (uint16_t)b),
			     (uint16_t)(x0 + (uint16_t)w), (uint16_t)(y0 + (uint16_t)(b + w)));
	}
}

static void E08_WaterBreathingLayers(uint16_t x0, uint16_t x1, uint16_t y0, uint16_t y1, uint32_t t)
{
	uint16_t band = (uint16_t)((y1 - y0) / 5U);
	uint16_t layer;

	for (layer = 0; layer < 3U; layer++) {
		uint16_t yy = (uint16_t)(y0 + layer * band + band / 2U);
		uint16_t amp = (uint16_t)(14U - layer * 3U);
		int32_t step = (int32_t)(4 + layer);
		int32_t x;

		LCD_SetColor((layer == 0U) ? 0x6680C8E8U : (layer == 1U) ? 0x446090B8U : 0x33486898U);

		for (x = (int32_t)x0; x <= (int32_t)x1; x += step) {
			float xf = (float)x * 0.04f + (float)t * 0.0018f + (float)layer * 1.7f;
			int32_t oy = (int32_t)(sinf(xf) * (float)amp);
			uint16_t py = (uint16_t)((int32_t)yy + oy);

			if (py >= y0 && py <= y1) {
				LCD_DrawPoint((uint16_t)x, py, E08_ArgbToRgb565(0x8890C8F0U));
			}
		}
	}
}

static void E08_IchimatsuBottom(uint16_t y_base)
{
	uint16_t cell = 18U;
	uint16_t rows = 2U;
	uint16_t cols = (uint16_t)((LCD_Width + cell - 1U) / cell);
	uint16_t ix;
	uint16_t iy;

	for (iy = 0; iy < rows; iy++) {
		for (ix = 0; ix < cols; ix++) {
			uint16_t px = (uint16_t)(ix * cell);
			uint16_t py = (uint16_t)(y_base + iy * cell);
			uint8_t alt = (uint8_t)((ix + iy) & 1U);

			LCD_SetColor(alt ? 0xFF141828U : 0xFF1C2038U);
			LCD_FillRect(px, py, cell, cell);
		}
	}
}

typedef struct {
	uint16_t x;
	uint16_t y;
	uint16_t seed;
} E08_Leaf;

static void E08_LeavesTick(E08_Leaf *leaves, uint16_t n, uint16_t x0, uint16_t x1, uint16_t y0, uint16_t y1, uint32_t t)
{
	uint16_t i;
	uint16_t span = (uint16_t)(x1 - x0 + 1U);
	uint16_t band = (uint16_t)(y1 - y0 + 1U);

	for (i = 0; i < n; i++) {
		uint32_t s = (uint32_t)leaves[i].seed * 2654435761U ^ t;
		int32_t drift = (int32_t)((s % 80U) - 40);
		uint16_t nx = (uint16_t)(x0 + (uint16_t)((s * 17U + t * (3U + (i % 4U))) % span));
		uint16_t ny = (uint16_t)(y0 + (uint16_t)((s * 11U + (t * (18U + i)) / 8U) % band));
		int32_t tilt = (int32_t)((s % 60U) - 30) + drift / 4;

		nx = (uint16_t)((int32_t)nx + drift / 8);
		if (nx < x0) {
			nx = x0;
		}
		if (nx > x1) {
			nx = x1;
		}

		leaves[i].x = nx;
		leaves[i].y = ny;

		if ((leaves[i].seed & 3U) == 0U) {
			LCD_DrawPoint(nx, ny, E08_ArgbToRgb565(0xFFF8C8E8U));
			LCD_DrawPoint((uint16_t)(nx + 1U), ny, E08_ArgbToRgb565(0xFFE8A8D0U));
			LCD_SetColor(0x88F8E0F0U);
		} else {
			LCD_SetColor(0xFFFFA040U);
			LCD_DrawPoint(nx, ny, E08_ArgbToRgb565(0xFFFFC868U));
			LCD_DrawPoint((uint16_t)(nx + 1U), ny, E08_ArgbToRgb565(0xFFFFB050U));
			LCD_DrawPoint(nx, (uint16_t)(ny + 1U), E08_ArgbToRgb565(0xFFFF9030U));
			LCD_SetColor(0xEEFFD870U);
		}
		LCD_DrawLine(nx, ny, (uint16_t)((int32_t)nx + tilt / 4), (uint16_t)((int32_t)ny + 10));
		LCD_DrawLine((uint16_t)((int32_t)nx + tilt / 4), (uint16_t)((int32_t)ny + 10),
			     (uint16_t)((int32_t)nx + tilt / 6), (uint16_t)((int32_t)ny + 4));
	}
}

static void E08_DrawSceneDecorAnimated(uint16_t band_top, uint16_t band_bottom, uint32_t tick_ms)
{
	static E08_Leaf leaves[10];
	static uint8_t leaves_init;

	uint16_t x0 = 8U;
	uint16_t x1 = (uint16_t)(LCD_Width - 9U);
	uint16_t y0 = band_top;
	uint16_t y1 = band_bottom;

	uint16_t ich_y = (uint16_t)(LCD_Height - 32U);

	if (ich_y <= y1) {
		y1 = (uint16_t)(ich_y - 1U);
	}

	if (!leaves_init) {
		uint16_t i;

		for (i = 0; i < 10U; i++) {
			leaves[i].seed = (uint16_t)(i * 7919U + 1337U);
			leaves[i].x = (uint16_t)(40U + i * 63U);
			leaves[i].y = (uint16_t)(y0 + 12U + (i * 17U) % (uint16_t)((y1 > y0) ? (y1 - y0) : 1U));
		}

		leaves_init = 1U;
	}

	E08_DecorClearBand(y0, ich_y);

	E08_DecorHorizDots(x0, x1, y0, y1, tick_ms);
	E08_PersonaScrollingDiagonals(x0, y0, x1, y1, tick_ms);
	E08_WaterBreathingLayers(x0, x1, y0, y1, tick_ms);
	{
		uint16_t lx = (uint16_t)(x0 + (x1 - x0) / 6U);
		uint16_t rx = (uint16_t)(x1 - (x1 - x0) / 6U);
		uint16_t my_l = (uint16_t)(y0 + (uint16_t)((y1 - y0) / 3U));
		uint16_t my_r = (uint16_t)(y0 + (uint16_t)((y1 - y0) * 2U / 5U));

		E08_PersonaBurstAt(lx, my_l, x0, y0, x1, y1, tick_ms + 719U, 118.f, 42U);
		E08_PersonaBurstAt(rx, my_r, x0, y0, x1, y1, tick_ms, 108.f, 38U);
	}
	E08_LeavesTick(leaves, 10U, x0, x1, y0, y1, tick_ms);

	E08_IchimatsuBottom(ich_y);
}

/* 参考图：左列标签、右列内容，无冒号。ASCII 尾（学号）仅在右列绘制。 */
static void E08_DrawProfileRowTwoCol(uint16_t y, const uint16_t *lab, uint16_t lab_n, const uint16_t *zh_val,
				     uint16_t zh_n, const char *ascii_tail, uint16_t label_x, uint16_t value_x,
				     uint16_t adv_lab, uint16_t adv_val, uint32_t fg, uint32_t bg)
{
	E08_DrawLine32Tracked(label_x, y, lab, lab_n, adv_lab, fg, bg);
	if ((zh_val != NULL) && (zh_n > 0U)) {
		E08_DrawLine32Tracked(value_x, y, zh_val, zh_n, adv_val, fg, bg);
	}
	if ((ascii_tail != NULL) && (ascii_tail[0] != '\0')) {
		LCD_SetFont(&Font32);
		LCD_SetColor(fg);
		LCD_SetBackColor(bg);
		LCD_DisplayString(value_x, y, (char *)ascii_tail);
	}
}

void E08_DrawInformationStatic(void)
{
	static const uint16_t L_lab_school[] = { 0xD1A7, 0xD0A3 };			     /* 学校 */
	static const uint16_t L_lab_major[] = { 0xD7A8, 0xD2B5 };			     /* 专业 */
	static const uint16_t L_lab_grade[] = { 0xC4EA, 0xBCB6 };			     /* 年级 */
	static const uint16_t L_lab_name[] = { 0xD0D5, 0xC3FB };			     /* 姓名 */
	static const uint16_t L_lab_sid[] = { 0xD1A7, 0xBAC5 };			     /* 学号 */
	static const uint16_t L_univ[] = { 0xCFC3, 0xC3C5, 0xB4F3, 0xD1A7 };		     /* 厦门大学 */
	static const uint16_t L_major[] = {
		0xBCAF, 0xB3C9, 0xB5E7, 0xC2B7, 0xC9E8, 0xBCC6, 0xD3EB, 0xBCAF, 0xB3C9, 0xCFB5, 0xCDB3,
	};
	static const uint16_t L_grade[] = { 0xB4F3, 0xC8FD };				     /* 大三 */
	static const uint16_t L_name[] = { 0xC2ED, 0xD4BE, 0xCFE8 };			     /* 马跃翔 */

	const uint32_t panel_bg = 0xFF161428U;
	const uint32_t fg = 0xFFF2F6FFU;
	const uint32_t accent_red = 0xFFFF2238U;

	const uint16_t header_line_y = 36U;
	const uint16_t vl_x = 22U;
	const uint16_t vl_w = 2U;
	const uint16_t header_text_x = 22U;
	const uint16_t col_label_x = 40U;
	const uint16_t col_value_x = 152U;
	const uint16_t adv_label = 34U;
	const uint16_t adv_value = 34U;
	const uint16_t profile_y0 = 44U;
	const uint16_t row_step = 34U;
	uint16_t y;
	uint16_t divider_y;
	uint16_t quote_y;
	uint16_t lyric_x;
	size_t lyric_len;
	static const char lyric_line[] =
		"Blackpink in your area.";

	E08_DrawMinimalAnimeBackdrop(0U, LCD_Height);

	LCD_SetFont(&Font24);
	LCD_SetColor(fg);
	LCD_SetBackColor(panel_bg);
	LCD_DisplayString(header_text_x, 8U, (char *)"PROFILE / LAB E11");

	LCD_SetColor(accent_red);
	LCD_FillRect(0, header_line_y, LCD_Width, 2U);

	y = profile_y0;
	E08_DrawProfileRowTwoCol(y, L_lab_school, 2U, L_univ, 4U, NULL, col_label_x, col_value_x, adv_label,
				 adv_value, fg, panel_bg);
	y = (uint16_t)(y + row_step);
	E08_DrawProfileRowTwoCol(y, L_lab_major, 2U, L_major, 11U, NULL, col_label_x, col_value_x, adv_label,
				 adv_value, fg, panel_bg);
	y = (uint16_t)(y + row_step);
	E08_DrawProfileRowTwoCol(y, L_lab_grade, 2U, L_grade, 2U, NULL, col_label_x, col_value_x, adv_label,
				 adv_value, fg, panel_bg);
	y = (uint16_t)(y + row_step);
	E08_DrawProfileRowTwoCol(y, L_lab_name, 2U, L_name, 3U, NULL, col_label_x, col_value_x, adv_label,
				 adv_value, fg, panel_bg);
	y = (uint16_t)(y + row_step);
	E08_DrawProfileRowTwoCol(y, L_lab_sid, 2U, NULL, 0U, "37520232205011", col_label_x, col_value_x,
				 adv_label, adv_value, fg, panel_bg);

	divider_y = (uint16_t)(y + 32U + 8U);

	LCD_SetColor(accent_red);
	LCD_FillRect(vl_x, (uint16_t)(header_line_y + 2U), vl_w, (uint16_t)(divider_y - (header_line_y + 2U)));

	LCD_SetColor(fg);
	LCD_FillRect(0, divider_y, LCD_Width, 2U);

	quote_y = (uint16_t)(divider_y + 14U);
	LCD_SetFont(&Font24);
	LCD_SetColor(fg);
	LCD_SetBackColor(panel_bg);
	lyric_len = strlen(lyric_line);
	lyric_x = (uint16_t)((LCD_Width > lyric_len * (size_t)Font24.Width)
				     ? (uint16_t)((LCD_Width - (uint16_t)(lyric_len * (size_t)Font24.Width)) / 2U)
				     : 8U);
	LCD_DisplayString(lyric_x, quote_y, (char *)lyric_line);

	E08_g_card_bottom_y = (uint16_t)(quote_y + (uint16_t)Font24.Height + 12U);

#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
	SCB_CleanDCache();
	__DSB();
#endif
}

void E08_DecorAnimationTick(uint32_t tick_ms)
{
	uint16_t margin = 6U;
	uint16_t top = (uint16_t)(E08_g_card_bottom_y + margin);
	uint16_t bottom = (uint16_t)(LCD_Height - 28U);

	if (bottom > top) {
		E08_DrawSceneDecorAnimated(top, bottom, tick_ms);
	}

#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
	SCB_CleanDCache();
	__DSB();
#endif
}

void E08_Init(void)
{
}

void E08_DisplayLxyInformation(void)
{
	E08_DrawInformationStatic();
}

void E08_RunAll(void)
{
	E08_DisplayLxyInformation();
}

void E08_RunLyrics(void)
{
	E08_DisplayLxyInformation();
}
