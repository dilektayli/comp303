#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <time.h>

typedef char* string;

int main(int argc, string* argv){
	if(argc < 5){
		perror("Not enough parameters\n");
		return 1;
	}

    system("rm -rf output.txt");

    clock_t t = clock();

	const string path = "shared.txt";

	int status = 0;
	int arrayLength = atoi(argv[2]);
	pid_t processes[arrayLength];

	for(int i = 0; i < arrayLength; i++){
		processes[i] = fork();
		if(processes[i] < 0){
			perror("Fork Failed\n");
			return 1;
		}else if(processes[i] == 0){
			//child

			execlp("./psearch3slave", "psearch3slave", argv[1], argv[3 + i]);

			exit(0);
		}
        while (wait(&status) > 0);
	}

	//parent

	int sharedFile = open(path, O_RDONLY, S_IRWXU);
    if (sharedFile == -1) {
        perror("Parent File Open Error!!");
        exit(EXIT_FAILURE);
    }

    struct stat fstatus;
    fstat(sharedFile, &fstatus);
    off_t statusSize = fstatus.st_size;

    string finalOutput = (string) mmap(0, statusSize , PROT_READ, MAP_SHARED, sharedFile, 0);

    if (finalOutput == MAP_FAILED) {
        perror("Map Failed while mapping final output!");
    }


    FILE* outputFile = fopen(argv[argc - 1], "a+");

    if (outputFile == NULL) {
        perror("Error occurred when writing file!");
        exit(0);
    }
    fprintf(outputFile, "%s", finalOutput);
    fclose(outputFile);

    if (munmap(finalOutput, strlen(finalOutput)) == -1) {
        perror("Parent Error while removing mapping the file!");
    }
    close(sharedFile);

    remove(path);
    
    t = clock() - t;
	printf("Time: %f seconds to execute \n", ((double)t)/CLOCKS_PER_SEC);
	return 0;
}


