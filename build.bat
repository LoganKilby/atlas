@echo off

ctime -begin timing.ctm

set opts=-FC -GR- -EHa- -nologo -Zi -DASSERTIONS_ENABLED
set code=%cd%

pushd build
cl %opts% %code%\atlas.cpp -Featlas.exe
set last_error=%ERRORLEVEL%
popd

ctime -end timing.ctm %last_error%