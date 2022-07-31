/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "opentx.h"

#if defined(SPLASH)

#if defined(PCBPL18)
  const uint8_t __bmp_splash[] {
    #include "splash_480x320.lbm"
  };

#else
  const uint8_t __bmp_splash[] {
    #include "splash.lbm"
  };
#endif

void draw_splash_cb(lv_event_t * e)
{
  auto draw_ctx = lv_event_get_draw_ctx(e);
  auto splashImg = (BitmapBuffer*)lv_event_get_user_data(e);

  if (splashImg) {
    lv_draw_img_dsc_t img_dsc;
    lv_draw_img_dsc_init(&img_dsc);

    // TODO: center image
    lv_area_t coords;
    coords.x1 = 0;
    coords.y1 = 0;
    coords.x2 = splashImg->width()-1;
    coords.y2 = splashImg->height()-1;

    lv_draw_img_decoded(draw_ctx, &img_dsc, &coords,
                        (const uint8_t *)splashImg->getData(),
                        LV_IMG_CF_TRUE_COLOR);
  }
}

void drawSplash()
{
  constexpr LcdFlags splash_background_color =
    COLOR2FLAGS(((0xC >> 3) << 11) | ((0x3F >> 2) << 5) | (0x66 >> 3));

  static bool loadSplashImg = true;
  static BitmapBuffer* splashImg = nullptr;
  static lv_obj_t* splashScreen = nullptr;

  // try splash from SD card first
  if (loadSplashImg && splashImg == nullptr) {
    if (!sdMounted()) sdInit();
    splashImg = BitmapBuffer::loadBitmap(BITMAPS_PATH "/" SPLASH_FILE);
    loadSplashImg = false;

    if (splashImg == nullptr) {
      splashImg = new LZ4Bitmap(BMP_RGB565, __bmp_splash);
    }

    splashScreen = window_create(nullptr);
    if (splashScreen) {
      lv_obj_add_event_cb(splashScreen, draw_splash_cb, LV_EVENT_DRAW_MAIN, splashImg);
    }
  }

  if (splashScreen) {
    lv_scr_load(splashScreen);
    lv_refr_now(nullptr);
  }

  MainWindow::instance()->setActiveScreen();
}
#endif
