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

#include <assert.h>
// #include "angelscript_wrapper.h"

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

    char buffer[16384];
    sprintf(buffer, "%s (%d, %d) : %s : %s", msg->section, msg->row, msg->col, type, msg->message);
    core->data->error(core->data->data, buffer);
}

static void as_cls(int color)
{
    asIScriptContext* ctx = asGetActiveContext();
    tic_core* core = static_cast<tic_core*>(ctx->GetUserData());
    tic_mem* mem = (tic_mem*)core;

    core->api.cls(mem, (u8)color);
}

static bool as_btn(int button)
{
    asIScriptContext* ctx = asGetActiveContext();
    tic_core* core = static_cast<tic_core*>(ctx->GetUserData());
    tic_mem* mem = (tic_mem*)core;

    return core->api.btn(mem, (s32)button) != 0;
}

static void as_spr(int id, int x, int y, int colorkey, int scale, int flip, int rotate, int w, int h)
{
    asIScriptContext* ctx = asGetActiveContext();
    tic_core* core = static_cast<tic_core*>(ctx->GetUserData());
    tic_mem* mem = (tic_mem*)core;

    u8 colors[TIC_PALETTE_SIZE];
    colors[0] = colorkey;
    int colors_count = 1;

    core->api.spr(mem, id, x, y, w, h, colors, colors_count, scale, tic_flip(flip), tic_rotate(rotate));
}

static void initAPI(tic_core* core)
{
    ANGELSCRIPTVM* vm = static_cast<ANGELSCRIPTVM*>(core->currentVM);
    int r = 0;

    r = vm->engine->SetMessageCallback(asFUNCTION(asMessageCallback), core, asCALL_CDECL);
    assert(r >= 0);

    r = vm->engine->RegisterGlobalFunction("void cls(int)", asFUNCTION(as_cls), asCALL_CDECL);
    assert(r >= 0);

    r = vm->engine->RegisterGlobalFunction("bool btn(int)", asFUNCTION(as_btn), asCALL_CDECL);
    assert(r >= 0);

    r = vm->engine->RegisterGlobalFunction("void spr(int, int, int, int, int, int, int, int, int)", asFUNCTION(as_spr), asCALL_CDECL);
    assert(r >= 0);
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
    vm->engine = asCreateScriptEngine();
    core->currentVM = vm;

    vm->context = vm->engine->CreateContext();
    vm->context->SetUserData(core);

    initAPI(core);

    vm->module = asCompileModule(vm->engine, code);

    if (!vm->module)
    {
        if (core->data)
        {
            core->data->error(core->data->data, "Module failed compilation.");
        }

        return false;
    }

    vm->bootFunction = vm->module->GetFunctionByDecl("void BOOT()");
    vm->tickFunction = vm->module->GetFunctionByDecl("void TIC()");

    // if (!module)
    // {
    //     core->data->error(core->data->data, "error building module");
    //     return false;
    // }

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
            vm->context->Prepare(vm->tickFunction);
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
            vm->context->Prepare(vm->bootFunction);
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