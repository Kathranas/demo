if exist build rd /s /q build
mkdir build
pushd build
cmake ..\ -G "NMake Makefiles"
move compile_commands.json ..\
popd
