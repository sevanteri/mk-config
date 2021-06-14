#include QMK_KEYBOARD_H
#include "sevanteri.h"
#include "timer.h"

#include "keymap_finnish.h"

#include "casemodes.h"
#include "leader.h"

#ifdef PIMORONI_TRACKBALL_ENABLE
#include "pimoroni_trackball.h"
#endif
#ifdef POINTING_DEVICE_ENABLE
#include "pointing_device.h"
#endif



__attribute__((weak)) bool process_record_keymap(uint16_t keycode, keyrecord_t *record) { return true; }

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    const bool pressed = record->event.pressed;
    static uint16_t code_timer;
    switch (keycode) {
        case CODEBLK:
            if (pressed) {
                code_timer = timer_read();
            } else {
                if (timer_elapsed(code_timer) > TAPPING_TERM) {
                    SEND_STRING(
                        "````````````"
                        SS_TAP(X_LEFT)
                        SS_TAP(X_LEFT)
                        SS_TAP(X_LEFT)
                    );
                } else {
                    SEND_STRING("``" SS_TAP(X_LEFT) "``");
                }
            }
            return false;
#ifdef PIMORONI_TRACKBALL_ENABLE
        case RESET:
#    ifdef RGBLIGHT_ENABLE
            trackball_set_rgbw(rgblight_get_val(), 0, 0, 0);
#    else
            trackball_set_rgbw(42, 0, 0, 0);
#    endif
            return true;
#endif
        case MY_PARENSBOTH:
            if (!pressed) break;
            tap_code16(FI_LPRN);
            tap_code16(KC_ENT);
            tap_code16(KC_END);
            tap_code16(KC_ENT);
            tap_code16(FI_RPRN);
            tap_code16(KC_UP);
            tap_code16(KC_END);
            return false;
        case MY_SQUAREBRACEBOTH:
            if (!pressed) break;
            tap_code16(FI_LBRC);
            tap_code16(KC_ENT);
            tap_code16(KC_END);
            tap_code16(KC_ENT);
            tap_code16(FI_RBRC);
            tap_code16(KC_UP);
            tap_code16(KC_END);
            return false;
        case MY_CURLYBRACEBOTH:
            if (!pressed) break;
            tap_code16(FI_LCBR);
            tap_code16(KC_ENT);
            tap_code16(KC_END);
            tap_code16(KC_ENT);
            tap_code16(FI_RCBR);
            tap_code16(KC_UP);
            tap_code16(KC_END);
            return false;
        case WORDCAPS:
            if (pressed && !xcase_enabled()) {
                enable_xcase();
            } else if (!pressed) {
                if (xcase_waiting()) {
                    disable_xcase();
                    toggle_caps_word();
                }
            }
            return false;
        case LEADER:
            if (pressed) return false;
            if (is_leading()) {
                stop_leading();
            } else {
                start_leading();
            }
            return false;
    }

#ifdef POINTING_DEVICE_ENABLE
    report_mouse_t report = pointing_device_get_report();
    switch(keycode) {
        case KC_BTN1:
            if (pressed) report.buttons |= MOUSE_BTN1;
            else report.buttons &= ~MOUSE_BTN1;
            pointing_device_set_report(report);
            pointing_device_send();
            return false;
        case KC_BTN2:
            if (pressed) report.buttons |= MOUSE_BTN2;
            else report.buttons &= ~MOUSE_BTN2;
            pointing_device_set_report(report);
            pointing_device_send();
            return false;
        case KC_BTN3:
            if (pressed) report.buttons |= MOUSE_BTN3;
            else report.buttons &= ~MOUSE_BTN3;
            pointing_device_set_report(report);
            pointing_device_send();
            return false;
    }
#endif

    if (!(
        process_case_modes(keycode, record) &&
        process_leader(keycode, record) &&
        process_record_keymap(keycode, record) &&
        true))
    {
        return false;
    }

    return true;
}
