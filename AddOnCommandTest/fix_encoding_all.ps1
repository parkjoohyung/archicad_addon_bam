$files = @(
    'D:\park\07_anti\01_archicadmcp\cpp_addon\AddOnCommandTest\Src\AddOnCommandTest.cpp',
    'D:\park\07_anti\01_archicadmcp\cpp_addon\AddOnCommandTest\RINT\AddOnCommandTest.grc'
)
$utf8withbom = New-Object System.Text.UTF8Encoding($true)
foreach ($f in $files) {
    $content = Get-Content -Path $f -Raw
    [System.IO.File]::WriteAllText($f, $content, $utf8withbom)
}
