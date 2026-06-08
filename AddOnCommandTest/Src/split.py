import os
import re

source_file = 'D:/park/07_anti/01_archicadmcp/cpp_addon/AddOnCommandTest/Src/AddOnCommandTest.cpp'
with open(source_file, 'r', encoding='utf-8') as f:
    content = f.read()

def extract_function_body(source, func_signature):
    idx = source.find(func_signature)
    if idx == -1: return None, source
    
    start_brace = source.find('{', idx)
    if start_brace == -1: return None, source
    
    brace_count = 0
    end_brace = -1
    for i in range(start_brace, len(source)):
        if source[i] == '{': brace_count += 1
        elif source[i] == '}': 
            brace_count -= 1
            if brace_count == 0:
                end_brace = i
                break
                
    if end_brace != -1:
        func_body = source[start_brace:end_brace+1]
        new_source = source[:start_brace] + '{ /* Logic Moved to AddOnLogic namespace */ }' + source[end_brace+1:]
        return func_body, new_source
    return None, source

functions_to_extract = [
    'void CreateFromSelection() ',
    'void ApplyLabeling() ',
    'void UpdateTotalArea() ',
    'void Resize3D(double scale = 1.0, Int32 targetW = 0, Int32 targetH = 0) ',
    'void Calc() ',
    'void GenerateContours() ',
    'void InterpolateContours(const GS::Array<API_Neig>& sel) ',
    'void DrawContoursFromMesh(const GS::Array<API_Neig>& sel) '
]

extracted_bodies = {}
new_content = content

for sig in functions_to_extract:
    body, new_content = extract_function_body(new_content, sig)
    if body:
        extracted_bodies[sig] = body

with open('extracted_bodies.txt', 'w', encoding='utf-8') as f:
    for sig, body in extracted_bodies.items():
        f.write('// ================================\n')
        f.write('// ' + sig + '\n')
        f.write('// ================================\n')
        f.write(body + '\n\n')

with open('AddOnCommandTest_stripped.cpp', 'w', encoding='utf-8') as f:
    f.write(new_content)

print(f'Extracted {len(extracted_bodies)} functions.')
