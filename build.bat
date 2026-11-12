@echo off
SET LINKLIBS=kernel32.lib user32.lib gdi32.lib Comdlg32.lib Winspool.lib Httpapi.lib
SET LFLAGS=%LINKLIBS% /incremental:no /opt:icf /opt:ref  /DEBUG:FULL
SET CFLAGS= /Zi /Oi /nologo /W3 /GS- /Gs999999999
SET SRC=win32.c
SET OBJS=win32.obj
cl %SRC% %CFLAGS% /c
cl %OBJS% /nologo /link %LFLAGS%
del %OBJS%