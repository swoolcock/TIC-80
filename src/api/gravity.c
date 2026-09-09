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

#include "gravity_compiler.h"
#include "gravity_core.h"
#include "gravity_macros.h"
#include "gravity_vm.h"
#include "gravity_vmmacros.h"

extern bool parse_note(const char* noteStr, s32* note, s32* octave);

// region Macros

#define TIC_GRAVITY_GET_CORE(VM, TICMEM, TICCORE) \
    tic_mem* TICMEM = (tic_mem*)gravity_vm_delegate(VM)->xdata; \
    tic_core* TICCORE = (tic_core*)(TICMEM);

// endregion

// region Types

typedef struct
{
    gravity_vm *vm;
    gravity_delegate_t delegate;
    gravity_closure_t *borderFunction;      // BDR_FN
    gravity_closure_t *bootFunction;        // BOOT_FN
    gravity_closure_t *menuFunction;        // MENU_FN
    gravity_closure_t *scanlineFunction;    // SCN_FN
    gravity_closure_t *tickFunction;        // TIC_FN
} GRAVITYVM;

// endregion

// region Helpers

static int grav_get_int_default(gravity_value_t value, int default_value)
{
    if (VALUE_ISA_INT(value) || VALUE_ISA_BOOL(value)) return VALUE_AS_INT(value);
    if (VALUE_ISA_FLOAT(value)) return (int)VALUE_AS_FLOAT(value);
    if (VALUE_ISA_STRING(value))
    {
        const gravity_string_t *str = VALUE_AS_STRING(value);
        char *end;
        float parsed = strtof(str->s, &end);
        if (end != str->s)
        {
            // ignore trailing whitespace as in Lua
            while (isspace((u8)*end)) end++;
            if (*end == '\0') return (int)parsed;
        }
    }
    return default_value;
}

static float grav_get_float_default(gravity_value_t value, float default_value)
{
    if (VALUE_ISA_INT(value) || VALUE_ISA_BOOL(value)) return (float)VALUE_AS_INT(value);
    if (VALUE_ISA_FLOAT(value)) return VALUE_AS_FLOAT(value);
    if (VALUE_ISA_STRING(value))
    {
        const gravity_string_t *str = VALUE_AS_STRING(value);
        char *end;
        float parsed = strtof(str->s, &end);
        if (end != str->s)
        {
            // ignore trailing whitespace as in Lua
            while (isspace((u8)*end)) end++;
            if (*end == '\0') return parsed;
        }
    }
    return default_value;
}

#define TIC_GRAVITY_DEF_CONVERT_OVERLOAD(TYPE, DEFAULT) \
    static TYPE grav_get_##TYPE(gravity_value_t value) { \
        return grav_get_##TYPE##_default(value, DEFAULT); \
    }

TIC_GRAVITY_DEF_CONVERT_OVERLOAD(int, 0);
TIC_GRAVITY_DEF_CONVERT_OVERLOAD(float, 0.0f);

#undef TIC_GRAVITY_DEF_CONVERT_OVERLOAD

// endregion

// region API

static bool grav_btn(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex)
{
    TIC_GRAVITY_GET_CORE(vm, tic, core);

    if (nargs == 1)
    {
        RETURN_VALUE(VALUE_FROM_INT(core->api.btn(tic, -1)), rindex);
    }

    if (nargs == 2)
    {
        s32 index = grav_get_int(args[1]) & 0x1f;
        RETURN_VALUE(VALUE_FROM_BOOL(core->api.btn(tic, index)), rindex);
    }

    RETURN_ERROR("invalid parameters, btn [ id=-1 ]");
}

static bool grav_btnp(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex)
{
    TIC_GRAVITY_GET_CORE(vm, tic, core);

    if (nargs == 1)
    {
        RETURN_VALUE(VALUE_FROM_INT(core->api.btnp(tic, -1, -1, -1)), rindex);
    }

    if (nargs == 2)
    {
        s32 index = grav_get_int(args[1]) & 0x1f;
        RETURN_VALUE(VALUE_FROM_BOOL(core->api.btnp(tic, index, -1, -1)), rindex);
    }

    if (nargs == 4)
    {
        s32 index = grav_get_int(args[1]) & 0x1f;
        u32 hold = grav_get_int(args[2]);
        u32 period = grav_get_int(args[3]);
        RETURN_VALUE(VALUE_FROM_BOOL(core->api.btnp(tic, index, hold, period)), rindex);
    }

    RETURN_ERROR("invalid parameters, btnp [ id=-1 [ hold=-1 period=-1 ] ]");
}

static bool grav_circ(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex)
{
    TIC_GRAVITY_GET_CORE(vm, tic, core);

    if (nargs == 5)
    {
        s32 x = grav_get_int(args[1]);
        s32 y = grav_get_int(args[2]);
        s32 radius = grav_get_int(args[3]);
        u8 color = grav_get_int(args[4]);
        core->api.circ(tic, x, y, radius, color);
        RETURN_NOVALUE();
    }

    RETURN_ERROR("invalid parameters, circ x y radius color");
}

static bool grav_circb(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex)
{
    TIC_GRAVITY_GET_CORE(vm, tic, core);

    if (nargs == 5)
    {
        s32 x = grav_get_int(args[1]);
        s32 y = grav_get_int(args[2]);
        s32 radius = grav_get_int(args[3]);
        u8 color = grav_get_int(args[4]);
        core->api.circb(tic, x, y, radius, color);
        RETURN_NOVALUE();
    }

    RETURN_ERROR("invalid parameters, circb x y radius color");
}

static bool grav_clip(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}

static bool grav_cls(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex)
{
    TIC_GRAVITY_GET_CORE(vm, tic, core);

    if (nargs > 2) RETURN_ERROR("invalid parameters, cls [ color=0 ]");

    u8 cls_color = (u8)grav_get_int(args[1]);
    core->api.cls(tic, cls_color);

    RETURN_NOVALUE();
}

static bool grav_elli(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex)
{
    TIC_GRAVITY_GET_CORE(vm, tic, core);

    if (nargs == 5)
    {
        s32 x = grav_get_int(args[1]);
        s32 y = grav_get_int(args[2]);
        s32 a = grav_get_int(args[3]);
        s32 b = grav_get_int(args[4]);
        u8 color = grav_get_int(args[5]);
        core->api.elli(tic, x, y, a, b, color);
        RETURN_NOVALUE();
    }

    RETURN_ERROR("invalid parameters, elli x y a b color");
}

static bool grav_ellib(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex)
{
    TIC_GRAVITY_GET_CORE(vm, tic, core);

    if (nargs == 5)
    {
        s32 x = grav_get_int(args[1]);
        s32 y = grav_get_int(args[2]);
        s32 a = grav_get_int(args[3]);
        s32 b = grav_get_int(args[4]);
        u8 color = grav_get_int(args[5]);
        core->api.ellib(tic, x, y, a, b, color);
        RETURN_NOVALUE();
    }

    RETURN_ERROR("invalid parameters, ellib x y a b color");
}

static bool grav_exit(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_fget(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_font(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_fset(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_key(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_keyp(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}

static bool grav_line(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex)
{
    TIC_GRAVITY_GET_CORE(vm, tic, core);

    if (nargs == 5)
    {
        float x0 = grav_get_float(args[1]);
        float y0 = grav_get_float(args[2]);
        float x1 = grav_get_float(args[3]);
        float y1 = grav_get_float(args[4]);
        u8 color = grav_get_int(args[5]);
        core->api.line(tic, x0, y0, x1, y1, color);
        RETURN_NOVALUE();
    }

    RETURN_ERROR("invalid parameters, line x0 y0 x1 y1 color");
}

static bool grav_map(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_memcpy(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_memset(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_mget(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_mouse(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_mset(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_music(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_peek(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_peek1(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_peek2(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_peek4(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_pix(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_pmem(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_poke(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_poke1(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_poke2(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_poke4(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_print(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}

static bool grav_rect(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex)
{
    TIC_GRAVITY_GET_CORE(vm, tic, core);

    if (nargs == 5)
    {
        s32 x = grav_get_int(args[1]);
        s32 y = grav_get_int(args[2]);
        s32 w = grav_get_int(args[3]);
        s32 h = grav_get_int(args[4]);
        u8 color = grav_get_int(args[5]);
        core->api.rect(tic, x, y, w, h, color);
        RETURN_NOVALUE();
    }

    RETURN_ERROR("invalid parameters, rect x y w h color");
}

static bool grav_rectb(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex)
{
    TIC_GRAVITY_GET_CORE(vm, tic, core);

    if (nargs == 5)
    {
        s32 x = grav_get_int(args[1]);
        s32 y = grav_get_int(args[2]);
        s32 w = grav_get_int(args[3]);
        s32 h = grav_get_int(args[4]);
        u8 color = grav_get_int(args[5]);
        core->api.rectb(tic, x, y, w, h, color);
        RETURN_NOVALUE();
    }

    RETURN_ERROR("invalid parameters, rectb x y w h color");
}

static bool grav_reset(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_sfx(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_spr(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_sync(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_time(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_trace(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_tri(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_trib(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_tstamp(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_ttri(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_vbank(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}

// unused?
static bool grav_paint(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_ffts(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_fft(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}

// endregion

// region Callbacks

static void report_error(gravity_vm *vm, error_type_t type,
                         const char *description, error_desc_t desc, void *xdata) {

    tic_mem *mem = xdata;
    tic_core *core = (tic_core*)mem;

    if (core->data)
    {
        char buf[1024];
        snprintf(buf, sizeof(buf), "%s\n", description);
        core->data->error(core->data->data, buf);
    }
}

static const char* get_precode(void* xdata)
{
    static char buffer[16384];
    buffer[0] = '\0';
#define TIC_GRAVITY_EXTERN(name, ...) {strcat(buffer, "extern var " #name ";\n");}
    TIC_API_LIST(TIC_GRAVITY_EXTERN);
#undef TIC_GRAVITY_EXTERN
    return buffer;
}

static void initAPI(GRAVITYVM *vm)
{
    // bind TIC-80 API
#define API_FUNC_DEF(name, ...) {grav_ ## name, #name},
    static const struct{gravity_c_internal func; const char* name;} ApiItems[] = {TIC_API_LIST(API_FUNC_DEF)};
#undef API_FUNC_DEF

    for (s32 i = 0; i < COUNT_OF(ApiItems); i++)
        gravity_vm_setvalue(vm->vm, ApiItems[i].name, NEW_CLOSURE_VALUE(ApiItems[i].func));

    // TODO: remove things we shouldn't have access to such as filesystem, env, etc.
}

static void closeGravity(tic_mem* tic)
{
    tic_core* core = (tic_core*)tic;
    if (core->currentVM)
    {
        GRAVITYVM *currentVM = core->currentVM;
        core->currentVM = NULL;

        gravity_vm_free(currentVM->vm);
        gravity_core_free();
        free(currentVM);
    }
}

static bool initGravity(tic_mem* tic, const char* code)
{
    tic_core* core = (tic_core*)tic;

    // ensure we close any previous vm
    closeGravity(tic);

    // allocate and populate the vm wrapper
    core->currentVM = malloc(sizeof(GRAVITYVM));
    GRAVITYVM *currentVM = core->currentVM;
    gravity_delegate_t delegate = {
        .error_callback = report_error,
        .precode_callback = get_precode,
        .xdata = tic
    };
    currentVM->delegate = delegate;

    // create the gravity vm
    gravity_vm *vm = gravity_vm_new(&currentVM->delegate);
    currentVM->vm = vm;

    // compile the code
    // TODO: strip any shebangs, includes, imports, externs (we'll add those ourselves)
    gravity_compiler_t *compiler = gravity_compiler_create(&currentVM->delegate);
    gravity_closure_t *closure = gravity_compiler_run(compiler, code, strlen(code), 0, true, true);
    gravity_compiler_transfer(compiler, vm);
    gravity_compiler_free(compiler);

    // if the code failed compilation, bail
    if (!closure) return false;

    // register TIC-80 functions
    initAPI(currentVM);

    // execute the code
    gravity_vm_loadclosure(vm, closure);
    gravity_vm_runclosure(vm, closure, VALUE_FROM_NULL, NULL, 0);

    // register TIC-80 callbacks
#define TIC_GRAVITY_GET_CALLBACK(STORE, VM, FUNC_NAME) \
    { gravity_value_t funcval = gravity_vm_getvalue(VM, FUNC_NAME, strlen(FUNC_NAME)); \
      STORE = VALUE_ISA_CLOSURE(funcval) ? VALUE_AS_CLOSURE(funcval) : NULL; }
    TIC_GRAVITY_GET_CALLBACK(currentVM->borderFunction, vm, BDR_FN);
    TIC_GRAVITY_GET_CALLBACK(currentVM->bootFunction, vm, BOOT_FN);
    TIC_GRAVITY_GET_CALLBACK(currentVM->menuFunction, vm, MENU_FN);
    TIC_GRAVITY_GET_CALLBACK(currentVM->scanlineFunction, vm, SCN_FN);
    TIC_GRAVITY_GET_CALLBACK(currentVM->tickFunction, vm, TIC_FN);
#undef TIC_GRAVITY_GET_CALLBACK

    return true;
}

static void callGravityTick(tic_mem* tic)
{
    GRAVITYVM* currentVM = ((tic_core*)tic)->currentVM;
    if (currentVM->tickFunction)
    {
        gravity_vm_loadclosure(currentVM->vm, currentVM->tickFunction);
        gravity_vm_runclosure(currentVM->vm, currentVM->tickFunction, VALUE_FROM_NULL, NULL, 0);
    }
}

static void callGravityBoot(tic_mem* tic)
{
    GRAVITYVM* currentVM = ((tic_core*)tic)->currentVM;
    if (currentVM->bootFunction)
    {
        gravity_vm_loadclosure(currentVM->vm, currentVM->bootFunction);
        gravity_vm_runclosure(currentVM->vm, currentVM->bootFunction, VALUE_FROM_NULL, NULL, 0);
    }
}

static void callGravityScanline(tic_mem* tic, s32 row, void* data)
{
    GRAVITYVM* currentVM = ((tic_core*)tic)->currentVM;
    if (currentVM->scanlineFunction)
    {
        gravity_vm_loadclosure(currentVM->vm, currentVM->scanlineFunction);
        gravity_value_t params[1] = { VALUE_FROM_INT(row) };
        gravity_vm_runclosure(currentVM->vm, currentVM->scanlineFunction, VALUE_FROM_NULL, params, 1);
    }
}

static void callGravityBorder(tic_mem* tic, s32 row, void* data)
{
    GRAVITYVM* currentVM = ((tic_core*)tic)->currentVM;
    if (currentVM->borderFunction)
    {
        gravity_vm_loadclosure(currentVM->vm, currentVM->borderFunction);
        gravity_value_t params[1] = { VALUE_FROM_INT(row) };
        gravity_vm_runclosure(currentVM->vm, currentVM->borderFunction, VALUE_FROM_NULL, params, 1);
    }
}

static void callGravityMenu(tic_mem* tic, s32 index, void* data)
{
    GRAVITYVM* currentVM = ((tic_core*)tic)->currentVM;
    if (currentVM->menuFunction)
    {
        gravity_vm_loadclosure(currentVM->vm, currentVM->menuFunction);
        gravity_value_t params[1] = { VALUE_FROM_INT(index) };
        gravity_vm_runclosure(currentVM->vm, currentVM->menuFunction, VALUE_FROM_NULL, params, 1);
    }
}

static const tic_outline_item* getGravityOutline(const char* code, s32* size)
{
    // TODO
    return NULL;
}

static void evalGravity(tic_mem* tic, const char* code)
{
    // TODO
}

// endregion

// region Script Config

static const char* const GravityKeywords [] =
{
"if","in","or","is","for","var","and","not","func","else","true","enum","case","null","file","lazy","super",
"false","break","while","class","const","event","_func","_args","struct","repeat","switch","return","public",
"static","default","private","continue","internal","undefined",
    // "import","extern","module", // these shouldn't be available in TIC-80
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

TIC_EXPORT const tic_script EXPORT_SCRIPT(Gravity) =
{
    .id                 = 22,
    .name               = "gravity",
    .fileExtension      = ".gravity",
    .projectComment     = "//",
    {
        .init               = initGravity,
        .close              = closeGravity,
        .tick               = callGravityTick,
        .boot               = callGravityBoot,

        .callback           =
        {
            .scanline       = callGravityScanline,
            .border         = callGravityBorder,
            .menu           = callGravityMenu,
          },
        },

        .getOutline         = getGravityOutline,
        .eval               = evalGravity,

        .blockCommentStart  = "/*",
        .blockCommentEnd    = "*/",
        .blockCommentStart2 = NULL,
        .blockCommentEnd2   = NULL,
        .singleComment      = "//",
        .blockStringStart   = "@\"",
        .blockStringEnd     = "\"",
        .blockEnd           = "}",
        .stdStringStartEnd  = "\'\"",

        .keywords           = GravityKeywords,
        .keywordsCount      = COUNT_OF(GravityKeywords),

        .demo = {DemoRom, sizeof DemoRom},
        .mark = {MarkRom, sizeof MarkRom, "gravitymark.tic"},
    };

// endregion
