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

extern bool parse_note(const char* noteStr, s32* note, s32* octave);

static void closeGravity(tic_mem* tic)
{
}

static bool initGravity(tic_mem* tic, const char* code)
{
    return true;
}

static void callGravityTick(tic_mem* tic)
{
}

static void callGravityBoot(tic_mem* tic)
{
}

static void callGravityScanline(tic_mem* tic, s32 row, void* data)
{
}

static void callGravityBorder(tic_mem* tic, s32 row, void* data)
{
}

static void callGravityMenu(tic_mem* tic, s32 index, void* data)
{
}

static const char* const GravityKeywords [] =
{
};

static const tic_outline_item* getGravityOutline(const char* code, s32* size)
{
    return NULL;
}

static void evalGravity(tic_mem* tic, const char* code)
{
}

static const u8 DemoRom[] =
{
// #include "../build/assets/squirreldemo.tic.dat"
};

static const u8 MarkRom[] =
{
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

        .keywords           = GravityKeywords,
        .keywordsCount      = COUNT_OF(GravityKeywords),

        .demo = {DemoRom, sizeof DemoRom},
        .mark = {MarkRom, sizeof MarkRom, "gravitymark.tic"},
    };
