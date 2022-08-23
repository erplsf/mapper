# mapper

0. Download the export from Strava -> Settings -> Account -> Export or delete Account.
1. `mkdir build && cd build`
2. `conan install ../conanfile.txt`
3. `cmake -S ../ && make`
4. `./bin/mapper export_XXX.zip > doc.kml`
5. Import the doc into a new layer on the Google Earth.
