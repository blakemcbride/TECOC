@echo off
if "%1"=="" goto usage

echo tecoc mung squ %1.tec=%1.tes/d:n/l:y/b:y/t:y/c:y/w:n/a:y/e:n
tecoc mung squ %1.tec=%1.tes/d:n/l:y/b:y/t:y/c:y/w:n/a:y/e:n
goto fini

:usage
echo : usage:  squ filespec
echo :
echo : squ.bat will run the squ.tec "squish" macro on the teco macro
echo : source file filespec.tes, and produce a filespec.tec squished
echo : file.  the following squish switches are used:
echo :
echo :   /d:n  don't simply delete unnecessary CR/LF's...
echo :   /l:y  ...delete CR/LF's then go back & add them for 70 char lines
echo :   /b:y  delete blank lines
echo :   /t:y  delete unnecessary tabs and formfeeds
echo :   /c:y  delete comments !(SP)...! and !(TAB)...!
echo :   /w:n  don't watch progress
echo :   /a:y  automatic mode, don't subsquish @^U%% macros
echo :   /e:n  don't allow adjacent escapes
echo :

:fini
exit
