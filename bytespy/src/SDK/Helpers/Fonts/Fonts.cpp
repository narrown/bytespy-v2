#include "Fonts.h"
#include <windows.h>

void RegisterEmbeddedFonts()
{
    DWORD numFonts = 0;

    PBYTE fontData = const_cast<BYTE*>(
        reinterpret_cast<const BYTE*>(
            RobotoMedium_compressed_data));
    DWORD fontSize = RobotoMedium_compressed_size;
    if (AddFontMemResourceEx(fontData, fontSize, nullptr, &numFonts) == 0
        || numFonts == 0)
    {
        // do something here
    }
}

void CFonts::Init(float flDPI)
{
    static bool bRegistered = false;
    if (!bRegistered)
    {
        RegisterEmbeddedFonts();
        bRegistered = true;
    }
    Reload(flDPI);
}

#undef CreateFont // oh my god fuck you windows and your shit fucking os LOVE YOU TORVALDS

void CFonts::Reload(float flDPI)
{
#define FW_DONTCARE         0
#define FW_THIN             100
#define FW_EXTRALIGHT       200
#define FW_LIGHT            300
#define FW_NORMAL           400
#define FW_MEDIUM           500
#define FW_SEMIBOLD         600
#define FW_BOLD             700
#define FW_EXTRABOLD        800
#define FW_HEAVY            900

#define FW_ULTRALIGHT       FW_EXTRALIGHT
#define FW_REGULAR          FW_NORMAL
#define FW_DEMIBOLD         FW_SEMIBOLD
#define FW_ULTRABOLD        FW_EXTRABOLD
#define FW_BLACK            FW_HEAVY

    m_mFonts[FONT_ESP] =
    {
        "Tahoma",              
        int(12.0f * flDPI),    
        FONTFLAG_NONE,
        FW_DONTCARE
    };

    m_mFonts[FONT_ESP_CONDS] =
    {
        "Small Fonts",
        int(9.0f * flDPI),
        FONTFLAG_ANTIALIAS,
        FW_DONTCARE
    };

    m_mFonts[FONT_INDICATORS] =
    {
        "Tahoma",
        int(13.0f * flDPI),
        FONTFLAG_ANTIALIAS,
        FW_DONTCARE
    };

    for (auto& [_, fFont] : m_mFonts)
    {
        fFont.m_dwFont = I::MatSystemSurface->CreateFont();
        I::MatSystemSurface->SetFontGlyphSet(
            fFont.m_dwFont,     
            fFont.m_szName,     
            fFont.m_nTall,      
            fFont.m_nWeight,     
            0,                   
            0,                   
            fFont.m_nFlags
        );
    }
}

const Font_t& CFonts::GetFont(EFonts eFont)
{
    return m_mFonts[eFont];
}
