//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "vgui_controls/CvarTextEntry.h"
#include "tier1/KeyValues.h"
#include "tier1/convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

static const int MAX_CVAR_TEXT = 64;

DECLARE_BUILD_FACTORY_DEFAULT_TEXT(CvarTextEntry, "");

CvarTextEntry::CvarTextEntry(Panel *parent, const char *panelName, char const *cvarname, int precision)
    : TextEntry(parent, panelName), m_cvarRef(cvarname, true)
{
    InitSettings();
    SetPrecision(precision);
    m_pszStartValue[0] = 0;

    if (m_cvarRef.IsValid())
    {
        Reset();
    }
    
    AddActionSignalTarget(this);
}

void CvarTextEntry::SetPrecision(int precision)
{
    m_iPrecision = precision;
    if (precision) // dont worry about setting if 0 as float will be rounded
    {
        Q_snprintf(m_szNumberFormat, sizeof(m_szNumberFormat), "%s.%i%s", "%", precision, "f");
    }
}

void CvarTextEntry::ApplySchemeSettings(IScheme *pScheme)
{
    BaseClass::ApplySchemeSettings(pScheme);
    if (GetMaximumCharCount() < 0 || GetMaximumCharCount() > MAX_CVAR_TEXT)
    {
        SetMaximumCharCount(MAX_CVAR_TEXT - 1);
    }
}

void CvarTextEntry::ApplySettings(KeyValues* inResourceData)
{
    BaseClass::ApplySettings(inResourceData);

    const char *cvarName = inResourceData->GetString("cvar_name", "");

    m_cvarRef.Init(cvarName);

    if (m_cvarRef.IsValid())
    {
        Reset();
    }
}

void CvarTextEntry::GetSettings(KeyValues* pOutResource)
{
    BaseClass::GetSettings(pOutResource);

    pOutResource->SetString("cvar_name", m_cvarRef.GetName());
}

void CvarTextEntry::SetText(const char* text)
{
    if (!text || !text[0]) 
        return;

    if (GetAllowNumericInputOnly())
    {
        if (m_iPrecision)
        {
            if (text[strnlen(text, MAX_CVAR_TEXT) - 1] == '.')
            {
                // trying to enter a decimal number so set the text normally
                BaseClass::SetText(text);
            }
            else // not in the middle of setting a decimal number
            {
                char newText[MAX_CVAR_TEXT];
                Q_snprintf(newText, MAX_CVAR_TEXT, m_szNumberFormat, atof(text));
                BaseClass::SetText(newText);
            }
        }
        else
        {
            char newText[MAX_CVAR_TEXT];
            Q_snprintf(newText, MAX_CVAR_TEXT, "%i", atoi(text));
            BaseClass::SetText(newText);
        }
    }
    else
    {
        BaseClass::SetText(text);
    }
}

void CvarTextEntry::InitSettings()
{
    BEGIN_PANEL_SETTINGS()
    {"cvar_name", TYPE_STRING}
    END_PANEL_SETTINGS();
}

void CvarTextEntry::OnApplyChanges()
{
    ApplyChanges();
}

void CvarTextEntry::ApplyChanges()
{
    if (!m_cvarRef.IsValid())
        return;

    char szText[MAX_CVAR_TEXT];
    GetText(szText, MAX_CVAR_TEXT);

    if (!szText[0])
        return;

    m_cvarRef.SetValue(szText);

    Q_strncpy(m_pszStartValue, szText, sizeof(m_pszStartValue));
}

void CvarTextEntry::Reset()
{
    if (!m_cvarRef.IsValid())
        return;

    const char *value = m_cvarRef.GetString();
    if (value && value[0])
    {
        SetText(value);
        Q_strncpy(m_pszStartValue, value, sizeof(m_pszStartValue));
        GotoTextEnd();
    }
}

void CvarTextEntry::OnThink()
{
    if (HasBeenModifiedExternally())
        Reset();
}

void CvarTextEntry::OnKillFocus()
{
    if (!m_cvarRef.IsValid())
        return;

    char entryValue[MAX_CVAR_TEXT];
    GetText(entryValue, MAX_CVAR_TEXT);
    if (!entryValue[0] || stricmp(m_cvarRef.GetString(), m_pszStartValue) != 0)
    {
        Reset();
    }
}

bool CvarTextEntry::HasBeenModified()
{
    char szText[MAX_CVAR_TEXT];
    GetText(szText, MAX_CVAR_TEXT);

    return stricmp(szText, m_pszStartValue) != 0;
}

bool CvarTextEntry::HasBeenModifiedExternally() const
{
    return m_cvarRef.IsValid() && stricmp(m_cvarRef.GetString(), m_pszStartValue) != 0;
}

void CvarTextEntry::OnTextChanged()
{
    if (!m_cvarRef.IsValid())
        return;

    if (HasBeenModified())
    {
        PostActionSignal(new KeyValues("ControlModified"));
        ApplyChanges();
    }
}
