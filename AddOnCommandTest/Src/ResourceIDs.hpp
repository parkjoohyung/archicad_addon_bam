#ifndef RESOURCE_IDS_HPP
#define RESOURCE_IDS_HPP

// Dialog & Palette IDs
enum DialogIDs {
    DLG_MAIN_PALETTE            = 32500,
    DLG_RESOLUTION_PALETTE      = 32501,
    DLG_HEIGHT_PALETTE          = 32502,
    DLG_TERRAIN_PALETTE         = 32503,
    DLG_ACTIVATION              = 32510
};

// ---------------------------------------------------------
// Main Palette (32500) Controls
// 순서대로 나열해야 GRC 파일의 암시적 인덱스와 일치합니다.
// ---------------------------------------------------------
enum MainPaletteControls {
    MAIN_BTN_AUTO_CREATE = 1,
    MAIN_BTN_BROWSE,
    MAIN_TXT_EXCEL_TITLE,
    MAIN_TXT_EXCEL_PATH,
    MAIN_SEP_1,
    MAIN_TXT_CALC_TITLE,
    MAIN_TXT_POLY_REDUCE,
    MAIN_TXT_TARGET_AREA,
    MAIN_REAL_TARGET_AREA,
    MAIN_TXT_UNIT,
    MAIN_REAL_UNIT,
    MAIN_TXT_TOLERANCE,
    MAIN_REAL_TOLERANCE,
    MAIN_CHK_FIX_X,
    MAIN_CHK_FIX_Y,
    MAIN_BTN_RESHAPE,
    MAIN_BTN_SUM_SELECTED,
    MAIN_TXT_TOTAL_AREA_LBL,
    MAIN_TXT_TOTAL_AREA_VAL,
    MAIN_SEP_2,
    MAIN_TXT_MAGIC_TITLE,
    MAIN_RAD_ZONE,
    MAIN_RAD_MESH,
    MAIN_BTN_CREATE_MAGIC,
    MAIN_RAD_MORPH,
    MAIN_SEP_3,
    MAIN_TXT_LABEL_TITLE,
    MAIN_TXT_PREFIX,
    MAIN_EDIT_PREFIX,
    MAIN_BTN_PICK,
    MAIN_TXT_STYLE,
    MAIN_POP_STYLE,
    MAIN_TXT_START,
    MAIN_EDIT_START,
    MAIN_TXT_PREVIEW,
    MAIN_EDIT_PREVIEW,
    MAIN_BTN_APPLY_LABEL,
    MAIN_BTN_CALC,
    MAIN_SEP_4,
    MAIN_TXT_HEIGHT,
    MAIN_REAL_HEIGHT,
    MAIN_TXT_HYPHEN,
    MAIN_BTN_SIMPLIFY_POLY,
    MAIN_TXT_SIMPLIFY_TOL,
    MAIN_REAL_SIMPLIFY_TOL,
    MAIN_CHK_PRESERVE_CURVE,
    MAIN_BTN_CHECK_UPDATE,
    MAIN_TXT_VERSION
};

// ---------------------------------------------------------
// Resolution Palette (32501) Controls
// ---------------------------------------------------------
enum ResolutionPaletteControls {
    RES_TXT_TARGET_TITLE = 1,
    RES_TXT_WIDTH,
    RES_REAL_WIDTH,
    RES_TXT_HEIGHT,
    RES_REAL_HEIGHT,
    RES_CHK_LOCK_RATIO,
    RES_BTN_MATCH_DRAWING,
    RES_TXT_UPSCALE,
    RES_BTN_UPSCALE_05,
    RES_BTN_UPSCALE_20,
    RES_BTN_UPSCALE_40,
    RES_BTN_APPLY_TARGET,
    RES_BTN_RESTORE,
    RES_TXT_NOTE
};

// ---------------------------------------------------------
// Height Palette (32502) Controls
// ---------------------------------------------------------
enum HeightPaletteControls {
    HGT_BTN_SELECT = 1,
    HGT_SEP_1,
    HGT_TXT_BASE_ELEV,
    HGT_TXT_BASE_ELEV_VAL,
    HGT_BTN_CALCULATE,
    HGT_SEP_2,
    HGT_TXT_AREA,
    HGT_TXT_AREA_VAL,
    HGT_TXT_PERIMETER,
    HGT_TXT_PERIMETER_VAL,
    HGT_TXT_AVG_HEIGHT,
    HGT_TXT_AVG_HEIGHT_VAL,
    HGT_BTN_CREATE_GROUND
};

// ---------------------------------------------------------
// Terrain Palette (32503) Controls
// ---------------------------------------------------------
enum TerrainPaletteControls {
    TER_TXT_DIVISIONS = 1,
    TER_REAL_DIVISIONS,
    TER_TXT_PEN,
    TER_REAL_PEN,
    TER_BTN_GENERATE
};

// ---------------------------------------------------------
// Activation Dialog (32510) Controls
// ---------------------------------------------------------
enum ActivationDialogControls {
    ACT_TXT_TITLE = 1,
    ACT_TXT_DESC,
    ACT_EDIT_EMAIL,
    ACT_TXT_EMAIL,
    ACT_EDIT_KEY,
    ACT_BTN_ACTIVATE,
    ACT_BTN_CANCEL,
    ACT_TXT_STATUS
};

#endif // RESOURCE_IDS_HPP
