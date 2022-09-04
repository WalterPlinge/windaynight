@echo off
call cl.exe windaynight.c /nologo /W3 /WX /O1 /GS- /link /fixed /incremental:no /opt:icf /opt:ref /subsystem:console kernel32.lib advapi32.lib shell32.lib
