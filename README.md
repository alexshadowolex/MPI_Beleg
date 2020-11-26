# MPI_Beleg
Project for my parallel systems module

## Info
Before running the program, make sure the lib-folder exists with the fitting files (folder is **NOT** included in the repository).

## Compile
Compile with<br>
gcc -omain src/main.c src/list.c src/functions.c -lm; mv main bin<br>
or run ./compile.sh<br>
For Debug, compile with<br>
gcc -omain src/main.c src/list.c src/functions.c -lm -DDEBUG; mv main bin<br>
or run ./compile.sh DEBUG<br>

## Run
Run with<br>
./bin/main \<distanze_motion_vector_seatch> \<picture1> \<picture2><br>
Run with test-files<br>
./bin/main 5 files/test_pictures/paint1.jpg files/test_pictures/paint2.jpg<br>
or use ./run.sh \<distanze_motion_vector_seatch> \<picture1> \<picture2> (giving no args uses the test pictures)


## Command line args
* arg 1: Defines the distance around each macro-block, in which the possible motion vector could be. Note that it will take more time, the bigger the number is. Only give int-values!
* arg 2: Defines the references picture, which will be compared to all other pictures.
* arg 3: Defines the first picture, which will be compared to the reference picture given with arg 2.
* arg 4-...: Defines more pictures to compare, which are optional. Note that it will take longer time to execute with more pictures given.