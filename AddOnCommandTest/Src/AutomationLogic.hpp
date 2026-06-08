#ifndef AUTOMATION_LOGIC_HPP
#define AUTOMATION_LOGIC_HPP

#include "ACAPinc.h"
#include <vector>

namespace AddOnLogic {

    // MagicPalette Logic
    void SimplifyPolyline(double tolMM, bool preserveCurve);
    void CreateFromSelection(double heightVal, bool isZone, bool isMesh, bool isMorph);
    void ApplyLabeling(const GS::UniString& prefix, Int32 start, int style);
    void UpdateTotalArea(double& outTotalArea, int& outCount);
    bool AutoCreateZonesFromExcel(const GS::UniString& excelPath);
    void ReshapeElements(double targetArea, double unit, bool fixX, bool fixY);

    // SecondPalette Logic
    void Resize3D(double scale, Int32 targetW, Int32 targetH, Int32& inOutOrigWidth, Int32& inOutOrigHeight);
    bool MatchDrawingAspectRatio(double& outRatio);

    // HeightPalette Logic
    struct GLResult {
        double baseZ;
        double maxRelZ;
        double finalZ;
        double contactArea;
        int ptCount;
    };
    
    bool CalculateGL(const API_Guid& morphGuid, const API_Guid& meshGuid, GLResult& result, std::vector<API_Coord>& outFootprint, short& outFloorInd);
    void CreateGLGround(const std::vector<API_Coord>& footprint, double finalZ, short floorInd);

    // TerrainPalette Logic
    void GenerateContours(double divisionsVal, short penIndex);

} // namespace AddOnLogic

#endif // AUTOMATION_LOGIC_HPP

