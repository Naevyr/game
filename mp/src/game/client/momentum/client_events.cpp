#include "cbase.h"

#include "client_events.h"

#include "filesystem.h"
#include "MessageboxPanel.h"
#include "fmtstr.h"
#include "steam/steam_api.h"
#include "util/mom_util.h"

#include "icommandline.h"

#include "tier0/memdbgon.h"

extern IFileSystem *filesystem;

inline void UnloadConVarOrCommand(const char *pName)
{
    const auto pCmd = g_pCVar->FindCommandBase(pName);
    if (pCmd)
        g_pCVar->UnregisterConCommand(pCmd);
}

inline void OverrideConVarDefault(const char *pName, const char *pValue)
{
    const auto pVar = g_pCVar->FindVar(pName);

    if (pVar)
    {
        if (FStrEq(pVar->GetDefault(), pVar->GetString()))
        {
            pVar->SetValue(pValue);
        }

        pVar->SetDefault(pValue);
    }
}

bool CMOMClientEvents::Init()
{
    // Mount CSS content even if it's on a different drive than this game
    MomUtil::MountGameFiles();

    if (!CommandLine()->FindParm("-mapping"))
    {
        // Unregister FCVAR_MAPPING convars
        auto pCvar = g_pCVar->GetCommands();
        while (pCvar)
        {
            const auto pNext = pCvar->GetNext();

            if (pCvar->IsFlagSet(FCVAR_MAPPING))
                g_pCVar->UnregisterConCommand(pCvar);

            pCvar = pNext;
        }
    }

    UnloadConVarOrCommand("retry");

    static ConCommand retry("retry", []()
    {
        engine->ExecuteClientCmd("reload");
    });

    // Override the default values with those of Portal 2/CS:GO to fix projected shadow clipping
    // Done this way because the vars are originally defined in shaderapidx9
    OverrideConVarDefault("mat_slopescaledepthbias_shadowmap", "3");
    OverrideConVarDefault("mat_depthbias_shadowmap", "0.000025");

    return true;
}

void CMOMClientEvents::PostInit()
{
    // enable console by default
    ConVarRef con_enable("con_enable");
    con_enable.SetValue(true);

    // Version warning
    // MOM_TODO: Change this once we hit Beta
    // MOM_CURRENT_VERSION
    g_pMessageBox->CreateMessageboxVarRef("#MOM_StartupMsg_Alpha_Title", "#MOM_StartupMsg_Alpha",
                                            "mom_toggle_versionwarn", "#MOM_IUnderstand");
    
    if (!SteamAPI_IsSteamRunning() || !SteamHTTP())
    {
        vgui::Panel *pPanel = g_pMessageBox->CreateMessagebox("#MOM_StartupMsg_NoSteamApiContext_Title",
                                                        "#MOM_StartupMsg_NoSteamApiContext", "#MOM_IUnderstand");
        pPanel->MoveToFront();
        pPanel->RequestFocus();
    }
}

void CMOMClientEvents::LevelInitPreEntity()
{
    //Precache();
}

void CMOMClientEvents::Precache()
{
    // MOM_TODO: Precache anything here
}

CMOMClientEvents g_MOMClientEvents("CMOMClientEvents");