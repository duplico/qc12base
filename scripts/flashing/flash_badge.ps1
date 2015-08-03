Param(
  [string]$id,
  [string]$workspace,
  [switch]$f,
  [switch]$nf
)

if (-not $f) {

    echo "// Generated file." | Out-File -FilePath "badgeconf.h" -Encoding utf8
    echo "#define BADGE_ID ${id}" | Out-File -FilePath "badgeconf.h" -Encoding utf8 -Append

    cd img

    Invoke-Expression "python ..\scripts\image_composition\compose.py frames.ini play.ini -f -i ${id}" | Out-File -FilePath ..\generated_images.h -Encoding utf8

    cd ..

    Invoke-Expression ".\scripts\build.bat ${workspace}"

    cmd /C eclipsec -noSplash -data "${workspace}" -application com.ti.ccstudio.apps.projectBuild -ccs.projects qc12

    Copy-Item Debug\qc12.txt "images\${id}.txt"

}

if (-not $nf) {
    Invoke-Expression "msp430flasher.exe -n msp430fr5949 -m SBW2 -w images\${id}.txt -v"
}