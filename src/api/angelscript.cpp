//
// Created by samah on 5/09/2026.
//
#include "core/core.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "tools.h"
#include "angelscript.h"

#include "scriptany.h"
#include "scriptarray.h"
#include "scriptdictionary.h"
#include "scriptgrid.h"
#include "scripthandle.h"
#include "scriptmath.h"
#include "scriptstdstring.h"
#include "weakref.h"

#include <assert.h>
#include <inttypes.h>

#include "scriptmathcomplex.h"

#include <string>

typedef std::string string;

#define REGISTER_TIC(vm, name, decl) \
    { \
        int _result = (vm)->engine->RegisterGlobalFunction( \
            (decl), \
            asFUNCTION(name), \
            asCALL_GENERIC \
        ); \
        assert(_result >= 0); \
    }

#define GET_TIC_CORE(CTX, CORE, MEM) \
    asIScriptContext* CTX = asGetActiveContext(); \
    tic_core* CORE = static_cast<tic_core*>(CTX->GetUserData()); \
    tic_mem* MEM = (tic_mem*)CORE;

typedef struct
{
    asIScriptEngine* engine;
    asIScriptContext* context;
    asIScriptModule* module;
    asIScriptFunction* tickFunction;
    asIScriptFunction* bootFunction;
} ANGELSCRIPTVM;

extern bool parse_note(const char* noteStr, s32* note, s32* octave);

static asIScriptModule* asCompileModule(asIScriptEngine* engine, const char* code)
{
    asIScriptModule* module = engine->GetModule("tic", asGM_ALWAYS_CREATE);

    int r = module->AddScriptSection("tic.as", code, strlen(code));
    if (r < 0) return nullptr;

    r = module->Build();
    if (r < 0) return nullptr;

    return module;
}

static void asMessageCallback(const asSMessageInfo *msg, void *param)
{
    tic_core* core = static_cast<tic_core*>(param);

    const char *type = "ERR ";
    if( msg->type == asMSGTYPE_WARNING )
        type = "WARN";
    else if( msg->type == asMSGTYPE_INFORMATION )
        type = "INFO";

    char buf[1024];
    snprintf(buf, sizeof(buf), "%s (%d, %d) : %s : %s", msg->section, msg->row, msg->col, type, msg->message);
    core->data->error(core->data->data, buf);
}

static string asToString(asIScriptGeneric* gen, asUINT arg)
{
    const int typeId = gen->GetArgTypeId(arg);
    void *value = gen->GetArgAddress(arg);
    const int baseTypeId = typeId & asTYPEID_MASK_SEQNBR;

    switch (baseTypeId)
    {
    case asTYPEID_BOOL:
        return *(bool*)value ? "true" : "false";
    case asTYPEID_INT8:
        return std::to_string(*(s8*)value);
    case asTYPEID_INT16:
        return std::to_string(*(s16*)value);
    case asTYPEID_INT32:
        return std::to_string(*(s32*)value);
    case asTYPEID_UINT8:
        return std::to_string(*(u8*)value);
    case asTYPEID_UINT16:
        return std::to_string(*(u16*)value);
    case asTYPEID_UINT32:
        return std::to_string(*(u32*)value);
    case asTYPEID_INT64:
        return std::to_string(*(s64*)value);
    case asTYPEID_UINT64:
        return std::to_string(*(u64*)value);
    case asTYPEID_FLOAT:
        {
            char buf[64];
            snprintf(buf, sizeof(buf), "%g", *(float*)value);
            return buf;
        }
    case asTYPEID_DOUBLE:
        {
            char buf[64];
            snprintf(buf, sizeof(buf), "%g", *(double*)value);
            return buf;
        }
    default:
        break;
    }

    asITypeInfo *type = gen->GetEngine()->GetTypeInfoById(typeId);

    if (!type) return "<unknown>";

    const char *typeName = type->GetName();

    if (strcmp(typeName, "string") == 0)
        return *(string*)value;

    {
        char buf[128];
        snprintf(buf, sizeof(buf), "%s 0x%" PRIxPTR, typeName, (uintptr_t)value);
        return buf;
    }
}

static void as_peek(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_poke(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_peek1(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_poke1(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_peek2(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_poke2(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_peek4(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_poke4(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

// static void as_cls(uint8 color)
static void as_cls(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const u8 color = gen->GetArgByte(0);
    core->api.cls(mem, color);
}

static void as_pix(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_line(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_rect(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_rectb(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_circ(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_circb(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_elli(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_ellib(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_paint(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_tri(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_trib(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_ttri(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_clip(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_btnp(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_btn(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 button = (s32)gen->GetArgDWord(0);
    bool rv = core->api.btn(mem, button) != 0;
    *(bool*)gen->GetAddressOfReturnLocation() = rv;
}

static void as_spr(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 id = (s32)gen->GetArgDWord(0);
    const s32 x = (s32)gen->GetArgDWord(1);
    const s32 y = (s32)gen->GetArgDWord(2);
    const s32 scale = (s32)gen->GetArgDWord(4);
    const u8 flip = gen->GetArgByte(5);
    const u8 rotate = gen->GetArgByte(6);
    const s32 w = (s32)gen->GetArgDWord(7);
    const s32 h = (s32)gen->GetArgDWord(8);

    u8 colors[TIC_PALETTE_SIZE];
    int colors_count = 1;

    if (gen->GetArgTypeId(3) == asTYPEID_INT32)
    {
        colors[0] = (u8)gen->GetArgDWord(3);
    }
    else
    {
        const CScriptArray* colorkeys = static_cast<const CScriptArray*>(gen->GetArgObject(3));
        for (asUINT i = 0; i < colorkeys->GetSize() && i < TIC_PALETTE_SIZE; i++)
        {
            asDWORD color = *(asDWORD*)colorkeys->At(i);
            colors[i] = (u8)color;
            colors_count++;
        }
        colorkeys->Release();
    }

    core->api.spr(mem, id, x, y, w, h, colors, colors_count, scale, tic_flip(flip), tic_rotate(rotate));
}

static void as_mget(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_mset(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_map(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_music(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_sfx(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_vbank(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_sync(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_reset(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_key(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_keyp(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_memcpy(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_memset(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_font(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_print(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    string text = asToString(gen, 0);
    const s32 x = (s32)gen->GetArgDWord(1);
    const s32 y = (s32)gen->GetArgDWord(2);
    const u8 color = gen->GetArgByte(3);
    const bool fixed = *(bool*)gen->GetAddressOfArg(4);
    const s32 scale = (s32)gen->GetArgDWord(5);
    const bool alt = *(bool*)gen->GetAddressOfArg(6);

    s32 width = core->api.print(mem, text.c_str(), x, y, color, fixed, scale, alt);
    *(s32*)gen->GetAddressOfReturnLocation() = width;
}

static void as_trace(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_pmem(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_time(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_tstamp(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_exit(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_mouse(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    s32* x = (s32*)gen->GetArgAddress(0);
    s32* y = (s32*)gen->GetArgAddress(1);
    bool* left = (bool*)gen->GetArgAddress(1);
    bool* middle = (bool*)gen->GetArgAddress(1);
    bool* right = (bool*)gen->GetArgAddress(1);
    s16* scrollx = (s16*)gen->GetArgAddress(1);
    s16* scrolly = (s16*)gen->GetArgAddress(1);

    const tic80_mouse* mouse = &core->memory.ram->input.mouse;
    tic_point pt = core->api.mouse(mem);

    if (x) *x = pt.x;
    if (y) *y = pt.y;
    if (left) *left = (bool)mouse->left;
    if (middle) *middle = (bool)mouse->middle;
    if (right) *right = (bool)mouse->right;
    if (scrollx) *scrollx = mouse->scrollx;
    if (scrolly) *scrolly = mouse->scrolly;
}

static void as_fget(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_fset(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_fft(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void as_ffts(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);
}

static void initAPI(tic_core* core)
{
    ANGELSCRIPTVM* vm = static_cast<ANGELSCRIPTVM*>(core->currentVM);
    int r = 0;

    r = vm->engine->SetMessageCallback(asFUNCTION(asMessageCallback), core, asCALL_CDECL);
    assert(r >= 0);

    REGISTER_TIC(vm, as_peek, "uint8 peek(int address, int bits=8)");
    REGISTER_TIC(vm, as_poke, "void poke(int address, uint8 value, int bits=8)");
    REGISTER_TIC(vm, as_peek1, "uint8 peek1(int address)");
    REGISTER_TIC(vm, as_poke1, "void poke1(int address, uint8 value)");
    REGISTER_TIC(vm, as_peek2, "uint8 peek2(int address)");
    REGISTER_TIC(vm, as_poke2, "void poke2(int address, uint8 value)");
    REGISTER_TIC(vm, as_peek4, "uint8 peek4(int address)");
    REGISTER_TIC(vm, as_poke4, "void poke4(int address, uint8 value)");
    REGISTER_TIC(vm, as_cls, "void cls(uint8)");
    REGISTER_TIC(vm, as_pix, "void pix(int x, int y, uint8 color)");
    REGISTER_TIC(vm, as_pix, "uint8 pix(int x, int y)");
    REGISTER_TIC(vm, as_line, "void line(float x0, float y0, float x1, float y1, uint8 color)");
    REGISTER_TIC(vm, as_rect, "void rect(int x, int y, int w, int h, uint8 color)");
    REGISTER_TIC(vm, as_rectb, "void rectb(int x, int y, int w, int h, uint8 color)");
    REGISTER_TIC(vm, as_circ, "void circ(int x, int y, int radius, uint8 color)");
    REGISTER_TIC(vm, as_circb, "void circb(int x, int y, int radius, uint8 color)");
    REGISTER_TIC(vm, as_elli, "void elli(int x, int y, int a, int b, uint8 color)");
    REGISTER_TIC(vm, as_ellib, "void ellib(int x, int y, int a, int b, uint8 color)");
    // REGISTER_TIC(vm, as_paint, "");
    REGISTER_TIC(vm, as_tri, "void tri(float x1, float y1, float x2, float y2, float x3, float y3, uint8 color)");
    REGISTER_TIC(vm, as_trib, "void trib(float x1, float y1, float x2, float y2, float x3, float y3, uint8 color)");
    REGISTER_TIC(vm, as_ttri, "void ttri(float x1, float y1, float x2, float y2, float x3, float y3, float u1, float v1, float u2, float v2, float u3, float v3)");
    REGISTER_TIC(vm, as_ttri, "void ttri(float x1, float y1, float x2, float y2, float x3, float y3, float u1, float v1, float u2, float v2, float u3, float v3, int texsrc)");
    REGISTER_TIC(vm, as_ttri, "void ttri(float x1, float y1, float x2, float y2, float x3, float y3, float u1, float v1, float u2, float v2, float u3, float v3, int texsrc, uint8 chromakey, float z1=0, float z2=0, float z3=0)");
    REGISTER_TIC(vm, as_ttri, "void ttri(float x1, float y1, float x2, float y2, float x3, float y3, float u1, float v1, float u2, float v2, float u3, float v3, int texsrc, const array<int>@ chromakey, float z1=0, float z2=0, float z3=0)");
    REGISTER_TIC(vm, as_clip, "void clip()");
    REGISTER_TIC(vm, as_clip, "void clip(int x, int y, int width, int height)");
    REGISTER_TIC(vm, as_btnp, "bool btnp(int id, int hold=-1, int period=-1)");
    REGISTER_TIC(vm, as_btn, "bool btn(int id)");
    REGISTER_TIC(vm, as_spr, "void spr(int id, int x, int y, int colorkey=-1, int scale=1, uint8 flip=0, uint8 rotate=0, int w=1, int h=1)");
    REGISTER_TIC(vm, as_spr, "void spr(int id, int x, int y, const array<int>@ colorkey, int scale=1, uint8 flip=0, uint8 rotate=0, int w=1, int h=1)");
    REGISTER_TIC(vm, as_mget, "uint8 mget(int x, int y)");
    REGISTER_TIC(vm, as_mset, "void mset(int x, int y, uint8 tile_id)");
    // REGISTER_TIC(vm, as_map, ""); // TODO: map requires remap callback
    REGISTER_TIC(vm, as_music, "void music(int track=-1, int frame=-1, int row=-1, bool loop=true, bool sustain=false, int tempo=-1, int speed=-1)");
    REGISTER_TIC(vm, as_sfx, "void sfx(int id, int note=-1, int duration=-1, int channel=0, int volume=15, int speed=0)");
    REGISTER_TIC(vm, as_vbank, "int vbank(int bank)");
    REGISTER_TIC(vm, as_vbank, "int vbank()");
    REGISTER_TIC(vm, as_sync, "void sync(uint mask=0, int bank=0, bool tocart=false)");
    REGISTER_TIC(vm, as_reset, "void reset()");
    REGISTER_TIC(vm, as_key, "bool key(int code=-1)");
    REGISTER_TIC(vm, as_keyp, "bool keyp(int code=-1, int hold=-1, int period=-1)");
    REGISTER_TIC(vm, as_memcpy, "void memcpy(int dest, int source, int size)");
    REGISTER_TIC(vm, as_memset, "void memset(int dest, uint8 value, int size)");
    REGISTER_TIC(vm, as_font, "int font(const ?&in text, int x, int y, int chromakey, int char_width, int char_height, bool fixed=false, int scale=1, bool alt=false)");
    REGISTER_TIC(vm, as_print, "int print(const ?&in text, int x=0, int y=0, uint8 color=15, bool fixed=false, int scale=1, bool smallfont=false)");
    REGISTER_TIC(vm, as_trace, "void trace(const ?&in text, uint8 color=15)");
    REGISTER_TIC(vm, as_pmem, "void pmem(int index, uint value)");
    REGISTER_TIC(vm, as_pmem, "uint pmem(int index)");
    REGISTER_TIC(vm, as_time, "double time()");
    REGISTER_TIC(vm, as_tstamp, "int tstamp()");
    REGISTER_TIC(vm, as_exit, "void exit()");
    REGISTER_TIC(vm, as_mouse, "void mouse(int &out x, int &out y, bool &out left=void, bool &out middle=void, bool &out right=void, int &out scrollx=void, int &out scrolly=void)");
    REGISTER_TIC(vm, as_fget, "bool fget(int sprite_id, uint8 flag)");
    REGISTER_TIC(vm, as_fset, "void fset(int sprite_id, uint8 flag, bool value)");
    // REGISTER_TIC(vm, as_fft, "");
    // REGISTER_TIC(vm, as_ffts, "");
}

static void closeAngelScript(tic_mem* tic)
{
    tic_core* core = (tic_core*)tic;

    if (core->currentVM)
    {
        ANGELSCRIPTVM* vm = static_cast<ANGELSCRIPTVM*>(core->currentVM);
        if (vm->context) vm->context->Release();
        if (vm->engine) vm->engine->ShutDownAndRelease();
        core->currentVM = nullptr;
        delete vm;
    }
}

static bool initAngelScript(tic_mem* tic, const char* code)
{
    tic_core* core = (tic_core*)tic;

    closeAngelScript(tic);

    ANGELSCRIPTVM* vm = new ANGELSCRIPTVM{};
    core->currentVM = vm;

    vm->engine = asCreateScriptEngine();
    if (!vm->engine)
    {
        core->data->error(core->data->data, "Couldn't create engine.");
        return false;
    }

    // RegisterScriptAny(vm->engine);
    RegisterScriptArray(vm->engine, true);
    // RegisterScriptDictionary(vm->engine);
    // RegisterScriptGrid(vm->engine);
    // RegisterScriptHandle(vm->engine);
    RegisterScriptMath(vm->engine);
    // RegisterScriptMathComplex(vm->engine);
    RegisterStdString(vm->engine);
    // RegisterStdStringUtils(vm->engine);
    // RegisterScriptWeakRef(vm->engine);

    vm->context = vm->engine->CreateContext();
    if (!vm->context)
    {
        core->data->error(core->data->data, "Couldn't create context.");
        return false;
    }

    vm->context->SetUserData(core);

    initAPI(core);

    vm->module = asCompileModule(vm->engine, code);
    if (!vm->module)
    {
        core->data->error(core->data->data, "Module failed compilation.");
        return false;
    }

    vm->bootFunction = vm->module->GetFunctionByDecl("void BOOT()");
    vm->tickFunction = vm->module->GetFunctionByDecl("void TIC()");

    return true;
}

static void callAngelScriptTick(tic_mem* tic)
{
    tic_core* core = (tic_core*)tic;

    ANGELSCRIPTVM* vm = static_cast<ANGELSCRIPTVM*>(core->currentVM);
    if (vm)
    {
        if (vm->tickFunction)
        {
            int r = vm->context->Prepare(vm->tickFunction);
            assert(r >= 0);

            vm->context->Execute();
        }
    }
}

static void callAngelScriptBoot(tic_mem* tic)
{
    tic_core* core = (tic_core*)tic;

    ANGELSCRIPTVM* vm = static_cast<ANGELSCRIPTVM*>(core->currentVM);
    if (vm)
    {
        if (vm->bootFunction)
        {
            int r = vm->context->Prepare(vm->bootFunction);
            assert(r >= 0);

            vm->context->Execute();
        }
    }
}
static void callAngelScriptScanline(tic_mem* tic, s32 row, void* data) { }
static void callAngelScriptBorder(tic_mem* tic, s32 row, void* data) { }
static void callAngelScriptMenu(tic_mem* tic, s32 index, void* data) { }
static const tic_outline_item* getAngelScriptOutline(const char* code, s32* size) { return NULL; }
static void evalAngelScript(tic_mem* tic, const char* code) { }

static const char* const AngelScriptKeywords [] =
{
"and",
"auto",
"bool",
"break",
"case",
"cast",
"catch",
"class",
"const",
"continue",
"default",
"do",
"double",
"else",
"enum",
"false",
"float",
"for",
"foreach",
"funcdef",
"if",
"import",
"in",
"inout",
"int",
"interface",
"int8",
"int16",
"int32",
"int64",
"is",
"mixin",
"namespace",
"not",
"null",
"or",
"out",
"private",
"protected",
"return",
"switch",
"true",
"try",
"typedef",
"uint",
"uint8",
"uint16",
"uint32",
"uint64",
"using",
"void",
"while",
"xor",
"abstract",
"delete",
"explicit",
"external",
"final",
"from",
"function",
"get",
"override",
"property",
"set",
"shared",
"super",
"this",
};

static const u8 DemoRom[] =
{
#include "../build/assets/angelscriptdemo.tic.dat"
};

static const u8 MarkRom[] =
{
// #include "../build/assets/angelscriptmark.tic.dat"
};

TIC_EXPORT extern const tic_script EXPORT_SCRIPT(AngelScript) =
{
    .id                 = 21,
    .name               = "angelscript",
    .fileExtension      = ".as",
    .projectComment     = "//",
    {
        .init           = initAngelScript,
        .close          = closeAngelScript,
        .tick           = callAngelScriptTick,
        .boot           = callAngelScriptBoot,

        .callback =
        {
            .scanline   = callAngelScriptScanline,
            .border     = callAngelScriptBorder,
            .menu       = callAngelScriptMenu,
        },
    },

    .getOutline         = getAngelScriptOutline,
    .eval               = evalAngelScript,

    .blockCommentStart  = "/*",
    .blockCommentEnd    = "*/",
    .blockCommentStart2 = NULL,
    .blockCommentEnd2   = NULL,
    .blockStringStart   = NULL,
    .blockStringEnd     = NULL,
    .singleComment      = "//",
    .blockEnd           = "}",

    .keywords           = AngelScriptKeywords,
    .keywordsCount      = COUNT_OF(AngelScriptKeywords),

    .demo = {DemoRom, sizeof DemoRom},
    .mark = {MarkRom, sizeof MarkRom, "angelscriptmark.tic"},
};