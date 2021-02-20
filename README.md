# MPI_Beleg
Project for my parallel systems module

## Info
Before running the program, make sure the lib-folder exists with the fitting files (folder is **NOT** included in the repository). You also need to install mpi on your device. 
<br>
Before running anything, start with following commands:
```
chmod +x prepare_scripts.sh
dos2unix prepare_scripts.sh
prepare_scripts.sh
```
<br><br>

## Compile
Compile with<br>
```
mpicc -omain src/main.c src/list.c src/functions.c -lm; mv main bin
```
or run <br>
```
./compile.sh
```
For Debug, compile with<br>
```
mpicc -omain src/main.c src/list.c src/functions.c -lm -DDEBUG; mv main bin
```
or run <br>
```
./compile.sh DEBUG
```
<br>
Compiling is possible with more options. Giving "DEBUG" and another argument (e.g. TEST_MACRO_CALC) will give more Output. Check main.h for more info!
<br><br>

## Run
Run with<br>
```
mpiexec -f machinefile -n <amount_processes> ./bin/main <distanze_motion_vector_search> <picture1> <picture2>
```
Run with test-files<br>
```
mpiexec -f machinefile -n <amount_processes> ./bin/main 5 files/test_pictures/paint1.jpg files/test_pictures/paint2.jpg
```
or use (giving no picture args uses the test pictures)<br>
```
./run.sh <amount_processes> <distanze_motion_vector_search> <picture1> <picture2>
```
<br>
Note that the argument "amount_processes" will only be used for running the mpiexec-command and is not an argument of the programm.

## Test
The script test_programm.sh runs the programm several times with the same amount of motion-vectors, but different amount of processors. Run it with<br>
```
./test_programm.sh <amount_processes> <distanze_motion_vector_search>
```

## Command line args
* arg 1: Defines the distance around each macro-block, in which the possible motion vector could be. Note that it will take more time, the bigger the number is. Only give int-values!
* arg 2: Defines the references picture, which will be compared to all other pictures.
* arg 3: Defines the first picture, which will be compared to the reference picture given with arg 2.
* arg 4-...: Defines more pictures to compare, which are optional. Note that it will take longer time to execute with more pictures given.