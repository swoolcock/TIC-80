//
// Created by samah on 5/09/2026.
//
#include "core/core.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "tools.h"

extern bool parse_note(const char* noteStr, s32* note, s32* octave);

static bool initAngelScript(tic_mem* tic, const char* code)
{
    int *a = NULL;
    *a = 1;
    return true;
}
static void closeAngelScript(tic_mem* tic) { }
static void callAngelScriptTick(tic_mem* tic) { }
static void callAngelScriptBoot(tic_mem* tic) { }
static void callAngelScriptScanline(tic_mem* tic, s32 row, void* data) { }
static void callAngelScriptBorder(tic_mem* tic, s32 row, void* data) { }
static void callAngelScriptMenu(tic_mem* tic, s32 index, void* data) { }
static const tic_outline_item* getAngelScriptOutline(const char* code, s32* size) { return NULL; }
static void evalAngelScript(tic_mem* tic, const char* code) { }

static const char* const AngelScriptKeywords [] =
{
    "as", "break", "class", "construct", "continue", "else", "false",
    "for", "foreign", "if", "import", "in", "is", "null", "return",
    "static", "super", "this", "true", "var", "while"
};

static const u8 DemoRom[] =
{
// #include "../build/assets/angelscriptdemo.tic.dat"
};

static const u8 MarkRom[] =
{
// #include "../build/assets/angelscriptmark.tic.dat"
};

TIC_EXPORT const tic_script EXPORT_SCRIPT(AngelScript) =
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