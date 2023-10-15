@echo off
set m_TextureScriberPath=TextureScriberPC64.exe

IF NOT EXIST %m_TextureScriberPath% (
    echo [ ERROR ] '%m_TextureScriberPath%' is missing.
    goto G_END
)

:: Check for param
IF "%~1"=="" (
    echo [ ERROR ] You need to specify directory to scribe!
    goto G_END
)

:: Setup stuff
set m_ModelsPath=Models\%1
set m_TexturesPath=Textures\%1

set m_DiffuseCompression="DXT5"
set m_NormalCompression="DXT1_N"

set m_DiffuseFilename=Diffuse.png
set m_NormalFilename=Normal.png

IF EXIST %m_TexturesPath%\%m_DiffuseFilename% (
    set m_DiffuseFilePath=%m_TexturesPath%\%m_DiffuseFilename%
) else (
    set m_DiffuseFilePath=%m_ModelsPath%\%m_DiffuseFilename%
)

IF EXIST %m_TexturesPath%\%m_NormalFilename% (
    set m_NormalFilePath=%m_TexturesPath%\%m_NormalFilename%
) else (
    set m_NormalFilePath=%m_ModelsPath%\%m_NormalFilename%
)

:: Scribe path
IF "%~2"=="" (
    set m_ScribePath=Data
) else (
    set m_ScribePath=Data\%2
)

:: Texture Scriber config
set m_ConfigPath=config.xml
echo ^<MediaPack^> > %m_ConfigPath%
echo ^<Resource OutputName="%1_D" Compression=%m_DiffuseCompression%^>%m_DiffuseFilePath%^</Resource^> >> %m_ConfigPath%

:: Setup additional textures
IF EXIST %m_NormalFilePath% (
    echo ^<Resource OutputName="%1_N" Compression=%m_NormalCompression%^>%m_NormalFilePath%^</Resource^> >> %m_ConfigPath%
)
echo ^</MediaPack^> >> %m_ConfigPath%

:: Process
TextureScriberPC64.exe PC64 "%m_ScribePath%\%1_TS001.perm.bin" "%m_ScribePath%\%1_TS001.temp.bin" "%m_ConfigPath%"
del %m_ConfigPath%

:G_END