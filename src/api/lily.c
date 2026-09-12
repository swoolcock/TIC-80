// MIT License

// Copyright (c) 2017 Vadim Grigoruk @nesbox // grigoruk@gmail.com

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "core/core.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "lily.h"

#include "lily_value.h"

extern bool parse_note(const char* noteStr, s32* note, s32* octave);

#define GET_FLOAT(INDEX) ((float)lily_arg_double(s, INDEX))
#define GET_INT(INDEX) ((s32)lily_arg_integer(s, INDEX))
#define GET_BYTE(INDEX) ((u8)lily_arg_integer(s, INDEX))
#define GET_BOOL(INDEX) (lily_arg_boolean(s, INDEX))
#define GET_STRING(INDEX) (lily_arg_string(s, INDEX))
#define GET_OPT_FLOAT(INDEX, DEFAULT) ((float)lily_optional_double(s, INDEX, DEFAULT))
#define GET_OPT_INT(INDEX, DEFAULT) ((s32)lily_optional_integer(s, INDEX, DEFAULT))
#define GET_OPT_BYTE(INDEX, DEFAULT) ((u8)lily_optional_integer(s, INDEX, DEFAULT))
#define GET_OPT_BOOL(INDEX, DEFAULT) (lily_optional_boolean(s, INDEX, DEFAULT))
#define GET_OPT_STRING(INDEX, DEFAULT) (lily_optional_string_raw(s, INDEX, DEFAULT))

#define RETURN_DOUBLE(VAL) { lily_return_double(s, VAL); return; }
#define RETURN_INT(VAL) { lily_return_integer(s, VAL); return; }
#define RETURN_BYTE(VAL) { lily_return_byte(s, VAL); return; }
#define RETURN_STRING(VAL) { lily_return_string(s, VAL); return; }
#define RETURN_BOOL(VAL) { lily_return_boolean(s, VAL); return; }
#define RETURN_VALUE(VAL) { lily_return_value(s, VAL); return; }
#define RETURN_ERROR(VAL) { lily_ValueError(s, VAL); return; }
#define RETURN_NOVALUE() { lily_return_none(s); return; }

// region Types

typedef struct
{
    tic_mem *mem;
    lily_config config;
    lily_state *state;
    lily_function_val *borderFunction;      // BDR_FN
    lily_function_val *bootFunction;        // BOOT_FN
    lily_function_val *menuFunction;        // MENU_FN
    lily_function_val *scanlineFunction;    // SCN_FN
    lily_function_val *tickFunction;        // TIC_FN
} LILYVM;

// endregion

// region API

#define TIC_LILY_GET_CORE(STATE, TICMEM, TICCORE) \
    tic_mem* TICMEM = ((LILYVM*)lily_config_get(s)->data)->mem; \
    tic_core* TICCORE = (tic_core*)(TICMEM);

// MARK: btn
static void lily_btn(lily_state *s)
{
    TIC_LILY_GET_CORE(s, tic, core);

    s32 index = GET_OPT_INT(0, -1);
    u32 result = core->api.btn(tic, index <= 0x1f ? index : index & 0x1f);

    RETURN_INT(result);
}

// MARK: btnp
static void lily_btnp(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    s32 index = GET_OPT_INT(0, -1);
    s32 hold = GET_OPT_INT(1, -1);
    s32 period = GET_OPT_INT(2, -1);
    u32 result = core->api.btnp(tic, index <= 0x1f ? index : index & 0x1f, hold, period);

    RETURN_INT(result);
}

// MARK: circ
static void lily_circ(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    s32 x = GET_INT(0);
    s32 y = GET_INT(1);
    s32 radius = GET_INT(2);
    u8 color = GET_BYTE(3);
    core->api.circ(tic, x, y, radius, color);

    RETURN_NOVALUE();
}

// MARK: circb
static void lily_circb(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    s32 x = GET_INT(0);
    s32 y = GET_INT(1);
    s32 radius = GET_INT(2);
    u8 color = GET_BYTE(3);
    core->api.circb(tic, x, y, radius, color);

    RETURN_NOVALUE();
}

// MARK: clip
static void lily_clip(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    int nargs = lily_arg_count(s);

    if (nargs == 0)
    {
        core->api.clip(tic, 0, 0, TIC80_WIDTH, TIC80_HEIGHT);
        RETURN_NOVALUE();
    }

    if (nargs == 4)
    {
        s32 x = GET_OPT_INT(0, 0);
        s32 y = GET_OPT_INT(1, 0);
        s32 w = GET_OPT_INT(2, TIC80_WIDTH);
        s32 h = GET_OPT_INT(3, TIC80_HEIGHT);
        core->api.clip(tic, x, y, w, h);
        RETURN_NOVALUE();
    }

    RETURN_ERROR("invalid parameters, clip [x y width height]");
}

// MARK: cls
static void lily_cls(lily_state *s)
{
    TIC_LILY_GET_CORE(s, mem, core);

    u8 color = GET_OPT_BYTE(0, 0);
    core->api.cls(mem, color);

    RETURN_NOVALUE();
}

// MARK: elli
static void lily_elli(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    s32 x    = GET_INT(0);
    s32 y    = GET_INT(1);
    s32 a    = GET_INT(2);
    s32 b    = GET_INT(3);
    u8 color = GET_BYTE(4);
    core->api.elli(tic, x, y, a, b, color);

    RETURN_NOVALUE();
}

// MARK: ellib
static void lily_ellib(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    s32 x    = GET_INT(0);
    s32 y    = GET_INT(1);
    s32 a    = GET_INT(2);
    s32 b    = GET_INT(3);
    u8 color = GET_BYTE(4);
    core->api.ellib(tic, x, y, a, b, color);

    RETURN_NOVALUE();
}

// MARK: exit
static void lily_exit(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    core->api.exit(tic);

    RETURN_NOVALUE();
}

// MARK: fget
static void lily_fget(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    s32 index = GET_INT(0);
    u8 flag   = GET_BYTE(1);

    RETURN_BOOL(core->api.fget(tic, index, flag));
}

// MARK: font
static void lily_font(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    lily_string_val *text = GET_STRING(0);
    s32 x                 = GET_OPT_INT(1, 0);
    s32 y                 = GET_OPT_INT(2, 0);
    u8 chromakey          = GET_OPT_BYTE(3, 0);
    s32 width             = GET_OPT_INT(4, TIC_SPRITESIZE);
    s32 height            = GET_OPT_INT(5, TIC_SPRITESIZE);
    bool fixed            = GET_OPT_BOOL(6, false);
    s32 scale             = GET_OPT_INT(7, 1);
    bool alt              = GET_OPT_BOOL(8, false);

    if (scale == 0) RETURN_INT(0);

    s32 size = core->api.font(tic, text->string, x, y, &chromakey, 1, width, height, fixed, scale, alt);

    RETURN_INT(size);
}

// MARK: fset
static void lily_fset(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    s32 index = GET_INT(0);
    u8 flag = GET_BYTE(1);
    bool value = GET_BOOL(2);
    core->api.fset(tic, index, flag, value);

    RETURN_NOVALUE();
}

// MARK: key
static void lily_key(lily_state *s)
{
    TIC_LILY_GET_CORE(s, tic, core);

    tic_key key = GET_OPT_INT(0, tic_key_unknown);
    if (key >= tic_keys_count) RETURN_ERROR("unknown keyboard code");
    u32 result = core->api.key(tic, key);

    RETURN_INT(result);
}

// MARK: keyp
static void lily_keyp(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    tic_key key = (tic_key)GET_OPT_INT(0, tic_key_unknown);
    if (key >= tic_keys_count) RETURN_ERROR("unknown keyboard code");
    s32 hold = GET_OPT_INT(1, -1);
    s32 period = GET_OPT_INT(2, -1);
    u32 result = core->api.keyp(tic, key, hold, period);

    RETURN_INT(result);
}

// MARK: line
static void lily_line(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    float x0 = GET_FLOAT(0);
    float y0 = GET_FLOAT(1);
    float x1 = GET_FLOAT(2);
    float y1 = GET_FLOAT(3);
    u8 color = GET_BYTE(4);
    core->api.line(tic, x0, y0, x1, y1, color);

    RETURN_NOVALUE();
}

// MARK: remap

// typedef struct
// {
//     gravity_vm *vm;
//     gravity_closure_t *callback;
// } RemapData;

static void remapCallback(void* data, s32 x, s32 y, RemapResult* result)
{
    // gravity_value_t params[3] = {
    //     VALUE_FROM_INT(result->index),
    //     VALUE_FROM_INT(x),
    //     VALUE_FROM_INT(y),
    // };
    //
    // RemapData* remap = (RemapData*)data;
    // gravity_vm *vm = remap->vm;
    // gravity_vm_loadclosure(vm, remap->callback);
    // gravity_vm_runclosure(vm, remap->callback, VALUE_FROM_NULL, params, COUNT_OF(params));
    //
    // gravity_value_t rv = gravity_vm_result(vm);
    //
    // if (VALUE_ISA_LIST(rv))
    // {
    //     gravity_list_t *list = VALUE_AS_LIST(rv);
    //     u8 new_index = list->array.n >= 1 ? grav_get_int(list->array.p[0]) : result->index;
    //     tic_flip new_flip = list->array.n >= 2 ? grav_get_int(list->array.p[1]) : result->flip;
    //     tic_rotate new_rotate = list->array.n >= 3 ? grav_get_int(list->array.p[2]) : result->rotate;
    //     result->index = new_index;
    //     result->flip = new_flip;
    //     result->rotate = new_rotate;
    //
    //     // if we've been passed a list in remap, we need to aggressively garbage collect to prevent lag
    //     gravity_gc_start(vm);
    // }
    // else
    // {
    //     u8 new_index = grav_get_int(rv);
    //     result->index = new_index;
    // }
}

// MARK: map
static void lily_map(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs > 10) RETURN_ERROR("invalid parameters, map [x=0 [y=0 [w=30 [h=17 [sx=0 [sy=0 [colorkey=-1 [scale=1 [remap=null]]]]]]]]]");
    //
    // s32 x       = nargs > 1 ? grav_get_int(args[1]) : 0;
    // s32 y       = nargs > 2 ? grav_get_int(args[2]) : 0;
    // s32 w       = nargs > 3 ? grav_get_int(args[3]) : TIC_MAP_SCREEN_WIDTH;
    // s32 h       = nargs > 4 ? grav_get_int(args[4]) : TIC_MAP_SCREEN_HEIGHT;
    // s32 sx      = nargs > 5 ? grav_get_int(args[5]) : 0;
    // s32 sy      = nargs > 6 ? grav_get_int(args[6]) : 0;
    //
    // static u8 colors[TIC_PALETTE_SIZE];
    // s32 count = 0;
    //
    // if (nargs > 7)
    // {
    //     if (VALUE_ISA_LIST(args[7]))
    //     {
    //         gravity_list_t *list = VALUE_AS_LIST(args[7]);
    //         for(s32 i = 0; i < TIC_PALETTE_SIZE && i < list->array.n; i++)
    //         {
    //             colors[i] = grav_get_int(list->array.p[i]);
    //             count++;
    //         }
    //     }
    //     else
    //     {
    //         s32 color_val = grav_get_int_default(args[7], -1);
    //         colors[0] = (u8)color_val;
    //         count = color_val < 0 ? 0 : 1;
    //     }
    // }
    //
    // s32 scale = nargs > 8 ? grav_get_int(args[8]) : 1;
    //
    // if (nargs > 9 && !VALUE_ISA_NULL(args[9]) && !VALUE_ISA_CLOSURE(args[9]))
    //     RETURN_ERROR("invalid remap function");
    //
    // gravity_closure_t *remap = nargs > 9 ? VALUE_AS_CLOSURE(args[9]) : NULL;
    //
    // if (remap)
    // {
    //     RemapData data = { vm, remap };
    //     core->api.map(tic, x, y, w, h, sx, sy, colors, count, scale, remapCallback, &data);
    // }
    // else
    // {
    //     core->api.map(tic, x, y, w, h, sx, sy, colors, count, scale, NULL, NULL);
    // }

    RETURN_NOVALUE();
}

// MARK: memcpy
static void lily_memcpy(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    s32 dest = GET_INT(0);
    s32 src  = GET_INT(1);
    s32 size = GET_INT(2);
    core->api.memcpy(tic, dest, src, size);

    RETURN_NOVALUE();
}

// MARK: memset
static void lily_memset(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    s32 dest = GET_INT(0);
    u8 value  = GET_INT(1);
    s32 size = GET_INT(2);
    core->api.memset(tic, dest, value, size);

    RETURN_NOVALUE();
}

// MARK: mget
static void lily_mget(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    s32 x = GET_INT(0);
    s32 y = GET_INT(1);

    RETURN_INT(core->api.mget(tic, x, y));
}

// MARK: mouse
static void lily_mouse(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs > 2 || nargs == 2 && !VALUE_ISA_NULL(args[1]) && !VALUE_ISA_LIST(args[1]))
    //     RETURN_ERROR("invalid parameters, mouse [array]");
    //
    // const int out_value_count = 7;
    //
    // tic_point pos = core->api.mouse((tic_mem*)core);
    // const tic80_mouse* mouse = &core->memory.ram->input.mouse;
    //
    // gravity_list_t *list = nargs == 2 ? VALUE_AS_LIST(args[1]) : NULL;
    // if (!list || gravity_list_size(vm, list) != out_value_count)
    //     list = gravity_list_new(vm, out_value_count);
    //
    // list->array.n = out_value_count;
    // list->array.p[0] = VALUE_FROM_INT(pos.x);
    // list->array.p[1] = VALUE_FROM_INT(pos.y);
    // list->array.p[2] = VALUE_FROM_BOOL(mouse->left);
    // list->array.p[3] = VALUE_FROM_BOOL(mouse->middle);
    // list->array.p[4] = VALUE_FROM_BOOL(mouse->right);
    // list->array.p[5] = VALUE_FROM_INT(mouse->scrollx);
    // list->array.p[6] = VALUE_FROM_INT(mouse->scrolly);
    //
    // RETURN_VALUE(VALUE_FROM_OBJECT(list), rindex);
    RETURN_NOVALUE();
}

// MARK: mset
static void lily_mset(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    s32 x   = GET_INT(0);
    s32 y   = GET_INT(1);
    u8 val  = GET_BYTE(2);
    core->api.mset(tic, x, y, val);

    RETURN_NOVALUE();
}

// MARK: music
static void lily_music(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    int nargs = lily_arg_count(s);
    if (nargs == 0)
    {
        core->api.music(tic, -1, 0, 0, false, false, -1, -1);
        RETURN_NOVALUE();
    }

    s32 track = GET_OPT_INT(0, -1);

    if (track > MUSIC_TRACKS - 1) RETURN_ERROR("invalid music track index");

    core->api.music(tic, -1, 0, 0, false, false, -1, -1);

    s32 frame       = GET_OPT_INT(1, -1);
    s32 row         = GET_OPT_INT(2, -1);
    bool loop       = GET_OPT_BOOL(3, true);
    bool sustain    = GET_OPT_BOOL(4, false);
    s32 tempo       = GET_OPT_INT(5, -1);
    s32 speed       = GET_OPT_INT(6, -1);

    core->api.music(tic, track, frame, row, loop, sustain, tempo, speed);

    RETURN_NOVALUE();
}

// MARK: peek
static void lily_peek(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs < 2 || nargs > 3) RETURN_ERROR("invalid parameters, peek addr [bits=8]");
    //
    // s32 address = grav_get_int(args[1]);
    // s32 bits    = nargs == 3 ? grav_get_int_default(args[2], BITS_IN_BYTE) : BITS_IN_BYTE;
    //
    // RETURN_VALUE(VALUE_FROM_INT(core->api.peek(tic, address, bits)), rindex);
    RETURN_NOVALUE();
}

// MARK: peek1
static void lily_peek1(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs != 2) RETURN_ERROR("invalid parameters, peek1 bitaddr");
    //
    // s32 address = grav_get_int(args[1]);
    //
    // RETURN_VALUE(VALUE_FROM_INT(core->api.peek1(tic, address)), rindex);
    RETURN_NOVALUE();
}

// MARK: peek2
static void lily_peek2(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs != 2) RETURN_ERROR("invalid parameters, peek2 addr2");
    //
    // s32 address = grav_get_int(args[1]);
    //
    // RETURN_VALUE(VALUE_FROM_INT(core->api.peek2(tic, address)), rindex);
    RETURN_NOVALUE();
}

// MARK: peek4
static void lily_peek4(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs != 2) RETURN_ERROR("invalid parameters, peek4 addr4");
    //
    // s32 address = grav_get_int(args[1]);
    //
    // RETURN_VALUE(VALUE_FROM_INT(core->api.peek4(tic, address)), rindex);
    RETURN_NOVALUE();
}

// MARK: pix
static void lily_pix(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs < 3 || nargs > 4) RETURN_ERROR("invalid parameters, pix x y [color]");
    //
    // s32 x       = grav_get_int(args[1]);
    // s32 y       = grav_get_int(args[2]);
    // u8 color    = nargs == 4 ? grav_get_int(args[3]) : 0;
    // u8 result   = core->api.pix(tic, x, y, color, nargs == 3);
    //
    // if (nargs == 3) RETURN_VALUE(VALUE_FROM_INT(result), rindex);

    RETURN_NOVALUE();
}

// MARK: pmem
static void lily_pmem(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs < 2 || nargs > 3) RETURN_ERROR("invalid parameters, pmem index [val32]");
    //
    // s32 index = grav_get_int(args[1]);
    // if (index >= TIC_PERSISTENT_SIZE) RETURN_ERROR("invalid persistent tic index");
    //
    // u32 current = core->api.pmem(tic, index, 0, false);
    // if (nargs == 3) core->api.pmem(tic, index, (u32)grav_get_int(args[2]), true);
    //
    // RETURN_VALUE(VALUE_FROM_INT(current), rindex);
    RETURN_NOVALUE();
}

// MARK: poke
static void lily_poke(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs < 3 || nargs > 4) RETURN_ERROR("invalid parameters, poke addr val [bits=8]");
    //
    // s32 address = grav_get_int(args[1]);
    // u8 value    = grav_get_int(args[2]);
    // s32 bits    = nargs == 4 ? grav_get_int_default(args[3], BITS_IN_BYTE) : BITS_IN_BYTE;
    //
    // core->api.poke(tic, address, value, bits);

    RETURN_NOVALUE();
}

// MARK: poke1
static void lily_poke1(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs != 3) RETURN_ERROR("invalid parameters, poke1 bitaddr bitval");
    //
    // s32 address = grav_get_int(args[1]);
    // u8 value    = grav_get_int(args[2]);
    //
    // core->api.poke1(tic, address, value);

    RETURN_NOVALUE();
}

// MARK: poke2
static void lily_poke2(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs != 3) RETURN_ERROR("invalid parameters, poke2 addr2 val2");
    //
    // s32 address = grav_get_int(args[1]);
    // u8 value    = grav_get_int(args[2]);
    // core->api.poke2(tic, address, value);

    RETURN_NOVALUE();
}

// MARK: poke4
static void lily_poke4(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs != 3) RETURN_ERROR("invalid parameters, poke4 addr4 val4");
    //
    // s32 address = grav_get_int(args[1]);
    // u8 value    = grav_get_int(args[2]);
    // core->api.poke4(tic, address, value);

    RETURN_NOVALUE();
}

// MARK: print
static void lily_print(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs < 2 || nargs > 8)
    //     RETURN_ERROR("invalid parameters, print text [x=0 [y=0 [color=15 [fixed=false [scale=1 [smallfont=false]]]]]]");
    //
    // gravity_string_t *text  = grav_get_string(vm, args[1]);
    // s32 x                   = nargs >= 3 ? grav_get_int(args[2]) : 0;
    // s32 y                   = nargs >= 4 ? grav_get_int(args[3]) : 0;
    // u8 color                = nargs >= 5 ? grav_get_int(args[4]) : TIC_DEFAULT_COLOR;
    // bool fixed              = nargs >= 6 ? grav_get_int(args[5]) != 0 : false;
    // s32 scale               = nargs >= 7 ? grav_get_int(args[6]) : 1;
    // bool smallfont          = nargs >= 8 ? grav_get_int(args[7]) != 0 : false;
    //
    // s32 width = core->api.print(tic, text->s, x, y, color, fixed, scale, smallfont);
    //
    // RETURN_VALUE(VALUE_FROM_INT(width), rindex);
    RETURN_NOVALUE();
}

// MARK: rect
static void lily_rect(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs != 6) RETURN_ERROR("invalid parameters, rect x y w h color");
    //
    // s32 x       = grav_get_int(args[1]);
    // s32 y       = grav_get_int(args[2]);
    // s32 w       = grav_get_int(args[3]);
    // s32 h       = grav_get_int(args[4]);
    // u8 color    = grav_get_int(args[5]);
    // core->api.rect(tic, x, y, w, h, color);

    RETURN_NOVALUE();
}

// MARK: rectb
static void lily_rectb(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs != 6) RETURN_ERROR("invalid parameters, rectb x y w h color");
    //
    // s32 x       = grav_get_int(args[1]);
    // s32 y       = grav_get_int(args[2]);
    // s32 w       = grav_get_int(args[3]);
    // s32 h       = grav_get_int(args[4]);
    // u8 color    = grav_get_int(args[5]);
    // core->api.rectb(tic, x, y, w, h, color);

    RETURN_NOVALUE();
}

// MARK: reset
static void lily_reset(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs > 1) RETURN_ERROR("invalid parameters, reset");
    //
    // core->api.reset(tic);

    RETURN_NOVALUE();
}

// MARK: sfx
static void lily_sfx(lily_state *s)
{
    RETURN_NOVALUE();
}

// MARK: spr
static void lily_spr(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs < 4 || nargs > 10)
    //     RETURN_ERROR("invalid parameters, spr id x y [colorkey=-1 [scale=1 [flip=0 [rotate=0 [w=1 [h=1]]]]]]");
    //
    // s32 index           = grav_get_int(args[1]);
    // s32 x               = grav_get_int(args[2]);
    // s32 y               = grav_get_int(args[3]);
    // s32 scale           = nargs > 5 ? grav_get_int(args[5]) : 1;
    // tic_flip flip       = nargs > 6 ? grav_get_int(args[6]) : tic_no_flip;
    // tic_rotate rotate   = nargs > 7 ? grav_get_int(args[7]) : tic_no_rotate;
    // s32 w               = nargs > 8 ? grav_get_int(args[8]) : 1;
    // s32 h               = nargs > 9 ? grav_get_int(args[9]) : 1;
    //
    // static u8 colors[TIC_PALETTE_SIZE];
    // s32 count = 0;
    //
    // if (nargs > 4)
    // {
    //     if (VALUE_ISA_LIST(args[4]))
    //     {
    //         gravity_list_t *list = VALUE_AS_LIST(args[4]);
    //         for(s32 i = 0; i < TIC_PALETTE_SIZE && i < list->array.n; i++)
    //         {
    //             colors[i] = grav_get_int(list->array.p[i]);
    //             count++;
    //         }
    //     }
    //     else
    //     {
    //         s32 color_val = grav_get_int_default(args[4], -1);
    //         colors[0] = (u8)color_val;
    //         count = color_val < 0 ? 0 : 1;
    //     }
    // }
    //
    // core->api.spr(tic, index, x, y, w, h, colors, count, scale, flip, rotate);

    RETURN_NOVALUE();
}

// MARK: sync
static void lily_sync(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs > 4) RETURN_ERROR("invalid parameters, sync [mask=0 [bank=0 [tocart=false]]]");
    //
    // u32 mask    = nargs >= 2 ? grav_get_int(args[1]) : 0;
    // s32 bank    = nargs >= 3 ? grav_get_int(args[2]) : 0;
    // bool toCart = nargs == 4 ? grav_get_int(args[3]) != 0 : false;
    //
    // if(bank >= 0 && bank < TIC_BANKS)
    //     core->api.sync(tic, mask, bank, toCart);
    // else
    //     RETURN_ERROR("sync() error, invalid bank");

    RETURN_NOVALUE();
}

// MARK: time
static void lily_time(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs > 1) RETURN_ERROR("invalid parameters, time");
    //
    // RETURN_VALUE(VALUE_FROM_FLOAT(core->api.time(tic)), rindex);
    RETURN_NOVALUE();
}

// MARK: trace
static void lily_trace(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs < 2 || nargs > 3) RETURN_ERROR("invalid parameters, trace text [color=15]");
    //
    // gravity_string_t *text  = grav_get_string(vm, args[1]);
    // u8 color                = nargs == 3 ? grav_get_int(args[2]) : TIC_DEFAULT_COLOR;
    //
    // core->api.trace(tic, text->s, color);

    RETURN_NOVALUE();
}

// MARK: tri
static void lily_tri(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs != 8) RETURN_ERROR("invalid parameters, tri x1 y1 x2 y2 x3 y3 color");
    //
    // float pt[6];
    // for(s32 i = 0; i < COUNT_OF(pt); i++)
    //     pt[i] = grav_get_float(args[i + 1]);
    //
    // u8 color = grav_get_int(args[7]);
    //
    // core->api.tri(tic, pt[0], pt[1], pt[2], pt[3], pt[4], pt[5], color);

    RETURN_NOVALUE();
}

// MARK: trib
static void lily_trib(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs != 8) RETURN_ERROR("invalid parameters, trib x1 y1 x2 y2 x3 y3 color");
    //
    // float pt[6];
    // for(s32 i = 0; i < COUNT_OF(pt); i++)
    //     pt[i] = grav_get_float(args[i + 1]);
    //
    // u8 color = grav_get_int(args[7]);
    //
    // core->api.trib(tic, pt[0], pt[1], pt[2], pt[3], pt[4], pt[5], color);

    RETURN_NOVALUE();
}

// MARK: tstamp
static void lily_tstamp(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs > 1) RETURN_ERROR("invalid parameters, tstamp");
    //
    // RETURN_VALUE(VALUE_FROM_INT(core->api.tstamp(tic)), rindex);
    RETURN_NOVALUE();
}

// MARK: ttri
static void lily_ttri(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // if (nargs < 13 || nargs > 18)
    //     RETURN_ERROR(
    //         "invalid parameters, ttri x1 y1 x2 y2 x3 y3 u1 v1 u2 v2 u3 v3 "
    //         "[src=0 [chroma=off [z1=0 z2=0 z3=0]]]");
    //
    // float pt[12];
    // for (s32 i = 0; i < COUNT_OF(pt); i++)
    //     pt[i] = grav_get_float(args[i + 1]);
    //
    // // check for texture src
    // tic_texture_src src = nargs >= 14 ? grav_get_int(args[13]) : tic_tiles_texture;
    //
    // static u8 colors[TIC_PALETTE_SIZE];
    // s32 count = 0;
    //
    // // check for chroma
    // if (nargs >= 15)
    // {
    //     if (VALUE_ISA_LIST(args[14]))
    //     {
    //         gravity_list_t *list = VALUE_AS_LIST(args[14]);
    //         for(s32 i = 0; i < TIC_PALETTE_SIZE && i < list->array.n; i++)
    //         {
    //             colors[i] = grav_get_int(list->array.p[i]);
    //             count++;
    //         }
    //     }
    //     else
    //     {
    //         s32 color_val = grav_get_int_default(args[14], -1);
    //         colors[0] = (u8)color_val;
    //         count = color_val < 0 ? 0 : 1;
    //     }
    // }
    //
    // float z[3] = {0, 0, 0};
    // bool depth = false;
    //
    // if (nargs == 18)
    // {
    //     for (s32 i = 0; i < COUNT_OF(z); i++)
    //         z[i] = grav_get_float(args[15 + i]);
    //
    //     depth = true;
    // }
    //
    // core->api.ttri(tic, pt[0], pt[1],   //  xy 1
    //                pt[2], pt[3],        //  xy 2
    //                pt[4], pt[5],        //  xy 3
    //                pt[6], pt[7],        //  uv 1
    //                pt[8], pt[9],        //  uv 2
    //                pt[10], pt[11],      //  uv 3
    //                src,                 // texture source
    //                colors, count,       // chroma
    //                z[0], z[1], z[2], depth); // depth

    RETURN_NOVALUE();
}

// MARK: vbank
static void lily_vbank(lily_state *s)
{
    TIC_LILY_GET_CORE(vm, tic, core);

    // s32 prev = core->state.vbank.id;
    //
    // if (nargs > 2) RETURN_ERROR("invalid parameters, vbank [id]");
    // if (nargs == 2) core->api.vbank(tic, grav_get_int(args[1]));
    //
    // RETURN_VALUE(VALUE_FROM_INT(prev), rindex);
    RETURN_NOVALUE();
}

// unused?
static void lily_paint(lily_state *s)
{
    // RETURN_ERROR("paint is unimplemented");
    RETURN_NOVALUE();
}

static void lily_fft(lily_state *s)
{
    // RETURN_ERROR("fft is unimplemented");
    RETURN_NOVALUE();
}

static void lily_ffts(lily_state *s)
{
    // RETURN_ERROR("ffts is unimplemented");
    RETURN_NOVALUE();
}

// endregion

// The Lily developer promotes the use of a bindgen tool to generate these,
// but it really doesn't suit our build chain and it's easy enough to do by hand.
static const char *lily_tic80_info_table[] = {
    "\0\0"
    ,"F\0btn\0(*Integer): Integer"
    ,"F\0btnp\0(*Integer, *Integer, *Integer): Integer"
    ,"F\0circ\0(Integer, Integer, Integer, Integer)"
    ,"F\0circb\0(Integer, Integer, Integer, Integer)"
    ,"F\0clip\0(*Integer, *Integer, *Integer, *Integer)"
    ,"F\0cls\0(*Integer)"
    ,"F\0elli\0(Integer, Integer, Integer, Integer, Integer)"
    ,"F\0ellib\0(Integer, Integer, Integer, Integer, Integer)"
    ,"F\0exit\0"
    ,"F\0fget\0(Integer, flag: Integer): Boolean"
    ,"F\0font\0(String, *Integer, *Integer, *Integer, *Integer=8, *Integer=8, *Boolean, *Integer, *Boolean)"
    ,"F\0fset\0(Integer, Integer, Boolean)"
    ,"F\0key\0(*Integer): Integer"
    ,"F\0keyp\0(*Integer, *Integer, *Integer): Integer"
    ,"F\0line\0(Double, Double, Double, Double, Integer)"
    ,"F\0map\0(*Integer, *Integer, *Integer, *Integer, *Integer, *Integer, *List[Integer], *List[Integer], *Option[Function(Integer, Integer, Integer => Tuple[Integer, Integer, Integer])])"
    ,"F\0memcpy\0(Integer, Integer, Integer)"
    ,"F\0memset\0(Integer, Integer, Integer)"
    ,"F\0mget\0(Integer, Integer): Integer"
    ,"F\0mouse\0: Tuple[Integer, Integer, Boolean, Boolean, Boolean, Integer, Integer]"
    ,"F\0mset\0(Integer, Integer, Integer)"
    ,"F\0music\0(*Integer, *Integer, *Integer, *Boolean, *Boolean, *Integer, *Integer)"
    ,"F\0peek\0(Integer, *Integer=8): Integer"
    ,"F\0peek1\0(Integer): Integer"
    ,"F\0peek2\0(Integer): Integer"
    ,"F\0peek4\0(Integer): Integer"
    ,"F\0pix\0(Integer, Integer, *Integer): Integer"
    ,"F\0pmem\0(Integer, *Option[Integer]): Integer"
    ,"F\0poke\0(Integer, Integer, *Integer=8)"
    ,"F\0poke1\0(Integer, Integer)"
    ,"F\0poke2\0(Integer, Integer)"
    ,"F\0poke4\0(Integer, Integer)"
    ,"F\0print\0(String, *Integer, *Integer, *Integer, *Boolean, *Integer, *Boolean): Integer"
    ,"F\0rect\0(Integer, Integer, Integer, Integer, Integer)"
    ,"F\0rectb\0(Integer, Integer, Integer, Integer, Integer)"
    ,"F\0reset\0"
    ,"F\0sfx\0(Integer, *Integer, *Integer, *Integer, *Integer, *Integer)"
    ,"F\0spr\0(Integer, Integer, Integer, *List[Integer], *Integer, *Integer, *Integer, *Integer, *Integer)"
    ,"F\0sync\0(*Integer, *Integer, *Boolean)"
    ,"F\0time\0: Double"
    ,"F\0trace\0(String, *Integer)"
    ,"F\0tri\0(Integer, Integer, Integer, Integer, Integer, Integer, Integer)"
    ,"F\0trib\0(Integer, Integer, Integer, Integer, Integer, Integer, Integer)"
    ,"F\0tstamp\0: Integer"
    ,"F\0ttri\0(Integer, Integer, Integer, Integer, Integer, Integer, Integer, Integer, Integer, Integer, Integer, Integer, *Integer, *List[Integer], *Integer, *Integer, *Integer)"
    ,"F\0vbank\0(*Integer): Integer"
    ,"Z"
};
static lily_call_entry_func lily_tic80_call_table[] = {
    NULL,
    lily_btn,
    lily_btnp,
    lily_circ,
    lily_circb,
    lily_clip,
    lily_cls,
    lily_elli,
    lily_ellib,
    lily_exit,
    lily_fget,
    lily_font,
    lily_fset,
    lily_key,
    lily_keyp,
    lily_line,
    lily_map,
    lily_memcpy,
    lily_memset,
    lily_mget,
    lily_mouse,
    lily_mset,
    lily_music,
    lily_peek,
    lily_peek1,
    lily_peek2,
    lily_peek4,
    lily_pix,
    lily_pmem,
    lily_poke,
    lily_poke1,
    lily_poke2,
    lily_poke4,
    lily_print,
    lily_rect,
    lily_rectb,
    lily_reset,
    lily_sfx,
    lily_spr,
    lily_sync,
    lily_time,
    lily_trace,
    lily_tri,
    lily_trib,
    lily_tstamp,
    lily_ttri,
    lily_vbank,
};

// region Callbacks

// static void report_error(gravity_vm *vm, error_type_t type,
//                          const char *description, error_desc_t desc, void *xdata) {
//
//     tic_mem *mem = xdata;
//     tic_core *core = (tic_core*)mem;
//
//     if (core->data)
//     {
//         char buf[1024];
//         snprintf(buf, sizeof(buf), "%s\n", description);
//         core->data->error(core->data->data, buf);
//     }
// }
//
// static const char* get_precode(void* xdata)
// {
//     static char buffer[16384];
//     buffer[0] = '\0';
// #define TIC_GRAVITY_EXTERN(name, ...) {strcat(buffer, "extern var " #name ";\n");}
//     TIC_API_LIST(TIC_GRAVITY_EXTERN);
// #undef TIC_GRAVITY_EXTERN
//     return buffer;
// }

static void closeLily(tic_mem* tic)
{
    tic_core* core = (tic_core*)tic;
    if (core->currentVM)
    {
        LILYVM *currentVM = core->currentVM;
        core->currentVM = NULL;

        lily_free_state(currentVM->state);
        free(currentVM);
    }
}

static bool initLily(tic_mem* tic, const char* code)
{
    tic_core* core = (tic_core*)tic;

    // ensure we close any previous vm
    closeLily(tic);

    // create the vm
    core->currentVM = malloc(sizeof(LILYVM));
    LILYVM *currentVM = core->currentVM;
    currentVM->mem = tic;
    lily_config_init(&currentVM->config);
    currentVM->config.data = currentVM;
    currentVM->state = lily_new_state(&currentVM->config);

    lily_module_register(currentVM->state, "tic", lily_tic80_info_table, lily_tic80_call_table);

    // compile the code and bail if it failed
    if (!lily_load_string(currentVM->state, "[cart]", code))
    {
        core->data->error(core->data->data, lily_error_message(currentVM->state));
        return false;
    }

    if (!lily_parse_content(currentVM->state))
    {
        core->data->error(core->data->data, lily_error_message(currentVM->state));
        return false;
    }

    // register TIC-80 callbacks
#define TIC_LILY_GET_CALLBACK(STORE, STATE, FUNC_NAME) \
        { STORE = lily_find_function(STATE, FUNC_NAME); }
    TIC_LILY_GET_CALLBACK(currentVM->borderFunction, currentVM->state, BDR_FN);
    TIC_LILY_GET_CALLBACK(currentVM->bootFunction, currentVM->state, BOOT_FN);
    TIC_LILY_GET_CALLBACK(currentVM->menuFunction, currentVM->state, MENU_FN);
    TIC_LILY_GET_CALLBACK(currentVM->scanlineFunction, currentVM->state, SCN_FN);
    TIC_LILY_GET_CALLBACK(currentVM->tickFunction, currentVM->state, TIC_FN);
#undef TIC_LILY_GET_CALLBACK

    return true;
}

static void callLilyTick(tic_mem* tic)
{
    LILYVM* currentVM = ((tic_core*)tic)->currentVM;
    if (currentVM->tickFunction)
    {
        lily_call_prepare(currentVM->state, currentVM->tickFunction);
        lily_call(currentVM->state, 0);
    }
}

static void callLilyBoot(tic_mem* tic)
{
    LILYVM* currentVM = ((tic_core*)tic)->currentVM;
    if (currentVM->bootFunction)
    {
        lily_call_prepare(currentVM->state, currentVM->bootFunction);
        lily_call(currentVM->state, 0);
    }
}

static void callLilyScanline(tic_mem* tic, s32 row, void* data)
{
    LILYVM* currentVM = ((tic_core*)tic)->currentVM;
    if (currentVM->scanlineFunction)
    {
        lily_call_prepare(currentVM->state, currentVM->scanlineFunction);
        lily_push_integer(currentVM->state, row);
        lily_call(currentVM->state, 1);
    }
}

static void callLilyBorder(tic_mem* tic, s32 row, void* data)
{
    LILYVM* currentVM = ((tic_core*)tic)->currentVM;
    if (currentVM->borderFunction)
    {
        lily_call_prepare(currentVM->state, currentVM->borderFunction);
        lily_push_integer(currentVM->state, row);
        lily_call(currentVM->state, 1);
    }
}

static void callLilyMenu(tic_mem* tic, s32 index, void* data)
{
    LILYVM* currentVM = ((tic_core*)tic)->currentVM;
    if (currentVM->menuFunction)
    {
        lily_call_prepare(currentVM->state, currentVM->menuFunction);
        lily_push_integer(currentVM->state, index);
        lily_call(currentVM->state, 1);
    }
}

static const tic_outline_item* getLilyOutline(const char* code, s32* size)
{
    // TODO
    return NULL;
}

static void evalLily(tic_mem* tic, const char* code)
{
    // TODO
}

// endregion

// region Script Config

static const char* const LilyKeywords [] =
{
    "if",
    "do",
    "var",
    "for",
    "try",
    "case",
    "else",
    "elif",
    "with",
    "enum",
    "while",
    "raise",
    "match",
    "break",
    "class",
    "public",
    "static",
    "define",
    "return",
    "except",
    "import",
    "forward",
    "private",
    "virtual",
    "protected",
    "continue",
    "constant",
};

static const u8 DemoRom[] =
{
    // TODO
// #include "../build/assets/squirreldemo.tic.dat"
};

static const u8 MarkRom[] =
{
    // TODO
// #include "../build/assets/squirrelmark.tic.dat"
};

TIC_EXPORT const tic_script EXPORT_SCRIPT(Lily) =
{
    .id                 = 22,
    .name               = "lily",
    .fileExtension      = ".lily",
    .projectComment     = "#",
    {
        .init               = initLily,
        .close              = closeLily,
        .tick               = callLilyTick,
        .boot               = callLilyBoot,

        .callback           =
        {
            .scanline       = callLilyScanline,
            .border         = callLilyBorder,
            .menu           = callLilyMenu,
          },
        },

        .getOutline         = getLilyOutline,
        .eval               = evalLily,

        .blockCommentStart  = "#[",
        .blockCommentEnd    = "]#",
        .blockCommentStart2 = NULL,
        .blockCommentEnd2   = NULL,
        .singleComment      = "#",
        .blockStringStart   = "@\"",
        .blockStringEnd     = "\"",
        .blockEnd           = "}",
        .stdStringStartEnd  = "\'\"",

        .keywords           = LilyKeywords,
        .keywordsCount      = COUNT_OF(LilyKeywords),

        .demo = {DemoRom, sizeof DemoRom},
        .mark = {MarkRom, sizeof MarkRom, "lilymark.tic"},
    };

// endregion
