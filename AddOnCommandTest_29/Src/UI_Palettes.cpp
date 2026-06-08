#include "UI_Palettes.hpp"
#include "Licensing.hpp"
#include "Updater.hpp"
#include "AutomationLogic.hpp"

#ifdef WINDOWS
#include <windows.h>
#endif

#ifndef __ACDLL_CALL
#define __ACDLL_CALL
#endif

namespace MyProjectNamespace {

    // Helper GUID definitions
    static const GS::Guid customAddonPaletteGuid("{A06821CF-7C13-4BCE-A14E-002D5A8FCE53}");
    static const GS::Guid secondAddonPaletteGuid("{B17932DF-8D24-4CDE-B25F-113D6A9FDE64}");
    static const GS::Guid heightAddonPaletteGuid("{C28043E0-9E35-4DEF-C36A-224E7B0FDF75}");
    static const GS::Guid terrainAddonPaletteGuid("{D39154F1-AF46-4EFE-D47B-335F8C10EF86}");

    // ==========================================
    // ActivationDialog Implementation
    // ==========================================

    ActivationDialog::ActivationDialog() : DG::ModalDialog(ACAPI_GetOwnResModule(), 32510, ACAPI_GetOwnResModule()) {
        editEmail = new DG::TextEdit(GetReference(), 3);
        editKey = new DG::TextEdit(GetReference(), 5);
        btnActivate = new DG::Button(GetReference(), 6);
        btnCancel = new DG::Button(GetReference(), 7);
        txtStatus = new DG::LeftText(GetReference(), 8);

        Attach(*this);
        btnActivate->Attach(*this);
        btnCancel->Attach(*this);
    }

    ActivationDialog::~ActivationDialog() {
        Detach(*this);
        btnActivate->Detach(*this);
        btnCancel->Detach(*this);
        delete editEmail; delete editKey; delete btnActivate; delete btnCancel; delete txtStatus;
    }

    void ActivationDialog::ButtonClicked(const DG::ButtonClickEvent& ev) {
        if (ev.GetSource() == btnActivate) {
            txtStatus->SetText("Checking license...");
            GS::UniString email = editEmail->GetText();
            GS::UniString key = editKey->GetText();

            if (CheckLicenseOnline(email, key)) {
                SaveLicenseLocal(email, key);
                PostCloseRequest(DG::ModalDialog::Accept);
            } else {
                txtStatus->SetText("Activation failed. Invalid email or key.");
            }
        } else if (ev.GetSource() == btnCancel) {
            PostCloseRequest(DG::ModalDialog::Cancel);
        }
    }


    // ==========================================
    // MagicPalette Implementation
    // ==========================================

    MagicPalette* MagicPalette::instance = nullptr;

    void MagicPalette::Show() {
        CheckForUpdates(); // Automatically check for updates once per session
        if (instance == nullptr) {
            instance = new MagicPalette();
        }
        if (instance != nullptr) {
            instance->DG::Palette::Show();
        }
    }

    void MagicPalette::Destroy() {
        if (instance != nullptr) {
            delete instance;
            instance = nullptr;
        }
    }

    GSErrCode __ACDLL_CALL MagicPalette::StaticCallback(Int32, API_PaletteMessageID msg, GS::IntPtr) {
        if (msg == APIPalMsg_OpenPalette) Show();
        else if (msg == APIPalMsg_ClosePalette) Destroy();
        return NoError;
    }

    MagicPalette::MagicPalette() : DG::Palette(ACAPI_GetOwnResModule(), 32500, ACAPI_GetOwnResModule(), customAddonPaletteGuid) {
        BeginEventProcessing();
        
        btnAutoCreate = new DG::Button(GetReference(), 1);
        btnBrowse = new DG::Button(GetReference(), 2);
        txtExcelPath = new DG::LeftText(GetReference(), 4);
        editTargetArea = new DG::RealEdit(GetReference(), 9);
        editTolerance = new DG::RealEdit(GetReference(), 13);
        btnReshape = new DG::Button(GetReference(), 16);
        btnSumSelected = new DG::Button(GetReference(), 17);
        txtTotalArea = new DG::LeftText(GetReference(), 19);
        rbZone = new DG::RadioButton(GetReference(), 22);
        rbMesh = new DG::RadioButton(GetReference(), 23);
        btnCreate = new DG::Button(GetReference(), 24);
        rbMorph = new DG::RadioButton(GetReference(), 25);
        editHeight = new DG::RealEdit(GetReference(), 41);
        editUnit = new DG::RealEdit(GetReference(), 11);
        checkFixX = new DG::CheckBox(GetReference(), 14);
        checkFixY = new DG::CheckBox(GetReference(), 15);
        editPrefix = new DG::TextEdit(GetReference(), 29);
        btnPickPrefix = new DG::Button(GetReference(), 30);
        popStyle = new DG::PopUp(GetReference(), 32);
        editStartNo = new DG::TextEdit(GetReference(), 34);
        editPreview = new DG::MultiLineEdit(GetReference(), 36);
        btnApplyLabeling = new DG::Button(GetReference(), 37);
        btnCalcRange = new DG::Button(GetReference(), 38);
        txtAllowedRange = new DG::LeftText(GetReference(), 42);
        btnSimplifyPoly = new DG::Button(GetReference(), 43);
        txtSimplifyTolLabel = new DG::LeftText(GetReference(), 44);
        editSimplifyTol = new DG::RealEdit(GetReference(), 45);
        checkPreserveCurve = new DG::CheckBox(GetReference(), 46);
        btnCheckUpdate = new DG::Button(GetReference(), 47);
        txtVersion = new DG::LeftText(GetReference(), 48);

        // Attach
        Attach(*this);
        btnAutoCreate->Attach(*this);
        btnBrowse->Attach(*this);
        btnReshape->Attach(*this);
        btnSumSelected->Attach(*this);
        rbZone->Attach(*this);
        rbMesh->Attach(*this);
        btnCreate->Attach(*this);
        rbMorph->Attach(*this);
        btnPickPrefix->Attach(*this);
        btnApplyLabeling->Attach(*this);
        btnCalcRange->Attach(*this);
        editPrefix->Attach(*this);
        editStartNo->Attach(*this);
        popStyle->Attach(*this);
        btnSimplifyPoly->Attach(*this);
        btnCheckUpdate->Attach(*this);

        // Styles
        popStyle->InsertItem(1); popStyle->SetItemText(1, "a, b, c, ...");
        popStyle->InsertItem(2); popStyle->SetItemText(2, "A, B, C, ...");
        popStyle->InsertItem(3); popStyle->SetItemText(3, "1, 2, 3, ...");
        popStyle->InsertItem(4); popStyle->SetItemText(4, "01, 02, 03, ...");
        popStyle->InsertItem(5); popStyle->SetItemText(5, "001, 002, 003, ...");
        popStyle->InsertItem(6); popStyle->SetItemText(6, "0001, 0002, 0003, ...");
        popStyle->InsertItem(7); popStyle->SetItemText(7, GS::UniString("No ID", CC_UTF8));
        popStyle->SelectItem(5);

        editPrefix->SetText("ZONE-");
        editStartNo->SetText("1");
        rbZone->Select();
        txtVersion->SetText(MyProjectNamespace::GetCurrentVersion());
        UpdatePreview();
        editSimplifyTol->SetValue(50.0);
    }

    MagicPalette::~MagicPalette() {
        EndEventProcessing();
        Detach(*this);
        
        delete btnAutoCreate; delete btnBrowse; delete txtExcelPath; delete editTargetArea; delete editTolerance;
        delete btnReshape; delete btnSumSelected; delete txtTotalArea; delete rbZone; delete rbMesh;
        delete btnCreate; delete rbMorph; delete editHeight; delete editPrefix; delete editUnit;
        delete checkFixX; delete checkFixY; delete btnPickPrefix; delete popStyle; delete editStartNo;
        delete editPreview; delete btnApplyLabeling; delete btnCalcRange; delete txtAllowedRange;
        delete btnSimplifyPoly; delete txtSimplifyTolLabel; delete editSimplifyTol; delete checkPreserveCurve;
        delete btnCheckUpdate; delete txtVersion;
    }

    void MagicPalette::SimplifyPolyline() {
        double tolMM = (editSimplifyTol ? editSimplifyTol->GetValue() : 50.0);
        bool preserveCurve = (checkPreserveCurve && checkPreserveCurve->IsChecked());
        AddOnLogic::SimplifyPolyline(tolMM, preserveCurve);
    }

    void MagicPalette::CreateFromSelection() {
        double hVal = editHeight ? editHeight->GetValue() : 0.0;
        bool isZone = rbZone->IsSelected();
        bool isMesh = rbMesh->IsSelected();
        bool isMorph = rbMorph->IsSelected();
        AddOnLogic::CreateFromSelection(hVal, isZone, isMesh, isMorph);
    }

    void MagicPalette::UpdatePreview() {
        if (popStyle == nullptr || editPrefix == nullptr || editStartNo == nullptr || editPreview == nullptr) return;
        
        GS::UniString prefix = editPrefix->GetText();
        Int32 start = (Int32)atoi(editStartNo->GetText().ToCStr().Get());
        if (start == 0 && editStartNo->GetText() != "0") start = 1;
        
        auto getStyled = [&](int style, int val) -> GS::UniString {
            int idx = val;
            if (style == 1) return GS::UniString::Printf("%c", 'a' + (idx - 1) % 26);
            if (style == 2) return GS::UniString::Printf("%c", 'A' + (idx - 1) % 26);
            if (style == 4) return GS::UniString::Printf("%02d", idx);
            if (style == 5) return GS::UniString::Printf("%03d", idx);
            if (style == 6) return GS::UniString::Printf("%04d", idx);
            if (style == 7) return GS::UniString("No ID", CC_UTF8);
            return GS::UniString::Printf("%d", idx);
        };

        int style = popStyle->GetSelectedItem();
        GS::UniString preview;
        for (int i = 0; i < 3; ++i) {
            preview += prefix + getStyled(style, start + i) + "\n";
        }
        preview.TrimRight();
        editPreview->SetText(preview);
    }

    void MagicPalette::ApplyLabeling() {
        GS::UniString prefix = editPrefix->GetText();
        Int32 start = (Int32)atoi(editStartNo->GetText().ToCStr().Get());
        if (start == 0 && editStartNo->GetText() != "0") start = 1;
        int style = popStyle->GetSelectedItem();

        AddOnLogic::ApplyLabeling(prefix, start, style);
    }

    void MagicPalette::UpdateTotalArea() {
        double totalArea = 0.0;
        int count = 0;
        AddOnLogic::UpdateTotalArea(totalArea, count);
        if (txtTotalArea) {
            txtTotalArea->SetText(GS::UniString::Printf("%.2f", totalArea));
            txtTotalArea->Redraw();
        }
    }

    void MagicPalette::TextEditChanged(const DG::TextEditChangeEvent&) { UpdatePreview(); }
    void MagicPalette::PopUpChanged(const DG::PopUpChangeEvent&) { UpdatePreview(); }

    void MagicPalette::PanelCloseRequested(const DG::PanelCloseRequestEvent&, bool* accept) {
        *accept = true;
        Hide();
    }

    void MagicPalette::ButtonClicked(const DG::ButtonClickEvent& ev) {
        DG::Item* item = (DG::Item*) ev.GetSource();
        if (item == nullptr) return;
        short id = item->GetId();
        ACAPI_WriteReport(GS::UniString::Printf("[Click] ID: %d", id), false);

        if (id == 2) { // Browse
            DG::FileDialog fd (DG::FileDialog::OpenFile);
            fd.SetTitle (GS::UniString("Select Excel File", CC_UTF8));
            
            FTM::FileTypeManager ftm ("ZoneHelperManager");
            FTM::FileType excelType ("Excel File", "xlsx", 0, 0, 0);
            FTM::TypeID typeID = ftm.AddType (excelType);
            fd.AddFilter (typeID);

            if (fd.Invoke ()) {
                IO::Location loc = fd.GetSelectedFile ();
                GS::UniString path;
                loc.ToPath (&path);
                txtExcelPath->SetText (path);
                ACAPI_WriteReport (GS::UniString ("File Selected: ") + path, false);
            }
        }
        else if (id == 1) { // Auto Create from Excel
            GS::UniString excelPath = txtExcelPath ? txtExcelPath->GetText() : "";
            AddOnLogic::AutoCreateZonesFromExcel(excelPath);
        }
        else if (id == 16) { // Reshape
            double targetArea = editTargetArea ? editTargetArea->GetValue() : 0.0;
            double unit = editUnit ? editUnit->GetValue() : 0.0;
            bool fixX = checkFixX ? checkFixX->IsChecked() : false;
            bool fixY = checkFixY ? checkFixY->IsChecked() : false;
            AddOnLogic::ReshapeElements(targetArea, unit, fixX, fixY);
        }
        else if (id == 17) UpdateTotalArea();
        else if (id == 37) ApplyLabeling();
        else if (id == 30) { // Pick
            API_SelectionInfo si; BNZeroMemory(&si, sizeof(si));
            GS::Array<API_Neig> sel;
            if (ACAPI_Selection_Get(&si, &sel, false) == NoError) {
                if (!sel.IsEmpty()) {
                    API_Element el; BNZeroMemory(&el, sizeof(el)); el.header.guid = sel[0].guid;
                    if (ACAPI_Element_Get(&el) == NoError) {
                        if (el.header.type.typeID == API_TextID || el.header.type.typeID == API_LabelID) {
                            API_ElementMemo memo; BNZeroMemory(&memo, sizeof(memo));
#if defined(ARCHICAD_VERSION_28) || defined(ARCHICAD_VERSION_29)
                            if (ACAPI_Element_GetMemo(el.header.guid, &memo, APIMemoMask_TextContent) == NoError && memo.textContent != nullptr) {
                                editPrefix->SetText(*memo.textContent);
                                ACAPI_DisposeElemMemoHdls(&memo);
                            }
#else
                            if (ACAPI_Element_GetMemo(el.header.guid, &memo, APIMemoMask_TextContent) == NoError && memo.textContent != nullptr && *memo.textContent != nullptr) {
                                editPrefix->SetText(GS::UniString(*memo.textContent, CC_UTF8));
                                ACAPI_DisposeElemMemoHdls(&memo);
                            }
#endif
                        } else {
                            API_ElementMemo memo; BNZeroMemory(&memo, sizeof(memo));
                            if (ACAPI_Element_GetMemo(el.header.guid, &memo, APIMemoMask_ElemInfoString) == NoError && memo.elemInfoString != nullptr) {
                                editPrefix->SetText(*memo.elemInfoString);
                                ACAPI_DisposeElemMemoHdls(&memo);
                            }
                        }
                        if (popStyle) {
                            popStyle->SelectItem(7);
                        }
                        UpdatePreview();
                    }
                }
            }
        }
        else if (id == 38) { // Calc
            double targetArea = editTargetArea->GetValue();
            double tolerance = editTolerance->GetValue();
            double minArea = targetArea * (1.0 - tolerance / 100.0);
            double maxArea = targetArea * (1.0 + tolerance / 100.0);
            
            GS::UniString resultStr = GS::UniString("Range: ", CC_UTF8) + GS::UniString::Printf("%.2f ~ %.2f", minArea, maxArea);
            txtAllowedRange->SetText(resultStr);
            txtAllowedRange->Redraw();
            ACAPI_WriteReport(GS::UniString::Printf("[Calc] Target=%.2f, Tol=%.2f%% -> %.2f ~ %.2f", targetArea, tolerance, minArea, maxArea), false);
        }
        else if (id == 24) { // Create
            CreateFromSelection();
        }
        else if (id == 43) { // Simplify Polyline
            SimplifyPolyline();
        }
        else if (id == 47) { // Check Update
            CheckForUpdates(true);
        }
    }


    // ==========================================
    // SecondPalette Implementation
    // ==========================================

    SecondPalette* SecondPalette::instance = nullptr;

    void SecondPalette::Show() {
        if (instance == nullptr) {
            instance = new SecondPalette();
        }
        if (instance != nullptr) {
            instance->DG::Palette::Show();
        }
    }

    void SecondPalette::Destroy() {
        if (instance != nullptr) {
            delete instance;
            instance = nullptr;
        }
    }

    GSErrCode __ACDLL_CALL SecondPalette::StaticCallback(Int32, API_PaletteMessageID msg, GS::IntPtr) {
        if (msg == APIPalMsg_OpenPalette) Show();
        else if (msg == APIPalMsg_ClosePalette) Destroy();
        return NoError;
    }

    SecondPalette::SecondPalette() : DG::Palette(ACAPI_GetOwnResModule(), 32501, ACAPI_GetOwnResModule(), secondAddonPaletteGuid), origWidth(0), origHeight(0) {
        BeginEventProcessing();
        
        editWidth        = new DG::RealEdit(GetReference(), 3);
        editHeight       = new DG::RealEdit(GetReference(), 5);
        checkLockRatio   = new DG::CheckBox(GetReference(), 6);
        btnMatchDrawing  = new DG::Button(GetReference(), 7);
        btn05x           = new DG::Button(GetReference(), 9);
        btn20x           = new DG::Button(GetReference(), 10);
        btn40x           = new DG::Button(GetReference(), 11);
        btnApplyTarget   = new DG::Button(GetReference(), 12);
        btnRestore       = new DG::Button(GetReference(), 13);

        Attach(*this);
        editWidth->Attach(*this);
        editHeight->Attach(*this);
        btnMatchDrawing->Attach(*this);
        btn05x->Attach(*this);
        btn20x->Attach(*this);
        btn40x->Attach(*this);
        btnApplyTarget->Attach(*this);
        btnRestore->Attach(*this);

        // Fetch current 3D window resolution
        API_3DWindowInfo windowInfo;
        BNZeroMemory(&windowInfo, sizeof(API_3DWindowInfo));
        if (ACAPI_View_Get3DWindowSets(&windowInfo) == NoError) {
            origWidth = windowInfo.hSize;
            origHeight = windowInfo.vSize;
            editWidth->SetValue((double)origWidth);
            editHeight->SetValue((double)origHeight);
        }

        SetTitle("3D Resolution Helper");
    }

    SecondPalette::~SecondPalette() {
        EndEventProcessing();
        Detach(*this);
        
        delete editWidth; delete editHeight; delete checkLockRatio; delete btnMatchDrawing;
        delete btn05x; delete btn20x; delete btn40x; delete btnApplyTarget; delete btnRestore;
    }

    void SecondPalette::Resize3DWindow(double scale, Int32 targetW, Int32 targetH) {
        AddOnLogic::Resize3D(scale, targetW, targetH, origWidth, origHeight);
    }

    void SecondPalette::RealEditChanged(const DG::RealEditChangeEvent& ev) {
        if (checkLockRatio->IsChecked()) {
            double w = editWidth->GetValue();
            double h = editHeight->GetValue();
            static bool isSyncing = false;
            if (isSyncing) return;
            isSyncing = true;
            
            if (ev.GetSource() == editWidth && origHeight > 0 && origWidth > 0) {
                editHeight->SetValue(w * (double)origHeight / (double)origWidth);
            } else if (ev.GetSource() == editHeight && origWidth > 0 && origHeight > 0) {
                editWidth->SetValue(h * (double)origWidth / (double)origHeight);
            }
            isSyncing = false;
        }
    }

    void SecondPalette::ButtonClicked(const DG::ButtonClickEvent& ev) {
        if (ev.GetSource() == btnMatchDrawing) {
            double ratio = 0.0;
            if (AddOnLogic::MatchDrawingAspectRatio(ratio) && ratio > 0.0) {
                double currentW = editWidth->GetValue();
                editHeight->SetValue(currentW / ratio);
                ACAPI_WriteReport("Matched Drawing Aspect Ratio.", true);
            }
        } else if (ev.GetSource() == btn05x) {
            Resize3DWindow(0.5);
        } else if (ev.GetSource() == btn20x) {
            Resize3DWindow(2.0);
        } else if (ev.GetSource() == btn40x) {
            Resize3DWindow(4.0);
        } else if (ev.GetSource() == btnApplyTarget) {
            Resize3DWindow(1.0, (Int32)editWidth->GetValue(), (Int32)editHeight->GetValue());
        } else if (ev.GetSource() == btnRestore) {
            Resize3DWindow(1.0, origWidth, origHeight);
        }
    }

    void SecondPalette::PanelCloseRequested(const DG::PanelCloseRequestEvent&, bool* accept) {
        *accept = true;
        DG::Palette::Hide();
    }


    // ==========================================
    // HeightPalette Implementation
    // ==========================================

    HeightPalette* HeightPalette::instance = nullptr;

    void HeightPalette::Show() {
        if (instance == nullptr) {
            instance = new HeightPalette();
        }
        if (instance != nullptr) {
            instance->DG::Palette::Show();
        }
    }

    void HeightPalette::Destroy() {
        if (instance != nullptr) {
            delete instance;
            instance = nullptr;
        }
    }

    GSErrCode __ACDLL_CALL HeightPalette::StaticCallback(Int32, API_PaletteMessageID msg, GS::IntPtr) {
        if (msg == APIPalMsg_OpenPalette) Show();
        else if (msg == APIPalMsg_ClosePalette) Destroy();
        return NoError;
    }

    HeightPalette::HeightPalette() : DG::Palette(ACAPI_GetOwnResModule(), 32502, ACAPI_GetOwnResModule(), heightAddonPaletteGuid), lastFinalZ(0.0), lastFloorInd(0) {
        BeginEventProcessing();
        
        editBaseElevation = new DG::LeftText(GetReference(), 4);
        btnCalculate = new DG::Button(GetReference(), 5);
        txtAreaResult = new DG::LeftText(GetReference(), 8);
        txtPerimeterResult = new DG::LeftText(GetReference(), 10);
        txtAverageHeight = new DG::LeftText(GetReference(), 12);
        btnCreateGround = new DG::Button(GetReference(), 13);

        Attach(*this);
        btnCalculate->Attach(*this);
        btnCreateGround->Attach(*this);
        SetTitle("GL Calc");
    }

    HeightPalette::~HeightPalette() {
        EndEventProcessing();
        btnCalculate->Detach(*this);
        btnCreateGround->Detach(*this);
        Detach(*this);

        delete editBaseElevation; delete btnCalculate; delete txtAreaResult;
        delete txtPerimeterResult; delete txtAverageHeight; delete btnCreateGround;
    }

    void HeightPalette::ButtonClicked(const DG::ButtonClickEvent& ev) {
        if (ev.GetSource() == btnCalculate) Calc();
        else if (ev.GetSource() == btnCreateGround) CreateG();
    }

    void HeightPalette::Calc() {
        API_SelectionInfo si; GS::Array<API_Neig> sel;
        if (ACAPI_Selection_Get(&si, &sel, false) != NoError || sel.IsEmpty()) return;

        API_Guid moG = APINULLGuid, meG = APINULLGuid;
        for (auto& n : sel) {
            API_Elem_Head h; h.guid = n.guid;
            if (ACAPI_Element_GetHeader(&h) == NoError) {
                if (h.type == API_MorphID) moG = h.guid;
                else if (h.type == API_MeshID) meG = h.guid;
            }
        }
        if (moG == APINULLGuid || meG == APINULLGuid) return;

        AddOnLogic::GLResult res = {};
        if (AddOnLogic::CalculateGL(moG, meG, res, lastFootprint, lastFloorInd)) {
            lastFinalZ = res.finalZ;
            editBaseElevation->SetText(GS::UniString::Printf("Base: %.2f / MaxRel: %.2f", res.baseZ, res.maxRelZ));
            txtAreaResult->SetText(GS::UniString::Printf("Contact: %.2f m2 (%d pts)", res.contactArea, res.ptCount));
            txtAverageHeight->SetText(GS::UniString::Printf("%.2f + %.2f = GL %.2f m", res.baseZ, (res.finalZ - res.baseZ), res.finalZ));
        }
    }

    void HeightPalette::CreateG() {
        if (lastFootprint.empty()) return;
        AddOnLogic::CreateGLGround(lastFootprint, lastFinalZ, lastFloorInd);
    }

    void HeightPalette::PanelCloseRequested(const DG::PanelCloseRequestEvent&, bool* accept) {
        *accept = true;
        DG::Palette::Hide();
    }


    // ==========================================
    // TerrainPalette Implementation
    // ==========================================

    TerrainPalette* TerrainPalette::instance = nullptr;

    void TerrainPalette::Show() {
        if (instance == nullptr) {
            instance = new TerrainPalette();
        }
        if (instance != nullptr) {
            instance->DG::Palette::Show();
        }
    }

    void TerrainPalette::Destroy() {
        if (instance != nullptr) {
            delete instance;
            instance = nullptr;
        }
    }

    GSErrCode __ACDLL_CALL TerrainPalette::StaticCallback(Int32, API_PaletteMessageID msg, GS::IntPtr) {
        if (msg == APIPalMsg_OpenPalette) Show();
        else if (msg == APIPalMsg_ClosePalette) Destroy();
        return NoError;
    }

    TerrainPalette::TerrainPalette() : DG::Palette(ACAPI_GetOwnResModule(), 32503, ACAPI_GetOwnResModule(), terrainAddonPaletteGuid) {
        BeginEventProcessing();
        
        editDivisions = new DG::RealEdit(GetReference(), 2);
        editPenIndex = new DG::RealEdit(GetReference(), 4);
        btnGenerate = new DG::Button(GetReference(), 5);

        Attach(*this);
        btnGenerate->Attach(*this);

        SetTitle("Terrain");
        editDivisions->SetValue(5.0);
        editPenIndex->SetValue(1.0);
    }

    TerrainPalette::~TerrainPalette() {
        EndEventProcessing();
        btnGenerate->Detach(*this);
        Detach(*this);

        delete editDivisions; delete editPenIndex; delete btnGenerate;
    }

    void TerrainPalette::ButtonClicked(const DG::ButtonClickEvent& ev) {
        if (ev.GetSource() == btnGenerate) {
            GenerateContours();
        }
    }

    void TerrainPalette::GenerateContours() {
        double divisions = editDivisions->GetValue();
        short penIndex = (short)editPenIndex->GetValue();
        if (penIndex < 1) penIndex = 1;
        if (penIndex > 255) penIndex = 255;

        AddOnLogic::GenerateContours(divisions, penIndex);
    }

    void TerrainPalette::PanelCloseRequested(const DG::PanelCloseRequestEvent&, bool* accept) {
        *accept = true;
        DG::Palette::Hide();
    }

} // namespace MyProjectNamespace
