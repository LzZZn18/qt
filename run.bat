@echo off
chcp 65001 > nul
echo ========================================
echo    Talk文件夹监控程序
echo ========================================
echo.

REM 检查Python
where python >nul 2>nul
if %errorlevel% neq 0 (
    echo ❌ 未找到Python，请先安装Python 3.9+
    echo 下载地址: https://www.python.org/downloads/
    pause
    exit /b 1
)

REM 检查FFmpeg
where ffmpeg >nul 2>nul
if %errorlevel% neq 0 (
    echo ⚠️  未找到FFmpeg，可能会影响音频转换
    echo 请从 https://ffmpeg.org/download.html 下载
    echo 或使用: choco install ffmpeg
    echo.
    timeout /t 3 /nobreak >nul
)

REM 检查依赖
if not exist "venv" (
    echo ⏳ 正在安装依赖...
    python -m venv venv
    call venv\Scripts\activate.bat
    pip install -r requirements.txt
) else (
    call venv\Scripts\activate.bat
)

REM 运行主程序
echo.
echo 🚀 启动监控程序...
echo 📁 监控文件夹: %USERPROFILE%\Desktop\talk
echo 🛑 按 Ctrl+C 停止程序
echo ========================================
echo.
python talk_processor.py

pause