@ECHO OFF
chcp 65001
cls
@SETLOCAL

echo "========先にREADME.mdを参照し、ビルド環境を準備してください========"
echo.

echo "========ビルドオプション========"
echo "注意：このプロジェクトでは、デフォルトでRelease版のライブラリを使用します。"
echo "Debug版のOnnxruntimeとOpenCVを自分でビルドしていない場合は、Debugビルドを選択しないでください。"

@REM echo "ビルドオプションを入力してEnterキーを押してください：1) Release、2) Debug"
@REM set /p flag=

set BUILD_TYPE=Release
set flag=1

if %flag% == 1 (
    set BUILD_TYPE=Release
)^
else if %flag% == 2 (
    set BUILD_TYPE=Debug
)^
else (
    echo 入力エラー！Input Error!
)

echo.

@REM echo "注意：2) JNIダイナミックライブラリを選択する場合は、Oracle JDKをインストールして設定する必要があります"
@REM echo "ビルド出力形式を選択してEnterキーを押してください：1) BIN実行ファイル、2) JNIダイナミックライブラリ、3) Cダイナミックライブラリ"
@REM set /p flag=

set flag=1

if %flag% == 1 (
    set BUILD_OUTPUT="BIN"
)^
else if %flag% == 2 (
    set BUILD_OUTPUT="JNI"
)^
else if %flag% == 3 (
    set BUILD_OUTPUT="CLIB"
)^
else (
    echo 入力エラー！Input Error!
)

echo.

@REM echo "使用するライブラリ形式：1) 静的CRT（mt）、2) 動的CRT（md）"
@REM echo "注意：サンプルプロジェクトには、デフォルトでmt版ライブラリが組み込まれています"
@REM set /p flag=

set flag=1

if %flag% == 1 (
    set MT_ENABLED="True"
)^
else (
    set MT_ENABLED="False"
)

echo.

@REM echo "onnxruntime：1) CPU（デフォルト）、2) GPU（CUDA）"
@REM echo "注意：サンプルプロジェクトには、デフォルトでCPU版が組み込まれています。CUDA版はx64のみ対応しており、別途ダウンロードが必要です"
@REM set /p flag=

set flag=1

if %flag% == 1 (
    set ONNX_TYPE="CPU"
)^
else if %flag% == 2 (
    set ONNX_TYPE="CUDA"
)^
else (
    echo 入力エラー！Input Error!
)

echo.

@REM echo "オプションを入力してEnterキーを押してください：0) すべて、1) vs2019-x86、2) vs2019-x64"
@REM set /p flag=

set flag=2

if %flag% == 0 (
    call :buildALL
)^
else if %flag% == 1 (
    call :gen2022-x86
)^
else if %flag% == 2 (
    call :gen2022-x64
)^
else (
    echo "入力エラー！Input Error!"
)

GOTO:EOF

:buildALL
call :gen2022-x86
call :gen2022-x64
GOTO:EOF

:gen2022-x86
mkdir build-win-vs2022-x86
pushd build-win-vs2022-x86
call :cmakeParams "Visual Studio 17 2022" "Win32"
popd
GOTO:EOF

:gen2022-x64
mkdir build-win-vs2022-x64
pushd build-win-vs2022-x64
call :cmakeParams "Visual Studio 17 2022" "x64"
popd
GOTO:EOF

:cmakeParams
echo cmake -G "%~1" -A "%~2" -DOCR_OUTPUT=%BUILD_OUTPUT% -DOCR_BUILD_CRT=%MT_ENABLED% -DOCR_ONNX=%ONNX_TYPE% ..
cmake -G "%~1" -A "%~2" -DOCR_OUTPUT=%BUILD_OUTPUT% -DOCR_BUILD_CRT=%MT_ENABLED% -DOCR_ONNX=%ONNX_TYPE% ..
GOTO:EOF

@ENDLOCAL
