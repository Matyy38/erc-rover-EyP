@echo off
rem ============================================================================
rem  subir-cambios.bat - Sube a GitHub (rama main) todo lo que cambiaste aca.
rem
rem  Doble clic y listo: commit, PR, merge y borrado de la rama temporal.
rem  Uso por consola:  subir-cambios.bat [/s]    (/s = sin confirmar ni pausar)
rem
rem  Solo sirve para quien tiene permiso de escritura en el repo (Matias).
rem  No cambia de rama: si la carpeta no esta en main, se detiene.
rem ============================================================================
chcp 65001 >nul
setlocal EnableDelayedExpansion
title Subir cambios a GitHub
cd /d "%~dp0"

set "SILENCIO="
if /i "%~1"=="/s" set "SILENCIO=1"

where git >nul 2>nul || (echo [ERROR] No encuentro git instalado. & goto :fin_error)
where gh  >nul 2>nul || (echo [ERROR] No encuentro gh ^(GitHub CLI^) instalado. & goto :fin_error)

rem --- 1. Tiene que estar en main --------------------------------------------
set "RAMA="
for /f "delims=" %%b in ('git branch --show-current') do set "RAMA=%%b"
if /i not "!RAMA!"=="main" (
    echo [ERROR] Esta carpeta esta en la rama "!RAMA!", no en main.
    echo         No cambio de rama solo: puede haber un chat de Claude usandola.
    goto :fin_error
)

rem --- 2. Traer lo ultimo de GitHub ------------------------------------------
echo Trayendo lo ultimo de GitHub...
git fetch -q origin || (echo [ERROR] No pude conectarme a GitHub. & goto :fin_error)
git pull -q --ff-only origin main
if errorlevel 1 (
    echo.
    echo [ERROR] No pude traer main. Lo mas probable: alguien cambio en GitHub
    echo         el mismo archivo que editaste vos. No subi nada. Pedile a Claude
    echo         que lo resuelva.
    goto :fin_error
)

rem --- 3. Hay algo para subir? -----------------------------------------------
set "HAY_CAMBIOS="
for /f "delims=" %%l in ('git status --porcelain') do set "HAY_CAMBIOS=1"
set "ADELANTE=0"
for /f %%n in ('git rev-list --count origin/main..HEAD') do set "ADELANTE=%%n"
if not defined HAY_CAMBIOS if "!ADELANTE!"=="0" (
    echo.
    echo No hay cambios para subir. Todo esta igual que en GitHub.
    goto :fin_ok
)

echo.
echo Cambios a subir:
git -c core.quotepath=false status --short
if not "!ADELANTE!"=="0" echo   ^(mas !ADELANTE! commit^(s^) de una subida anterior que no llego a GitHub^)

rem Archivo de Office abierto = puede haber cambios sin guardar
set "ABIERTOS="
for /f "delims=" %%f in ('git -c core.quotepath^=false ls-files --others --ignored --exclude-standard --directory ^| findstr /c:"~$"') do (
    set "ABIERTOS=1"
    echo   [OJO] Abierto en Office: %%f
)
if defined ABIERTOS (
    echo   [OJO] Lo que no hayas guardado en esos archivos NO se sube.
)

if not defined SILENCIO (
    echo.
    choice /c SN /m "Subir estos cambios a GitHub"
    if errorlevel 2 (
        echo Cancelado. No se subio nada.
        goto :fin_ok
    )
)

rem --- 4. Commit ---------------------------------------------------------------
set "TS="
for /f %%t in ('powershell -NoProfile -Command "Get-Date -Format yyyyMMdd-HHmmss"') do set "TS=%%t"
set "FECHA="
for /f "delims=" %%t in ('powershell -NoProfile -Command "Get-Date -Format 'yyyy-MM-dd HH:mm'"') do set "FECHA=%%t"
set "RAMA_PR=edicion-rapida/!TS!"
set "MSG=%TEMP%\subir-cambios-!TS!.txt"

if defined HAY_CAMBIOS (
    git add -A || (echo [ERROR] Fallo git add. & goto :fin_error)
    > "!MSG!" echo Edicion rapida !FECHA!
    >>"!MSG!" echo.
    for /f "delims=" %%f in ('git -c core.quotepath^=false diff --cached --name-only') do >>"!MSG!" echo - %%f
    git commit -q -F "!MSG!" || (echo [ERROR] Fallo el commit. & goto :fin_error)
)
> "!MSG!" echo Subido con subir-cambios.bat.
>>"!MSG!" echo.
for /f "delims=" %%f in ('git -c core.quotepath^=false diff --name-only origin/main HEAD') do >>"!MSG!" echo - %%f

rem --- 5. Rama temporal, PR y merge ------------------------------------------
echo.
echo Subiendo...
git push -q origin HEAD:refs/heads/!RAMA_PR!
if errorlevel 1 (
    echo [ERROR] No pude subir. Tus cambios quedaron guardados en un commit local;
    echo         volve a correr este archivo y los reintenta.
    goto :fin_error
)

set "URL="
for /f "delims=" %%u in ('gh pr create --base main --head "!RAMA_PR!" --title "Edicion rapida !FECHA!" --body-file "!MSG!" 2^>nul') do set "URL=%%u"
if not defined URL (
    echo [ERROR] Se subio la rama !RAMA_PR! pero no pude abrir el PR.
    echo         Pedile a Claude que lo termine.
    goto :fin_error
)

gh pr merge "!RAMA_PR!" --merge >nul
if errorlevel 1 (
    echo [ERROR] PR abierto pero no se pudo mergear: !URL!
    echo         Pedile a Claude que lo resuelva.
    goto :fin_error
)

git push -q origin --delete "!RAMA_PR!" >nul 2>nul
git pull -q --ff-only origin main >nul 2>nul
del "!MSG!" >nul 2>nul

echo.
echo ============================================================
echo  LISTO. Ya esta en main de GitHub.
echo  !URL!
echo ============================================================
goto :fin_ok

:fin_error
echo.
if not defined SILENCIO pause
exit /b 1

:fin_ok
echo.
if not defined SILENCIO pause
exit /b 0
