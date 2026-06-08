$content = Get-Content -Path 'D:\park\07_anti\01_archicadmcp\cpp_addon\AddOnCommandTest\Src\AddOnCommandTest.cpp' -Raw
$functions = @('void CreateFromSelection() ', 'void ApplyLabeling() ', 'void UpdateTotalArea() ', 'void Resize3D(double scale = 1.0, Int32 targetW = 0, Int32 targetH = 0) ', 'void Calc() ', 'void GenerateContours() ', 'void InterpolateContours(const GS::Array<API_Neig>& sel) ', 'void DrawContoursFromMesh(const GS::Array<API_Neig>& sel) ')

foreach ($func in $functions) {
    $idx = $content.IndexOf($func)
    if ($idx -ne -1) {
        $startBrace = $content.IndexOf('{', $idx)
        if ($startBrace -ne -1) {
            $braceCount = 0
            $endBrace = -1
            for ($i = $startBrace; $i -lt $content.Length; $i++) {
                if ($content[$i] -eq '{') { $braceCount++ }
                elseif ($content[$i] -eq '}') {
                    $braceCount--
                    if ($braceCount -eq 0) {
                        $endBrace = $i
                        break
                    }
                }
            }
            if ($endBrace -ne -1) {
                $body = $content.Substring($startBrace, $endBrace - $startBrace + 1)
                $content = $content.Substring(0, $startBrace) + '{ /* Logic Moved to AddOnLogic namespace */ }' + $content.Substring($endBrace + 1)
                $body | Out-File -FilePath 'extracted_bodies.txt' -Append -Encoding UTF8
            }
        }
    }
}
$content | Out-File -FilePath 'AddOnCommandTest_stripped.cpp' -Encoding UTF8
