@echo off
chcp 65001 >nul
echo 코드를 읽고 6등분으로 나누는 중입니다... 기다려주세요!

powershell -NoProfile -ExecutionPolicy Bypass -Command "$files = Get-ChildItem -Include *.h,*.cpp -Recurse; $allLines = @(foreach ($f in $files) { '// =========================================='; '// FILE: ' + $f.Name; '// =========================================='; Get-Content $f.FullName -Encoding UTF8 }); $total = $allLines.Count; if ($total -eq 0) { Write-Host '처리할 파일이 없습니다.'; exit }; $chunkSize = [math]::Ceiling($total / 6.0); for ($i = 0; $i -lt 6; $i++) { $start = $i * $chunkSize; if ($start -ge $total) { break }; $end = [math]::Min($start + $chunkSize - 1, $total - 1); $outFile = 'allcodes_' + ($i + 1) + '.txt'; $allLines[$start..$end] | Out-File -FilePath $outFile -Encoding UTF8 }"

echo 완료! allcodes_1.txt 부터 allcodes_6.txt까지 생성되었습니다.
pause