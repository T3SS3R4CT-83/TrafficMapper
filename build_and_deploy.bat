msbuild TrafficMapper.sln /property:Configuration=Release
"3rd-party\qt\bin\windeployqt.exe" "binary\TrafficMapper.exe" --qmldir "resources\qml"
xcopy /y /d  "3rd-party\opencv\x64\vc16\bin\opencv_world440.dll" "binary"
