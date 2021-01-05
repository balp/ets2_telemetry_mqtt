mkdir cmake-build-local
cd cmake-build-local
cmake ..
cmake --build .
cd ..
mkdir "C:\Program Files (x86)\Steam\steamapps\common\American Truck Simulator\bin\win_x64\plugins"
mkdir "C:\Program Files (x86)\Steam\steamapps\common\Euro Truck Simulator 2\bin\win_x64\plugins"
copy cmake-build-debug\ets2telemery.dll "C:\Program Files (x86)\Steam\steamapps\common\American Truck Simulator\bin\win_x64\plugins\"
copy cmake-build-debug\hello_ets2.dll "C:\Program Files (x86)\Steam\steamapps\common\American Truck Simulator\bin\win_x64\plugins\"
copy cmake-build-release\ets2telemery.dll "C:\Program Files (x86)\Steam\steamapps\common\Euro Truck Simulator 2\bin\win_x64\plugins\"
copy cmake-build-release\hello_ets2.dll "C:\Program Files (x86)\Steam\steamapps\common\Euro Truck Simulator 2\bin\win_x64\plugins\"
copy cmake-build-release\telemetry.dll "C:\Program Files (x86)\Steam\steamapps\common\Euro Truck Simulator 2\bin\win_x64\plugins\"

