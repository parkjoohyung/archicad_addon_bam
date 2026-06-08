#ifndef UI_PALETTES_HPP
#define UI_PALETTES_HPP

#include "ACAPinc.h"

#ifndef __ACDLL_CALL
#define __ACDLL_CALL
#endif

#include "DGButton.hpp"
#include "DGCheckItem.hpp"
#include "DGDialog.hpp"
#include "DGEditControl.hpp"
#include "DGPopUp.hpp"
#include "DGRadioItem.hpp"
#include "DGStaticItem.hpp"
#include "DGFileDialog.hpp"

namespace MyProjectNamespace {

    // Licensing Activation Dialog
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
        ActivationDialog();
        virtual ~ActivationDialog();

        virtual void ButtonClicked(const DG::ButtonClickEvent& ev) override;
    };

    // MagicPalette: Main UI Panel
    class MagicPalette : public DG::Palette,
                         public DG::PanelObserver,
                         public DG::ButtonItemObserver,
                         public DG::TextEditBaseObserver,
                         public DG::PopUpObserver,
                         public DG::RadioItemObserver,
                         public DG::RealEditObserver,
                         public DG::CheckItemObserver 
    {
    private:
        static MagicPalette* instance;

        // UI controls matching GRC
        DG::Button* btnAutoCreate;
        DG::Button* btnBrowse;
        DG::LeftText* txtExcelPath;
        DG::RealEdit* editTargetArea;
        DG::RealEdit* editTolerance;
        DG::Button* btnReshape;
        DG::Button* btnSumSelected;
        DG::LeftText* txtTotalArea;
        DG::RadioButton* rbZone;
        DG::RadioButton* rbMesh;
        DG::Button* btnCreate;
        DG::RadioButton* rbMorph;
        DG::RealEdit* editHeight;
        DG::TextEdit* editPrefix;
        DG::RealEdit* editUnit;
        DG::CheckBox* checkFixX;
        DG::CheckBox* checkFixY;
        DG::Button* btnPickPrefix;
        DG::PopUp* popStyle;
        DG::TextEdit* editStartNo;
        DG::MultiLineEdit* editPreview;
        DG::Button* btnApplyLabeling;
        DG::Button* btnCalcRange;
        DG::LeftText* txtAllowedRange;
        DG::Button* btnSimplifyPoly;
        DG::LeftText* txtSimplifyTolLabel;
        DG::RealEdit* editSimplifyTol;
        DG::CheckBox* checkPreserveCurve;
        DG::Button* btnCheckUpdate;
        DG::LeftText* txtVersion;

        void SimplifyPolyline();
        void CreateFromSelection();
        void UpdatePreview();
        void ApplyLabeling();
        void UpdateTotalArea();

    public:
        static void Show();
        static void Destroy();
        static GSErrCode __ACDLL_CALL StaticCallback(Int32, API_PaletteMessageID msg, GS::IntPtr);

        MagicPalette();
        virtual ~MagicPalette();

        virtual void TextEditChanged(const DG::TextEditChangeEvent& ev) override;
        virtual void PopUpChanged(const DG::PopUpChangeEvent& ev) override;
        virtual void PanelCloseRequested(const DG::PanelCloseRequestEvent&, bool* accept) override;
        virtual void ButtonClicked(const DG::ButtonClickEvent& ev) override;

        void Hide() { DG::Palette::Hide(); }
    };

    // SecondPalette: 3D Resolution Helper
    class SecondPalette : public DG::Palette,
                          public DG::PanelObserver,
                          public DG::ButtonItemObserver,
                          public DG::RealEditObserver 
    {
    private:
        static SecondPalette* instance;
        
        DG::RealEdit* editWidth;
        DG::RealEdit* editHeight;
        DG::CheckBox* checkLockRatio;
        DG::Button* btnMatchDrawing;
        DG::Button* btn05x;
        DG::Button* btn20x;
        DG::Button* btn40x;
        DG::Button* btnApplyTarget;
        DG::Button* btnRestore;

        Int32 origWidth;
        Int32 origHeight;

        void Resize3DWindow(double scale = 1.0, Int32 targetW = 0, Int32 targetH = 0);

    public:
        static void Show();
        static void Destroy();
        static GSErrCode __ACDLL_CALL StaticCallback(Int32, API_PaletteMessageID msg, GS::IntPtr);

        SecondPalette();
        virtual ~SecondPalette();

        virtual void RealEditChanged(const DG::RealEditChangeEvent& ev) override;
        virtual void ButtonClicked(const DG::ButtonClickEvent& ev) override;
        virtual void PanelCloseRequested(const DG::PanelCloseRequestEvent&, bool* accept) override;
    };

    // HeightPalette: GL Ground calculator
    class HeightPalette : public DG::Palette,
                          public DG::PanelObserver,
                          public DG::ButtonItemObserver 
    {
    private:
        static HeightPalette* instance;
        
        DG::LeftText* editBaseElevation;
        DG::LeftText* txtAreaResult;
        DG::LeftText* txtPerimeterResult;
        DG::LeftText* txtAverageHeight;
        DG::Button* btnCalculate;
        DG::Button* btnCreateGround;
        
        std::vector<API_Coord> lastFootprint;
        double lastFinalZ;
        short lastFloorInd;

        void Calc();
        void CreateG();

    public:
        static void Show();
        static void Destroy();
        static GSErrCode __ACDLL_CALL StaticCallback(Int32, API_PaletteMessageID msg, GS::IntPtr);

        HeightPalette();
        virtual ~HeightPalette();

        virtual void ButtonClicked(const DG::ButtonClickEvent& ev) override;
        virtual void PanelCloseRequested(const DG::PanelCloseRequestEvent&, bool* accept) override;
    };

    // TerrainPalette: Terrain Generator & Interpolator
    class TerrainPalette : public DG::Palette,
                           public DG::PanelObserver,
                           public DG::ButtonItemObserver 
    {
    private:
        static TerrainPalette* instance;
        DG::RealEdit* editDivisions;
        DG::RealEdit* editPenIndex;
        DG::Button* btnGenerate;

        void GenerateContours();

    public:
        static void Show();
        static void Destroy();
        static GSErrCode __ACDLL_CALL StaticCallback(Int32, API_PaletteMessageID msg, GS::IntPtr);

        TerrainPalette();
        virtual ~TerrainPalette();

        virtual void ButtonClicked(const DG::ButtonClickEvent& ev) override;
        virtual void PanelCloseRequested(const DG::PanelCloseRequestEvent&, bool* accept) override;
    };

} // namespace MyProjectNamespace

#endif // UI_PALETTES_HPP
