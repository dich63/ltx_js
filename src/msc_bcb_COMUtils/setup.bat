rem COMMON_CPP_UTILS=V:\msc_bcb_COMUtils
set op=/s
set op=%1
set ap=%~dp0
@echo %ap%
setx.exe COMMON_CPP_UTILS    %ap%
