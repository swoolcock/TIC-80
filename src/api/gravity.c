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

// region API

static bool grav_btn(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_btnp(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_circ(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_circb(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_clip(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}

static bool grav_cls(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex)
{
    TIC_GRAVITY_GET_CORE(vm, mem, core);

    u8 cls_color = (u8)VALUE_AS_INT(args[1]);
    core->api.cls(mem, cls_color);

    RETURN_NOVALUE();
}

static bool grav_elli(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_ellib(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_exit(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_fget(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_font(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_fset(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_key(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_keyp(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_line(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
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
static bool grav_rect(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
static bool grav_rectb(gravity_vm *vm, gravity_value_t *args, uint16_t nargs, uint32_t rindex){}
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
