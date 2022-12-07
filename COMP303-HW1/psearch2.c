#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef char* string;

int readLine(string* pointer, FILE* fp);
int checkIfExists(string line, string word);

int main(int argc, string* argv){
	if(argc < 5){
		perror("Not enough parameters\n");
		return 1;
	}

	system("rm -rf output.txt");

	clock_t t = clock();

	int arrayLength = atoi(argv[2]);
	pid_t processes[arrayLength];
	int fd[arrayLength][2];

	for(int i = 0; i < arrayLength; i++){
		if(pipe(fd[i]) == -1){
			perror("An error occured with pipe\n");
			return 1;
		}
		processes[i] = fork();
		if(processes[i] < 0){
			perror("Fork Failed\n");
			return 1;
		}else if(processes[i] == 0){
			//child
			close(fd[i][0]);
			FILE* input;
			string pointer = NULL;
			int size = 0;
			int numberOfLine = 1;

			if((input = fopen(argv[3 + i], "r")) == NULL){
				printf("%s ", argv[3 + i]);
				perror("File could not be open\n");
				return 1;
			}

			while(readLine(&pointer, input) != NULL){
				string line;

				if((line = (string) malloc(strlen(pointer) + 1)) == NULL){
					perror("Malloc could not allocate memory\n");
					return 1;
				}
				
				strcpy(line, pointer);

				int boolean = checkIfExists(line, argv[1]);

				if(boolean){
					int sizeOfString = strlen(pointer) * sizeof(char);
					write(fd[i][1], &sizeOfString, sizeof(int));
					write(fd[i][1], pointer, sizeOfString);
					write(fd[i][1], &numberOfLine, sizeof(int));
				}
				free(line);
				numberOfLine++;
			}

			close(fd[i][1]);
			free(pointer);
			fclose(input);
			exit(0);
		}
	}

	//parent
	wait(NULL);
	FILE* output;
	if((output = fopen(argv[argc - 1], "a+")) == NULL){
		printf("%s ", argv[argc - 1]);
		perror("File could not be open\n");
		return 1;
	}
	for(int i = 0; i < arrayLength; i++){
		close(fd[i][1]);
			
		int size;
		int numberOfLine;
		while(read(fd[i][0], &size, sizeof(int)) > 0){
			string pointer;
			if((pointer = (string) malloc(size * sizeof(char))) == NULL){
				perror("Malloc could not allocate memory\n");
				return 1;
			}
			read(fd[i][0], pointer, size);
			read(fd[i][0], &numberOfLine, sizeof(int));
			fprintf(output, "<%s>, <%i>: <%s>\n", argv[3 + i], numberOfLine, pointer);		
			free(pointer);	
		}

		close(fd[i][0]);
	}
	fclose(output);

	t = clock() - t;
	printf("Time: %f seconds to execute \n", ((double)t)/CLOCKS_PER_SEC);
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