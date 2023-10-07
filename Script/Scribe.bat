@echo off
:: Set executable paths
set m_TextureScriberPath=TextureScriberPC64.exe
set m_ModelScriberPath=ModelScriber.exe

:: Check for executables
IF NOT EXIST %m_TextureScriberPath% (
    echo [ ERROR ] '%m_TextureScriberPath%' is missing.
    goto G_END
)

IF NOT EXIST %m_ModelScriberPath% (
    echo [ ERROR ] '%m_ModelScriberPath%' is missing.
    goto G_END
)

:: Check for param
IF "%~1"=="" (
    echo [ ERROR ] You need to specify directory of the model to scribe!
    goto G_END
)

:: Setup paths
set m_ModelsPath=Models
set m_ScribePath=Data
IF NOT "%~2"=="" (
    set m_ScribePath=%m_ScribePath%\%2
)

set m_ModelTexturePrefix=%1

IF "%~3"=="" (
    set m_TexturesPath=%m_ModelsPath%\%1
) else (
    set m_ModelTexturePrefix=%3
    set m_TexturesPath=Textures\%3
)

set m_TextureScribeFilePath=%m_ScribePath%\%m_ModelTexturePrefix%_TS001
set m_TexturePath==%m_ProjectPath%\%1\

:: Setup stuff...
set m_ModelPath=%m_ProjectPath%\%1\Model.obj
set m_TextureDiffuseName=%m_ModelTexturePrefix%_Diffuse
set m_TextureDiffusePath=%m_TexturesPath%\Diffuse.png
set m_TextureNormalName=%m_ModelTexturePrefix%_Normal
set m_TextureNormalPath=%m_TexturesPath%\Normal.png

:: Check model & texture path
IF NOT EXIST %m_ModelPath% (
    echo [ ERROR ] Model '%m_ModelPath%' doesn't exist.
    goto G_END
)

IF NOT EXIST %m_TextureDiffusePath% (
    echo [ ERROR ] Texture diffuse '%m_TextureDiffusePath%' doesn't exist.
    goto G_END
)

:: Model Scriber launch options
set m_ModelScriberArgs=-obj "%m_ModelPath%" -dir "%m_ScribePath%" -name "%1" -texdiffuse "%m_TextureDiffuseName%"

:: Texture Scriber paths
set m_TextureDiffuseTempPath=Temp\%m_TextureDiffuseName%.png
set m_TextureNormalTempPath=Temp\%m_TextureNormalName%.png

:: Texture Scriber config
mkdir Temp
set m_ConfigPath=Temp\config.xml
echo ^<MediaPack^> > %m_ConfigPath%

copy %m_TextureDiffusePath% %m_TextureDiffuseTempPath% > NUL
echo ^<Resource^>%m_TextureDiffuseTempPath%^</Resource^> >> %m_ConfigPath%

:: Setup additional textures
IF EXIST %m_TextureNormalPath% (
    copy %m_TextureNormalPath% %m_TextureNormalTempPath% > NUL
    echo ^<Resource^>%m_TextureNormalTempPath%^</Resource^> >> %m_ConfigPath%
    set m_ModelScriberArgs=%m_ModelScriberArgs% -texnormal "%m_TextureNormalName%"
)

echo ^</MediaPack^> >> %m_ConfigPath%

:: Process
TextureScriberPC64.exe PC64 "%m_TextureScribeFilePath%.perm.bin" "%m_TextureScribeFilePath%.temp.bin" "%m_ConfigPath%"
ModelScriber.exe %m_ModelScriberArgs%

:: Delete temp stuff
rmdir /S /Q Temp

:G_END