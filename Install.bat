@ECHO OFF

TASKKILL /IM "RunCat Clone.exe" /F
COPY /Y "RunCat Clone.exe" "%AppData%\Microsoft\Windows\Start Menu\Programs\Startup"
START "" "%AppData%\Microsoft\Windows\Start Menu\Programs\Startup\RunCat Clone.exe"
