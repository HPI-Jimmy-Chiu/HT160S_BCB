@echo off
chcp 65001  // 设置为UTF-8编码
setlocal

set zPath=C:\Program Files\7-Zip\7z.exe
set password=BVL-3766

if "%~1"=="" (
    echo 請將要壓縮的資料夾拖曳到此批次檔上
    pause
    goto :eof
)
"%zPath%" a -t7z "%~n1.7z" "%~1" -p%password% -mhe

if %errorlevel% equ 0 (
    echo 解壓縮成功
) else (
    echo 解壓縮失敗
)
endlocal