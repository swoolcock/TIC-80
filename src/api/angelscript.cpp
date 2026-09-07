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
// static void asMessageCallback(asIScriptGeneric* gen)
{
    tic_core* core = static_cast<tic_core*>(param);

    const char *type = "ERR ";
    if( msg->type == asMSGTYPE_WARNING )
        type = "WARN";
    else if( msg->type == asMSGTYPE_INFORMATION )
        type = "INFO";

    char buffer[16384];
    sprintf(buffer, "%s (%d, %d) : %s : %s", msg->section, msg->row, msg->col, type, msg->message);
    core->data->error(core->data->data, buffer);
}

// static void as_cls(uint8 color)
static void as_cls(asIScriptGeneric* gen)
{
    asIScriptContext* ctx = asGetActiveContext();
    tic_core* core = static_cast<tic_core*>(ctx->GetUserData());
    tic_mem* mem = (tic_mem*)core;

    const u8 color = gen->GetArgByte(0);
    core->api.cls(mem, color);
}

// static bool as_btn(int button)
static void as_btn(asIScriptGeneric* gen)
{
    asIScriptContext* ctx = asGetActiveContext();
    tic_core* core = static_cast<tic_core*>(ctx->GetUserData());
    tic_mem* mem = (tic_mem*)core;

    const s32 button = (s32)gen->GetArgDWord(0);
    bool rv = core->api.btn(mem, button) != 0;
    *(bool*)gen->GetAddressOfReturnLocation() = rv;
}

// void spr(int id, int x, int y, int colorkey, int scale, uint8 flip, uint8 rotate, int w, int h)
static void as_spr(asIScriptGeneric* gen)
{
    asIScriptContext* ctx = asGetActiveContext();
    tic_core* core = static_cast<tic_core*>(ctx->GetUserData());
    tic_mem* mem = (tic_mem*)core;

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

// void print(const string& in text, int x, int y, uint8 color, bool fixed, int scale, bool alt)
static void as_print(asIScriptGeneric* gen)
{
    asIScriptContext* ctx = asGetActiveContext();
    tic_core* core = static_cast<tic_core*>(ctx->GetUserData());
    tic_mem* mem = (tic_mem*)core;

    const string* text = static_cast<const string*>(gen->GetArgObject(0));
    const s32 x = (s32)gen->GetArgDWord(1);
    const s32 y = (s32)gen->GetArgDWord(2);
    const u8 color = gen->GetArgByte(3);
    const bool fixed = *(bool*)gen->GetAddressOfArg(4);
    const s32 scale = (s32)gen->GetArgDWord(5);
    const bool alt = *(bool*)gen->GetAddressOfArg(6);

    core->api.print(mem, text->c_str(), x, y, color, fixed, scale, alt);
}

static void initAPI(tic_core* core)
{
    ANGELSCRIPTVM* vm = static_cast<ANGELSCRIPTVM*>(core->currentVM);
    int r = 0;

    r = vm->engine->SetMessageCallback(asFUNCTION(asMessageCallback), core, asCALL_CDECL);
    assert(r >= 0);

    REGISTER_TIC(vm, as_cls, "void cls(uint8)");
    REGISTER_TIC(vm, as_btn, "bool btn(int)");
    REGISTER_TIC(vm, as_spr, "void spr(int id, int x, int y, int colorkey=-1, int scale=1, uint8 flip=0, uint8 rotate=0, int w=1, int h=1)");
    REGISTER_TIC(vm, as_spr, "void spr(int id, int x, int y, const array<int>@ colorkey, int scale=1, uint8 flip=0, uint8 rotate=0, int w=1, int h=1)");
    REGISTER_TIC(vm, as_print, "void print(const string& in, int, int, uint8, bool, int, bool)");
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