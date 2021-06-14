/* Copyright 2019 Thomas Baart <thomas@splitkb.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include QMK_KEYBOARD_H

#include "sevanteri.h"
#include "wrappers.h"
#include "keymap_finnish.h"

#ifdef PIMORONI_TRACKBALL_ENABLE
#include "pointing_device.h"
#include "pimoroni_trackball.h"
# if defined(RAW_ENABLE)
# include "raw_hid.h"
# include "usb_descriptor.h"
# endif
#endif


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT_stack_wrapper( // {{{

        _QWERTY_L1_______________,
        _QWERTY_L2_______________,
        _QWERTY_L3_______________, MO(_SYMB), MO(_FUNC),
        KC_BTN2, MY_THL4, MY_THL3, MY_THL2,   MY_THL1, 

        /******/ /******/ _______________QWERTY_R1_,
        /******/ /******/ _______________QWERTY_R2_,
        KC_SPC,  XXXXXXX, _______________QWERTY_R3_,
        MY_THR1, MY_THR2, MY_THR3, MY_THR4, KC_PSCR
    ), // }}}
    [_FUNC] = LAYOUT_stack_wrapper( // {{{
        MO(_STUF),  _________________FUNCL_L1__________________,
        _______, _________________FUNCL_L2__________________,
        _______, _________________FUNCL_L3__________________, _______, _______,
                                   _______, _______, ___FUNC_THUMBLN_,

                          _________________FUNCL_R1__________________, _______,
                          _________________FUNCL_R2__________________, _______,
        _______, _______, _________________FUNCL_R3__________________, _______,
        _______, _______, _______, _______, MO(_STUF)
    ), // }}}
    [_SYMB] = LAYOUT_stack_wrapper( // {{{
        _SYMB_L1_________________,
        _SYMB_L2_________________,
        _SYMB_L3_________________, _______, _______,
                                   _______, _______, ___SYMB_THUMBLN_,

                          _________________SYMB_R1_,
                          _________________SYMB_R2_,
        _______, _______, _________________SYMB_R3_,
        _______, _______, _______, _______, _______
    ), // }}}
    [_STUF] = LAYOUT_stack_wrapper( // {{{
        MO(_STUF), _________________STUFF_L1__________________,
        _______, _________________STUFF_L2__________________,
        _______, _________________STUFF_L3__________________, ________________,
                                   ___________________________________________,

                          _________________STUFF_L1__________________, _______,
                          _________________STUFF_L2__________________, _______,
        _______, _______, _________________STUFF_L3__________________, _______,
        __________________________________, MO(_STUF)
    ), // }}}
    [_MOUS] = LAYOUT_stack_wrapper( // {{{
        _______, ___________________________________________,
        _______, _______, _______, _______, KC_BTN1, KC_BTN3,
        _______, _______, _______, _______, KC_BTN2, _______, _______, _______,
                                   _______, _______, _______, _______, _______,

                          ___________________________________________, _______,
                          ___________________________________________, _______,
        _______, _______, ___________________________________________, _______,
        _______, _______, _______, _______, _______
    ), // }}}
    /* [_LAYERINDEX] = LAYOUT_stack_wrapper( // {{{ */
    /*     _______, ___________________________________________, */
    /*     _______, ___________________________________________, */
    /*     _______, ___________________________________________, _______, _______, */
    /*                                _______, _______, _______, _______, _______, */

    /*                       ___________________________________________, _______, */
    /*                       ___________________________________________, _______, */
    /*     _______, _______, ___________________________________________, _______, */
    /*     _______, _______, _______, _______, _______ */
    /* ), // }}} */
};


#ifdef PIMORONI_TRACKBALL_ENABLE
// {{{
#include "timer.h"
#include "quantum/quantum.h"
#include "i2c_master.h"

#ifndef TRACKBALL_NO_MATH
#include "math.h"
#endif

static int16_t mouse_auto_layer_timer = 0;
#define MOUSE_TIMEOUT 600
#define TRACKBALL_TIMEOUT 5

#define SIGN(x) ((x > 0) - (x < 0))

static bool rgb_changed = false;
static uint8_t rgb_hue = 1;
static uint8_t rgb_brightness = 1;
static uint8_t rgb_sat = 255;

void keyboard_post_init_user(void) {
    rgb_hue = rgblight_get_hue();
    rgb_brightness = rgblight_get_val();
    rgb_sat = rgblight_get_sat();
    trackball_set_hsv(rgb_hue, rgb_sat, rgb_brightness);
}

void eeconfig_init_user(void) {
  rgblight_sethsv(0, 255, 5);
}

void matrix_init_user() {
    trackball_init();
}

void suspend_power_down_user(void) {
    trackball_set_brightness(0);
    trackball_sleep();
}

void update_member(int8_t* member, int16_t* offset) {//{{{
    if (*offset > 127) {
        *member = 127;
        *offset -= 127;
    } else if (*offset < -127) {
        *member = -127;
        *offset += 127;
    } else {
        *member = *offset;
        *offset = 0;
    }
}//}}}

static int16_t x_offset = 0;
static int16_t y_offset = 0;
static int16_t v_offset = 0;
static int16_t h_offset = 0;
static int16_t tb_timer = 0;
void pointing_device_task() {
    report_mouse_t mouse = pointing_device_get_report();

    if (trackball_get_interrupt() && (!tb_timer || timer_elapsed(tb_timer) > TRACKBALL_TIMEOUT)) {
        tb_timer = timer_read() | 1;

        trackball_state_t state = trackball_get_state();

        __attribute__((unused))
        uint8_t mods = 0;
        if (state.x || state.y) {
            trigger_tapping();
            mods = get_mods();
        }

        if (state.button_triggered) {
            if(state.button_down) {
                mouse.buttons |= MOUSE_BTN1;
            } else {
                mouse.buttons &= ~MOUSE_BTN1;
            }
            pointing_device_set_report(mouse);
            pointing_device_send();
        } else {

            if (layer_state_is(_STUF)) {

                if (mods & MOD_MASK_SHIFT) {
                    int16_t new_sat = rgb_sat + state.x * 2;
                    if (new_sat > 100 && new_sat <= 255) {
                        rgb_sat = new_sat;
                        rgb_changed = true;
                        rgblight_sethsv_noeeprom(rgb_hue, rgb_sat, rgb_brightness);
                        trackball_set_hsv(rgb_hue, rgb_sat, rgb_brightness);
                    }
                } else {
                    rgb_hue += state.x % 256;
                    int16_t new_brightness = rgb_brightness + state.y * 2;
                    if (new_brightness < RGBLIGHT_LIMIT_VAL && new_brightness >= 0) {
                        rgb_brightness = new_brightness;
                    }
                    rgb_changed = true;
                    rgblight_sethsv_noeeprom(rgb_hue, rgb_sat, rgb_brightness);
                    trackball_set_hsv(rgb_hue, rgb_sat, rgb_brightness);
                }

            } else if (layer_state_is(_FUNC)) {
                h_offset += state.x;
                v_offset -= state.y;

#if defined(RAW_ENABLE)
            } else if (layer_state_is(SYMB)) {
                // set volume
                uint8_t msg[RAW_EPSIZE];
                sprintf((char *)msg, "VOL:%3d ", state.y);
                raw_hid_send(msg, RAW_EPSIZE);
#endif

            } else if (state.x || state.y) {

                if (!mouse_auto_layer_timer && !layer_state_is(_FUNC)) {
                    layer_on(_MOUS);
                }
                mouse_auto_layer_timer = timer_read() | 1;
#ifndef TRACKBALL_NO_MATH
                float power = 3;
                if (mods & MOD_MASK_CTRL) power = 1.8;
                /* else if (mods & MOD_MASK_ALT) power = 3.2; */

                double newlen = pow(state.vector_length, power);

                x_offset += (newlen * cos(state.angle_rad));
                y_offset += (newlen * sin(state.angle_rad));
#else
                uint8_t scale = 3;
                if (mods & MOD_MASK_CTRL) scale = 2;
                x_offset += state.x * state.x * SIGN(state.x) * scale;
                y_offset += state.y * state.y * SIGN(state.y) * scale;
#endif

            }
        }

        /* xprintf("%d, %d, %d, %d, %d\n", */
        /*         s[0], s[1], */
        /*         s[2], s[3], */
        /*         s[4]); */

    }

    while (x_offset || y_offset || h_offset || v_offset) {
        update_member(&mouse.x, &x_offset);
        update_member(&mouse.y, &y_offset);

        update_member(&mouse.v, &v_offset);
        update_member(&mouse.h, &h_offset);

        pointing_device_set_report(mouse);
        pointing_device_send();
    }
}

void matrix_scan_user(void) {//{{{
    if (mouse_auto_layer_timer && timer_elapsed(mouse_auto_layer_timer) > MOUSE_TIMEOUT) {
        report_mouse_t rep = pointing_device_get_report();
        if (rep.buttons) { return; }
        layer_off(_MOUS);
        mouse_auto_layer_timer = 0;
    }
}//}}}

bool process_record_keymap(uint16_t keycode, keyrecord_t *record) {//{{{
    if ((keycode < KC_BTN1 || keycode > KC_BTN5)
            && layer_state_is(_MOUS)
            && record->event.pressed) {
        layer_off(_MOUS);
        mouse_auto_layer_timer = 0;
    }
    return true;
}//}}}

static uint8_t last_layer = _BASE;
layer_state_t layer_state_set_user(layer_state_t state) {//{{{
    uint8_t layer = get_highest_layer(state);
    switch(layer) {
        case _SYMB:
            trackball_set_rgbw(0, 0, rgb_brightness, 0);
            // rgblight_setrgb(0, 0, rgb_brightness);
            break;
        case _FUNC:
            trackball_set_rgbw(0, rgb_brightness, 0, 0);
            // rgblight_setrgb(0, rgb_brightness, 0);
            break;
        case _STUF:
            trackball_set_rgbw(rgb_brightness, 0, rgb_brightness, 0);
            // rgblight_setrgb(rgb_brightness, 0, rgb_brightness);
            break;
        case _MOUS:
            trackball_set_rgbw(0, rgb_brightness, rgb_brightness, 0);
            // rgblight_setrgb(0, rgb_brightness, rgb_brightness);
            break;
        default:
            trackball_set_hsv(rgb_hue, rgb_sat, rgb_brightness);
            // rgblight_sethsv_noeeprom(rgb_hue, rgb_sat, rgb_brightness);
            break;
    }

    // if layer *was* STUF, save eeprom
    if (
            last_layer == _STUF
            && last_layer != layer
       ) {

        if (rgb_changed) {
            rgblight_sethsv(rgb_hue, rgb_sat, rgb_brightness);
            rgb_changed = false;
        }

    }

    last_layer = layer;
    return state;
}//}}}

//}}}
#endif
