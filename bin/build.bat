@echo off
setlocal enabledelayedexpansion
REM ==============================
REM  编译 zhh_log.exe 的脚本
REM ==============================

REM 切换到当前脚本所在目录
cd ../

REM 确保输出目录存在
if not exist bin mkdir bin

REM 设置库文件路径
set LIB_PATH=-L../lib

REM 设置要链接的库（根据lib文件夹中的文件自动调整）
set LINK_LIBS=

REM 检查是否存在其他库文件并添加到链接参数
if exist lib\*.lib (
    echo find static library files, add to link parameters...
    for %%f in (lib\*.lib) do (
        set LINK_LIBS=!LINK_LIBS! lib\%%~nxf
        )
    )
    if exist lib\*.dll (
        echo find dynamic library files, copy to output directory...
        copy lib\*.dll bin\
    )

REM 调用 g++ 进行编译，链接lib文件夹中的库
g++ -fdiagnostics-color=always -g -DFMT_HEADER_ONLY src\*.cpp -o bin\zhh_log.exe

REM 输出结果提示
if %errorlevel% neq 0 (
    echo build failed!
    exit /b 1
) else (
    echo build success! generate file: bin\zhh_log.exe
)
pause