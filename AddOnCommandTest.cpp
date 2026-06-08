#include "ACAPinc.h"

// Body index types - same for both AC27 and AC29
using VertIdx = UInt32;
using EdgeIdx = Int32;
using PolyIdx = UInt32;

#ifndef __ACDLL_CALL
#define __ACDLL_CALL
#endif

#ifdef WINDOWS
#include <windows.h>
#endif
#include "DGButton.hpp"
#include "DGCheckItem.hpp"
#include "DGDialog.hpp"
#include "DGEditControl.hpp"
#include "DGPopUp.hpp"
#include "DGRadioItem.hpp"
#include "DGStaticItem.hpp"
#include "DGFileDialog.hpp"
#include "FileTypeManager.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <cmath>
#ifdef WINDOWS
#include <wininet.h>
#include <shellapi.h>
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "shell32.lib")
#endif

#include <vector>
#include <algorithm>

#pragma warning(disable : 4828)

static const GS::Guid customAddonPaletteGuid("{A06821CF-7C13-4BCE-A14E-002D5A8FCE53}");
static const GS::Guid secondAddonPaletteGuid("{B17932DF-8D24-4CDE-B25F-113D6A9FDE64}");
static const GS::Guid heightAddonPaletteGuid("{C28043E0-9E35-4DEF-C36A-224E7B0FDF75}");
static const GS::Guid terrainAddonPaletteGuid("{D39154F1-AF46-4EFE-D47B-335F8C10EF86}");

namespace MyProjectNamespace {

// --- Auto Updater ---
static const char* CURRENT_VERSION = "v1.0.0";
static const char* VERSION_URL = "https://raw.githubusercontent.com/parkjoohyung/archicad_addon_bam/main/version.txt";
static const char* RELEASES_URL = "https://github.com/parkjoohyung/archicad_addon_bam/releases/latest";

static void CheckForUpdates(bool manual = false) {
    static bool updateChecked = false;
    if (updateChecked && !manual) return;
    updateChecked = true;

    HINTERNET hInternet = InternetOpenA("ArchiCAD_Addon_Updater", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return;
    HINTERNET hConnect = InternetOpenUrlA(hInternet, VERSION_URL, NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (hConnect) {
        char buffer[128]; DWORD bytesRead; std::string content;
        while (InternetReadFile(hConnect, buffer, sizeof(buffer)-1, &bytesRead) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            content += buffer;
        }
        InternetCloseHandle(hConnect);
        
        content.erase(content.find_last_not_of(" \n\r\t") + 1);
        content.erase(0, content.find_first_not_of(" \n\r\t"));
        
        if (!content.empty()) {
            GS::UniString latestVersion(content.c_str(), CC_UTF8);
            latestVersion.TrimLeft(); latestVersion.TrimRight();
            GS::UniString currentVersion(CURRENT_VERSION, CC_UTF8);

            if (latestVersion != currentVersion) {
                GS::UniString title = "Update Notification";
                GS::UniString msg = GS::UniString::Printf("A new update is available!\n\nLatest version: %s\nCurrent version: %s\n\nWould you like to go to the download page now?", 
                                                          latestVersion.ToCStr().Get(), currentVersion.ToCStr().Get());
                
                const WCHAR* pMsg = (const WCHAR*)msg.ToUStr().Get();
                const WCHAR* pTitle = (const WCHAR*)title.ToUStr().Get();
                if (::MessageBoxW(nullptr, pMsg, pTitle, MB_YESNO | MB_ICONINFORMATION | MB_TOPMOST) == IDYES) {
                    ShellExecuteA(NULL, "open", RELEASES_URL, NULL, NULL, SW_SHOWNORMAL);
                }
            } else if (manual) {
                GS::UniString title = "Update Notification";
                GS::UniString msg = "You are using the latest version.";
                const WCHAR* pMsg = (const WCHAR*)msg.ToUStr().Get();
                const WCHAR* pTitle = (const WCHAR*)title.ToUStr().Get();
                ::MessageBoxW(nullptr, pMsg, pTitle, MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
            }
        }
    }
    InternetCloseHandle(hInternet);
}

// --- Licensing and Security Logic ---

static GS::UniString ComputeLicenseHash(const GS::UniString& email, const GS::UniString& key) {
    std::string combined = email.ToCStr().Get();
    combined += "|";
    combined += key.ToCStr().Get();
    combined += "|Salt!Secret2026"; // Secret Salt to prevent easy decryption

    // Simple FNV-1a Hash
    uint64_t hash = 0xcbf29ce484222325ULL;
    for (unsigned char c : combined) {
        hash ^= (uint64_t)c;
        hash *= 0x100000001b3ULL;
    }
    return GS::UniString::Printf("%016llx", hash);
}

static bool CheckLicenseOnline(const GS::UniString& email, const GS::UniString& key) {
    HINTERNET hInternet = InternetOpenA("ArchiCAD-Addon", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return false;

    // GitHub Raw URL for parkjoohyung's repo
    const char* url = "https://raw.githubusercontent.com/parkjoohyung/archicad_addon_bam/main/licenses.txt";
    
    HINTERNET hUrl = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hUrl) { InternetCloseHandle(hInternet); return false; }

    char buffer[4096]; DWORD read;
    std::string content;
    while (InternetReadFile(hUrl, buffer, sizeof(buffer)-1, &read) && read > 0) {
        buffer[read] = '\0';
        content += buffer;
    }

    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);

    GS::UniString myHash = ComputeLicenseHash(email, key);
    std::stringstream ss(content);
    std::string line;
    while (std::getline(ss, line)) {
        // Remove trailing \r or \n
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
        if (myHash == GS::UniString(line.c_str())) return true;
    }

    return false;
}

static bool IsAlreadyLicensed() {
    char emailBuf[255] = {0}, keyBuf[255] = {0};
    DWORD szEmail = 255, szKey = 255;
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\ArchiCAD_Premium_Addon", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        bool ok = (RegQueryValueExA(hKey, "Email", NULL, NULL, (BYTE*)emailBuf, &szEmail) == ERROR_SUCCESS &&
                   RegQueryValueExA(hKey, "Key", NULL, NULL, (BYTE*)keyBuf, &szKey) == ERROR_SUCCESS);
        RegCloseKey(hKey);
        if (ok) {
            // Optional: You could cache this to avoid checking online every time
            return CheckLicenseOnline(GS::UniString(emailBuf), GS::UniString(keyBuf));
        }
    }
    return false;
}

class ActivationDialog : public DG::ModalDialog,
                         public DG::PanelObserver,
                         public DG::ButtonItemObserver
{
private:
    DG::TextEdit* editEmail;
    DG::TextEdit* editKey;
    DG::Button* btnActivate;
    DG::Button* btnCancel;
    DG::LeftText* txtStatus;

public:
    ActivationDialog() : DG::ModalDialog(ACAPI_GetOwnResModule(), 32510, ACAPI_GetOwnResModule()) {
        editEmail = new DG::TextEdit(GetReference(), 3);
        editKey = new DG::TextEdit(GetReference(), 5);
        btnActivate = new DG::Button(GetReference(), 6);
        btnCancel = new DG::Button(GetReference(), 7);
        txtStatus = new DG::LeftText(GetReference(), 8);

        Attach(*this);
        btnActivate->Attach(*this);
        btnCancel->Attach(*this);
    }

    ~ActivationDialog() {
        Detach(*this);
        btnActivate->Detach(*this);
        btnCancel->Detach(*this);
        delete editEmail; delete editKey; delete btnActivate; delete btnCancel; delete txtStatus;
    }

    virtual void ButtonClicked(const DG::ButtonClickEvent& ev) override {
        if (ev.GetSource() == btnActivate) {
            txtStatus->SetText("Checking license...");
            GS::UniString email = editEmail->GetText();
            GS::UniString key = editKey->GetText();

            if (CheckLicenseOnline(email, key)) {
                // Success: Save to Registry
#ifdef WINDOWS
                HKEY hKey;
                if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\ArchiCAD_Premium_Addon", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
                    RegSetValueExA(hKey, "Email", 0, REG_SZ, (const BYTE*)email.ToCStr().Get(), (DWORD)strlen(email.ToCStr().Get())+1);
                    RegSetValueExA(hKey, "Key", 0, REG_SZ, (const BYTE*)key.ToCStr().Get(), (DWORD)strlen(key.ToCStr().Get())+1);
                    RegCloseKey(hKey);
                }
#endif
                PostCloseRequest(DG::ModalDialog::Accept);
            } else {
                txtStatus->SetText("Activation failed. Invalid email or key.");
            }
        } else if (ev.GetSource() == btnCancel) {
            PostCloseRequest(DG::ModalDialog::Cancel);
        }
    }
};

namespace AddOnLogic {
    void SimplifyPolyline(double tolMM, bool preserveCurve) {
        API_SelectionInfo si; GS::Array<API_Neig> sel;
        if (ACAPI_Selection_Get(&si, &sel, false) != NoError || sel.IsEmpty()) {
            ACAPI_WriteReport("Please select elements first.", CC_UTF8);
            return;
        }

        ACAPI_CallUndoableCommand("Simplify Polyline", [&]() -> GSErrCode {
            int updatedCount = 0;
            for (auto &n : sel) {
                API_Element el; BNZeroMemory(&el, sizeof(el)); el.header.guid = n.guid;
                if (ACAPI_Element_Get(&el) != NoError) continue;

                API_ElementMemo memo; BNZeroMemory(&memo, sizeof(memo));
                if (ACAPI_Element_GetMemo(el.header.guid, &memo) != NoError) continue;
                if (!memo.coords || !*memo.coords) { ACAPI_DisposeElemMemoHdls(&memo); continue; }

                Int32 nCoords = 0, nSubPolys = 0, nArcs = 0;
                if (el.header.type.typeID == API_PolyLineID) {
                    nCoords = el.polyLine.poly.nCoords;
                    nSubPolys = el.polyLine.poly.nSubPolys;
                    nArcs = el.polyLine.poly.nArcs;
                } else {
                    ACAPI_DisposeElemMemoHdls(&memo);
                    continue;
                }

                if (nCoords < 3 || nArcs > 0 || nSubPolys > 1) { 
                    ACAPI_DisposeElemMemoHdls(&memo); 
                    continue; 
                }

                std::vector<API_Coord> points;
                for (Int32 i = 1; i <= nCoords; ++i) {
                    points.push_back((*memo.coords)[i]);
                }

                double epsilon = tolMM / 1000.0;
                std::vector<API_Coord> simplified;

                if (preserveCurve) {
                    simplified.push_back(points[0]);
                    double tolAngle = epsilon * 0.4;
                    if (tolAngle < 0.005) tolAngle = 0.005;
                    if (tolAngle > 0.1) tolAngle = 0.1;

                    for (size_t i = 1; i + 1 < points.size(); ++i) {
                        const API_Coord& prev = simplified.back();
                        const API_Coord& curr = points[i];
                        const API_Coord& next = points[i + 1];

                        double d = std::sqrt(std::pow(curr.x - prev.x, 2) + std::pow(curr.y - prev.y, 2));
                        if (d < epsilon) continue;

                        double dx1 = curr.x - prev.x, dy1 = curr.y - prev.y;
                        double dx2 = next.x - curr.x, dy2 = next.y - curr.y;
                        double mag1 = std::sqrt(dx1*dx1 + dy1*dy1), mag2 = std::sqrt(dx2*dx2 + dy2*dy2);
                        if (mag1 > 0 && mag2 > 0) {
                            double cross = std::abs(dx1 * dy2 - dy1 * dx2);
                            double dot = dx1 * dx2 + dy1 * dy2;
                            if (cross / (mag1 * mag2) < tolAngle && dot > 0) continue;
                        }
                        simplified.push_back(curr);
                    }
                    simplified.push_back(points.back());
                } else {
                    std::vector<bool> keep(points.size(), false);
                    keep[0] = true;
                    keep[points.size() - 1] = true;
                    
                    std::vector<std::pair<int, int>> stack;
                    stack.push_back({0, (int)points.size() - 1});
                    
                    while (!stack.empty()) {
                        auto [si2, ei] = stack.back();
                        stack.pop_back();
                        
                        double maxDist = 0;
                        int maxIdx = si2;
                        
                        double dx = points[ei].x - points[si2].x;
                        double dy = points[ei].y - points[si2].y;
                        double lineLenSq = dx * dx + dy * dy;
                        
                        for (int k = si2 + 1; k < ei; ++k) {
                            double dist;
                            if (lineLenSq < 1e-20) {
                                dist = std::sqrt(std::pow(points[k].x - points[si2].x, 2) + std::pow(points[k].y - points[si2].y, 2));
                            } else {
                                double cross = std::abs((points[k].x - points[si2].x) * dy - (points[k].y - points[si2].y) * dx);
                                dist = cross / std::sqrt(lineLenSq);
                            }
                            if (dist > maxDist) {
                                maxDist = dist;
                                maxIdx = k;
                            }
                        }
                        
                        if (maxDist > epsilon) {
                            keep[maxIdx] = true;
                            if (maxIdx - si2 > 1) stack.push_back({si2, maxIdx});
                            if (ei - maxIdx > 1) stack.push_back({maxIdx, ei});
                        }
                    }
                    
                    for (size_t i = 0; i < points.size(); ++i) {
                        if (keep[i]) simplified.push_back(points[i]);
                    }
                }
                Int32 newNCoords = (Int32)simplified.size();
                ACAPI_WriteReport(GS::UniString::Printf("[Debug] Type=%d Nodes: %d -> %d", el.header.type.typeID, nCoords, newNCoords), false);

                if (newNCoords < nCoords && newNCoords >= 2) {
                    GS::Array<API_Guid> guidArr;
                    guidArr.Push(el.header.guid);
                    if (ACAPI_Element_Delete(guidArr) != NoError) continue;

                    API_Element newEl; BNZeroMemory(&newEl, sizeof(newEl));
                    newEl = el;
                    newEl.header.guid = APINULLGuid;
                    newEl.polyLine.poly.nCoords = newNCoords;
                    newEl.polyLine.poly.nSubPolys = 1;
                    newEl.polyLine.poly.nArcs = 0;

                    API_ElementMemo newMemo; BNZeroMemory(&newMemo, sizeof(newMemo));
                    newMemo.coords = (API_Coord**)BMAllocateHandle((newNCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
                    newMemo.pends = (Int32**)BMAllocateHandle(2 * sizeof(Int32), ALLOCATE_CLEAR, 0);

                    if (newMemo.coords && newMemo.pends) {
                        for (Int32 j = 0; j < newNCoords; ++j) {
                            (*newMemo.coords)[j + 1] = simplified[j];
                        }
                        (*newMemo.pends)[0] = 0;
                        (*newMemo.pends)[1] = newNCoords;

                        GSErrCode createErr = ACAPI_Element_Create(&newEl, &newMemo);
                        if (createErr == NoError) {
                            updatedCount++;
                        } else {
                            ACAPI_WriteReport(GS::UniString::Printf("[Debug] Create failed: %d", createErr), false);
                        }
                    }
                    ACAPI_DisposeElemMemoHdls(&newMemo);
                }
            } // Close for (auto &n : sel)
            return NoError;
        });
    }
};

class MagicPalette : public DG::Palette,
                     public DG::PanelObserver,
                     public DG::ButtonItemObserver,
                     public DG::TextEditBaseObserver,
                     public DG::PopUpObserver,
                     public DG::RadioItemObserver,
                     public DG::RealEditObserver,
                     public DG::CheckItemObserver {
private:
  static MagicPalette *instance;

  // UI Item Pointers - Exactly matching GRC
  DG::Button* btnAutoCreate = nullptr;    // ID 1
  DG::Button* btnBrowse = nullptr;        // ID 2
  DG::LeftText* txtExcelPath = nullptr;   // ID 4
  DG::RealEdit* editTargetArea = nullptr; // ID 9
  DG::RealEdit* editTolerance = nullptr;  // ID 13
  DG::Button* btnReshape = nullptr;       // ID 16
  DG::Button* btnSumSelected = nullptr;   // ID 17
  DG::LeftText* txtTotalArea = nullptr;   // ID 19
  DG::RadioButton* rbZone = nullptr;      // ID 22
  DG::RadioButton* rbMesh = nullptr;      // ID 23
  DG::Button* btnCreate = nullptr;        // ID 24
  DG::RadioButton* rbMorph = nullptr;     // ID 25
  DG::RealEdit* editHeight = nullptr;     // ID 41
  DG::TextEdit* editPrefix = nullptr;     // ID 29
  DG::RealEdit* editUnit = nullptr;       // ID 11
  DG::CheckBox* checkFixX = nullptr;      // ID 14
  DG::CheckBox* checkFixY = nullptr;      // ID 15
  DG::Button* btnPickPrefix = nullptr;    // ID 30
  DG::PopUp* popStyle = nullptr;          // ID 32
  DG::TextEdit* editStartNo = nullptr;    // ID 34
  DG::MultiLineEdit* editPreview = nullptr; // ID 36
  DG::Button* btnApplyLabeling = nullptr; // ID 37
  DG::Button* btnCalcRange = nullptr;     // ID 38
  DG::LeftText* txtAllowedRange = nullptr; // ID 42
  DG::Button* btnSimplifyPoly = nullptr;  // ID 43
  DG::LeftText* txtSimplifyTolLabel = nullptr; // ID 44
  DG::RealEdit* editSimplifyTol = nullptr; // ID 45
  DG::CheckBox* checkPreserveCurve = nullptr; // ID 46
  DG::Button* btnCheckUpdate = nullptr; // ID 47
  DG::LeftText* txtVersion = nullptr; // ID 48
  
  void SimplifyPolyline() {
      double tolMM = (editSimplifyTol ? editSimplifyTol->GetValue() : 50.0);
      bool preserveCurve = (checkPreserveCurve && checkPreserveCurve->IsChecked());
      AddOnLogic::SimplifyPolyline(tolMM, preserveCurve);
  }
  
  void CreateFromSelection() {
    ACAPI_WriteReport(GS::UniString("---- [Debug] CreateFromSelection Start ----", CC_UTF8), false);
    API_SelectionInfo si; GS::Array<API_Neig> sel;
    if (ACAPI_Selection_Get(&si, &sel, false) != NoError || sel.IsEmpty()) {
        ACAPI_WriteReport(GS::UniString("[Debug] Error: No geometry selected.", CC_UTF8), false);
        return;
    }

    struct PolySource { API_ElementMemo memo; Int32 nCoords; Int32 nSubPolys; Int32 nArcs; API_Guid originalGuid; API_ElemType originalType; };
    std::vector<PolySource> polySources;

    // 1. Try to find existing polygons
    for (auto& n : sel) {
        API_Element el = {}; el.header.guid = n.guid;
        if (ACAPI_Element_Get(&el) == NoError) {
            if (el.header.type == API_HatchID || el.header.type == API_SlabID || el.header.type == API_PolyLineID || el.header.type == API_ZoneID || el.header.type == API_MorphID) {
                PolySource ps = {}; 
                ps.originalGuid = el.header.guid;
                ps.originalType = el.header.type;
                bool foundPoly = false;
                if (el.header.type == API_MorphID) {
                    API_ElemInfo3D info3D;
                    if (ACAPI_ModelAccess_Get3DInfo(el.header, &info3D) == NoError) {
                        double minZ = 1e30;
                        // First pass: find minZ
                        for (Int32 i = info3D.fbody; i <= info3D.lbody; ++i) {
                            API_Component3D bodyComp; BNZeroMemory(&bodyComp, sizeof(bodyComp));
                            bodyComp.header.typeID = API_BodyID; bodyComp.header.index = i;
                            if (ACAPI_ModelAccess_GetComponent(&bodyComp) == NoError) {
                                for (Int32 j = 1; j <= bodyComp.body.nVert; ++j) {
                                    API_Component3D vertComp; BNZeroMemory(&vertComp, sizeof(vertComp));
                                    vertComp.header.typeID = API_VertID; vertComp.header.index = j;
                                    if (ACAPI_ModelAccess_GetComponent(&vertComp) == NoError) {
                                        double x = vertComp.vert.x, y = vertComp.vert.y, z = vertComp.vert.z;
                                        double* tm = bodyComp.body.tranmat.tmx;
                                        double wz = tm[8]*x + tm[9]*y + tm[10]*z + tm[11];
                                        if (wz < minZ) minZ = wz;
                                    }
                                }
                            }
                        }
                        // Second pass: find polygons at minZ with normal pointing down
                        std::vector<API_Coord> pts;
                        for (Int32 i = info3D.fbody; i <= info3D.lbody; ++i) {
                            API_Component3D bodyComp; BNZeroMemory(&bodyComp, sizeof(bodyComp));
                            bodyComp.header.typeID = API_BodyID; bodyComp.header.index = i;
                            if (ACAPI_ModelAccess_GetComponent(&bodyComp) == NoError) {
                                for (Int32 k = 1; k <= bodyComp.body.nPgon; ++k) {
                                    API_Component3D pgonComp; BNZeroMemory(&pgonComp, sizeof(pgonComp));
                                    pgonComp.header.typeID = API_PgonID; pgonComp.header.index = k;
                                    if (ACAPI_ModelAccess_GetComponent(&pgonComp) == NoError) {
                                        API_Component3D vectComp; BNZeroMemory(&vectComp, sizeof(vectComp));
                                        vectComp.header.typeID = API_VectID; vectComp.header.index = std::abs(pgonComp.pgon.ivect);
                                        if (ACAPI_ModelAccess_GetComponent(&vectComp) == NoError) {
                                            double vx = vectComp.vect.x, vy = vectComp.vect.y, vz = vectComp.vect.z;
                                            if (pgonComp.pgon.ivect < 0) { vx = -vx; vy = -vy; vz = -vz; }
                                            // Normal should be (0,0,-1)
                                            if (std::abs(vx) < 0.01 && std::abs(vy) < 0.01 && vz < -0.99) {
                                                // Check if it's at minZ
                                                bool atMinZ = true;
                                                std::vector<API_Coord> currentFacePts;
                                                for (Int32 p = pgonComp.pgon.fpedg; p <= pgonComp.pgon.lpedg; ++p) {
                                                    API_Component3D pedgComp; BNZeroMemory(&pedgComp, sizeof(pedgComp));
                                                    pedgComp.header.typeID = API_PedgID; pedgComp.header.index = p;
                                                    if (ACAPI_ModelAccess_GetComponent(&pedgComp) == NoError) {
                                                        if (pedgComp.pedg.pedg == 0) continue; // Hole start
                                                        API_Component3D edgeComp; BNZeroMemory(&edgeComp, sizeof(edgeComp));
                                                        edgeComp.header.typeID = API_EdgeID; edgeComp.header.index = std::abs(pedgComp.pedg.pedg);
                                                        if (ACAPI_ModelAccess_GetComponent(&edgeComp) == NoError) {
                                                            Int32 vIdx = (pedgComp.pedg.pedg > 0) ? edgeComp.edge.vert1 : edgeComp.edge.vert2;
                                                            API_Component3D vComp; BNZeroMemory(&vComp, sizeof(vComp));
                                                            vComp.header.typeID = API_VertID; vComp.header.index = vIdx;
                                                            if (ACAPI_ModelAccess_GetComponent(&vComp) == NoError) {
                                                                double x = vComp.vert.x, y = vComp.vert.y, z = vComp.vert.z;
                                                                double* tm = bodyComp.body.tranmat.tmx;
                                                                double wx = tm[0]*x + tm[1]*y + tm[2]*z + tm[3];
                                                                double wy = tm[4]*x + tm[5]*y + tm[6]*z + tm[7];
                                                                double wz = tm[8]*x + tm[9]*y + tm[10]*z + tm[11];
                                                                if (std::abs(wz - minZ) > 0.1) { atMinZ = false; break; }
                                                                currentFacePts.push_back({wx, wy});
                                                            }
                                                        }
                                                    }
                                                }
                                                if (atMinZ && !currentFacePts.empty()) {
                                                    if (pts.empty()) pts = currentFacePts;
                                                    // For now, we take the first face at minZ as the footprint
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                                if (!pts.empty()) break;
                            }
                        }
                        if (pts.size() >= 3) {
                            pts.push_back(pts[0]);
                            ps.nCoords = (Int32)pts.size(); ps.nSubPolys = 1; ps.nArcs = 0;
                            ps.memo.coords = (API_Coord**)BMAllocateHandle((ps.nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
                            ps.memo.pends = (Int32**)BMAllocateHandle(2 * sizeof(Int32), ALLOCATE_CLEAR, 0);
                            if (ps.memo.coords && ps.memo.pends) {
                                for (Int32 i = 0; i < ps.nCoords; ++i) (*ps.memo.coords)[i + 1] = pts[i];
                                (*ps.memo.pends)[1] = ps.nCoords;
                                foundPoly = true;
                            }
                        }
                    }
                } else {
                    if (ACAPI_Element_GetMemo(el.header.guid, &ps.memo, APIMemoMask_Polygon) == NoError) {
                        if (el.header.type == API_HatchID) { ps.nCoords = el.hatch.poly.nCoords; ps.nSubPolys = el.hatch.poly.nSubPolys; ps.nArcs = el.hatch.poly.nArcs; }
                        else if (el.header.type == API_SlabID) { ps.nCoords = el.slab.poly.nCoords; ps.nSubPolys = el.slab.poly.nSubPolys; ps.nArcs = el.slab.poly.nArcs; }
                        else if (el.header.type == API_ZoneID) { ps.nCoords = el.zone.poly.nCoords; ps.nSubPolys = el.zone.poly.nSubPolys; ps.nArcs = el.zone.poly.nArcs; }
                        else if (el.header.type == API_PolyLineID) { ps.nCoords = el.polyLine.poly.nCoords; ps.nSubPolys = el.polyLine.poly.nSubPolys; ps.nArcs = el.polyLine.poly.nArcs; }
                        foundPoly = true;
                    }
                }
                if (foundPoly) polySources.push_back(ps);
            }
        }
    }

    // 2. Line Chaining (Note: Line chaining doesn't have an original Guid for deletion)
    if (polySources.empty()) {
        std::vector<API_LineType> allLines;
        for (auto& n : sel) {
            API_Element el = {}; el.header.guid = n.guid;
            if (ACAPI_Element_Get(&el) == NoError) {
                if (el.header.type == API_LineID) { allLines.push_back(el.line); }
                else if (el.header.type == API_CircleID || el.header.type == API_ArcID) {
                    API_Coord origC; double r, bA, eA;
                    if (el.header.type == API_CircleID) { origC = el.circle.origC; r = el.circle.r; bA = 0; eA = 2*3.14159265; }
                    else { origC = el.arc.origC; r = el.arc.r; bA = el.arc.begAng; eA = el.arc.endAng; }
                    if (eA < bA) eA += 2*3.14159265;
                    int segs = 16; double step = (eA-bA)/segs;
                    API_Coord pP = {origC.x+r*cos(bA), origC.y+r*sin(bA)};
                    for (int i=1; i<=segs; ++i) {
                        API_Coord pt = {origC.x+r*cos(bA+i*step), origC.y+r*sin(bA+i*step)};
                        API_LineType s; BNZeroMemory(&s, sizeof(s)); s.begC = pP; s.endC = pt;
                        allLines.push_back(s); pP = pt;
                    }
                }
            }
        }
        while (allLines.size() >= 3) {
            std::vector<API_Coord> pts; pts.push_back(allLines[0].begC); API_Coord nP = allLines[0].endC; allLines.erase(allLines.begin());
            bool closed = false;
            while (true) {
                pts.push_back(nP); bool fN = false;
                for (size_t i=0; i<allLines.size(); ++i) {
                    if (std::abs(allLines[i].begC.x-nP.x)<0.01 && std::abs(allLines[i].begC.y-nP.y)<0.01) { nP = allLines[i].endC; allLines.erase(allLines.begin()+i); fN=true; break; }
                    else if (std::abs(allLines[i].endC.x-nP.x)<0.01 && std::abs(allLines[i].endC.y-nP.y)<0.01) { nP = allLines[i].begC; allLines.erase(allLines.begin()+i); fN=true; break; }
                }
                if (std::abs(pts[0].x-nP.x)<0.01 && std::abs(pts[0].y-nP.y)<0.01) { pts.push_back(pts[0]); closed = true; break; }
                if (!fN) break;
            }
            if (closed && pts.size() >= 3) {
                PolySource ps; BNZeroMemory(&ps.memo, sizeof(ps.memo));
                ps.originalType = API_ZombieElemID; // No original element to delete
                ps.nCoords = (Int32)pts.size(); ps.nSubPolys = 1; ps.nArcs = 0;
                ps.memo.coords = (API_Coord**)BMAllocateHandle((ps.nCoords+1)*sizeof(API_Coord), ALLOCATE_CLEAR, 0);
                ps.memo.pends = (Int32**)BMAllocateHandle(2*sizeof(Int32), ALLOCATE_CLEAR, 0);
                if (ps.memo.coords && ps.memo.pends) {
                    for (Int32 i=0; i<ps.nCoords; ++i) (*ps.memo.coords)[i+1] = pts[i];
                    (*ps.memo.pends)[1] = ps.nCoords; polySources.push_back(ps);
                }
            }
            if (!closed && allLines.empty()) break;
        }
    }

    if (polySources.empty()) return;

    double hVal = editHeight ? editHeight->GetValue() : 0.0;
    bool isZone = rbZone->IsSelected();
    bool isMesh = rbMesh->IsSelected();
    bool isMorph = rbMorph->IsSelected();
    API_AttributeIndex matIdx = ACAPI_CreateAttributeIndex(1);

    for (auto& ps : polySources) {
        ACAPI_CallUndoableCommand("Create Element", [&]() -> GSErrCode {
            API_Element el = {}; API_ElementMemo memo = {};
            if (isZone) { el.header.type = API_ZoneID; ACAPI_Element_GetDefaults(&el, nullptr); }
            else if (isMesh) { el.header.type = API_MeshID; ACAPI_Element_GetDefaults(&el, nullptr); }
            else if (isMorph) { el.header.type = API_MorphID; ACAPI_Element_GetDefaults(&el, nullptr); }

            if (el.header.type == API_MorphID) {
                el.morph.tranmat.tmx[0]=1.0; el.morph.tranmat.tmx[5]=1.0; el.morph.tranmat.tmx[10]=1.0;
                void* bD = nullptr; ACAPI_Body_Create(nullptr, nullptr, &bD);
                double dz = hVal/1000.0;
                API_OverriddenAttribute mat = {}; mat = matIdx; 
                
                // Clean points: remove duplicate consecutive vertices
                std::vector<API_Coord> cleanPts;
                Int32 nRaw = ps.nCoords - 1;
                for (Int32 i = 0; i < nRaw; ++i) {
                    API_Coord c1 = (*ps.memo.coords)[i+1];
                    API_Coord c2 = (*ps.memo.coords)[(i % nRaw) + 1]; // Previous point for distance check
                    if (i == 0) {
                        cleanPts.push_back(c1);
                    } else {
                        API_Coord prev = cleanPts.back();
                        double dist = std::sqrt((c1.x-prev.x)*(c1.x-prev.x) + (c1.y-prev.y)*(c1.y-prev.y));
                        if (dist > 0.0001) cleanPts.push_back(c1);
                    }
                }
                // Check last and first
                if (cleanPts.size() >= 3) {
                    API_Coord c1 = cleanPts.front();
                    API_Coord c2 = cleanPts.back();
                    double dist = std::sqrt((c1.x-c2.x)*(c1.x-c2.x) + (c1.y-c2.y)*(c1.y-c2.y));
                    if (dist < 0.0001) cleanPts.pop_back();
                }

                if (cleanPts.size() < 3) {
                    ACAPI_Body_Dispose(&bD);
                    return APIERR_BADPARS;
                }

                Int32 nUnique = (Int32)cleanPts.size();
                std::vector<UInt32> bV(nUnique); std::vector<Int32> bE(nUnique);
                
                for (Int32 i=0; i<nUnique; ++i) { 
                    API_Coord3D v={cleanPts[i].x, cleanPts[i].y, 0}; 
                    ACAPI_Body_AddVertex(bD,v,bV[i]); 
                }
                for (Int32 i=0; i<nUnique; ++i) {
                    ACAPI_Body_AddEdge(bD, bV[i], bV[(i+1)%nUnique], bE[i]);
                }
                
                if (std::abs(dz) > 0.0001) {
                    std::vector<UInt32> tV(nUnique); std::vector<Int32> tE(nUnique); std::vector<Int32> sE(nUnique);
                    for (Int32 i=0; i<nUnique; ++i) { 
                        API_Coord3D v={cleanPts[i].x, cleanPts[i].y, dz}; 
                        ACAPI_Body_AddVertex(bD,v,tV[i]); 
                        ACAPI_Body_AddEdge(bD, bV[i], tV[i], sE[i]);
                    }
                    for (Int32 i=0; i<nUnique; ++i) {
                        ACAPI_Body_AddEdge(bD, tV[i], tV[(i+1)%nUnique], tE[i]);
                    }
                    for (Int32 i=0; i<nUnique; ++i) {
                        API_Coord c1 = cleanPts[i];
                        API_Coord c2 = cleanPts[(i+1)%nUnique];
                        API_Vector3D sn={c2.y-c1.y, -(c2.x-c1.x), 0};
                        double len = std::sqrt(sn.x*sn.x + sn.y*sn.y);
                        if (len > 1e-8) { sn.x /= len; sn.y /= len; }
                        else { sn.x = 1.0; sn.y = 0.0; } // Fallback

                        Int32 pn; ACAPI_Body_AddPolyNormal(bD,sn,pn);
                        GS::Array<Int32> sp; 
                        sp.Push(bE[i]); sp.Push(sE[(i+1)%nUnique]); sp.Push(-tE[i]); sp.Push(-sE[i]);
                        UInt32 fi; ACAPI_Body_AddPolygon(bD,sp,pn,mat,fi);
                    }
                    Int32 bn; API_Vector3D bv={0,0,-1}; if (dz < 0) bv.z = 1.0;
                    ACAPI_Body_AddPolyNormal(bD,bv,bn);
                    GS::Array<Int32> bp; for(Int32 i=nUnique-1;i>=0;--i) bp.Push(-bE[i]); 
                    UInt32 bf; ACAPI_Body_AddPolygon(bD,bp,bn,mat,bf);
                    
                    Int32 tn; API_Vector3D tv={0,0,1}; if (dz < 0) tv.z = -1.0;
                    ACAPI_Body_AddPolyNormal(bD,tv,tn);
                    GS::Array<Int32> tp; for(Int32 i=0;i<nUnique;++i) tp.Push(tE[i]); 
                    UInt32 tf; ACAPI_Body_AddPolygon(bD,tp,tn,mat,tf);
                } else {
                    Int32 bn; API_Vector3D bv={0,0,1}; ACAPI_Body_AddPolyNormal(bD,bv,bn);
                    GS::Array<Int32> bp; for(Int32 i=0;i<nUnique;++i) bp.Push(bE[i]); 
                    UInt32 bf; ACAPI_Body_AddPolygon(bD,bp,bn,mat,bf);
                }
                ACAPI_Body_Finish(bD, &memo.morphBody, &memo.morphMaterialMapTable); ACAPI_Body_Dispose(&bD);
            } else {
                memo.coords = ps.memo.coords;
                memo.pends = ps.memo.pends;
                memo.parcs = ps.memo.parcs;
                
                if (isMesh) {
                    memo.meshPolyZ = (double**)BMAllocateHandle((ps.nCoords + 1) * sizeof(double), ALLOCATE_CLEAR, 0);
                } else if (isZone) {
                    double cx = 0, cy = 0;
                    for (Int32 i = 1; i < ps.nCoords; ++i) { cx += (*memo.coords)[i].x; cy += (*memo.coords)[i].y; }
                    if (ps.nCoords > 1) { el.zone.pos.x = cx / (ps.nCoords - 1); el.zone.pos.y = cy / (ps.nCoords - 1); }
                }

                if (el.header.type == API_ZoneID || el.header.type == API_MeshID) {
                    if (memo.pends == nullptr) {
                        memo.pends = (Int32**)BMAllocateHandle(2 * sizeof(Int32), ALLOCATE_CLEAR, 0);
                        if (memo.pends) {
                            (*memo.pends)[0] = 0;
                            (*memo.pends)[1] = ps.nCoords;
                        }
                    }
                    if (memo.coords && ps.nCoords >= 3) {
                        API_Coord firstPt = (*memo.coords)[1];
                        API_Coord lastPt = (*memo.coords)[ps.nCoords];
                        double dist = std::sqrt(std::pow(firstPt.x - lastPt.x, 2) + std::pow(firstPt.y - lastPt.y, 2));
                        if (dist > 0.001) {
                            API_Coord** newCoords = (API_Coord**)BMAllocateHandle((ps.nCoords + 2) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
                            if (newCoords) {
                                for (Int32 i = 0; i <= ps.nCoords; ++i) {
                                    (*newCoords)[i] = (*memo.coords)[i];
                                }
                                (*newCoords)[ps.nCoords + 1] = firstPt;
                                ps.nCoords += 1;
                                memo.coords = newCoords;
                                if (memo.pends) {
                                    (*memo.pends)[1] = ps.nCoords;
                                }
                            }
                        }
                    }
                    if (el.header.type == API_ZoneID) {
                        el.zone.poly.nCoords = ps.nCoords;
                        el.zone.poly.nSubPolys = (ps.nSubPolys == 0) ? 1 : ps.nSubPolys;
                        el.zone.poly.nArcs = ps.nArcs;
                        el.zone.manual = true;
                        
                        double xmin = 1e15, xmax = -1e15, ymin = 1e15, ymax = -1e15;
                        for (Int32 i = 1; i <= ps.nCoords; ++i) {
                            double vx = (*memo.coords)[i].x;
                            double vy = (*memo.coords)[i].y;
                            if (vx < xmin) xmin = vx; if (vx > xmax) xmax = vx;
                            if (vy < ymin) ymin = vy; if (vy > ymax) ymax = vy;
                        }
                        double cx = (xmin + xmax) / 2.0;
                        double cy = (ymin + ymax) / 2.0;
                        el.zone.pos = { cx, cy };
                        el.zone.refPos = { cx, cy };
                    }
                    else if (el.header.type == API_MeshID) {
                        el.mesh.poly.nCoords = ps.nCoords;
                        el.mesh.poly.nSubPolys = (ps.nSubPolys == 0) ? 1 : ps.nSubPolys;
                        el.mesh.poly.nArcs = ps.nArcs;
                        
                        memo.meshPolyZ = (double**)BMAllocateHandle((ps.nCoords + 1) * sizeof(double), ALLOCATE_CLEAR, 0);
                        if (memo.meshPolyZ) {
                            for (Int32 i = 0; i <= ps.nCoords; ++i) {
                                (*memo.meshPolyZ)[i] = 0.0;
                            }
                        }
                    }
                }
            }
            if (el.header.type == API_ZoneID || el.header.type == API_MeshID) {
                API_RegularizedPoly regPoly = {};
                regPoly.coords = memo.coords;
                regPoly.pends = memo.pends;
                regPoly.parcs = memo.parcs;
                regPoly.needVertexAncestry = false;
                
                Int32 nResult = 0;
                API_RegularizedPoly** polys = nullptr;
                GSErrCode regErr = ACAPI_Polygon_RegularizePolygon(&regPoly, &nResult, &polys);
                
                if (regErr == NoError && nResult > 0) {
                    for (Int32 i = 0; i < nResult; ++i) {
                        API_Element newEl = {};
                        newEl.header.type = el.header.type;
                        API_ElementMemo newMemo = {};
                        ACAPI_Element_GetDefaults(&newEl, &newMemo);
                        
                        if (newMemo.coords) BMKillHandle((GSHandle*)&newMemo.coords);
                        if (newMemo.pends) BMKillHandle((GSHandle*)&newMemo.pends);
                        if (newMemo.parcs) BMKillHandle((GSHandle*)&newMemo.parcs);
                        if (newMemo.meshPolyZ) BMKillHandle((GSHandle*)&newMemo.meshPolyZ);
                        
                        if (newEl.header.type == API_ZoneID) {
                            newEl.zone.poly.nCoords = BMhGetSize((GSHandle)(*polys)[i].coords) / sizeof(API_Coord) - 1;
                            newEl.zone.poly.nSubPolys = BMhGetSize((GSHandle)(*polys)[i].pends) / sizeof(Int32) - 1;
                            newEl.zone.poly.nArcs = BMhGetSize((GSHandle)(*polys)[i].parcs) / sizeof(API_PolyArc);
                            newEl.zone.manual = true;
                            
                            double xmin = 1e15, xmax = -1e15, ymin = 1e15, ymax = -1e15;
                            for (Int32 k = 1; k <= newEl.zone.poly.nCoords; ++k) {
                                double vx = (*(*polys)[i].coords)[k].x;
                                double vy = (*(*polys)[i].coords)[k].y;
                                if (vx < xmin) xmin = vx; if (vx > xmax) xmax = vx;
                                if (vy < ymin) ymin = vy; if (vy > ymax) ymax = vy;
                            }
                            double cx = (xmin + xmax) / 2.0;
                            double cy = (ymin + ymax) / 2.0;
                            newEl.zone.pos = { cx, cy };
                            newEl.zone.refPos = { cx, cy };
                        } else if (newEl.header.type == API_MeshID) {
                            newEl.mesh.poly.nCoords = BMhGetSize((GSHandle)(*polys)[i].coords) / sizeof(API_Coord) - 1;
                            newEl.mesh.poly.nSubPolys = BMhGetSize((GSHandle)(*polys)[i].pends) / sizeof(Int32) - 1;
                            newEl.mesh.poly.nArcs = BMhGetSize((GSHandle)(*polys)[i].parcs) / sizeof(API_PolyArc);
                        }
                        
                        newMemo.coords = (*polys)[i].coords;
                        newMemo.pends = (*polys)[i].pends;
                        newMemo.parcs = (*polys)[i].parcs;
                        
                        if (newEl.header.type == API_MeshID) {
                            newMemo.meshPolyZ = (double**)BMAllocateHandle((newEl.mesh.poly.nCoords + 1) * sizeof(double), ALLOCATE_CLEAR, 0);
                        }
                        
                        GSErrCode createErr = ACAPI_Element_Create(&newEl, &newMemo);
                        if (createErr != NoError) {
                            ACAPI_WriteReport(GS::UniString::Printf("Error creating polygon element: %d", createErr), true);
                        }
                        
                        if (newEl.header.type != API_MorphID) {
                            newMemo.coords = nullptr;
                            newMemo.pends = nullptr;
                            newMemo.parcs = nullptr;
                        }
                        ACAPI_DisposeElemMemoHdls(&newMemo);
                    }
                    for (Int32 j = 0; j < nResult; ++j) {
                        ACAPI_Polygon_DisposeRegularizedPoly(&(*polys)[j]);
                    }
                    BMKillHandle((GSHandle*)&polys);
                } else {
                    ACAPI_WriteReport(GS::UniString::Printf("Error regularizing polygon: %d", regErr), true);
                }
            } else {
                GSErrCode err = ACAPI_Element_Create(&el, &memo);
                if (err != NoError) {
                    ACAPI_WriteReport(GS::UniString::Printf("Error creating element: %d", err), true);
                }
            }
            
            if (el.header.type != API_MorphID) {
                if (memo.coords == ps.memo.coords) memo.coords = nullptr;
                if (memo.pends == ps.memo.pends) memo.pends = nullptr;
                if (memo.parcs == ps.memo.parcs) memo.parcs = nullptr;
                if (memo.coords) BMKillHandle((GSHandle*)&memo.coords);
                if (memo.pends) BMKillHandle((GSHandle*)&memo.pends);
                if (memo.meshPolyZ) BMKillHandle((GSHandle*)&memo.meshPolyZ);
            }
            ACAPI_DisposeElemMemoHdls(&memo);
            return NoError;
        });
        ACAPI_DisposeElemMemoHdls(&ps.memo);
    }
    ACAPI_WriteReport("---- [Debug] CreateFromSelection End ----", false);
  }

  void UpdatePreview() {
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

  void ApplyLabeling() {
    API_SelectionInfo si; GS::Array<API_Neig> sel;
    if (ACAPI_Selection_Get(&si, &sel, false) != NoError || sel.IsEmpty()) {
        ACAPI_WriteReport(GS::UniString("Please select elements first.", CC_UTF8), true);
        return;
    }
    
    GS::UniString prefix = editPrefix->GetText();
    Int32 start = (Int32)atoi(editStartNo->GetText().ToCStr().Get());
    if (start == 0 && editStartNo->GetText() != "0") start = 1;

    int style = popStyle->GetSelectedItem();

    ACAPI_CallUndoableCommand("Apply Labeling", [sel, prefix, start, style]() -> GSErrCode {
      int count = 0;
      for (auto &n : sel) {
        API_Element el = {}; el.header.guid = n.guid;
        if (ACAPI_Element_Get(&el) == NoError) {
          int idx = start + count;
          GS::UniString styledID;
          if (style == 1) styledID = GS::UniString::Printf("%c", 'a' + (idx - 1) % 26);
          else if (style == 2) styledID = GS::UniString::Printf("%c", 'A' + (idx - 1) % 26);
          else if (style == 4) styledID = GS::UniString::Printf("%02d", idx);
          else if (style == 5) styledID = GS::UniString::Printf("%03d", idx);
          else if (style == 6) styledID = GS::UniString::Printf("%04d", idx);
          else if (style == 7) styledID = GS::UniString("");
          else styledID = GS::UniString::Printf("%d", idx);
          
          GS::UniString newID = prefix + styledID;

          if (el.header.type == API_TextID || el.header.type == API_LabelID) {
              API_ElementMemo memo = {};
              if (ACAPI_Element_GetMemo(el.header.guid, &memo, APIMemoMask_TextContent | APIMemoMask_Paragraph) == NoError) {
                  bool multiStyle = false;
                  if (el.header.type == API_TextID) multiStyle = el.text.multiStyle;
                  else multiStyle = el.label.u.text.multiStyle;

#ifdef ServerMainVers_2900
                  if (memo.textContent != nullptr) delete memo.textContent;
                  memo.textContent = new GS::UniString(newID);
#else
                  if (memo.textContent != nullptr) BMKillHandle((GSHandle*)&memo.textContent);
                  auto cstrBuf = newID.ToCStr();
                  const char* utf8str = cstrBuf.Get();
                  size_t len = strlen(utf8str) + 1;
                  memo.textContent = (char**)BMAllocateHandle(len, ALLOCATE_CLEAR, 0);
                  if (memo.textContent) strcpy(*memo.textContent, utf8str);
#endif

                  Int32 memoMask = APIMemoMask_TextContent;

                  if (memo.paragraphs != nullptr && *memo.paragraphs != nullptr) {
                      (*memo.paragraphs)[0].range = (Int32)newID.GetLength();
                      if ((*memo.paragraphs)[0].run != nullptr) {
                          (*memo.paragraphs)[0].run[0].range = (Int32)newID.GetLength();
                      }
                      memoMask |= APIMemoMask_Paragraph;
                  }

                  API_Element mask; ACAPI_ELEMENT_MASK_CLEAR(mask);
                  ACAPI_Element_Change(&el, &mask, &memo, memoMask, true);
                  ACAPI_DisposeElemMemoHdls(&memo);
              }
          } else {
              API_ElementMemo memo; BNZeroMemory(&memo, sizeof(memo));
              if (ACAPI_Element_GetMemo(el.header.guid, &memo, APIMemoMask_ElemInfoString) == NoError) {
                  if (memo.elemInfoString != nullptr) delete memo.elemInfoString;
                  memo.elemInfoString = new GS::UniString(newID);
                  
                  API_Element mask; ACAPI_ELEMENT_MASK_CLEAR(mask);
                  ACAPI_Element_Change(&el, &mask, &memo, APIMemoMask_ElemInfoString, true);
                  ACAPI_DisposeElemMemoHdls(&memo);
              }
              if (el.header.type.typeID == API_ZoneID) {
                  API_Element changedEl, mask;
                  BNZeroMemory(&changedEl, sizeof(changedEl));
                  changedEl.header.guid = el.header.guid;
                  ACAPI_ELEMENT_MASK_CLEAR(mask);
                  ACAPI_ELEMENT_MASK_SET(mask, API_ZoneType, roomNoStr);
                  const GS::uchar_t* src = (const GS::uchar_t*)newID.ToUStr();
                  size_t cLen = GS::ucslen(src);
                  if (cLen > 31) cLen = 31;
                  memcpy(changedEl.zone.roomNoStr, src, cLen * sizeof(GS::uchar_t));
                  changedEl.zone.roomNoStr[cLen] = 0;
                  ACAPI_Element_Change(&changedEl, &mask, nullptr, 0, true);
              }
          }
          count++;
        }
      }
      ACAPI_WriteReport(GS::UniString::Printf("Updated %d elements.", count), false);
      return NoError;
    });
  }

  void UpdateTotalArea() {
    API_SelectionInfo si; GS::Array<API_Neig> sel;
    if (ACAPI_Selection_Get(&si, &sel, false) != NoError || sel.IsEmpty()) return;

    auto CalcPolyArea = [](const API_ElementMemo& memo, Int32 nCoords) -> double {
        if (!memo.coords || !*memo.coords || nCoords < 3) return 0.0;
        double area = 0.0;
        for (Int32 i = 1; i < nCoords; ++i) {
            area += ((*memo.coords)[i].x * (*memo.coords)[i + 1].y - (*memo.coords)[i + 1].x * (*memo.coords)[i].y);
        }
        return std::abs(area) / 2.0;
    };

    double totalArea = 0;
    int count = 0;
    for (auto &n : sel) {
      API_Element el; BNZeroMemory(&el, sizeof(el)); el.header.guid = n.guid;
      if (ACAPI_Element_Get(&el) == NoError) {
        if (el.header.type.typeID == API_ZoneID) {
          API_ElementMemo memo; BNZeroMemory(&memo, sizeof(memo));
          if (ACAPI_Element_GetMemo(el.header.guid, &memo, APIMemoMask_Polygon) == NoError) {
              totalArea += CalcPolyArea(memo, el.zone.poly.nCoords);
              ACAPI_DisposeElemMemoHdls(&memo);
          }
          count++;
        }
        else if (el.header.type.typeID == API_HatchID) {
          API_ElementMemo memo; BNZeroMemory(&memo, sizeof(memo));
          if (ACAPI_Element_GetMemo(el.header.guid, &memo, APIMemoMask_Polygon) == NoError) {
              totalArea += CalcPolyArea(memo, el.hatch.poly.nCoords);
              ACAPI_DisposeElemMemoHdls(&memo);
          }
          count++;
        }
      }
    }
    if (txtTotalArea) {
        txtTotalArea->SetText(GS::UniString::Printf("%.2f", totalArea));
        txtTotalArea->Redraw();
    }
    ACAPI_WriteReport(GS::UniString::Printf("[Sum] %d Elements. Total: %.2f m2", count, totalArea), false);
  }

public:
  static void Show() {
    CheckForUpdates(); // Automatically check for updates once per session

    if (!IsAlreadyLicensed()) {
        ActivationDialog dlg;
        if (dlg.Invoke() != DG::ModalDialog::Accept) {
            return; // Exit if user cancelled or failed activation
        }
    }

    if (instance == nullptr) {
      instance = new MagicPalette();
    }
    if (instance != nullptr) {
      instance->DG::Palette::Show();
    }
  }

  static void Destroy() {
    if (instance != nullptr) {
      delete instance;
      instance = nullptr;
    }
  }

  static GSErrCode __ACDLL_CALL StaticCallback(Int32, API_PaletteMessageID msg, GS::IntPtr) {
    if (msg == APIPalMsg_OpenPalette) Show();
    else if (msg == APIPalMsg_ClosePalette) Destroy();
    return NoError;
  }

  MagicPalette() : DG::Palette(ACAPI_GetOwnResModule(), 32500, ACAPI_GetOwnResModule(), customAddonPaletteGuid) {
    BeginEventProcessing();
    
    // Items
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
    rbZone->Select(); // Default select the Zone creation tab
    txtVersion->SetText(CURRENT_VERSION);
    UpdatePreview();

    editSimplifyTol->SetValue(50.0); // Default 50mm
}

  virtual ~MagicPalette() {
    EndEventProcessing();
    Detach(*this);
  }

  virtual void TextEditChanged(const DG::TextEditChangeEvent &ev) override { UpdatePreview(); }
  virtual void PopUpChanged(const DG::PopUpChangeEvent &ev) override { UpdatePreview(); }

  void Hide() { DG::Palette::Hide(); }
  virtual void PanelCloseRequested(const DG::PanelCloseRequestEvent &, bool *accept) override {
    *accept = true;
    Hide();
  }

  virtual void ButtonClicked(const DG::ButtonClickEvent &ev) override {
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
        if (excelPath.IsEmpty() || (!excelPath.EndsWith(".xlsx") && !excelPath.EndsWith(".csv"))) {
            ACAPI_WriteReport("Please select a valid excel (.xlsx) or .csv file first.", true);
            return;
        }
        
        GS::UniString csvPath = excelPath;
        if (excelPath.EndsWith(".xlsx")) {
            csvPath.ReplaceAll(".xlsx", ".csv");
            GS::UniString psCommand = GS::UniString::Printf(
                "powershell -WindowStyle Hidden -Command \"$excel = New-Object -ComObject Excel.Application; "
                "$excel.Visible = $false; "
                "$wb = $excel.Workbooks.Open('%s'); "
                "$wb.SaveAs('%s', 62); "
                "$wb.Close($false); "
                "$excel.Quit();\"", 
                excelPath.ToCStr().Get(), csvPath.ToCStr().Get());
            system(psCommand.ToCStr().Get());
        }
        
        std::ifstream file(csvPath.ToCStr().Get());
        if (!file.is_open()) {
            ACAPI_WriteReport("Failed to generate or read CSV. Check Excel installation or file permissions.", true);
            return;
        }
        std::string line;
        std::getline(file, line); // Skip header
        
        std::map<int, double> storyCurX;
        
        ACAPI_CallUndoableCommand("Auto Create Zones from Excel", [&]() -> GSErrCode {
            int count = 0;
            while (std::getline(file, line)) {
                if (line.empty()) continue;
                std::stringstream ss(line);
                std::string stStory, stName, stArea;
                if (!std::getline(ss, stStory, ',')) continue;
                if (!std::getline(ss, stName, ',')) continue;
                if (!std::getline(ss, stArea, ',')) continue;
                
                int storyIdx = -999; // Sentinel
                API_StoryInfo storyInfo;
                if (ACAPI_ProjectSetting_GetStorySettings(&storyInfo) == NoError) {
                    GS::UniString excelStoryName(stStory.c_str(), CC_UTF8);
                    for (Int32 i = storyInfo.firstStory; i <= storyInfo.lastStory; ++i) {
                        if ((*storyInfo.data)[i - storyInfo.firstStory].index == i) {
                            GS::UniString currentStoryName((*storyInfo.data)[i - storyInfo.firstStory].uName);
                            if (currentStoryName == excelStoryName) {
                                storyIdx = i;
                                break;
                            }
                        }
                    }
                }

                // If name match failed, try parsing as index
                if (storyIdx == -999) {
                    try { storyIdx = std::stoi(stStory); } catch(...) { 
                        if (storyInfo.data) BMKillHandle((GSHandle*)&storyInfo.data);
                        continue; 
                    }
                }
                
                double area = 0.0;
                try { area = std::stod(stArea); } catch(...) { 
                    if (storyInfo.data) BMKillHandle((GSHandle*)&storyInfo.data);
                    continue; 
                }
                if (area <= 0) {
                    if (storyInfo.data) BMKillHandle((GSHandle*)&storyInfo.data);
                    continue;
                }
                
                if (storyInfo.data) {
                    // Re-fetch story info to ensure we have the latest if we need to create
                    while (storyIdx > storyInfo.lastStory) {
                        API_StoryCmdType storyCmd; BNZeroMemory(&storyCmd, sizeof(API_StoryCmdType));
                        storyCmd.action = APIStory_InsAbove;
                        storyCmd.index = storyInfo.lastStory;
                        storyCmd.height = 3.0; // Metric
                        storyCmd.dispOnSections = true;
                        ACAPI_ProjectSetting_ChangeStorySettings(&storyCmd);
                        if (storyInfo.data) BMKillHandle((GSHandle*)&storyInfo.data);
                        ACAPI_ProjectSetting_GetStorySettings(&storyInfo);
                    }
                    while (storyIdx < storyInfo.firstStory) {
                        API_StoryCmdType storyCmd; BNZeroMemory(&storyCmd, sizeof(API_StoryCmdType));
                        storyCmd.action = APIStory_InsBelow;
                        storyCmd.index = storyInfo.firstStory;
                        storyCmd.height = 3.0;
                        storyCmd.dispOnSections = true;
                        ACAPI_ProjectSetting_ChangeStorySettings(&storyCmd);
                        if (storyInfo.data) BMKillHandle((GSHandle*)&storyInfo.data);
                        ACAPI_ProjectSetting_GetStorySettings(&storyInfo);
                    }
                    if (storyInfo.data) BMKillHandle((GSHandle*)&storyInfo.data);
                }
                
                double side = std::sqrt(area);
                double curX = storyCurX[storyIdx];
                double curY = 0;
                
                API_Element zoneElem;
                BNZeroMemory(&zoneElem, sizeof(API_Element));
                zoneElem.header.type = API_ZoneID;
                if (ACAPI_Element_GetDefaults(&zoneElem, nullptr) == NoError) {
                    zoneElem.header.floorInd = storyIdx;
                    
                    if (!stName.empty() && stName.back() == '\r') stName.pop_back();
                    
                    GS::UniString uniName(stName.c_str(), CC_UTF8);
                    const GS::uchar_t* src = uniName.ToUStr();
                    size_t len = GS::ucslen(src);
                    if (len > 31) len = 31;
                    memcpy(zoneElem.zone.roomName, src, len * sizeof(GS::uchar_t));
                    zoneElem.zone.roomName[len] = 0;
                    
                    zoneElem.zone.poly.nCoords = 5;
                    zoneElem.zone.poly.nSubPolys = 1;
                    zoneElem.zone.poly.nArcs = 0;
                    zoneElem.zone.manual = true;
                    
                    API_ElementMemo memo;
                    BNZeroMemory(&memo, sizeof(API_ElementMemo));
                    memo.coords = (API_Coord**)BMAllocateHandle(6 * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
                    memo.pends = (Int32**)BMAllocateHandle(2 * sizeof(Int32), ALLOCATE_CLEAR, 0);
                    
                    if (memo.coords && memo.pends) {
                        (*memo.coords)[1] = { curX, curY };
                        (*memo.coords)[2] = { curX + side, curY };
                        (*memo.coords)[3] = { curX + side, curY + side };
                        (*memo.coords)[4] = { curX, curY + side };
                        (*memo.coords)[5] = (*memo.coords)[1];
                        
                        (*memo.pends)[0] = 0;
                        (*memo.pends)[1] = 5;
                        
                        if (ACAPI_Element_Create(&zoneElem, &memo) == NoError) count++;
                    }
                    ACAPI_DisposeElemMemoHdls(&memo);
                }
                storyCurX[storyIdx] = curX + side + 1.0;
            }
            ACAPI_WriteReport(GS::UniString::Printf("Created %d Zones from Excel.", count), false);
            return NoError;
        });
        
        if (excelPath.EndsWith(".xlsx")) {
            std::remove(csvPath.ToCStr().Get());
        }
        return;
    }
    else if (id == 16) { // Reshape logic
        double targetArea = editTargetArea ? editTargetArea->GetValue() : 0.0;
        double unit = editUnit ? editUnit->GetValue() : 0.0;
        if (targetArea <= 0) { ACAPI_WriteReport("Please enter a valid target area.", true); return; }
        bool fixX = checkFixX ? checkFixX->IsChecked() : false;
        bool fixY = checkFixY ? checkFixY->IsChecked() : false;
        API_SelectionInfo si; GS::Array<API_Neig> sel;
        if (ACAPI_Selection_Get(&si, &sel, false) == NoError && !sel.IsEmpty()) {
            ACAPI_CallUndoableCommand("Reshape Elements to Target Area", [&]() -> GSErrCode {
                for (auto &n : sel) {
                    API_Element el; BNZeroMemory(&el, sizeof(el)); el.header.guid = n.guid;
                    if (ACAPI_Element_Get(&el) == NoError) {
                        API_ElementMemo memo; BNZeroMemory(&memo, sizeof(memo));
                        if (ACAPI_Element_GetMemo(el.header.guid, &memo, APIMemoMask_Polygon | APIMemoMask_AddPars) == NoError) {
                            double xmin = 1e15, xmax = -1e15, ymin = 1e15, ymax = -1e15;
                            Int32 nCoords = (el.header.type.typeID == API_ZoneID) ? el.zone.poly.nCoords : el.hatch.poly.nCoords;
                            for (Int32 i = 1; i < nCoords; ++i) {
                                double vx = (*memo.coords)[i].x, vy = (*memo.coords)[i].y;
                                if (vx < xmin) xmin = vx; if (vx > xmax) xmax = vx;
                                if (vy < ymin) ymin = vy; if (vy > ymax) ymax = vy;
                            }
                            double cx = (xmin + xmax) / 2.0, cy = (ymin + ymax) / 2.0;
                            double curW = xmax - xmin, curH = ymax - ymin, newW = 0, newH = 0, unitM = unit / 1000.0;
                            if (fixX && !fixY) { newW = curW; newH = targetArea / newW; if (unitM > 0.0001) newH = std::round(newH / unitM) * unitM; }
                            else if (fixY && !fixX) { newH = curH; newW = targetArea / newH; if (unitM > 0.0001) newW = std::round(newW / unitM) * unitM; }
                            else { double s = std::sqrt(targetArea); if (unitM > 0.0001) s = std::round(s / unitM) * unitM; newW = newH = s; }
                            if (newW < 0.01) newW = 0.1; if (newH < 0.01) newH = 0.1;

                            API_ElementMemo newMemo; BNZeroMemory(&newMemo, sizeof(newMemo));
                            newMemo.coords = (API_Coord**)BMAllocateHandle(6 * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
                            newMemo.pends = (Int32**)BMAllocateHandle(2 * sizeof(Int32), ALLOCATE_CLEAR, 0);

                            ACAPI_DisposeElemMemoHdls(&memo);

                            if (newMemo.coords && newMemo.pends) {
                                double hw = newW / 2.0; double hh = newH / 2.0;
                                (*newMemo.coords)[1] = { cx - hw, cy - hh }; (*newMemo.coords)[2] = { cx + hw, cy - hh };
                                (*newMemo.coords)[3] = { cx + hw, cy + hh }; (*newMemo.coords)[4] = { cx - hw, cy + hh };
                                (*newMemo.coords)[5] = (*newMemo.coords)[1]; (*newMemo.pends)[0] = 0; (*newMemo.pends)[1] = 5;

                                API_Element mask; ACAPI_ELEMENT_MASK_CLEAR(mask);
                                if (el.header.type.typeID == API_ZoneID) {
                                    el.zone.poly.nCoords = 5;
                                    el.zone.poly.nSubPolys = 1;
                                    el.zone.poly.nArcs = 0;
                                    el.zone.manual = true;
                                    el.zone.pos = { cx, cy };
                                    el.zone.refPos = { cx, cy };

                                    ACAPI_ELEMENT_MASK_SET(mask, API_ZoneType, poly.nCoords);
                                    ACAPI_ELEMENT_MASK_SET(mask, API_ZoneType, poly.nSubPolys);
                                    ACAPI_ELEMENT_MASK_SET(mask, API_ZoneType, poly.nArcs);
                                    ACAPI_ELEMENT_MASK_SET(mask, API_ZoneType, manual);
                                    ACAPI_ELEMENT_MASK_SET(mask, API_ZoneType, pos);
                                    ACAPI_ELEMENT_MASK_SET(mask, API_ZoneType, refPos);
                                } else {
                                    el.hatch.poly.nCoords = 5;
                                    el.hatch.poly.nSubPolys = 1;
                                    el.hatch.poly.nArcs = 0;

                                    ACAPI_ELEMENT_MASK_SET(mask, API_HatchType, poly.nCoords);
                                    ACAPI_ELEMENT_MASK_SET(mask, API_HatchType, poly.nSubPolys);
                                    ACAPI_ELEMENT_MASK_SET(mask, API_HatchType, poly.nArcs);
                                }

                                ACAPI_Element_Change(&el, &mask, &newMemo, APIMemoMask_Polygon, true);
                            }
                            ACAPI_DisposeElemMemoHdls(&newMemo);
                        }
                    }
                }
                return NoError;
            });
        }
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
#ifdef ServerMainVers_2900
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
    return;
  }
};
MagicPalette* MagicPalette::instance = nullptr;

class SecondPalette : public DG::Palette,
                      public DG::PanelObserver,
                      public DG::ButtonItemObserver,
                      public DG::RealEditObserver {
private:
  static SecondPalette* instance;
  
  DG::RealEdit* editWidth = nullptr;
  DG::RealEdit* editHeight = nullptr;
  DG::CheckBox* checkLockRatio = nullptr;
  DG::Button* btnMatchDrawing = nullptr;
  DG::Button* btn05x = nullptr;
  DG::Button* btn20x = nullptr;
  DG::Button* btn40x = nullptr;
  DG::Button* btnApplyTarget = nullptr;
  DG::Button* btnRestore = nullptr;

  Int32 origWidth = 0;
  Int32 origHeight = 0;

public:
  static void Show() {
    if (instance == nullptr) {
      instance = new SecondPalette();
    }
    if (instance != nullptr) {
      instance->DG::Palette::Show();
    }
  }

  static void Destroy() {
    if (instance != nullptr) {
      delete instance;
      instance = nullptr;
    }
  }

  static GSErrCode __ACDLL_CALL StaticCallback(Int32, API_PaletteMessageID msg, GS::IntPtr) {
    if (msg == APIPalMsg_OpenPalette) Show();
    else if (msg == APIPalMsg_ClosePalette) Destroy();
    return NoError;
  }

  SecondPalette() : DG::Palette(ACAPI_GetOwnResModule(), 32501, ACAPI_GetOwnResModule(), secondAddonPaletteGuid) {
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

    // Initial state
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

  virtual ~SecondPalette() {
    EndEventProcessing();
    Detach(*this);
    
    delete editWidth;
    delete editHeight;
    delete checkLockRatio;
    delete btnMatchDrawing;
    delete btn05x;
    delete btn20x;
    delete btn40x;
    delete btnApplyTarget;
    delete btnRestore;
  }

  void Resize3D(double scale = 1.0, Int32 targetW = 0, Int32 targetH = 0) {
    API_3DWindowInfo windowInfo;
    BNZeroMemory(&windowInfo, sizeof(API_3DWindowInfo));
    if (ACAPI_View_Get3DWindowSets(&windowInfo) == NoError) {
        if (targetW > 0 && targetH > 0) {
            windowInfo.hSize = targetW;
            windowInfo.vSize = targetH;
        } else {
            windowInfo.hSize = (Int32)(windowInfo.hSize * scale);
            windowInfo.vSize = (Int32)(windowInfo.vSize * scale);
        }
        windowInfo.setWindowSize = true;
        ACAPI_View_Change3DWindowSets(&windowInfo);
        ACAPI_Database_RebuildCurrentDatabase();
    }
  }

  virtual void RealEditChanged(const DG::RealEditChangeEvent &ev) override {
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

  virtual void ButtonClicked(const DG::ButtonClickEvent &ev) override {
    if (ev.GetSource() == btnMatchDrawing) {
        API_SelectionInfo selectionInfo;
        GS::Array<API_Neig> selNeigs;
        if (ACAPI_Selection_Get(&selectionInfo, &selNeigs, true) == NoError) {
            for (const auto& neig : selNeigs) {
                API_Element element;
                BNZeroMemory(&element, sizeof(API_Element));
                element.header.guid = neig.guid;
                if (ACAPI_Element_GetHeader(&element.header) == NoError) {
                    if (element.header.type.typeID == API_DrawingID) {
                        if (ACAPI_Element_Get(&element) == NoError) {
                            double width = element.drawing.bounds.xMax - element.drawing.bounds.xMin;
                            double height = element.drawing.bounds.yMax - element.drawing.bounds.yMin;
                            if (height > 0) {
                                double ratio = width / height;
                                double currentW = editWidth->GetValue();
                                editHeight->SetValue(currentW / ratio);
                                ACAPI_WriteReport("Matched Drawing Aspect Ratio.", true);
                            }
                        }
                        break;
                    }
                }
            }
        }
    } else if (ev.GetSource() == btn05x) {
        Resize3D(0.5);
    } else if (ev.GetSource() == btn20x) {
        Resize3D(2.0);
    } else if (ev.GetSource() == btn40x) {
        Resize3D(4.0);
    } else if (ev.GetSource() == btnApplyTarget) {
        Resize3D(1.0, (Int32)editWidth->GetValue(), (Int32)editHeight->GetValue());
    } else if (ev.GetSource() == btnRestore) {
        Resize3D(1.0, origWidth, origHeight);
    }
  }

  virtual void PanelCloseRequested(const DG::PanelCloseRequestEvent &, bool *accept) override {
    *accept = true;
    DG::Palette::Hide();
  }
};
SecondPalette* SecondPalette::instance = nullptr;

class MeshElevator {
public:
    struct Triangle {
        API_Coord v1, v2, v3;
        double z1, z2, z3;
        double nx, ny, nz, d;
    };
    GS::Array<Triangle> triangles;

private:

    bool IsPointInTriangle(const API_Coord& pt, const Triangle& tri) const {
        auto sign = [](const API_Coord& p1, const API_Coord& p2, const API_Coord& p3) {
            return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
        };
        double d1 = sign(pt, tri.v1, tri.v2);
        double d2 = sign(pt, tri.v2, tri.v3);
        double d3 = sign(pt, tri.v3, tri.v1);
        bool has_neg = (d1 < -1e-6) || (d2 < -1e-6) || (d3 < -1e-6);
        bool has_pos = (d1 > 1e-6) || (d2 > 1e-6) || (d3 > 1e-6);
        return !(has_neg && has_pos);
    }

public:
    MeshElevator(const API_Guid& meshGuid) {
        API_ElemInfo3D info3D; BNZeroMemory(&info3D, sizeof(info3D));
        API_Elem_Head head = {}; head.guid = meshGuid;
        if (ACAPI_ModelAccess_Get3DInfo(head, &info3D) != NoError) return;

        for (Int32 iBody = info3D.fbody; iBody <= info3D.lbody; ++iBody) {
            API_Component3D bodyComp; BNZeroMemory(&bodyComp, sizeof(bodyComp));
            bodyComp.header.typeID = API_BodyID; bodyComp.header.index = iBody;
            if (ACAPI_ModelAccess_GetComponent(&bodyComp) != NoError) continue;

            const API_Tranmat& tm = bodyComp.body.tranmat;
            for (Int32 iPgon = 1; iPgon <= bodyComp.body.nPgon; ++iPgon) {
                API_Component3D pgonComp; BNZeroMemory(&pgonComp, sizeof(pgonComp));
                pgonComp.header.typeID = API_PgonID; pgonComp.header.index = iPgon;
                if (ACAPI_ModelAccess_GetComponent(&pgonComp) != NoError) continue;

                API_Component3D vectComp; BNZeroMemory(&vectComp, sizeof(vectComp));
                vectComp.header.typeID = API_VectID; vectComp.header.index = std::abs(pgonComp.pgon.ivect);
                if (ACAPI_ModelAccess_GetComponent(&vectComp) != NoError) continue;
                double nx = vectComp.vect.x, ny = vectComp.vect.y, nz = vectComp.vect.z;
                if (pgonComp.pgon.ivect < 0) { nx = -nx; ny = -ny; nz = -nz; }
                if (std::abs(nz) < 1e-6) continue;

                GS::Array<API_Coord3D> polyVerts;
                for (Int32 iPedg = pgonComp.pgon.fpedg; iPedg <= pgonComp.pgon.lpedg; ++iPedg) {
                    API_Component3D pedgComp; BNZeroMemory(&pedgComp, sizeof(pedgComp));
                    pedgComp.header.typeID = API_PedgID; pedgComp.header.index = iPedg;
                    if (ACAPI_ModelAccess_GetComponent(&pedgComp) != NoError || pedgComp.pedg.pedg == 0) break;
                    API_Component3D edgeComp; BNZeroMemory(&edgeComp, sizeof(edgeComp));
                    edgeComp.header.typeID = API_EdgeID; edgeComp.header.index = std::abs(pedgComp.pedg.pedg);
                    if (ACAPI_ModelAccess_GetComponent(&edgeComp) != NoError) continue;
                    Int32 vIdx = (pedgComp.pedg.pedg > 0) ? edgeComp.edge.vert1 : edgeComp.edge.vert2;
                    API_Component3D vComp; BNZeroMemory(&vComp, sizeof(vComp));
                    vComp.header.typeID = API_VertID; vComp.header.index = vIdx;
                    if (ACAPI_ModelAccess_GetComponent(&vComp) != NoError) continue;
                    API_Coord3D wv;
                    wv.x = tm.tmx[0]*vComp.vert.x + tm.tmx[1]*vComp.vert.y + tm.tmx[2]*vComp.vert.z + tm.tmx[3];
                    wv.y = tm.tmx[4]*vComp.vert.x + tm.tmx[5]*vComp.vert.y + tm.tmx[6]*vComp.vert.z + tm.tmx[7];
                    wv.z = tm.tmx[8]*vComp.vert.x + tm.tmx[9]*vComp.vert.y + tm.tmx[10]*vComp.vert.z + tm.tmx[11];
                    polyVerts.Push(wv);
                }
                if (polyVerts.GetSize() < 3) continue;
                for (USize i = 1; i < polyVerts.GetSize() - 1; ++i) {
                    Triangle tri;
                    tri.v1 = {polyVerts[0].x, polyVerts[0].y}; tri.z1 = polyVerts[0].z;
                    tri.v2 = {polyVerts[i].x, polyVerts[i].y}; tri.z2 = polyVerts[i].z;
                    tri.v3 = {polyVerts[i+1].x, polyVerts[i+1].y}; tri.z3 = polyVerts[i+1].z;
                    double vx1 = tri.v2.x - tri.v1.x, vy1 = tri.v2.y - tri.v1.y, vz1 = tri.z2 - tri.z1;
                    double vx2 = tri.v3.x - tri.v1.x, vy2 = tri.v3.y - tri.v1.y, vz2 = tri.z3 - tri.z1;
                    tri.nx = vy1 * vz2 - vz1 * vy2; tri.ny = vz1 * vx2 - vx1 * vz2; tri.nz = vx1 * vy2 - vy1 * vx2;
                    tri.d = -(tri.nx * tri.v1.x + tri.ny * tri.v1.y + tri.nz * tri.z1);
                    if (std::abs(tri.nz) > 1e-6) triangles.Push(tri);
                }
            }
        }
    }

    double GetElevation(const API_Coord& pt) const {
        double maxZ = -1e30;
        bool found = false;
        for (const auto& tri : triangles) {
            if (IsPointInTriangle(pt, tri)) {
                double z = -(tri.nx * pt.x + tri.ny * pt.y + tri.d) / tri.nz;
                if (z > maxZ) {
                    maxZ = z;
                    found = true;
                }
            }
        }
        return found ? maxZ : -1e30; // Return very low value if not on mesh
    }

    int GetTriangleCount() const { return (int)triangles.GetSize(); }
};

static bool IsPointInPolygon(const API_Coord& pt, const std::vector<API_Coord>& poly) {
    bool inside = false;
    for (size_t i = 0, j = poly.size() - 1; i < poly.size(); j = i++) {
        if (((poly[i].y > pt.y) != (poly[j].y > pt.y)) &&
            (pt.x < (poly[j].x - poly[i].x) * (pt.y - poly[i].y) / (poly[j].y - poly[i].y) + poly[i].x)) {
            inside = !inside;
        }
    }
    return inside;
}


// --- Helper: Offset Polygon ------------------------------------------------------------------------------------------
static std::vector<API_Coord> OffsetPolygon(const std::vector<API_Coord>& poly, double offset) {
    if (poly.size() < 3) return poly;
    std::vector<API_Coord> result;
    for (size_t i = 0; i < poly.size(); ++i) {
        API_Coord p1 = poly[(i + poly.size() - 1) % poly.size()];
        API_Coord p2 = poly[i];
        API_Coord p3 = poly[(i + 1) % poly.size()];

        double dx1 = p2.x - p1.x, dy1 = p2.y - p1.y;
        double len1 = std::sqrt(dx1*dx1 + dy1*dy1);
        double dx2 = p3.x - p2.x, dy2 = p3.y - p2.y;
        double len2 = std::sqrt(dx2*dx2 + dy2*dy2);

        if (len1 < 1e-6 || len2 < 1e-6) { result.push_back(p2); continue; }

        double nx1 = -dy1 / len1, ny1 = dx1 / len1;
        double nx2 = -dy2 / len2, ny2 = dx2 / len2;

        double nx = (nx1 + nx2);
        double ny = (ny1 + ny2);
        double len = std::sqrt(nx*nx + ny*ny);
        if (len < 1e-6) { result.push_back(p2); continue; }
        
        nx /= len; ny /= len;
        double dot = nx * nx1 + ny * ny1;
        if (std::abs(dot) < 1e-6) { result.push_back(p2); continue; }
        
        double actualOffset = offset / dot;
        result.push_back({ p2.x + nx * actualOffset, p2.y + ny * actualOffset });
    }
    return result;
}

class HeightPalette : public DG::Palette, public DG::PanelObserver, public DG::ButtonItemObserver {
private:
  static HeightPalette* instance;
  DG::LeftText *editBaseElevation, *txtAreaResult, *txtPerimeterResult, *txtAverageHeight;
  DG::Button *btnCalculate, *btnCreateGround;
  std::vector<API_Coord> lastFootprint;
  double lastFinalZ = 0; short lastFloorInd = 0;
public:
  static void Show() { if (!instance) instance = new HeightPalette(); instance->DG::Palette::Show(); }
  static void Destroy() { if (instance) { delete instance; instance = nullptr; } }
  static GSErrCode __ACDLL_CALL StaticCallback(Int32, API_PaletteMessageID msg, GS::IntPtr) {
    if (msg == APIPalMsg_OpenPalette) Show(); else if (msg == APIPalMsg_ClosePalette) Destroy(); return NoError;
  }
  HeightPalette() : DG::Palette(ACAPI_GetOwnResModule(), 32502, ACAPI_GetOwnResModule(), heightAddonPaletteGuid) {
    BeginEventProcessing();
    editBaseElevation = new DG::LeftText(GetReference(), 4); btnCalculate = new DG::Button(GetReference(), 5);
    txtAreaResult = new DG::LeftText(GetReference(), 8); txtPerimeterResult = new DG::LeftText(GetReference(), 10);
    txtAverageHeight = new DG::LeftText(GetReference(), 12); btnCreateGround = new DG::Button(GetReference(), 13);
    Attach(*this); btnCalculate->Attach(*this); btnCreateGround->Attach(*this); SetTitle("GL Calc");
  }
  virtual ~HeightPalette() { EndEventProcessing(); btnCalculate->Detach(*this); btnCreateGround->Detach(*this); Detach(*this); }
  virtual void ButtonClicked(const DG::ButtonClickEvent& ev) override {
    if (ev.GetSource() == btnCalculate) Calc(); else if (ev.GetSource() == btnCreateGround) CreateG();
  }
  void CreateG() {
    if (lastFootprint.empty()) return;
    auto op = OffsetPolygon(lastFootprint, 1.0);
    API_Element el; BNZeroMemory(&el, sizeof(el)); el.header.type = API_MorphID; ACAPI_Element_GetDefaults(&el, nullptr);
    el.header.floorInd = lastFloorInd; el.morph.level = 0.0;
    void* bD = nullptr; ACAPI_Body_Create(nullptr, nullptr, &bD);
    std::vector<VertIdx> vIs;
    for (auto& p : op) { VertIdx vI; API_Coord3D c; c.x=p.x; c.y=p.y; c.z=lastFinalZ; ACAPI_Body_AddVertex(bD, c, vI); vIs.push_back(vI); }
    if (vIs.size() >= 3) { PolyIdx pI;
      GS::Array<Int32> eds; for (auto i : vIs) eds.Push((Int32)i); 
      API_OverriddenAttribute m; BNZeroMemory(&m, sizeof(m)); 
      ACAPI_Body_AddPolygon(bD, eds, 0, m, pI);
    }
    API_ElementMemo memo; BNZeroMemory(&memo, sizeof(memo));
    if (ACAPI_Body_Finish(bD, &memo.morphBody, &memo.morphMaterialMapTable) == NoError) {
      ACAPI_CallUndoableCommand("Create GL", [&](){ return ACAPI_Element_Create(&el, &memo); });
      ACAPI_DisposeElemMemoHdls(&memo);
    }
  }
  void Calc() {
    API_SelectionInfo si; GS::Array<API_Neig> sel; if (ACAPI_Selection_Get(&si, &sel, false) != NoError || sel.IsEmpty()) return;
    API_Guid moG = APINULLGuid, meG = APINULLGuid;
    for (auto& n : sel) { API_Elem_Head h; h.guid = n.guid; if (ACAPI_Element_GetHeader(&h) == NoError) { if (h.type == API_MorphID) moG = h.guid; else if (h.type == API_MeshID) meG = h.guid; } }
    if (moG == APINULLGuid || meG == APINULLGuid) return;
    API_Element moE; moE.header.guid = moG; ACAPI_Element_Get(&moE); lastFloorInd = moE.header.floorInd;
    API_ElementMemo memo; ACAPI_Element_GetMemo(moG, &memo);
    std::vector<API_Coord> fp;
    if (memo.coords) { Int32 nC = BMGetHandleSize((GSHandle)memo.coords)/sizeof(API_Coord); for (Int32 i=1; i<nC; ++i) if ((*memo.coords)[i].x!=0||(*memo.coords)[i].y!=0) fp.push_back((*memo.coords)[i]); }
    ACAPI_DisposeElemMemoHdls(&memo); if (fp.empty()) return; lastFootprint = fp;
    double minX=fp[0].x, minY=fp[0].y, maxX=fp[0].x, maxY=fp[0].y;
    for (auto& p : fp) { minX=std::min(minX,p.x); maxX=std::max(maxX,p.x); minY=std::min(minY,p.y); maxY=std::max(maxY,p.y); }
    double moBZ = moE.morph.level, sumRZ = 0, sBZ = 1e30, maxRelZ = 0; std::vector<double> vZs; double step = 0.05;
    MeshElevator elevator(meG);
    if (elevator.GetTriangleCount() == 0) return;

    for (double x=minX; x<=maxX; x+=step) { for (double y=minY; y<=maxY; y+=step) {
      API_Coord p = {x,y}; if (IsPointInPolygon(p, fp)) { 
        double mZ = elevator.GetElevation(p);
        if (mZ > -1e9 && mZ >= moBZ) { vZs.push_back(mZ); sBZ = std::min(sBZ, mZ); } 
      }
    } }
    if (!vZs.empty()) {
      for (double z : vZs) { 
        double r = z - sBZ; if (r > maxRelZ) maxRelZ = r;
        sumRZ += std::min(3.0, std::max(0.0, r)); 
      }
      double aR = sumRZ / (double)vZs.size(); lastFinalZ = sBZ + aR;
      editBaseElevation->SetText(GS::UniString::Printf("Base: %.2f / MaxRel: %.2f", sBZ, maxRelZ)); 
      txtAreaResult->SetText(GS::UniString::Printf("Contact: %.2f m2 (%d pts)", (double)vZs.size()*step*step, (int)vZs.size()));
      txtAverageHeight->SetText(GS::UniString::Printf("%.2f + %.2f = GL %.2f m", sBZ, aR, lastFinalZ));
    }
  }
  virtual void PanelCloseRequested(const DG::PanelCloseRequestEvent&, bool* a) override { *a = true; DG::Palette::Hide(); }
};
HeightPalette* HeightPalette::instance = nullptr;


class TerrainPalette : public DG::Palette,
                       public DG::PanelObserver,
                       public DG::ButtonItemObserver {
private:
  static TerrainPalette* instance;
  DG::RealEdit* editDivisions = nullptr;
  DG::RealEdit* editPenIndex = nullptr;         // ID 4
  DG::Button* btnGenerate = nullptr;            // ID 5

public:
  static void Show() {
    if (instance == nullptr) {
      instance = new TerrainPalette();
    }
    if (instance != nullptr) {
      instance->DG::Palette::Show();
    }
  }

  static void Destroy() {
    if (instance != nullptr) {
      delete instance;
      instance = nullptr;
    }
  }

  static GSErrCode __ACDLL_CALL StaticCallback(Int32, API_PaletteMessageID msg, GS::IntPtr) {
    if (msg == APIPalMsg_OpenPalette) Show();
    else if (msg == APIPalMsg_ClosePalette) Destroy();
    return NoError;
  }

  TerrainPalette() : DG::Palette(ACAPI_GetOwnResModule(), 32503, ACAPI_GetOwnResModule(), terrainAddonPaletteGuid) {
    BeginEventProcessing();
    editDivisions      = new DG::RealEdit(GetReference(), 2);
    editPenIndex       = new DG::RealEdit(GetReference(), 4);
    btnGenerate        = new DG::Button(GetReference(), 5);

    Attach(*this);
    btnGenerate->Attach(*this);

    SetTitle("Terrain");
    editDivisions->SetValue(5.0); // Default to 5.0 divisions / 1.0m interval
    editPenIndex->SetValue(1.0);  // Default pen color index = 1 (typically black/blue)
  }

  virtual ~TerrainPalette() {
    EndEventProcessing();
    btnGenerate->Detach(*this);
    Detach(*this);

    delete editDivisions;
    delete editPenIndex;
    delete btnGenerate;
  }

  virtual void ButtonClicked(const DG::ButtonClickEvent& ev) override {
    if (ev.GetSource() == btnGenerate) {
      GenerateContours();
    }
  }

  virtual void PanelCloseRequested(const DG::PanelCloseRequestEvent&, bool* a) override {
    *a = true;
    DG::Palette::Hide();
  }

  void GenerateContours();
  void InterpolateContours(const GS::Array<API_Neig>& sel);
  void DrawContoursFromMesh(const GS::Array<API_Neig>& sel);
};

TerrainPalette* TerrainPalette::instance = nullptr;

void TerrainPalette::GenerateContours() {
    API_SelectionInfo si;
    GS::Array<API_Neig> sel;
    if (ACAPI_Selection_Get(&si, &sel, false) != NoError || sel.IsEmpty()) {
        ::MessageBoxW(nullptr, L"Please select Mesh or 2D Contour Polylines first.", L"Terrain", MB_OK | MB_ICONWARNING | MB_TOPMOST);
        return;
    }
    
    bool hasMesh = false;
    for (const auto& n : sel) {
        API_Elem_Head h;
        h.guid = n.guid;
        if (ACAPI_Element_GetHeader(&h) == NoError && h.type == API_MeshID) {
            hasMesh = true;
            break;
        }
    }
    
    if (hasMesh) {
        DrawContoursFromMesh(sel);
    } else {
        InterpolateContours(sel);
    }
}

void TerrainPalette::InterpolateContours(const GS::Array<API_Neig>& sel) {
    short targetPen = (short)editPenIndex->GetValue();
    if (targetPen < 1) targetPen = 1;
    if (targetPen > 255) targetPen = 255;

    struct PolylineData {
        API_Guid guid;
        std::vector<API_Coord> coords;
    };

    std::vector<PolylineData> polylines;

    for (const auto& n : sel) {
        API_Elem_Head h;
        h.guid = n.guid;
        if (ACAPI_Element_GetHeader(&h) == NoError) {
            std::vector<API_Coord> fp;
            if (h.type.typeID == API_PolyLineID || h.type.typeID == API_SplineID) {
                API_ElementMemo memo;
                if (ACAPI_Element_GetMemo(h.guid, &memo) == NoError) {
                    if (memo.coords) {
                        Int32 nC = BMGetHandleSize((GSHandle)memo.coords) / sizeof(API_Coord);
                        for (Int32 i = 1; i < nC; ++i) {
                            fp.push_back((*memo.coords)[i]);
                        }
                    }
                    ACAPI_DisposeElemMemoHdls(&memo);
                }
            } else if (h.type.typeID == API_LineID) {
                API_Element lineEl;
                lineEl.header = h;
                if (ACAPI_Element_Get(&lineEl) == NoError) {
                    fp.push_back(lineEl.line.begC);
                    fp.push_back(lineEl.line.endC);
                }
            }

            if (fp.size() >= 2) {
                PolylineData pd;
                pd.guid = h.guid;
                pd.coords = fp;
                polylines.push_back(pd);
            }
        }
    }

    if (polylines.size() < 2) {
        ::MessageBoxW(nullptr, L"Please select at least 2 contour lines.", L"Terrain", MB_OK | MB_ICONWARNING | MB_TOPMOST);
        return;
    }

    auto getAvgDistSq = [](const PolylineData& a, const PolylineData& b) {
        double sum = 0;
        for (const auto& pa : a.coords) {
            double minD = 1e30;
            for (const auto& pb : b.coords) {
                double d = std::pow(pa.x - pb.x, 2) + std::pow(pa.y - pb.y, 2);
                if (d < minD) minD = d;
            }
            sum += minD;
        }
        return sum / a.coords.size();
    };
    auto getSymmetricAvgDistSq = [&](const PolylineData& a, const PolylineData& b) {
        return getAvgDistSq(a, b) + getAvgDistSq(b, a);
    };

    size_t idxStart = 0, idxEnd = 0;
    double maxDistSq = -1.0;
    for (size_t i = 0; i < polylines.size(); ++i) {
        for (size_t j = i + 1; j < polylines.size(); ++j) {
            double distSq = getSymmetricAvgDistSq(polylines[i], polylines[j]);
            if (distSq > maxDistSq) {
                maxDistSq = distSq;
                idxStart = i;
                idxEnd = j;
            }
        }
    }

    std::vector<PolylineData> ordered;
    std::vector<bool> visited(polylines.size(), false);

    size_t current_idx = idxStart;
    visited[current_idx] = true;
    ordered.push_back(polylines[current_idx]);

    for (size_t step = 1; step < polylines.size(); ++step) {
        double bestDist = 1e30;
        size_t bestIdx = 0;
        for (size_t i = 0; i < polylines.size(); ++i) {
            if (!visited[i]) {
                double d = getSymmetricAvgDistSq(polylines[current_idx], polylines[i]);
                if (d < bestDist) {
                    bestDist = d;
                    bestIdx = i;
                }
            }
        }
        visited[bestIdx] = true;
        ordered.push_back(polylines[bestIdx]);
        current_idx = bestIdx;
    }

    polylines = ordered;

    double divVal = editDivisions->GetValue();
    int divisions = (int)divVal;
    if (divisions < 2) divisions = 2;

    int createdCount = 0;

    auto RamerDouglasPeucker = [](const std::vector<API_Coord>& points, double epsilon) {
        if (points.size() < 3) return points;
        std::vector<API_Coord> res;
        
        auto pointLineDist = [](const API_Coord& pt, const API_Coord& lineStart, const API_Coord& lineEnd) {
            double dx = lineEnd.x - lineStart.x;
            double dy = lineEnd.y - lineStart.y;
            if (dx == 0 && dy == 0) {
                return std::sqrt(std::pow(pt.x - lineStart.x, 2) + std::pow(pt.y - lineStart.y, 2));
            }
            double t = ((pt.x - lineStart.x) * dx + (pt.y - lineStart.y) * dy) / (dx * dx + dy * dy);
            if (t < 0) return std::sqrt(std::pow(pt.x - lineStart.x, 2) + std::pow(pt.y - lineStart.y, 2));
            if (t > 1) return std::sqrt(std::pow(pt.x - lineEnd.x, 2) + std::pow(pt.y - lineEnd.y, 2));
            double projX = lineStart.x + t * dx;
            double projY = lineStart.y + t * dy;
            return std::sqrt(std::pow(pt.x - projX, 2) + std::pow(pt.y - projY, 2));
        };

        struct Frame { int start; int end; };
        std::vector<Frame> stack;
        stack.push_back({0, (int)points.size() - 1});
        std::vector<bool> keep(points.size(), false);
        keep[0] = true;
        keep[points.size() - 1] = true;

        while (!stack.empty()) {
            Frame frame = stack.back();
            stack.pop_back();

            double dmax = 0;
            int index = frame.start;
            for (int i = frame.start + 1; i < frame.end; ++i) {
                double d = pointLineDist(points[i], points[frame.start], points[frame.end]);
                if (d > dmax) {
                    index = i;
                    dmax = d;
                }
            }
            if (dmax > epsilon) {
                keep[index] = true;
                stack.push_back({frame.start, index});
                stack.push_back({index, frame.end});
            }
        }
        for (size_t i = 0; i < points.size(); ++i) {
            if (keep[i]) res.push_back(points[i]);
        }
        return res;
    };

    auto ResamplePolyline = [](const std::vector<API_Coord>& poly, double stepLen) {
        if (poly.size() < 2) return poly;
        std::vector<API_Coord> resampled;
        resampled.push_back(poly.front());
        
        double accumulated = 0.0;
        for (size_t i = 1; i < poly.size(); ++i) {
            double dx = poly[i].x - poly[i-1].x;
            double dy = poly[i].y - poly[i-1].y;
            double segLen = std::hypot(dx, dy);
            if (segLen < 1e-9) continue;
            
            double remaining = segLen;
            API_Coord cur = poly[i-1];
            while (accumulated + remaining >= stepLen) {
                double needed = stepLen - accumulated;
                double t = needed / remaining;
                cur.x = cur.x + t * (poly[i].x - cur.x);
                cur.y = cur.y + t * (poly[i].y - cur.y);
                resampled.push_back(cur);
                remaining = std::hypot(poly[i].x - cur.x, poly[i].y - cur.y);
                accumulated = 0.0;
            }
            accumulated += remaining;
        }
        
        double lastDist = std::hypot(poly.back().x - resampled.back().x, poly.back().y - resampled.back().y);
        if (lastDist > 1e-4) {
            resampled.push_back(poly.back());
        }
        return resampled;
    };

    ACAPI_CallUndoableCommand("Interpolate 2D Contours", [&]() -> GSErrCode {
        for (size_t i = 0; i < polylines.size() - 1; ++i) {
            std::vector<API_Coord> poly1 = polylines[i].coords;
            std::vector<API_Coord> poly2 = polylines[i + 1].coords;

            if (poly1.empty() || poly2.empty()) continue;

            double d00_sq = std::pow(poly1.front().x - poly2.front().x, 2) + std::pow(poly1.front().y - poly2.front().y, 2);
            double d0K_sq = std::pow(poly1.front().x - poly2.back().x, 2) + std::pow(poly1.front().y - poly2.back().y, 2);
            if (d0K_sq < d00_sq) {
                std::reverse(poly2.begin(), poly2.end());
            }

            // Resample both lines to dense 0.1m step points
            std::vector<API_Coord> A = ResamplePolyline(poly1, 0.1);
            std::vector<API_Coord> B = ResamplePolyline(poly2, 0.1);

            int N = (int)A.size();
            int M = (int)B.size();
            if (N < 2 || M < 2) continue;

            // DTW cost matrix
            std::vector<std::vector<double>> cost(N, std::vector<double>(M, 0.0));
            auto getDist = [](const API_Coord& p1, const API_Coord& p2) {
                return std::hypot(p1.x - p2.x, p1.y - p2.y);
            };

            cost[0][0] = getDist(A[0], B[0]);
            for (int r = 1; r < N; ++r) {
                cost[r][0] = cost[r-1][0] + getDist(A[r], B[0]);
            }
            for (int c = 1; c < M; ++c) {
                cost[0][c] = cost[0][c-1] + getDist(A[0], B[c]);
            }

            for (int r = 1; r < N; ++r) {
                for (int c = 1; c < M; ++c) {
                    double minPrev = cost[r-1][c-1];
                    if (cost[r-1][c] < minPrev) minPrev = cost[r-1][c];
                    if (cost[r][c-1] < minPrev) minPrev = cost[r][c-1];
                    cost[r][c] = getDist(A[r], B[c]) + minPrev;
                }
            }

            // Backtrack
            std::vector<std::pair<int, int>> path;
            int r = N - 1;
            int c = M - 1;
            path.push_back({r, c});
            while (r > 0 || c > 0) {
                if (r == 0) {
                    c--;
                } else if (c == 0) {
                    r--;
                } else {
                    double c_diag = cost[r-1][c-1];
                    double c_left = cost[r][c-1];
                    double c_up = cost[r-1][c];

                    if (c_diag <= c_left && c_diag <= c_up) {
                        r--; c--;
                    } else if (c_left < c_diag && c_left <= c_up) {
                        c--;
                    } else {
                        r--;
                    }
                }
                path.push_back({r, c});
            }
            std::reverse(path.begin(), path.end());

            for (int step = 1; step < divisions; ++step) {
                double t_div = (double)step / divisions;
                std::vector<API_Coord> interpLine;

                for (const auto& p : path) {
                    API_Coord pt;
                    pt.x = A[p.first].x * (1.0 - t_div) + B[p.second].x * t_div;
                    pt.y = A[p.first].y * (1.0 - t_div) + B[p.second].y * t_div;

                    if (interpLine.empty()) {
                        interpLine.push_back(pt);
                    } else {
                        double distSq = std::pow(interpLine.back().x - pt.x, 2) + std::pow(interpLine.back().y - pt.y, 2);
                        if (distSq > 1e-6) interpLine.push_back(pt);
                    }
                }

                if (interpLine.size() > 2) {
                    interpLine = RamerDouglasPeucker(interpLine, 0.01);
                }
                if (interpLine.size() >= 2) {
                    API_Element element;
                    BNZeroMemory(&element, sizeof(element));
                    element.header.type = API_PolyLineID;
                    ACAPI_Element_GetDefaults(&element, nullptr);

                    element.polyLine.linePen.penIndex = targetPen;
                    element.polyLine.poly.nCoords = (Int32)interpLine.size();
                    element.polyLine.poly.nSubPolys = 1;
                    element.polyLine.poly.nArcs = 0;

                    API_ElementMemo memo;
                    BNZeroMemory(&memo, sizeof(memo));
                    memo.coords = (API_Coord**)BMAllocateHandle((interpLine.size() + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
                    if (memo.coords) {
                        (*memo.coords)[0].x = 0.0; (*memo.coords)[0].y = 0.0;
                        for (size_t j = 0; j < interpLine.size(); ++j) {
                            (*memo.coords)[j + 1] = interpLine[j];
                        }
                    }

                    memo.pends = (Int32**)BMAllocateHandle(2 * sizeof(Int32), ALLOCATE_CLEAR, 0);
                    if (memo.pends) {
                        (*memo.pends)[0] = 0;
                        (*memo.pends)[1] = (Int32)interpLine.size();
                    }

                    GSErrCode err = ACAPI_Element_Create(&element, &memo);
                    ACAPI_DisposeElemMemoHdls(&memo);
                    if (err == NoError) {
                        createdCount++;
                    }
                }
            }
        }
        return NoError;
    });

    GS::UniString msg = GS::UniString::Printf("Successfully created %d intermediate contour polylines.", createdCount);
    ::MessageBoxW(nullptr, (const WCHAR*)msg.ToUStr().Get(), L"Terrain", MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
}
void TerrainPalette::DrawContoursFromMesh(const GS::Array<API_Neig>& sel) {
    short targetPen = (short)editPenIndex->GetValue();
    if (targetPen < 1) targetPen = 1;
    if (targetPen > 255) targetPen = 255;

    API_Guid meshGuid = APINULLGuid;
    for (const auto& n : sel) {
        API_Elem_Head h;
        h.guid = n.guid;
        if (ACAPI_Element_GetHeader(&h) == NoError && h.type == API_MeshID) {
            meshGuid = h.guid;
            break;
        }
    }

    if (meshGuid == APINULLGuid) {
        ::MessageBoxW(nullptr, L"Please select a Mesh element first.", L"Terrain", MB_OK | MB_ICONWARNING | MB_TOPMOST);
        return;
    }

    MeshElevator elevator(meshGuid);
    if (elevator.triangles.IsEmpty()) {
        ::MessageBoxW(nullptr, L"No 3D geometry found in the selected Mesh.", L"Terrain", MB_OK | MB_ICONWARNING | MB_TOPMOST);
        return;
    }

    double step = editDivisions->GetValue();
    if (step <= 0.01) step = 1.0;

    double minZ = 1e30, maxZ = -1e30;
    for (const auto& tri : elevator.triangles) {
        minZ = std::min({ minZ, tri.z1, tri.z2, tri.z3 });
        maxZ = std::max({ maxZ, tri.z1, tri.z2, tri.z3 });
    }

    if (minZ >= maxZ) return;

    std::vector<double> levels;
    double startLevel = std::ceil(minZ / step) * step;
    for (double h = startLevel; h <= maxZ; h += step) {
        levels.push_back(h);
    }

    if (levels.empty()) {
        ::MessageBoxW(nullptr, L"No contour levels intersect with the selected Mesh Z range.", L"Terrain", MB_OK | MB_ICONWARNING | MB_TOPMOST);
        return;
    }

    struct LineSegment {
        API_Coord p1, p2;
    };

    int createdContours = 0;

    ACAPI_CallUndoableCommand("Generate Contours from Mesh", [&]() -> GSErrCode {
        for (double H : levels) {
            std::vector<LineSegment> segments;

            for (const auto& tri : elevator.triangles) {
                double d1 = tri.z1 - H;
                double d2 = tri.z2 - H;
                double d3 = tri.z3 - H;

                auto getIntersection = [&](const API_Coord& vA, double zA, const API_Coord& vB, double zB) -> API_Coord {
                    double t = (H - zA) / (zB - zA);
                    API_Coord pt;
                    pt.x = vA.x + t * (vB.x - vA.x);
                    pt.y = vA.y + t * (vB.y - vA.y);
                    return pt;
                };

                std::vector<API_Coord> intersectPts;

                if ((d1 >= 0.0 && d2 < 0.0) || (d1 < 0.0 && d2 >= 0.0)) {
                    if (std::abs(tri.z2 - tri.z1) > 1e-6)
                        intersectPts.push_back(getIntersection(tri.v1, tri.z1, tri.v2, tri.z2));
                }
                if ((d2 >= 0.0 && d3 < 0.0) || (d2 < 0.0 && d3 >= 0.0)) {
                    if (std::abs(tri.z3 - tri.z2) > 1e-6)
                        intersectPts.push_back(getIntersection(tri.v2, tri.z2, tri.v3, tri.z3));
                }
                if ((d3 >= 0.0 && d1 < 0.0) || (d3 < 0.0 && d1 >= 0.0)) {
                    if (std::abs(tri.z1 - tri.z3) > 1e-6)
                        intersectPts.push_back(getIntersection(tri.v3, tri.z3, tri.v1, tri.z1));
                }

                if (intersectPts.size() >= 2) {
                    LineSegment seg;
                    seg.p1 = intersectPts[0];
                    seg.p2 = intersectPts[1];
                    segments.push_back(seg);
                }
            }

            if (segments.empty()) continue;

            std::vector<std::vector<API_Coord>> polylines;
            std::vector<bool> used(segments.size(), false);

            for (size_t i = 0; i < segments.size(); ++i) {
                if (used[i]) continue;

                std::vector<API_Coord> currentPoly;
                currentPoly.push_back(segments[i].p1);
                currentPoly.push_back(segments[i].p2);
                used[i] = true;

                bool extended = true;
                while (extended) {
                    extended = false;
                    API_Coord head = currentPoly.front();
                    API_Coord tail = currentPoly.back();

                    for (size_t j = 0; j < segments.size(); ++j) {
                        if (used[j]) continue;

                        double d_tail_p1 = std::hypot(tail.x - segments[j].p1.x, tail.y - segments[j].p1.y);
                        double d_tail_p2 = std::hypot(tail.x - segments[j].p2.x, tail.y - segments[j].p2.y);
                        double d_head_p1 = std::hypot(head.x - segments[j].p1.x, head.y - segments[j].p1.y);
                        double d_head_p2 = std::hypot(head.x - segments[j].p2.x, head.y - segments[j].p2.y);

                        const double tolerance = 1e-3;

                        if (d_tail_p1 < tolerance) {
                            currentPoly.push_back(segments[j].p2);
                            used[j] = true;
                            extended = true;
                            break;
                        } else if (d_tail_p2 < tolerance) {
                            currentPoly.push_back(segments[j].p1);
                            used[j] = true;
                            extended = true;
                            break;
                        } else if (d_head_p1 < tolerance) {
                            currentPoly.insert(currentPoly.begin(), segments[j].p2);
                            used[j] = true;
                            extended = true;
                            break;
                        } else if (d_head_p2 < tolerance) {
                            currentPoly.insert(currentPoly.begin(), segments[j].p1);
                            used[j] = true;
                            extended = true;
                            break;
                        }
                    }
                }

                if (currentPoly.size() >= 2) {
                    polylines.push_back(currentPoly);
                }
            }

            for (const auto& polyCoords : polylines) {
                API_Element element;
                BNZeroMemory(&element, sizeof(element));
                element.header.type = API_PolyLineID;
                ACAPI_Element_GetDefaults(&element, nullptr);
                element.polyLine.linePen.penIndex = targetPen;

                Int32 nCoords = (Int32)polyCoords.size();
                element.polyLine.poly.nCoords = nCoords;
                element.polyLine.poly.nSubPolys = 1;
                element.polyLine.poly.nArcs = 0;

                API_ElementMemo memo;
                BNZeroMemory(&memo, sizeof(memo));

                memo.coords = (API_Coord**)BMAllocateHandle((nCoords + 1) * sizeof(API_Coord), ALLOCATE_CLEAR, 0);
                if (memo.coords) {
                    (*memo.coords)[0].x = 0.0; (*memo.coords)[0].y = 0.0;
                    for (Int32 j = 0; j < nCoords; ++j) {
                        (*memo.coords)[j + 1] = polyCoords[j];
                    }
                }

                memo.pends = (Int32**)BMAllocateHandle(2 * sizeof(Int32), ALLOCATE_CLEAR, 0);
                if (memo.pends) {
                    (*memo.pends)[0] = 0;
                    (*memo.pends)[1] = nCoords;
                }

                GSErrCode err = ACAPI_Element_Create(&element, &memo);
                ACAPI_DisposeElemMemoHdls(&memo);
                if (err == NoError) {
                    createdContours++;
                }
            }
        }
        return NoError;
    });

    GS::UniString msg = GS::UniString::Printf("Successfully generated %d contour polylines from Mesh.", createdContours);
    ::MessageBoxW(nullptr, (const WCHAR*)msg.ToUStr().Get(), L"Terrain", MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
}


} // namespace MyProjectNamespace

static GSErrCode __ACDLL_CALL MenuHandler1(const API_MenuParams*) {
  MyProjectNamespace::MagicPalette::Show();
  return NoError;
}

static GSErrCode __ACDLL_CALL MenuHandler2(const API_MenuParams*) {
  MyProjectNamespace::SecondPalette::Show();
  return NoError;
}

static GSErrCode __ACDLL_CALL MenuHandler3(const API_MenuParams*) {
  MyProjectNamespace::HeightPalette::Show();
  return NoError;
}

static GSErrCode __ACDLL_CALL MenuHandler4(const API_MenuParams*) {
  MyProjectNamespace::TerrainPalette::Show();
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
  ACAPI_MenuItem_InstallMenuHandler(32500, MenuHandler1);
  ACAPI_MenuItem_InstallMenuHandler(32501, MenuHandler2);
  ACAPI_MenuItem_InstallMenuHandler(32502, MenuHandler3);
  ACAPI_MenuItem_InstallMenuHandler(32503, MenuHandler4);
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


