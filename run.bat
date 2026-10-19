@echo off
pushd "%~dp0Exercise_1"
call build_and_flash.bat %*
popd
pause


