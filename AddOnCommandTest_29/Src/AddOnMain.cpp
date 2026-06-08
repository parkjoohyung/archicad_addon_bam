#include "ACAPinc.h"
#include "Licensing.hpp"
#include "Updater.hpp"
#include "UI_Palettes.hpp"

#ifndef __ACDLL_CALL
#define __ACDLL_CALL
#endif

// Modeless window GUIDs matching UI_Palettes.cpp
static const GS::Guid customAddonPaletteGuid("{A06821CF-7C13-4BCE-A14E-002D5A8FCE53}");
static const GS::Guid secondAddonPaletteGuid("{B17932DF-8D24-4CDE-B25F-113D6A9FDE64}");
static const GS::Guid heightAddonPaletteGuid("{C28043E0-9E35-4DEF-C36A-224E7B0FDF75}");
static const GS::Guid terrainAddonPaletteGuid("{D39154F1-AF46-4EFE-D47B-335F8C10EF86}");

static bool CheckAndActivateLicense() {
    if (!MyProjectNamespace::IsAlreadyLicensed()) {
        MyProjectNamespace::ActivationDialog dlg;
        if (dlg.Invoke() != DG::ModalDialog::Accept) {
            return false; // User cancelled or failed
        }
    }
    return true;
}

static GSErrCode __ACDLL_CALL MasterMenuHandler(const API_MenuParams* menuParams) {
    // 1. License Check
    if (!CheckAndActivateLicense()) return NoError;

    // 2. Open palette based on clicked menu ID
    switch (menuParams->menuItemRef.menuResID) {
        case 32500: MyProjectNamespace::MagicPalette::Show(); break;
        case 32501: MyProjectNamespace::SecondPalette::Show(); break;
        case 32502: MyProjectNamespace::HeightPalette::Show(); break;
        case 32503: MyProjectNamespace::TerrainPalette::Show(); break;
        default: break;
    }
    return NoError;
}

extern "C" {
API_AddonType __ACDLL_CALL CheckEnvironment(API_EnvirParams *e) {
    RSGetIndString(&e->addOnInfo.name, 32000, 1, ACAPI_GetOwnResModule());
    RSGetIndString(&e->addOnInfo.description, 32000, 2, ACAPI_GetOwnResModule());
    return APIAddon_Preload;
}

GSErrCode __ACDLL_CALL RegisterInterface() {
    ACAPI_MenuItem_RegisterMenu(32500, 0, MenuCode_UserDef, MenuFlag_Default);
    ACAPI_MenuItem_RegisterMenu(32501, 0, MenuCode_UserDef, MenuFlag_Default);
    ACAPI_MenuItem_RegisterMenu(32502, 0, MenuCode_UserDef, MenuFlag_Default);
    ACAPI_MenuItem_RegisterMenu(32503, 0, MenuCode_UserDef, MenuFlag_Default);
    return NoError;
}

GSErrCode __ACDLL_CALL Initialize() {
    ACAPI_KeepInMemory(true);
    ACAPI_MenuItem_InstallMenuHandler(32500, MasterMenuHandler);
    ACAPI_MenuItem_InstallMenuHandler(32501, MasterMenuHandler);
    ACAPI_MenuItem_InstallMenuHandler(32502, MasterMenuHandler);
    ACAPI_MenuItem_InstallMenuHandler(32503, MasterMenuHandler);
    
    ACAPI_RegisterModelessWindow(32500, MyProjectNamespace::MagicPalette::StaticCallback, 
                                 API_PalEnabled_FloorPlan | API_PalEnabled_3D | API_PalEnabled_Section | API_PalEnabled_Elevation | API_PalEnabled_Layout, 
                                 GSGuid2APIGuid(customAddonPaletteGuid));
    ACAPI_RegisterModelessWindow(32501, MyProjectNamespace::SecondPalette::StaticCallback,
                                 API_PalEnabled_FloorPlan | API_PalEnabled_3D | API_PalEnabled_Section | API_PalEnabled_Elevation | API_PalEnabled_Layout,
                                 GSGuid2APIGuid(secondAddonPaletteGuid));
    ACAPI_RegisterModelessWindow(32502, MyProjectNamespace::HeightPalette::StaticCallback,
                                 API_PalEnabled_FloorPlan | API_PalEnabled_3D | API_PalEnabled_Section | API_PalEnabled_Elevation | API_PalEnabled_Layout,
                                 GSGuid2APIGuid(heightAddonPaletteGuid));
    ACAPI_RegisterModelessWindow(32503, MyProjectNamespace::TerrainPalette::StaticCallback,
                                 API_PalEnabled_FloorPlan | API_PalEnabled_3D | API_PalEnabled_Section | API_PalEnabled_Elevation | API_PalEnabled_Layout,
                                 GSGuid2APIGuid(terrainAddonPaletteGuid));
    return NoError;
}

GSErrCode __ACDLL_CALL FreeData() {
    MyProjectNamespace::MagicPalette::Destroy();
    MyProjectNamespace::SecondPalette::Destroy();
    MyProjectNamespace::HeightPalette::Destroy();
    MyProjectNamespace::TerrainPalette::Destroy();
    return NoError;
}
}
