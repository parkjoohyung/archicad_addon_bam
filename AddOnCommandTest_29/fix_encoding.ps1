$path = 'D:\park\07_anti\01_archicadmcp\cpp_addon\AddOnCommandTest\Src\AddOnCommandTest.cpp'
$content = Get-Content -Path $path -Raw
$utf8withbom = New-Object System.Text.UTF8Encoding($true)
[System.IO.File]::WriteAllText($path, $content, $utf8withbom)
