#include "../SDK/SDK.h"
#include "../SDK/Vars.h"

MAKE_SIGNATURE(Q_snprintf, "client.dll", "4C 89 44 24 ? 4C 89 4C 24 ? 53 55 56 57 41 56 48 83 EC ? 49 8B D8 48 63 FA 48 8B F1 4C 8D 74 24 ? E8 ? ? ? ? 4C 89 74 24 ? 4C 8B CB 4C 8B C7", 0x0);

static bool g_bNextIsBytespy = false;
static const char* g_pBytespyStr = nullptr;

MAKE_HOOK(Q_snprintf, S::Q_snprintf(), int, char* buffer, int size, const char* format, ...)
{
    if (!format || !buffer)
        return -1;

    va_list args;
    va_start(args, format);

    if (Vars::Visuals::UI::NetGraphWatermark.Value && strcmp(format, "fps:%4i   ping: %i ms") == 0)
    {
        va_arg(args, int);
        va_arg(args, int);
        va_end(args);
        strcpy_s(buffer, size, "bytespy v2.5");
        g_bNextIsBytespy = true;
        g_pBytespyStr = buffer;
        return 12;
    }

    int result = vsnprintf(buffer, size, format, args);
    va_end(args);
    return result;
}

using DrawColoredTextDirectFn = int(__fastcall*)(void*, HFont, int, int, int, int, int, int, const char*, const char*);
static DrawColoredTextDirectFn g_OriginalDrawColoredText = nullptr;

MAKE_HOOK(DrawColoredText, U::Memory.GetVFunc(I::MatSystemSurface, 162), int, void* rcx, HFont font, int x, int y, int r, int g, int b, int a, const char* fmt, ...)
{
    if (!g_OriginalDrawColoredText)
        g_OriginalDrawColoredText = Hook.As<DrawColoredTextDirectFn>();

    va_list args;
    va_start(args, fmt);
    const char* str = (fmt && strcmp(fmt, "%s") == 0) ? va_arg(args, const char*) : nullptr;
    va_end(args);

    if (g_bNextIsBytespy && Vars::Visuals::UI::NetGraphWatermark.Value && str == g_pBytespyStr)
    {
        g_bNextIsBytespy = false;
        g_pBytespyStr = nullptr;
        Color_t accent = Vars::Menu::Theme::Accent.Value;
        return g_OriginalDrawColoredText(rcx, font, x, y, accent.r, accent.g, accent.b, 255, fmt, str);
    }

    g_bNextIsBytespy = false;

    if (str)
        return g_OriginalDrawColoredText(rcx, font, x, y, r, g, b, a, fmt, str);

    return 0;
}