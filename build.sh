#cmake -S . -B build
#cmake --build build

cd src || exit
make -f Makefile.emscripten
cd ..
