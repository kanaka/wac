#include <stdint.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "util.h"

// emscripten/SDL/GL wrappers

uint32_t _env___emscripten_memcpy_big_(uint32_t dest, uint32_t src,
                                       uint32_t num) {
    FATAL("_emscripten_memcpy_big unimplemented\n");
    return dest;
}

uint32_t _env___emscripten_asm_const_i_(uint32_t code) {
    FATAL("_emscripten_asm_const_i(%d) unimplemented\n", code);
}

uint32_t _env___emscripten_asm_const_ii_(uint32_t a,
                                         uint32_t b) {
    FATAL("_emscripten_asm_const_ii unimplemented\n");
}

uint32_t _env___emscripten_asm_const_iii_(uint32_t code,
                                          uint32_t a0, uint32_t a1) {
    FATAL("_emscripten_asm_const_iii unimplemented\n");
}

uint32_t _env___emscripten_asm_const_iiii_(uint32_t a, uint32_t b,
                                           uint32_t c, uint32_t d) {
    FATAL("_emscripten_asm_const_iiii unimplemented\n");
}

uint32_t _env___emscripten_asm_const_iiiii_(uint32_t a, uint32_t b,
                                            uint32_t c, uint32_t d,
                                            uint32_t e) {
    FATAL("_emscripten_asm_const_iiiii unimplemented\n");
}

void _env___emscripten_asm_const_v_(uint32_t a) {
    FATAL("_emscripten_asm_const_v unimplemented\n");
}



// function type: (func (param i32 i32))
void _emscripten_set_canvas_size(uint32_t a, uint32_t b) {
    FATAL("_emscripten_set_canvas_size unimplemented");
}

// function type: (func (result i32))
uint32_t _emscripten_get_num_gamepads() {
    FATAL("_emscripten_get_num_gamepads unimplemented");
}

uint32_t _emscripten_exit_fullscreen() {
    FATAL("_emscripten_exit_fullscreen unimplemented");
}

uint32_t _emscripten_exit_pointerlock() {
    FATAL("_emscripten_exit_pointerlock unimplemented");
}


// function type: (result f64))
double _emscripten_get_device_pixel_ratio() {
    FATAL("_emscripten_get_device_pixel_ratio unimplemented");
}

// function type: (func (param i32) (result i32))
uint32_t _emscripten_get_pointerlock_status(uint32_t a) {
    FATAL("_emscripten_get_pointerlock_status unimplemented");
}

// function type: (func (param i32 i32) (result i32))
uint32_t _emscripten_get_gamepad_status(uint32_t a, uint32_t b) {
    FATAL("_emscripten_get_gamepad_status unimplemented");
}

uint32_t _emscripten_request_pointerlock(uint32_t a, uint32_t b) {
    FATAL("_emscripten_request_pointerlock unimplemented");
}

// function type: (func (param i32 f64 f64) (result i32))
uint32_t _emscripten_set_element_css_size(uint32_t a,
                                          double b, double c) {
    FATAL("_emscripten_set_element_css_size unimplemented");
}

// function type: (func (param i32 i32 i32) (result i32))
uint32_t _emscripten_set_gamepadconnected_callback(uint32_t a, uint32_t b,
						   uint32_t c) {
    FATAL("_emscripten_set_gamepadconnected_callback unimplemented");
}

uint32_t _emscripten_set_gamepaddisconnected_callback(uint32_t a, uint32_t b,
                                                      uint32_t c) {
    FATAL("_emscripten_set_gamepaddisconnected_callback unimplemented");
}

uint32_t _emscripten_get_element_css_size(uint32_t a, uint32_t b,
                                          uint32_t c) {
    FATAL("_emscripten_get_element_css_size unimplemented");
}

uint32_t _emscripten_set_visibilitychange_callback(uint32_t a, uint32_t b,
                                                   uint32_t c) {
    FATAL("_emscripten_set_visibilitychange_callback unimplemented");
}

uint32_t _emscripten_request_fullscreen_strategy(uint32_t a, uint32_t b,
                                                 uint32_t c) {
    FATAL("_emscripten_request_fullscreen_strategy unimplemented");
}

// function type: (func (param i32 i32 i32 i32) (result i32))
uint32_t _env___emscripten_set_mouseleave_callback_(uint32_t target,
                                                    uint32_t userData,
                                                    uint32_t useCapture,
                                                    uint32_t callbackfunc) {
    FATAL("_env___emscripten_set_mouseleave_callback unimplemented\n");
}


uint32_t _emscripten_set_keyup_callback(uint32_t a, uint32_t b,
					uint32_t c, uint32_t d) {
    FATAL("_emscripten_set_keyup_callback unimplemented");
}

uint32_t _env___emscripten_set_fullscreenchange_callback_(uint32_t target,
                                                          uint32_t userData,
                                                          uint32_t useCapture,
                                                          uint32_t callbackfunc) {
    FATAL("_env___emscripten_set_fullscreenchange_callback unimplemented\n");
}


uint32_t _emscripten_set_touchmove_callback(uint32_t a, uint32_t b,
					    uint32_t c, uint32_t d) {
    FATAL("_emscripten_set_touchmove_callback unimplemented");
}

uint32_t _emscripten_set_touchstart_callback(uint32_t a, uint32_t b,
					     uint32_t c, uint32_t d) {
    FATAL("_emscripten_set_touchstart_callback unimplemented");
}

uint32_t _emscripten_set_mousedown_callback(uint32_t a, uint32_t b,
					    uint32_t c, uint32_t d) {
    FATAL("_emscripten_set_mousedown_callback unimplemented");
}

uint32_t _emscripten_set_mouseup_callback(uint32_t a, uint32_t b,
					  uint32_t c, uint32_t d) {
    FATAL("_emscripten_set_mouseup_callback unimplemented");
}

uint32_t _emscripten_set_resize_callback(uint32_t a, uint32_t b,
					 uint32_t c, uint32_t d) {
    FATAL("_emscripten_set_resize_callback unimplemented");
}

uint32_t _emscripten_set_keypress_callback(uint32_t a, uint32_t b,
					   uint32_t c, uint32_t d) {
    FATAL("_emscripten_set_keypress_callback unimplemented");
}

uint32_t _emscripten_set_blur_callback(uint32_t a, uint32_t b,
				       uint32_t c, uint32_t d) {
    FATAL("_emscripten_set_blur_callback unimplemented");
}

uint32_t _emscripten_set_keydown_callback(uint32_t a, uint32_t b,
					  uint32_t c, uint32_t d) {
    FATAL("_emscripten_set_keydown_callback unimplemented");
}

uint32_t _emscripten_set_mousemove_callback(uint32_t a, uint32_t b,
					    uint32_t c, uint32_t d) {
    FATAL("_emscripten_set_mousemove_callback unimplemented");
}

uint32_t _emscripten_set_touchcancel_callback(uint32_t a, uint32_t b,
					      uint32_t c, uint32_t d) {
    FATAL("_emscripten_set_touchcancel_callback unimplemented");
}

uint32_t _emscripten_set_touchend_callback(uint32_t a, uint32_t b,
					   uint32_t c, uint32_t d) {
    FATAL("_emscripten_set_touchend_callback unimplemented");
}

uint32_t _emscripten_set_focus_callback(uint32_t a, uint32_t b,
					uint32_t c, uint32_t d) {
    FATAL("_emscripten_set_focus_callback unimplemented");
}

uint32_t _emscripten_set_mouseenter_callback(uint32_t a, uint32_t b,
					     uint32_t c, uint32_t d) {
    FATAL("_emscripten_set_mouseenter_callback unimplemented");
}

uint32_t _emscripten_set_wheel_callback(uint32_t a, uint32_t b,
					uint32_t c, uint32_t d) {
    FATAL("_emscripten_set_wheel_callback unimplemented");
}

// TODO: why do identical _glClear and _emscripten_glClear get generated?
void _env___glClear_(uint32_t x0) {
    glClear(x0);
    return;
}


