@echo off
set m_TextureScriberPath=TextureScriberPC64.exe
set m_ModelScriberPath=ModelScriber.exe
IF NOT EXIST %m_TextureScriberPath% (
    echo [ ERROR ] '%m_TextureScriberPath%' is missing.
    goto G_END
)

IF NOT EXIST %m_ModelScriberPath% (
    echo [ ERROR ] '%m_ModelScriberPath%' is missing.
    goto G_END
)

IF "%~1"=="" (
    echo [ ERROR ] Model name is missing.
    goto G_USAGEHELP
)
IF "%~2"=="" (
    echo [ ERROR ] Texture name is missing.
    goto G_USAGEHELP
)

set m_ScribePath=Data
IF NOT "%~3"=="" (
    set m_ScribePath=%m_ScribePath%\%3
)

set m_ProjectPath=Models
set m_ModelPath=%m_ProjectPath%/%1/Model.obj
set m_TexturePath=%m_ProjectPath%/%1/%2.png
set m_ConfigPath=%m_ProjectPath%/%1/config.xml

IF NOT EXIST %m_ModelPath% (
    echo [ ERROR ] Model '%m_ModelPath%' doesn't exist.
    goto G_END
)

IF NOT EXIST %m_TexturePath% (
    echo [ ERROR ] Texture '%m_TexturePath%' doesn't exist.
    goto G_END
)
goto G_BEGIN

:G_USAGEHELP
echo USAGE: %0 [Model name] [Texture name]
pause
goto G_END

:G_BEGIN
echo ^<MediaPack^>^<Resource^>%m_TexturePath%^</Resource^>^</MediaPack^> > %m_ConfigPath%
TextureScriberPC64.exe PC64 "%m_ScribePath%\%1_TS001.perm.bin" "%m_ScribePath%\%1_TS001.temp.bin" "%m_ConfigPath%"
ModelScriber.exe -o "%m_ModelPath%" -d "%m_ScribePath%" -n "%1" -t "%2"

:G_END