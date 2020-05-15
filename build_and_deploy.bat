subst T: .
msbuild TrafficMapper.sln /property:Configuration=Release
"C:\Frameworks\Qt\5.14.1\msvc2017_64\bin\windeployqt.exe" "T:\Binary\TrafficMapper.exe" --qmldir "T:\Resources\qml"
xcopy /y settings.ini Binary