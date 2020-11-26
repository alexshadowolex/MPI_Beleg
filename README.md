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
./bin/main \<picture1> \<picture2><br>
Run with test-files<br>
./bin/main files/test_pictures/paint1.jpg files/test_pictures/paint2.jpg<br>
or use ./run.sh \<picture1> \<picture2> (giving no args uses the test pictures)