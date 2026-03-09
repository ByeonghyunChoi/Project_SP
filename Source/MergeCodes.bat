@echo off
chcp 65001 >nul
echo 코드를 하나로 모으는 중입니다... 기다려주세요!

powershell -NoProfile -ExecutionPolicy Bypass -Command "$outFile = 'allcodes.txt'; if (Test-Path $outFile) { Remove-Item $outFile }; Get-ChildItem -Include *.h,*.cpp -Recurse | ForEach-Object { $header = \"`r`n// ==========================================`r`n// FILE: $($_.Name)`r`n// ==========================================`r`n\"; Add-Content -Path $outFile -Value $header -Encoding UTF8; $content = Get-Content $_.FullName; Add-Content -Path $outFile -Value $content -Encoding UTF8 }"

echo 완료! allcodes.txt가 생성되었습니다.
pause