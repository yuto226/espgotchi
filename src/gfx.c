/*
 * MCUGotchi - A Tamagotchi P1 emulator for microcontrollers
 * Refactored to use u8g2 graphics library
 *
 * Copyright (C) 2021 Jean-Christophe Rona <jc@rona.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include <stdint.h>
#include <string.h>

#include "gfx.h"
#include "u8g2.h"

// u8g2のグローバルインスタンスへの参照
extern u8g2_t u8g2;

// u8g2フォントサイズマッピング
static const uint8_t *get_u8g2_font(uint8_t size) {
	switch(size) {
		case 0: return u8g2_font_5x7_tr;       // 5x7 (小)
		case 1: return u8g2_font_ncenB08_tr;   // 8pt (中)
		default: return u8g2_font_ncenB12_tr;  // 12pt (大)
	}
}

void gfx_pixel(uint8_t x, uint8_t y, color_t color)
{
	if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
		return;
	}

	if (color == COLOR_ON_BLACK) {
		u8g2_DrawPixel(&u8g2, x, y);
	} else {
		// u8g2では白ピクセルの個別設定は困難
		// 必要に応じてクリア領域として処理
	}
}

void gfx_square(uint8_t x, uint8_t y, uint8_t w, uint8_t h, color_t color)
{
	if (color == COLOR_ON_BLACK) {
		u8g2_DrawBox(&u8g2, x, y, w, h);
	} else {
		// 白の四角形 (クリア)
		u8g2_SetDrawColor(&u8g2, 0);
		u8g2_DrawBox(&u8g2, x, y, w, h);
		u8g2_SetDrawColor(&u8g2, 1);
	}
}

uint8_t gfx_char(unsigned char c, uint8_t x, uint8_t y, uint8_t size, color_t color, background_t bg)
{
	char str[2] = {c, '\0'};
	const uint8_t *font;
	uint8_t font_width;

	if (c < 0x20 || c > 0x7F) {
		return x;
	}

	// フォント選択
	font = get_u8g2_font(size);
	u8g2_SetFont(&u8g2, font);

	// 文字幅取得
	font_width = u8g2_GetStrWidth(&u8g2, str);

	// 背景処理
	if (bg == BACKGROUND_ON) {
		if (color == COLOR_ON_BLACK) {
			// 背景を白でクリア
			u8g2_SetDrawColor(&u8g2, 0);
			u8g2_DrawBox(&u8g2, x, y, font_width, 2);
			u8g2_SetDrawColor(&u8g2, 1);
		} else {
			// 背景を黒で描画
			u8g2_DrawBox(&u8g2, x, y, font_width, 2);
		}
	}

	// 文字色設定
	if (color == COLOR_OFF_WHITE) {
		u8g2_SetDrawColor(&u8g2, 0);
	} else {
		u8g2_SetDrawColor(&u8g2, 1);
	}

	// 文字描画
	u8g2_DrawStr(&u8g2, x, y + u8g2_GetFontDescent(&u8g2), str);

	// 描画色をデフォルトに戻す
	u8g2_SetDrawColor(&u8g2, 1);

	return x + font_width;
}

uint8_t gfx_string(char *str, uint8_t x, uint8_t y, uint8_t size, color_t color, background_t bg)
{
	const uint8_t *font;
	uint8_t str_width;

	// フォント選択
	font = get_u8g2_font(size);
	u8g2_SetFont(&u8g2, font);

	// 文字列幅取得
	str_width = u8g2_GetStrWidth(&u8g2, str);

	// 背景処理
	if (bg == BACKGROUND_ON) {
		if (color == COLOR_ON_BLACK) {
			// 背景を白でクリア
			u8g2_SetDrawColor(&u8g2, 0);
			u8g2_DrawBox(&u8g2, x, y, str_width, 2);
			u8g2_SetDrawColor(&u8g2, 1);
		} else {
			// 背景を黒で描画
			u8g2_DrawBox(&u8g2, x, y, str_width, 2);
		}
	}

	// 文字色設定
	if (color == COLOR_OFF_WHITE) {
		u8g2_SetDrawColor(&u8g2, 0);
	} else {
		u8g2_SetDrawColor(&u8g2, 1);
	}

	// 文字列描画
	u8g2_DrawStr(&u8g2, x, y + u8g2_GetFontDescent(&u8g2), str);

	// 描画色をデフォルトに戻す
	u8g2_SetDrawColor(&u8g2, 1);

	return x + str_width;
}

void gfx_clear(void)
{
	u8g2_ClearBuffer(&u8g2);
}

void gfx_register_display(void (*cb)(uint8_t *, uint16_t))
{
	// u8g2では不要（u8g2が直接ディスプレイを制御）
	// 互換性のため空実装
}

void gfx_print_screen(void)
{
	u8g2_SendBuffer(&u8g2);
}