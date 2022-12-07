#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

typedef char* string;

int readLine(string* pointer, FILE* fp);
int checkIfExists(string line, string word);

int main(int argc, string* argv){
    if(argc < 3){
		perror("Not enough parameters\n");
		return 1;
	}

    const string path = "shared.txt";

    int file;

    if((file = open(path, O_CREAT | O_RDWR, S_IRWXU)) == -1){
        perror("Child File open error!");
    }

    FILE* fp = fopen(argv[2], "r");
    string output = NULL;
    string pointer = NULL;
    int numberOfLine = 1;

    while(readLine(&pointer, fp) != NULL){
        string line;

        if((line = (string) malloc(strlen(pointer) + 1)) == NULL){
            perror("Malloc could not allocate memory\n");
            return 1;
        }
        
        strcpy(line, pointer);

        int boolean = checkIfExists(line, argv[1]);
        if(boolean){
            int size = strlen(pointer) + strlen(argv[2]) + 50;
            if(output == NULL){
                if((output = (string) malloc(size)) == NULL){
                    perror("Malloc could not allocate memory\n");
                    return 1;
                }
                sprintf(output, "<%s>, <%i>: <%s>\n", argv[2], numberOfLine, pointer);   
            }else{
                if(((output) = (string) realloc(output, strlen(output) + size)) == NULL){
				    perror("Realloc could not reallocate memory\n");
				    free(pointer);
				    return 1;
			    }
                char variable[size];
                sprintf(variable, "<%s>, <%i>: <%s>\n", argv[2], numberOfLine, pointer);   
                strcat(output, variable);	
            }
        }
        free(line);
        numberOfLine++;
    }

    struct stat fstatus;
    fstat(file, &fstatus);

    fallocate(file, 0, fstatus.st_size, strlen(output));

    string finalOutput = mmap(0, fstatus.st_size + strlen(output), PROT_WRITE, MAP_SHARED, file, 0);

    if(finalOutput == MAP_FAILED){
        perror("Map failed while mapping!\n");
    }

    for(int i = 0; i < strlen(output); i++){
        finalOutput[fstatus.st_size + i] = output[i];
    }

    if(munmap(finalOutput, strlen(finalOutput)) == -1){
        perror("CHild Error while removing mapping the file!");
    }

    free(output);
    free(pointer);
    fclose(fp);
    close(file);
    return 0;
}




int checkIfExists(string line, string target){
	
	for (string word = strtok(line, " "); word && *word; word = strtok(NULL, " \n")){
		if(strlen(word) == strlen(target)){
			if(strcmp(word, target) == 0){
				return 1;
			}
		}
    };
	return 0;
}

int readLine(string* pointer, FILE* fp){
	char line[128];
	int size = sizeof(line);
	if(*pointer == NULL){
		if(((*pointer) = (string) malloc(size)) == NULL){
			perror("Malloc could not allocate memory\n");
			return 1;
		}
	}
	
	(*pointer)[0] = '\0';
	while(fgets(line, sizeof(line), fp) != NULL){

		if(size - strlen(*pointer) < sizeof(line)){
			size *= 2;	
			if(((*pointer) = (string) realloc(*pointer, size)) == NULL){
				perror("Realloc could not reallocate memory\n");
				free(pointer);
				return 1;
			}	
		}

		strcat(*pointer, line);

		if((*pointer)[strlen(*pointer) - 1] == '\n'){
			(*pointer)[strlen(*pointer) - 1] = '\0';
			return (*pointer);
		}
	}
	return NULL;
}