import os

extracted_file = 'D:/park/07_anti/01_archicadmcp/cpp_addon/AddOnCommandTest/Src/extracted_bodies.txt'
with open(extracted_file, 'r', encoding='utf-8') as f:
    bodies = f.read()

# We know the exact text of the extracted bodies.
# Let's write the cpp file directly since we know what parameters to inject.

cpp_content = '''#include "AutomationLogic.hpp"
#include <vector>
#include <cmath>

namespace AutomationLogic {

// 1. SimplifyPolyline is already in AddOnLogic, we can just declare it or keep it here.
// Let's assume SimplifyPolyline logic will be moved here.

// 2. CreateFromSelection
GSErrCode CreateFromSelection(double hVal, bool isZone, bool isMesh, bool isMorph) {
'''

# Find the CreateFromSelection body
# We know it starts with { ACAPI_WriteReport ... CreateFromSelection Start
idx_create_start = bodies.find('ACAPI_WriteReport(GS::UniString("---- [Debug] CreateFromSelection Start ----"')
if idx_create_start != -1:
    # Find the end
    idx_create_end = bodies.find('ACAPI_WriteReport("---- [Debug] CreateFromSelection End ----", false);', idx_create_start)
    if idx_create_end != -1:
        # Extract the content inside the function
        create_body = bodies[idx_create_start:idx_create_end + len('ACAPI_WriteReport("---- [Debug] CreateFromSelection End ----", false);\n  }')]
        
        # Replace the hardcoded UI reads
        create_body = create_body.replace('double hVal = editHeight ? editHeight->GetValue() : 0.0;', '')
        create_body = create_body.replace('bool isZone = rbZone->IsSelected();', '')
        create_body = create_body.replace('bool isMesh = rbMesh->IsSelected();', '')
        create_body = create_body.replace('bool isMorph = rbMorph->IsSelected();', '')
        
        # Remove the closing bracket so we can append to it
        create_body = create_body.rsplit('}', 1)[0]
        cpp_content += '    ' + create_body.strip() + '\n    return NoError;\n}\n\n'

# 3. ApplyLabeling
cpp_content += 'GSErrCode ApplyLabeling(const GS::UniString& prefix, int start, int style) {\n'
idx_label = bodies.find('API_SelectionInfo si; GS::Array<API_Neig> sel;\n    if (ACAPI_Selection_Get(&si, &sel, false) != NoError || sel.IsEmpty()) {\n        ACAPI_WriteReport(GS::UniString("Please select elements first.", CC_UTF8), true);\n        return;')
if idx_label != -1:
    idx_label_end = bodies.find('return NoError;\n    });', idx_label)
    if idx_label_end != -1:
        label_body = bodies[idx_label:idx_label_end + len('return NoError;\n    });')]
        # Replace UI reads
        label_body = label_body.replace('GS::UniString prefix = editPrefix->GetText();', '')
        label_body = label_body.replace('Int32 start = (Int32)atoi(editStartNo->GetText().ToCStr().Get());\n    if (start == 0 && editStartNo->GetText() != "0") start = 1;', '')
        label_body = label_body.replace('int style = popStyle->GetSelectedItem();', '')
        
        cpp_content += '    ' + label_body + '\n    return NoError;\n}\n\n'

# 4. UpdateTotalArea
cpp_content += 'double CalculateTotalArea() {\n'
idx_area = bodies.find('API_SelectionInfo si; GS::Array<API_Neig> sel;\n    if (ACAPI_Selection_Get(&si, &sel, false) != NoError || sel.IsEmpty()) return;')
if idx_area != -1:
    idx_area_end = bodies.find('ACAPI_WriteReport(GS::UniString::Printf("[Sum] %d Elements. Total: %.2f m2", count, totalArea), false);', idx_area)
    if idx_area_end != -1:
        area_body = bodies[idx_area:idx_area_end + len('ACAPI_WriteReport(GS::UniString::Printf("[Sum] %d Elements. Total: %.2f m2", count, totalArea), false);')]
        
        # Replace return type from return; to return 0.0;
        area_body = area_body.replace('return;', 'return 0.0;')
        area_body = area_body.replace('if (txtTotalArea) {\n        txtTotalArea->SetText(GS::UniString::Printf("%.2f", totalArea));\n        txtTotalArea->Redraw();\n    }', '')
        
        cpp_content += '    ' + area_body + '\n    return totalArea;\n}\n\n'

# 5. Resize3DWindow
cpp_content += 'void Resize3DWindow(double scale, int targetW, int targetH) {\n'
idx_resize = bodies.find('API_3DWindowInfo windowInfo;')
if idx_resize != -1:
    idx_resize_end = bodies.find('ACAPI_Database_RebuildCurrentDatabase();\n    }', idx_resize)
    if idx_resize_end != -1:
        resize_body = bodies[idx_resize:idx_resize_end + len('ACAPI_Database_RebuildCurrentDatabase();\n    }')]
        cpp_content += '    ' + resize_body + '\n}\n\n'

cpp_content += '}\n'

with open('D:/park/07_anti/01_archicadmcp/cpp_addon/AddOnCommandTest/Src/AutomationLogic.cpp', 'w', encoding='utf-8') as f:
    f.write(cpp_content)

print("Created AutomationLogic.cpp successfully")
