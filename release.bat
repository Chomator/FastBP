set RELEASEDIR=.\release
mkdir %RELEASEDIR%\plugins\x32
mkdir %RELEASEDIR%\plugins\x64
copy bin\x32\FastBP.dp32 %RELEASEDIR%\plugins\x32\
copy FastBP.json %RELEASEDIR%\plugins\x32\
copy bin\x64\FastBP.dp64 %RELEASEDIR%\plugins\x64\
copy FastBP.json %RELEASEDIR%\plugins\x64\