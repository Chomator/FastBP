set RELEASEDIR=.\release
mkdir %RELEASEDIR%\plugins\x32
mkdir %RELEASEDIR%\plugins\x64


copy bin\x32\FastBP.dp32 %RELEASEDIR%\plugins\x32\
copy xlsx2json.py %RELEASEDIR%\plugins\x32\
copy example.xlsx %RELEASEDIR%\plugins\x32\

copy bin\x64\FastBP.dp64 %RELEASEDIR%\plugins\x64\
copy xlsx2json.py %RELEASEDIR%\plugins\x64\
copy example.xlsx %RELEASEDIR%\plugins\x64\