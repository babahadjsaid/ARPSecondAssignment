echo "Compiling the cmd program";
g++ src/processA.cpp ./src/Myutilities.cpp -lncurses -lrt -lbmp -lm -o bin/ProcessA
echo "Compiling the cmd program";
g++ src/processB.cpp ./src/Myutilities.cpp -lncurses -lrt -lbmp -lm -o bin/ProcessB
echo "Compiling the main program";
g++ src/master.cpp ./src/Myutilities.cpp  -o bin/master
echo "Runing the main program ";
./bin/master
