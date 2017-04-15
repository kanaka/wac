#include <stdint.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "util.h"

// emscripten/SDL/GL wrappers

uint32_t _emscripten_memcpy_big(uint32_t dest, uint32_t src,
                                uint32_t num) {
    FATAL("_emscripten_memcpy_big unimplemented\n");
    return dest;
}

uint32_t _emscripten_asm_const_i(uint32_t code) {
    if (TRACE) { debug("_emscripten_asm_const_i(%d)\n", code); }
    switch (code) {
    case 0:  // TODO: screen.width hardcoded
        warn("_emscripten_asm_const_i(%d) hardcoded to 640\n", code);
        return 640;
    case 1:  // TODO: screen.height hardcoded
        warn("_emscripten_asm_const_i(%d) hardcoded to 480\n", code);
        return 480;
    default:
        FATAL("_emscripten_asm_const_i invalid code %d\n", code);
    }
}

uint32_t _emscripten_asm_const_ii(uint32_t code,
                                  uint32_t *a0) {
    if (TRACE) { debug("_emscripten_asm_const_ii(%d, %p)\n", code, a0); }
    switch (code) {
    case 4:  // TODO:
        warn("_emscripten_asm_const_ii(%d, %p) set cursor ignored\n",
             code, a0);
        return 0;
    default:
        FATAL("_emscripten_asm_const_ii invalid code %d\n", code);
    }
}

uint32_t _emscripten_asm_const_iii(uint32_t code,
                                   uint32_t a0, uint32_t a1) {
    FATAL("_emscripten_asm_const_iii unimplemented\n");
}

uint32_t _emscripten_asm_const_iiii(uint32_t a, uint32_t b,
                                    uint32_t c, uint32_t d) {
    FATAL("_emscripten_asm_const_iiii unimplemented\n");
}

uint32_t _emscripten_asm_const_iiiii(uint32_t a, uint32_t b,
                                     uint32_t c, uint32_t d,
                                     uint32_t e) {
    FATAL("_emscripten_asm_const_iiiii unimplemented\n");
}

void _emscripten_asm_const_v(uint32_t a) {
    FATAL("_emscripten_asm_const_v unimplemented\n");
}



// function type: (func (param i32 i32))
void _emscripten_set_canvas_size(uint32_t a, uint32_t b) {
    warn("_emscripten_set_canvas_size(%d, %d) ignored\n", a, b);
}

// function type: (func (result i32))
uint32_t _emscripten_get_num_gamepads() {
    FATAL("_emscripten_get_num_gamepads unimplemented\n");
}

uint32_t _emscripten_exit_fullscreen() {
    FATAL("_emscripten_exit_fullscreen unimplemented\n");
}

uint32_t _emscripten_exit_pointerlock() {
    FATAL("_emscripten_exit_pointerlock unimplemented\n");
}


// function type: (result f64))
double _emscripten_get_device_pixel_ratio() {
    FATAL("_emscripten_get_device_pixel_ratio unimplemented\n");
}

// function type: (func (param i32) (result i32))
uint32_t _emscripten_get_pointerlock_status(uint32_t a) {
    FATAL("_emscripten_get_pointerlock_status unimplemented\n");
}

// function type: (func (param i32 i32) (result i32))
uint32_t _emscripten_get_gamepad_status(uint32_t a, uint32_t b) {
    FATAL("_emscripten_get_gamepad_status unimplemented\n");
}

uint32_t _emscripten_request_pointerlock(uint32_t a, uint32_t b) {
    FATAL("_emscripten_request_pointerlock unimplemented\n");
}

// function type: (func (param i32 f64 f64) (result i32))
uint32_t _emscripten_set_element_css_size(char *target,
                                          double w, double h) {
    FATAL("_emscripten_set_element_css_size unimplemented\n");
}

// function type: (func (param i32 i32 i32) (result i32))
uint32_t _emscripten_set_gamepadconnected_callback(uint32_t a, uint32_t b,
						   uint32_t c) {
    FATAL("_emscripten_set_gamepadconnected_callback unimplemented\n");
}

uint32_t _emscripten_set_gamepaddisconnected_callback(uint32_t a, uint32_t b,
                                                      uint32_t c) {
    FATAL("_emscripten_set_gamepaddisconnected_callback unimplemented\n");
}

uint32_t _emscripten_get_element_css_size(char *target,
                                          double *w, double *h) {
    warn("_emscripten_get_element_css_size hardcoded to (640,480)\n");
    *w = 640;  // TODO: width hardcoded
    *h = 480;  // TODO: height hardcoded
    return 0;
}

// function type: (func (param i32 i32 i32 i32) (result i32))


uint32_t _emscripten_set_mousedown_callback(uint32_t a, uint32_t b,
					    uint32_t c, uint32_t d) {
    warn("_emscripten_set_mousedown_callback ignored\n");
    return 0; // TODO: implement
}

uint32_t _emscripten_set_mouseup_callback(uint32_t a, uint32_t b,
					  uint32_t c, uint32_t d) {
    warn("_emscripten_set_mouseup_callback ignored\n");
    return 0; // TODO: implement
}

uint32_t _emscripten_set_mousemove_callback(uint32_t a, uint32_t b,
					    uint32_t c, uint32_t d) {
    warn("_emscripten_set_mousemove_callback ignored\n");
    return 0; // TODO: implement
}

uint32_t _emscripten_set_mouseenter_callback(uint32_t a, uint32_t b,
					     uint32_t c, uint32_t d) {
    warn("_emscripten_set_mouseenter_callback ignored\n");
    return 0; // TODO: implement
}

uint32_t _emscripten_set_mouseleave_callback(uint32_t target,
                                             uint32_t userData,
                                             uint32_t useCapture,
                                             uint32_t callbackfunc) {
    warn("_emscripten_set_mouseleave_callback ignored\n");
    return 0; // TODO: implement
}

uint32_t _emscripten_set_wheel_callback(uint32_t a, uint32_t b,
					uint32_t c, uint32_t d) {
    warn("_emscripten_set_wheel_callback ignored\n");
    return 0; // TODO: implement
}

uint32_t _emscripten_set_focus_callback(uint32_t a, uint32_t b,
					uint32_t c, uint32_t d) {
    warn("_emscripten_set_focus_callback ignored\n");
    return 0; // TODO: implement
}

uint32_t _emscripten_set_blur_callback(uint32_t a, uint32_t b,
				       uint32_t c, uint32_t d) {
    warn("_emscripten_set_blur_callback ignored\n");
    return 0; // TODO: implement
}


uint32_t _emscripten_set_touchstart_callback(uint32_t a, uint32_t b,
					     uint32_t c, uint32_t d) {
    warn("_emscripten_set_touchstart_callback ignored\n");
    return 0; // TODO: implement
}

uint32_t _emscripten_set_touchend_callback(uint32_t a, uint32_t b,
					   uint32_t c, uint32_t d) {
    warn("_emscripten_set_touchend_callback ignored\n");
    return 0; // TODO: implement
}

uint32_t _emscripten_set_touchmove_callback(uint32_t a, uint32_t b,
					    uint32_t c, uint32_t d) {
    warn("_emscripten_set_touchmove_callback ignored\n");
    return 0; // TODO: implement
}

uint32_t _emscripten_set_touchcancel_callback(uint32_t a, uint32_t b,
					      uint32_t c, uint32_t d) {
    warn("_emscripten_set_touchcancel_callback ignored\n");
    return 0; // TODO: implement
}

uint32_t _emscripten_set_keypress_callback(uint32_t a, uint32_t b,
					   uint32_t c, uint32_t d) {
    warn("_emscripten_set_keypress_callback ignored\n");
    return 0; // TODO: implement
}

uint32_t _emscripten_set_keydown_callback(uint32_t a, uint32_t b,
					  uint32_t c, uint32_t d) {
    warn("_emscripten_set_keydown_callback ignored\n");
    return 0; // TODO: implement
}

uint32_t _emscripten_set_keyup_callback(uint32_t a, uint32_t b,
					uint32_t c, uint32_t d) {
    warn("_emscripten_set_keyup_callback ignored\n");
    return 0; // TODO: implement
}

uint32_t _emscripten_set_fullscreenchange_callback(uint32_t target,
                                                   uint32_t userData,
                                                   uint32_t useCapture,
                                                   uint32_t callbackfunc) {
    warn("_emscripten_set_fullscreenchange_callback ignored\n");
    return 0; // TODO: implement
}

uint32_t _emscripten_set_resize_callback(uint32_t a, uint32_t b,
					 uint32_t c, uint32_t d) {
    warn("_emscripten_set_resize_callback ignored\n");
    return 0; // TODO: implement
}

uint32_t _emscripten_set_visibilitychange_callback(uint32_t a, uint32_t b,
                                                   uint32_t c) {
    warn("_emscripten_set_visibilitychange_callback ignored\n");
    return 0; // TODO: implement
}

uint32_t _emscripten_request_fullscreen_strategy(uint32_t a, uint32_t b,
                                                 uint32_t c) {
    FATAL("_emscripten_request_fullscreen_strategy unimplemented\n");
}

// TODO: why do identical _glClear and _emscripten_glClear get generated?
void _env___glClear_(uint32_t x0) {
    glClear(x0);
    return;
}


