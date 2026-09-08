//
// Created by samah on 5/09/2026.
//
#include "core/core.h"

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <string>

#include "tools.h"
#include "angelscript.h"

#include "scriptany.h"
#include "scriptarray.h"
#include "scriptdictionary.h"
// #include "scriptgrid.h"
#include "scripthandle.h"
#include "scripthelper.h"
#include "scriptmath.h"
// #include "scriptmathcomplex.h"
#include "scriptstdstring.h"
#include "weakref.h"

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
    asIScriptFunction* borderFunction;
    asIScriptFunction* bootFunction;
    asIScriptFunction* menuFunction;
    asIScriptFunction* scanlineFunction;
    asIScriptFunction* tickFunction;
} ANGELSCRIPTVM;

extern "C" {
    extern bool parse_note(const char* noteStr, s32* note, s32* octave);
}

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

static void asExceptionCallback(asIScriptContext *ctx, void *param)
{
    // if the exception is caught, do nothing
    if (ctx->WillExceptionBeCaught()) return;

    tic_core* core = static_cast<tic_core*>(param);
    const asIScriptFunction *func = ctx->GetExceptionFunction();

    char buf[2048];
    snprintf(buf, sizeof(buf),
        "desc: %s\nfunc: %s\nline: %d\n",
        ctx->GetExceptionString(),
        func->GetDeclaration(),
        ctx->GetExceptionLineNumber());
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

    const s32 address = (s32)gen->GetArgDWord(0);
    const s32 bits = (s32)gen->GetArgDWord(1);

    u8 rv = core->api.peek(mem, address, bits);
    gen->SetReturnByte(rv);
}

static void as_poke(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 address = (s32)gen->GetArgDWord(0);
    const u8 value = gen->GetArgByte(1);
    const s32 bits = (s32)gen->GetArgDWord(2);

    core->api.poke(mem, address, value, bits);
}

static void as_peek1(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 address = (s32)gen->GetArgDWord(0);

    u8 rv = core->api.peek1(mem, address);
    gen->SetReturnByte(rv);
}

static void as_poke1(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 address = (s32)gen->GetArgDWord(0);
    const u8 value = gen->GetArgByte(1);

    core->api.poke1(mem, address, value);
}

static void as_peek2(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 address = (s32)gen->GetArgDWord(0);

    u8 rv = core->api.peek2(mem, address);
    gen->SetReturnByte(rv);
}

static void as_poke2(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 address = (s32)gen->GetArgDWord(0);
    const u8 value = gen->GetArgByte(1);

    core->api.poke2(mem, address, value);
}

static void as_peek4(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 address = (s32)gen->GetArgDWord(0);

    u8 rv = core->api.peek4(mem, address);
    gen->SetReturnByte(rv);
}

static void as_poke4(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 address = (s32)gen->GetArgDWord(0);
    const u8 value = gen->GetArgByte(1);

    core->api.poke4(mem, address, value);
}

static void as_cls(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const u8 color = gen->GetArgByte(0);
    core->api.cls(mem, color);
}

static void as_pix(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 x = (s32)gen->GetArgDWord(0);
    const s32 y = (s32)gen->GetArgDWord(1);

    if (gen->GetArgCount() == 2)
    {
        u8 rv = core->api.pix(mem, x, y, 0, true);
        gen->SetReturnByte(rv);
    }
    else
    {
        const u8 color = gen->GetArgByte(2);
        core->api.pix(mem, x, y, color, false);
    }
}

static void as_line(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const float x0 = gen->GetArgFloat(0);
    const float y0 = gen->GetArgFloat(1);
    const float x1 = gen->GetArgFloat(2);
    const float y1 = gen->GetArgFloat(3);
    const u8 color = gen->GetArgByte(4);

    core->api.line(mem, x0, y0, x1, y1, color);
}

static void as_rect(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 x = (s32)gen->GetArgDWord(0);
    const s32 y = (s32)gen->GetArgDWord(1);
    const s32 w = (s32)gen->GetArgDWord(2);
    const s32 h = (s32)gen->GetArgDWord(3);
    const u8 color = gen->GetArgByte(4);

    core->api.rect(mem, x, y, w, h, color);
}

static void as_rectb(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 x = (s32)gen->GetArgDWord(0);
    const s32 y = (s32)gen->GetArgDWord(1);
    const s32 w = (s32)gen->GetArgDWord(2);
    const s32 h = (s32)gen->GetArgDWord(3);
    const u8 color = gen->GetArgByte(4);

    core->api.rectb(mem, x, y, w, h, color);
}

static void as_circ(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 x = (s32)gen->GetArgDWord(0);
    const s32 y = (s32)gen->GetArgDWord(1);
    const s32 radius = (s32)gen->GetArgDWord(2);
    const u8 color = gen->GetArgByte(3);

    core->api.circ(mem, x, y, radius, color);
}

static void as_circb(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 x = (s32)gen->GetArgDWord(0);
    const s32 y = (s32)gen->GetArgDWord(1);
    const s32 radius = (s32)gen->GetArgDWord(2);
    const u8 color = gen->GetArgByte(3);

    core->api.circb(mem, x, y, radius, color);
}

static void as_elli(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 x = (s32)gen->GetArgDWord(0);
    const s32 y = (s32)gen->GetArgDWord(1);
    const s32 a = (s32)gen->GetArgDWord(2);
    const s32 b = (s32)gen->GetArgDWord(3);
    const u8 color = gen->GetArgByte(4);

    core->api.elli(mem, x, y, a, b, color);
}

static void as_ellib(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 x = (s32)gen->GetArgDWord(0);
    const s32 y = (s32)gen->GetArgDWord(1);
    const s32 a = (s32)gen->GetArgDWord(2);
    const s32 b = (s32)gen->GetArgDWord(3);
    const u8 color = gen->GetArgByte(4);

    core->api.ellib(mem, x, y, a, b, color);
}

static void as_paint(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    // TODO: remove paint?
}

static void as_tri(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const float x1 = gen->GetArgFloat(0);
    const float y1 = gen->GetArgFloat(1);
    const float x2 = gen->GetArgFloat(2);
    const float y2 = gen->GetArgFloat(3);
    const float x3 = gen->GetArgFloat(4);
    const float y3 = gen->GetArgFloat(5);
    const u8 color = gen->GetArgByte(6);

    core->api.tri(mem, x1, y1, x2, y2, x3, y3, color);
}

static void as_trib(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const float x1 = gen->GetArgFloat(0);
    const float y1 = gen->GetArgFloat(1);
    const float x2 = gen->GetArgFloat(2);
    const float y2 = gen->GetArgFloat(3);
    const float x3 = gen->GetArgFloat(4);
    const float y3 = gen->GetArgFloat(5);
    const u8 color = gen->GetArgByte(6);

    core->api.trib(mem, x1, y1, x2, y2, x3, y3, color);
}

static void as_ttri(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    float pt[12];
    for (int i = 0; i < COUNT_OF(pt); i++)
        pt[i] = gen->GetArgFloat(i);

    static u8 colors[TIC_PALETTE_SIZE];
    s32 count = 0;
    tic_texture_src src = tic_tiles_texture;

    // check for texture src
    if (gen->GetArgCount() > 12)
    {
        src = (tic_texture_src)gen->GetArgByte(12);
    }

    // check for chroma
    if (gen->GetArgCount() > 13)
    {
        if (gen->GetArgTypeId(13) == asTYPEID_INT32)
        {
            colors[0] = (u8)gen->GetArgDWord(13);
            count = 1;
        }
        else
        {
            const CScriptArray* colorkeys = static_cast<const CScriptArray*>(gen->GetArgObject(13));
            for (asUINT i = 0; i < colorkeys->GetSize() && i < TIC_PALETTE_SIZE; i++)
            {
                colors[i] = (u8)*(asDWORD*)colorkeys->At(i);
                count++;
            }
            colorkeys->Release();
        }
    }

    float z[3] = {0, 0, 0};
    bool depth = false;

    if (gen->GetArgCount() == 17)
    {
        for (s32 i = 0; i < COUNT_OF(z); i++)
            z[i] = gen->GetArgFloat(i + 14);

        depth = true;
    }

    core->api.ttri(mem, pt[0], pt[1],   //  xy 1
                        pt[2], pt[3],   //  xy 2
                        pt[4], pt[5],   //  xy 3
                        pt[6], pt[7],   //  uv 1
                        pt[8], pt[9],   //  uv 2
                        pt[10], pt[11], //  uv 3
                        src,            // texture source
                        colors, count,  // chroma
                        z[0], z[1], z[2], depth); // depth
}

static void as_clip(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    if (gen->GetArgCount() == 0)
    {
        core->api.clip(mem, 0, 0, TIC80_WIDTH, TIC80_HEIGHT);
    }
    else
    {
        const s32 x = (s32)gen->GetArgDWord(0);
        const s32 y = (s32)gen->GetArgDWord(1);
        const s32 width = (s32)gen->GetArgDWord(2);
        const s32 height = (s32)gen->GetArgDWord(3);
        core->api.clip(mem, x, y, width, height);
    }
}

static void as_btnp(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    if (gen->GetArgCount() == 0)
    {
        bool rv = core->api.btnp(mem, -1, -1, -1);
        gen->SetReturnByte(rv ? 1 : 0);
        return;
    }

    const s32 index = (s32)gen->GetArgDWord(0);
    if (gen->GetArgCount() == 1)
    {
        bool rv = core->api.btnp(mem, index, -1, -1);
        gen->SetReturnByte(rv ? 1 : 0);
        return;
    }

    const s32 hold = (u32)gen->GetArgDWord(1);
    const s32 period = (u32)gen->GetArgDWord(2);

    bool rv = core->api.btnp(mem, index, hold, period);
    gen->SetReturnByte(rv ? 1 : 0);
}

static void as_btn(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 id = (s32)gen->GetArgDWord(0);

    bool rv = core->api.btn(mem, id) != 0;
    gen->SetReturnByte(rv ? 1 : 0);
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

    static u8 colors[TIC_PALETTE_SIZE];
    int colors_count = 0;

    if (gen->GetArgTypeId(3) == asTYPEID_INT32)
    {
        colors[0] = (u8)gen->GetArgDWord(3);
        colors_count = 1;
    }
    else
    {
        const CScriptArray* colorkeys = static_cast<const CScriptArray*>(gen->GetArgObject(3));
        for (asUINT i = 0; i < colorkeys->GetSize() && i < TIC_PALETTE_SIZE; i++)
        {
            colors[i] = (u8)*(asDWORD*)colorkeys->At(i);
            colors_count++;
        }
        colorkeys->Release();
    }

    core->api.spr(mem, id, x, y, w, h, colors, colors_count, scale, (tic_flip)flip, (tic_rotate)rotate);
}

static void as_mget(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 x = (s32)gen->GetArgDWord(0);
    const s32 y = (s32)gen->GetArgDWord(1);

    u8 rv = core->api.mget(mem, x, y);
    gen->SetReturnByte(rv);
}

static void as_mset(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 x = (s32)gen->GetArgDWord(0);
    const s32 y = (s32)gen->GetArgDWord(1);
    const u8 tile_id = gen->GetArgByte(2);

    core->api.mset(mem, x, y, tile_id);
}

typedef struct
{
    ANGELSCRIPTVM* vm;
    asIScriptFunction* remap;
} RemapData;

static void remapCallback(void* data, s32 x, s32 y, RemapResult* result)
{
    RemapData* remap_data = (RemapData*)data;
    ANGELSCRIPTVM* vm = remap_data->vm;
    asIScriptContext* ctx = vm->context;
    asIScriptFunction* remap = remap_data->remap;

    u8 outtile = result->index;
    tic_flip flip = tic_no_flip;
    tic_rotate rotate = tic_no_rotate;

    ctx->PushState();
    ctx->Prepare(remap);
    ctx->SetArgByte(0, result->index);
    ctx->SetArgDWord(1, x);
    ctx->SetArgDWord(2, y);
    ctx->SetArgAddress(3, &outtile);
    ctx->SetArgAddress(4, &flip);
    ctx->SetArgAddress(5, &rotate);
    ctx->Execute();
    ctx->PopState();

    result->index = outtile;
    result->flip = flip;
    result->rotate = rotate;
}

// void map(int x=0, int y=0, int w=30, int h=17, int sx=0, int sy=0, int colorkey=-1, int scale=1, REMAP_CALLBACK remap=null)
static void as_map(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 x = (s32)gen->GetArgDWord(0);
    const s32 y = (s32)gen->GetArgDWord(1);
    const s32 w = (s32)gen->GetArgDWord(2);
    const s32 h = (s32)gen->GetArgDWord(3);
    const s32 sx = (s32)gen->GetArgDWord(4);
    const s32 sy = (s32)gen->GetArgDWord(5);
    s32 scale = 1;
    asIScriptFunction* remap = nullptr;

    static u8 colors[TIC_PALETTE_SIZE];
    int colors_count = 0;

    if (gen->GetArgCount() > 6)
    {
        if (gen->GetArgTypeId(6) == asTYPEID_INT32)
        {
            colors[0] = (u8)gen->GetArgDWord(6);
            colors_count = 1;
        }
        else
        {
            const CScriptArray* colorkeys = static_cast<const CScriptArray*>(gen->GetArgObject(6));
            for (asUINT i = 0; i < colorkeys->GetSize() && i < TIC_PALETTE_SIZE; i++)
            {
                colors[i] = (u8)*(asDWORD*)colorkeys->At(i);
                colors_count++;
            }
            colorkeys->Release();
        }

        scale = (s32)gen->GetArgDWord(7);
        remap = static_cast<asIScriptFunction*>(gen->GetArgObject(8));
    }

    if (remap)
    {
        RemapData data = {(ANGELSCRIPTVM*)core->currentVM, remap};
        core->api.map(mem, x, y, w, h, sx, sy, colors, colors_count, scale, remapCallback, &data);
        remap->Release();
    }
    else
    {
        core->api.map(mem, x, y, w, h, sx, sy, colors, colors_count, scale, NULL, NULL);
    }
}

static void as_music(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 track = (s32)gen->GetArgDWord(0);
    const s32 frame = (s32)gen->GetArgDWord(1);
    const s32 row = (s32)gen->GetArgDWord(2);
    const bool loop = gen->GetArgByte(3) != 0;
    const bool sustain = gen->GetArgByte(4) != 0;
    const s32 tempo = (s32)gen->GetArgDWord(5);
    const s32 speed = (s32)gen->GetArgDWord(6);

    if (track >= MUSIC_TRACKS)
    {
        char buf[128];
        snprintf(buf, sizeof(buf), "invalid music track index %d", track);
        ctx->SetException(buf);
        return;
    }

    core->api.music(mem, track, frame, row, loop, sustain, tempo, speed);
}

static void as_sfx(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 index = (s32)gen->GetArgDWord(0);
    if (index >= SFX_COUNT)
    {
        char buf[128];
        snprintf(buf, sizeof(buf), "unknown sfx index %d", index);
        ctx->SetException(buf);
        return;
    }

    s32 note = -1;
    s32 octave = -1;
    s32 duration = -1;
    s32 channel = 0;
    s32 volumes[TIC80_SAMPLE_CHANNELS] = {MAX_VOLUME, MAX_VOLUME};
    s32 speed = SFX_DEF_SPEED;

    if (index >= 0)
    {
        tic_sample* effect = mem->ram->sfx.samples.data + index;
        note = effect->note;
        octave = effect->octave;
        speed = effect->speed;
    }

    if (gen->GetArgCount() >= 2)
    {
        if (gen->GetArgTypeId(1) == asTYPEID_INT32)
        {
            s32 notearg = (s32)gen->GetArgDWord(1);
            note = notearg % NOTES;
            octave = notearg / NOTES;
        }
        else
        {
            const string* notearg = static_cast<const string*>(gen->GetArgObject(1));
            if (!parse_note(notearg->c_str(), &note, &octave))
            {
                char buf[256];
                snprintf(buf, sizeof(buf), "invalid note %s, should be like C#4", notearg->c_str());
                ctx->SetException(buf);
                return;
            }
        }
    }

    if (gen->GetArgCount() >= 3)
    {
        duration = (s32)gen->GetArgDWord(2);
    }

    if (gen->GetArgCount() >= 4)
    {
        channel = (s32)gen->GetArgDWord(3);
        if (channel < 0 || channel >= TIC_SOUND_CHANNELS)
        {
            char buf[128];
            snprintf(buf, sizeof(buf), "unknown channel %d", channel);
            ctx->SetException(buf);
            return;
        }
    }

    if (gen->GetArgCount() >= 5)
    {
        if (gen->GetArgTypeId(4) == asTYPEID_INT32)
        {
            volumes[0] = volumes[1] = (s32)gen->GetArgDWord(4);
        }
        else
        {
            const CScriptArray* volumesarg = static_cast<const CScriptArray*>(gen->GetArgObject(4));
            for (asUINT i = 0; i < volumesarg->GetSize() && i < COUNT_OF(volumes); i++)
                volumes[i] = (s32)*(asDWORD*)volumesarg->At(i);
            volumesarg->Release();
        }
    }

    if (gen->GetArgCount() >= 6)
    {
        speed = (s32)gen->GetArgDWord(5);
    }

    core->api.sfx(mem, index, note, octave, duration, channel, volumes[0] & 0xf, volumes[1] & 0xf, speed);
}

static void as_vbank(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 prev = core->state.vbank.id;

    if (gen->GetArgCount() == 1)
    {
        const s32 bank = (s32)gen->GetArgDWord(0);
        core->api.vbank(mem, bank);
    }

    gen->SetReturnDWord(prev);
}

static void as_sync(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const u32 mask = (u32)gen->GetArgDWord(0);
    const s32 bank = (u32)gen->GetArgDWord(1);
    const bool tocart = gen->GetArgByte(2) != 0;

    if (bank < 0 || bank >= TIC_BANKS)
    {
        char buf[128];
        snprintf(buf, sizeof(buf), "sync() error, invalid bank %d", bank);
        ctx->SetException(buf);
        return;
    }

    core->api.sync(mem, mask, bank, tocart);
}

static void as_reset(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    core->api.reset(mem);
}

static void as_key(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    if (gen->GetArgCount() == 0)
    {
        bool rv = core->api.key(mem, tic_key_unknown);
        gen->SetReturnByte(rv ? 1 : 0);
        return;
    }

    const tic_key key = gen->GetArgByte(0);
    if (key < tic_keys_count)
    {
        bool rv = core->api.key(mem, key);
        gen->SetReturnByte(rv ? 1 : 0);
    }
    else
    {
        char buf[128];
        snprintf(buf, sizeof(buf), "unknown keyboard code %d", key);
        ctx->SetException(buf);
    }
}

static void as_keyp(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    if (gen->GetArgCount() == 0)
    {
        bool rv = core->api.keyp(mem, tic_key_unknown, -1, -1);
        gen->SetReturnByte(rv ? 1 : 0);
        return;
    }

    const tic_key key = gen->GetArgByte(0);
    if (key >= tic_keys_count)
    {
        char buf[128];
        snprintf(buf, sizeof(buf), "unknown keyboard code %d", key);
        ctx->SetException(buf);
        return;
    }

    if (gen->GetArgCount() == 1)
    {
        bool rv = core->api.keyp(mem, key, -1, -1);
        gen->SetReturnByte(rv ? 1 : 0);
        return;
    }

    const s32 hold = (u32)gen->GetArgDWord(1);
    const s32 period = (u32)gen->GetArgDWord(2);

    bool rv = core->api.keyp(mem, key, hold, period);
    gen->SetReturnByte(rv ? 1 : 0);
}

static void as_memcpy(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 dest = (s32)gen->GetArgDWord(0);
    const s32 source = (s32)gen->GetArgDWord(1);
    const s32 size = (s32)gen->GetArgDWord(2);

    core->api.memcpy(mem, dest, source, size);
}

static void as_memset(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 dest = (s32)gen->GetArgDWord(0);
    const u8 value = gen->GetArgByte(1);
    const s32 size = (s32)gen->GetArgDWord(2);

    core->api.memset(mem, dest, value, size);
}

static void as_font(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    string text = asToString(gen, 0);
    const s32 x = (s32)gen->GetArgDWord(1);
    const s32 y = (s32)gen->GetArgDWord(2);
    u8 chromakey = gen->GetArgByte(3);
    const s32 char_width = (s32)gen->GetArgDWord(4);
    const s32 char_height = (s32)gen->GetArgDWord(5);
    const bool fixed = gen->GetArgByte(6) != 0;
    const s32 scale = (s32)gen->GetArgDWord(7);
    const bool alt = gen->GetArgByte(8) != 0;

    s32 size = core->api.font(mem, text.c_str(), x, y, &chromakey, 1, char_width, char_height, fixed, scale, alt);
    gen->SetReturnDWord(size);
}

static void as_print(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    string text = asToString(gen, 0);
    const s32 x = (s32)gen->GetArgDWord(1);
    const s32 y = (s32)gen->GetArgDWord(2);
    const u8 color = gen->GetArgByte(3);
    const bool fixed = gen->GetArgByte(4) != 0;
    const s32 scale = (s32)gen->GetArgDWord(5);
    const bool alt = gen->GetArgByte(6) != 0;

    s32 width = core->api.print(mem, text.c_str(), x, y, color, fixed, scale, alt);
    gen->SetReturnDWord(width);
}

static void as_trace(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    string text = asToString(gen, 0);
    const u8 color = gen->GetArgByte(1);
    core->api.trace(mem, text.c_str(), color);
}

static void as_pmem(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 index = (s32)gen->GetArgDWord(0);

    if (gen->GetArgCount() == 1)
    {
        u32 value = core->api.pmem(mem, index, 0, false);
        gen->SetReturnDWord(value);
    }
    else
    {
        const u32 value = (u32)gen->GetArgDWord(1);
        core->api.pmem(mem, index, value, true);
    }
}

static void as_time(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    double rv = core->api.time(mem);
    gen->SetReturnDouble(rv);
}

static void as_tstamp(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    s32 rv = core->api.tstamp(mem);
    gen->SetReturnDWord(rv);
}

static void as_exit(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    core->api.exit(mem);
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

    const s32 index = (s32)gen->GetArgDWord(0);
    const u8 flag = gen->GetArgByte(1);

    bool rv = core->api.fget(mem, index, flag);
    gen->SetReturnByte(rv ? 1 : 0);
}

static void as_fset(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    const s32 index = (s32)gen->GetArgDWord(0);
    const u8 flag = gen->GetArgByte(1);
    const bool value = gen->GetArgByte(2) != 0;

    core->api.fset(mem, index, flag, value);
}

static void as_fft(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    // TODO: remove?
}

static void as_ffts(asIScriptGeneric* gen)
{
    GET_TIC_CORE(ctx, core, mem);

    // TODO: remove?
}

static void initAPI(ANGELSCRIPTVM* vm)
{
    vm->engine->RegisterFuncdef("void RemapCallback(uint8 intile, int x, int y, uint8 &out outtile, int &out flip, int &out rotate)");

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
    REGISTER_TIC(vm, as_ttri, "void ttri(float x1, float y1, float x2, float y2, float x3, float y3, float u1, float v1, float u2, float v2, float u3, float v3, uint8 texsrc)");
    REGISTER_TIC(vm, as_ttri, "void ttri(float x1, float y1, float x2, float y2, float x3, float y3, float u1, float v1, float u2, float v2, float u3, float v3, uint8 texsrc, int chromakey, float z1=0, float z2=0, float z3=0)");
    REGISTER_TIC(vm, as_ttri, "void ttri(float x1, float y1, float x2, float y2, float x3, float y3, float u1, float v1, float u2, float v2, float u3, float v3, uint8 texsrc, const array<int>@ chromakey, float z1=0, float z2=0, float z3=0)");
    REGISTER_TIC(vm, as_clip, "void clip()");
    REGISTER_TIC(vm, as_clip, "void clip(int x, int y, int width, int height)");
    REGISTER_TIC(vm, as_btnp, "bool btnp(int id)");
    REGISTER_TIC(vm, as_btnp, "bool btnp(int id, int hold, int period)");
    REGISTER_TIC(vm, as_btn, "bool btn(int id)");
    REGISTER_TIC(vm, as_spr, "void spr(int id, int x, int y, int colorkey=-1, int scale=1, uint8 flip=0, uint8 rotate=0, int w=1, int h=1)");
    REGISTER_TIC(vm, as_spr, "void spr(int id, int x, int y, const array<int>@ colorkey, int scale=1, uint8 flip=0, uint8 rotate=0, int w=1, int h=1)");
    REGISTER_TIC(vm, as_mget, "uint8 mget(int x, int y)");
    REGISTER_TIC(vm, as_mset, "void mset(int x, int y, uint8 tile_id)");
    REGISTER_TIC(vm, as_map, "void map(int x=0, int y=0, int w=30, int h=17, int sx=0, int sy=0)");
    REGISTER_TIC(vm, as_map, "void map(int x, int y, int w, int h, int sx, int sy, int colorkey, int scale=1, RemapCallback@ remap=null)");
    REGISTER_TIC(vm, as_map, "void map(int x, int y, int w, int h, int sx, int sy, const array<int>@ colorkey, int scale=1, RemapCallback@ remap=null)");
    REGISTER_TIC(vm, as_music, "void music(int track=-1, int frame=-1, int row=-1, bool loop=true, bool sustain=false, int tempo=-1, int speed=-1)");
    REGISTER_TIC(vm, as_sfx, "void sfx(int id)");
    REGISTER_TIC(vm, as_sfx, "void sfx(int id, int note)");
    REGISTER_TIC(vm, as_sfx, "void sfx(int id, int note, int duration)");
    REGISTER_TIC(vm, as_sfx, "void sfx(int id, int note, int duration, int channel)");
    REGISTER_TIC(vm, as_sfx, "void sfx(int id, int note, int duration, int channel, int volume)");
    REGISTER_TIC(vm, as_sfx, "void sfx(int id, int note, int duration, int channel, int volume, int speed)");
    REGISTER_TIC(vm, as_sfx, "void sfx(int id, int note, int duration, int channel, const array<int>@ volumes)");
    REGISTER_TIC(vm, as_sfx, "void sfx(int id, int note, int duration, int channel, const array<int>@ volumes, int speed)");
    REGISTER_TIC(vm, as_sfx, "void sfx(int id, const string &in note)");
    REGISTER_TIC(vm, as_sfx, "void sfx(int id, const string &in note, int duration)");
    REGISTER_TIC(vm, as_sfx, "void sfx(int id, const string &in note, int duration, int channel)");
    REGISTER_TIC(vm, as_sfx, "void sfx(int id, const string &in note, int duration, int channel, int volume)");
    REGISTER_TIC(vm, as_sfx, "void sfx(int id, const string &in note, int duration, int channel, int volume, int speed)");
    REGISTER_TIC(vm, as_sfx, "void sfx(int id, const string &in note, int duration, int channel, const array<int>@ volumes)");
    REGISTER_TIC(vm, as_sfx, "void sfx(int id, const string &in note, int duration, int channel, const array<int>@ volumes, int speed)");
    REGISTER_TIC(vm, as_vbank, "int vbank(int bank)");
    REGISTER_TIC(vm, as_vbank, "int vbank()");
    REGISTER_TIC(vm, as_sync, "void sync(uint mask=0, int bank=0, bool tocart=false)");
    REGISTER_TIC(vm, as_reset, "void reset()");
    REGISTER_TIC(vm, as_key, "bool key()");
    REGISTER_TIC(vm, as_key, "bool key(int code)");
    REGISTER_TIC(vm, as_keyp, "bool keyp()");
    REGISTER_TIC(vm, as_keyp, "bool keyp(int code)");
    REGISTER_TIC(vm, as_keyp, "bool keyp(int code, int hold, int period)");
    REGISTER_TIC(vm, as_memcpy, "void memcpy(int dest, int source, int size)");
    REGISTER_TIC(vm, as_memset, "void memset(int dest, uint8 value, int size)");
    REGISTER_TIC(vm, as_font, "int font(const ?&in text, int x, int y, uint8 chromakey, int char_width, int char_height, bool fixed=false, int scale=1, bool alt=false)");
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

    RegisterStdString(vm->engine);
    RegisterScriptAny(vm->engine);
    RegisterScriptArray(vm->engine, true);
    RegisterScriptDictionary(vm->engine);
    // TODO: RegisterScriptGrid(vm->engine); apparently doesn't support generic calling convention
    RegisterScriptHandle(vm->engine);
    RegisterExceptionRoutines(vm->engine);
    RegisterScriptMath(vm->engine);
    // TODO: RegisterScriptMathComplex(vm->engine); apparently doesn't support generic calling convention
    RegisterStdStringUtils(vm->engine);
    RegisterScriptWeakRef(vm->engine);

    vm->context = vm->engine->CreateContext();
    if (!vm->context)
    {
        core->data->error(core->data->data, "Couldn't create context.");
        return false;
    }

    vm->context->SetUserData(core);

    vm->engine->SetMessageCallback(asFUNCTION(asMessageCallback), core, asCALL_CDECL);
    vm->context->SetExceptionCallback(asFUNCTION(asExceptionCallback), core, asCALL_CDECL);

    initAPI(vm);

    vm->module = asCompileModule(vm->engine, code);
    if (!vm->module)
    {
        core->data->error(core->data->data, "Module failed compilation.");
        return false;
    }

    vm->borderFunction = vm->module->GetFunctionByDecl("void BDR(int row)");
    vm->bootFunction = vm->module->GetFunctionByDecl("void BOOT()");
    vm->menuFunction = vm->module->GetFunctionByDecl("void MENU(int index)");
    vm->scanlineFunction = vm->module->GetFunctionByDecl("void SCN(int row)");
    vm->tickFunction = vm->module->GetFunctionByDecl("void TIC()");

    return true;
}

static void callAngelScriptTick(tic_mem* tic)
{
    tic_core* core = (tic_core*)tic;

    ANGELSCRIPTVM* vm = static_cast<ANGELSCRIPTVM*>(core->currentVM);
    if (vm && vm->tickFunction)
    {
        int r = vm->context->Prepare(vm->tickFunction); assert(r >= 0);
        r = vm->context->Execute(); assert(r >= 0);
    }
}

static void callAngelScriptBoot(tic_mem* tic)
{
    tic_core* core = (tic_core*)tic;

    ANGELSCRIPTVM* vm = static_cast<ANGELSCRIPTVM*>(core->currentVM);
    if (vm && vm->bootFunction)
    {
        int r = vm->context->Prepare(vm->bootFunction); assert(r >= 0);
        r = vm->context->Execute(); assert(r >= 0);
    }
}

static void callAngelScriptScanline(tic_mem* tic, s32 row, void* data)
{
    tic_core* core = (tic_core*)tic;

    ANGELSCRIPTVM* vm = static_cast<ANGELSCRIPTVM*>(core->currentVM);
    if (vm && vm->scanlineFunction)
    {
        int r = vm->context->Prepare(vm->scanlineFunction); assert(r >= 0);
        r = vm->context->SetArgDWord(0, row); assert(r >= 0);
        r = vm->context->Execute(); assert(r >= 0);
    }
}

static void callAngelScriptBorder(tic_mem* tic, s32 row, void* data)
{
    tic_core* core = (tic_core*)tic;

    ANGELSCRIPTVM* vm = static_cast<ANGELSCRIPTVM*>(core->currentVM);
    if (vm && vm->borderFunction)
    {
        int r = vm->context->Prepare(vm->borderFunction); assert(r >= 0);
        r = vm->context->SetArgDWord(0, row); assert(r >= 0);
        r = vm->context->Execute(); assert(r >= 0);
    }
}

static void callAngelScriptMenu(tic_mem* tic, s32 index, void* data)
{
    tic_core* core = (tic_core*)tic;

    ANGELSCRIPTVM* vm = static_cast<ANGELSCRIPTVM*>(core->currentVM);
    if (vm && vm->menuFunction)
    {
        int r = vm->context->Prepare(vm->menuFunction); assert(r >= 0);
        r = vm->context->SetArgDWord(0, index); assert(r >= 0);
        r = vm->context->Execute(); assert(r >= 0);
    }
}

static const tic_outline_item* getAngelScriptOutline(const char* code, s32* size) { return NULL; }
static void evalAngelScript(tic_mem* tic, const char* code) { }

static const char* const AngelScriptKeywords [] =
{
    // official keywords
    "and","auto","bool","break","case","cast","catch","class","const","continue","default","do","double",
    "else","enum","false","float","for","foreach","funcdef","if","import","in","inout","int","interface","int8",
    "int16","int32","int64","is","mixin","namespace","not","null","or","out","private","protected","return",
    "switch","true","try","typedef","uint","uint8","uint16","uint32","uint64","using","void","while","xor",
    "abstract","delete","explicit","external","final","from","function","get","override","property","set","shared",
    "super","this",
};

static const char* AngelScriptAPIKeywords [] =
{
#define TIC_CALLBACK_DEF(name, ...) #name,
    TIC_CALLBACK_LIST(TIC_CALLBACK_DEF)
#undef  TIC_CALLBACK_DEF

#define API_KEYWORD_DEF(name, ...) #name,
    TIC_API_LIST(API_KEYWORD_DEF)
#undef  API_KEYWORD_DEF

    // addon types and global functions (not doing all the methods!)
    "any", // scriptany
    "array", // scriptarray
    "dictionary", "dictionaryIter", "dictionaryValue", // scriptdictionary
    // "grid", // scriptgrid
    "ref", // scripthandle
    "throw", "getExceptionInfo", // scripthelper
    "cos", "sin", "tan", "acos", "asin", "atan", "atan2", "cosh", "sinh", "tanh", "log", "log10", "pow", "sqrt", "ceil", "abs", "floor", "fraction", // scriptmath
    // "complex", // scriptmathcomplex
    "string", "scan", "format", "formatInt", "formatUInt", "formatFloat", "parseInt", "parseUInt", "parseFloat", // scriptstdstring
    "join", // scriptstdstring_utils
    "weakref", "const_weakref", // weakref
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
    .api_keywordsCount  = COUNT_OF(AngelScriptAPIKeywords),
    .api_keywords       = AngelScriptAPIKeywords,

    .demo = {DemoRom, sizeof DemoRom},
    .mark = {MarkRom, sizeof MarkRom, "angelscriptmark.tic"},
};