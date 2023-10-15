@echo off
set m_ModelScriberPath=ModelScriber.exe

IF NOT EXIST %m_ModelScriberPath% (
    echo [ ERROR ] '%m_ModelScriberPath%' is missing.
    goto G_END
)

:: Check for param
IF "%~1"=="" (
    echo [ ERROR ] You need to specify directory of the model to scribe!
    goto G_END
)

IF "%~2"=="" (
    echo [ ERROR ] You need to specify texture name!
    goto G_END
)

:: Check model path
set m_ModelPath=Models\%1\Model.obj
IF NOT EXIST %m_ModelPath% (
    echo [ ERROR ] Model '%m_ModelPath%' doesn't exist.
    goto G_END
)

:: Model Scriber launch options
set m_ModelScriberArgs=-obj "%m_ModelPath%" -dir "Data" -name "%1" -texdiffuse "%2_D"

:: Use normal map?
IF "%~3"=="1" (
    set m_ModelScriberArgs=%m_ModelScriberArgs% -texnormal "%2_N"
)

:: Use specular map?
IF "%~4"=="1" (
    set m_ModelScriberArgs=%m_ModelScriberArgs% -texspecular "%2_S"
)

:: Process
mkdir Data
%m_ModelScriberPath% %m_ModelScriberArgs% -rasterstate 5

:G_END