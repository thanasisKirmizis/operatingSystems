#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

//Declarations
void loop_until_quit(char *argv[]);
int check_delimiter(char *input);
char **split_input_to_commands(char *input, char delim[]);
int check_redirect(char *command);
int check_piping(char *command);
int find_position_of_redirect(char *command);
int find_position_of_piping(char *command);
char **split_command_to_args(char *command);
int execute(char **args, int redirect_type, int redirect_pos, int pipe_type, int pipe_pos);
int execute_pipe(char **args1, char **args2);

//Global Variables
char user_prompt[] = "user> ";
int buffer_size = 512;

//Main program
int main(int argc, char *argv[]){
	
	if(argc>2){
		printf("Too many arguments!\nExiting...\n");
	}
	else{
		loop_until_quit(argv);
	}
	
	exit(0);
    return 0;
}

void loop_until_quit(char *argv[]){
	
	int exec_status = 0;
	int opened_file_flag = 0;
	int command_number=1;
	FILE *fp = NULL;
	char *file_input = malloc(buffer_size*sizeof(char));
	
	//Attempt to open file
	if(argv[1]!=NULL){
		
		fp = fopen(argv[1], "r");
		
		if(fp==NULL){
			printf("Could not open batch file!\nExiting...\n");
			return;
		}
		else{
			printf("Opening batch file...\n");
			opened_file_flag = 1;
		}
	}

	//Loop for each line of input
	do{
		int delim_type;
		int redirect_type;
		int redirect_pos;
		int pipe_type;
		int pipe_pos;
		char delim1[] = ";";
		char delim2[] = "&&";
		char *input = malloc(buffer_size*sizeof(char));
		char *dummy1 = malloc(buffer_size*sizeof(char));
		char *dummy2 = malloc(buffer_size*sizeof(char));
		char *dummy3 = malloc(buffer_size*sizeof(char));
		char **commands;
		char **args;
		
		//Prompt
		printf("\n%s", user_prompt);
	    
		//Batch Mode: read from file
		if(opened_file_flag){
			
			file_input = fgets(file_input, buffer_size, fp);

			if(file_input != NULL){
				strcpy(input,file_input);
				printf("Reading line %d:\n",command_number++);
			}
			else{
				printf("EOF!\n");
				break;
			}
			
		}
		//Interactive Mode: read from keyboard
		else{
			input = fgets(input, buffer_size, stdin);
		}

		printf("\n");
		delim_type = check_delimiter(input);
		
		//Case 1: Delimiter ';'
		if (delim_type == 1){
			commands = split_input_to_commands(input, delim1);		
			
			int i = 0;
			while(commands[i]!= NULL){
				strcpy(dummy1, commands[i]);
				strcpy(dummy2, commands[i]);
				strcpy(dummy3, commands[i]);
				
				redirect_type = check_redirect(commands[i]);
				redirect_pos = find_position_of_redirect(dummy1);
				pipe_type = check_piping(dummy2);
				pipe_pos = find_position_of_piping(dummy3);
				
				args = split_command_to_args(commands[i]);
				exec_status = execute(args, redirect_type, redirect_pos, pipe_type, pipe_pos);
				
				i++;
				printf("\n------------------------------------------\n");
			}
				
		}
		
		//Case 2: Delimiter '&&'
		else if (delim_type == 2){
			commands = split_input_to_commands(input, delim2);
			
			int i=0;
			while(commands[i]!= NULL){
				strcpy(dummy1, commands[i]);
				strcpy(dummy2, commands[i]);
				strcpy(dummy3, commands[i]);
				
				redirect_type = check_redirect(commands[i]);
				redirect_pos = find_position_of_redirect(dummy1);
				pipe_type = check_piping(dummy2);
				pipe_pos = find_position_of_piping(dummy3);

				args = split_command_to_args(commands[i]);		
				exec_status = execute(args, redirect_type, redirect_pos, pipe_type, pipe_pos);
				
				i++;
				
				if(exec_status == 1){
					printf("\nERROR with && delimiter: Flow stopped!\n");
					break;
				}
				
				printf("\n------------------------------------------\n");
				
			}
			
		}
		//Case 3: No delimiter, only one command
		else{
			strcpy(dummy1, input);
			strcpy(dummy2, input);
			strcpy(dummy3, input);
			
			redirect_type = check_redirect(input);
			redirect_pos = find_position_of_redirect(dummy1);
			pipe_type = check_piping(dummy2);
			pipe_pos = find_position_of_piping(dummy3);
			
			args = split_command_to_args(input);		
			exec_status = execute(args, redirect_type, redirect_pos, pipe_type, pipe_pos);
			
			printf("\n------------------------------------------\n");

		}
		
		free(dummy1);
		free(dummy2);
		free(dummy3);
		free(input);
		
	}while(exec_status!=2);
	
	if(fp!=NULL){
		fclose(fp);
	}
	free(file_input);
	
	printf("Goodbye!\n");
	
	return;
}

//Checks which type of delimiter of commands is in the input, if at all
int check_delimiter(char *input){
	
	int type=0;
	
	if(strstr(input, ";") != NULL) {
		printf("Splitting commands with ';' delimiter:\n\n");
		type=1;
	}
	else if(strstr(input, "&&") != NULL) {
		printf("Splitting commands with '&&' delimiter:\n\n");
		type=2;
	}
	
	return type;
}

//Splits the input to commands separated with ; or &&
char **split_input_to_commands(char *input, char delim[]){
	
	char **commands = malloc(buffer_size*sizeof(char));
	int i=0;
	char *token;
   
   //Get the first token 
   token = strtok(input, delim);
   
   //Walk through other tokens 
   while( token != NULL ) {
	  commands[i] = token; 
	  i++;
   
      token = strtok(NULL, delim);
   }

	return commands;
}

//Checks which type of redirect is in a command, if at all
int check_redirect(char *command){
	
	int type=0;
	
	if(strstr(command, ">") != NULL) {
		printf("Redirecting Output...\n");
		type=1;
	}
	else if(strstr(command, "<") != NULL) {
		printf("Redirecting Input...\n\n");
		type=2;
	}
	
	return type;
}

//Checks if there is piping in a command 
int check_piping(char *command){
	
	int type=0;
	
	if(strstr(command, "|") != NULL) {
		printf("Piping Output...\n");
		type=1;
	}
	
	return type;
}

//Finds the position of redirect expression in the array of arguments 
int find_position_of_redirect(char *command){
	
	int i=0;
	int pos;
	char delim[] = " ";
	char *token;
	
	//Get the first token 
	token = strtok(command, delim);
      
	//Walk through other tokens and if token is > or <, return its position
	while( token != NULL ) {
		
		if(token[0] == '>' || token[0] == '<'){	
			pos = i;
			break;
		}
		i++;
		
		token = strtok(NULL, delim);
	}
	
	return pos;
	
}

//Finds the position of piping expression in the array of arguments
int find_position_of_piping(char *command){
	
	int i=0;
	int pos;
	char delim[] = " ";
	char *token;
	
	//Get the first token 
	token = strtok(command, delim);
      
	//Walk through other tokens and if token is > or <, return its position
	while( token != NULL ) {
		
		if(token[0] == '|'){	
			pos = i;
			break;
		}
		i++;
		
		token = strtok(NULL, delim);
	}
	
	return pos;
	
}

//Splits the command to arguments separated with whitespace
char **split_command_to_args(char *command){
	
	char **args = malloc(buffer_size*sizeof(char));
	int i=0;
	int pos;
	char *temp1 = strstr(command, ">");
	char *temp2 = strstr(command, "<");
	char *temp3 = strstr(command, "|");
	char delim[] = " \n\r";
	char *token;
	
	//First replace the redirect and piping expressions with whitespace
	if(temp1 != NULL) {
		pos = temp1 - command;
		command[pos] = ' ';
	}
	if(temp2 != NULL) {
		pos = temp2 - command;
		command[pos] = ' ';
	}
	if(temp3 != NULL) {
		pos = temp3 - command;
		command[pos] = ' ';
	}
	
	//Get the first token 
	token = strtok(command, delim);
  
	//Walk through other tokens 
	while( token != NULL ) {
		args[i] = token;	  
		i++;

		token = strtok(NULL, delim);
	}
	args[i]=NULL;
	
	return args;
}

//Executes one command, with or without redirection and piping
int execute(char **args, int redirect_type, int redirect_pos, int pipe_type, int pipe_pos){
	
	int exec_status = 0;
	pid_t  pid;
    int status;

	if(args[0]==NULL){
		printf("Empty command given... Try again.");
		exec_status = 1;
	}
	else if (strcmp(args[0], "quit") == 0){
		exec_status = 2;
	}
	else if(pipe_type == 1){
		
		//Split one array of strings into two separated by expression "|"		
		int i;
		char **args1 = malloc(buffer_size*sizeof(char));
		char **args2 = malloc(buffer_size*sizeof(char));
	
		for(i=0; i<pipe_pos; i++){
			args1[i] = strdup(args[i]);
		}
		args1[pipe_pos] = NULL;	
		
		i=pipe_pos;
		int j=0;
		while(args[i]!=NULL){
			args2[j] = strdup(args[i]);
			j++;
			i++;
		}
		args2[j] = NULL;
		
		//Simple execute of piped commands
		exec_status = execute_pipe(args1, args2);
			
		free(args1);
		free(args2);
			
	}
	else{
		
		//Duplicate process
		if ((pid = fork()) < 0) {     
			printf("ERROR: Forking child process failed\n");
			exec_status = 1;
			exit(1);
		}
		else if (pid == 0) {
			
			//////I AM THE SON//////
						
			//Redirect Ouput
			if(redirect_type == 1){
				
				int fdout = open(args[redirect_pos], O_CREAT|O_TRUNC|O_WRONLY, 0644);	
				if (fdout < 0){
					printf("ERROR: Open file failed\n");
					exit(1);
				}
				dup2(fdout, STDOUT_FILENO);
				
				close(fdout);
				
				//So that the redirected file will not pass as argument in execvp
				args[redirect_pos] = NULL;
				
			}
			//Redirect Input
			else if(redirect_type == 2){
				
				int fdin = open(args[redirect_pos], O_RDONLY);	
				if (fdin < 0){
					printf("ERROR: Open file failed\n");
					exit(1);
				}
				dup2(fdin, STDIN_FILENO);
				
				close(fdin);
				
				//So that the redirected file will not pass as argument in execvp
				args[redirect_pos] = NULL;
			}
			
			//Execute one command
			int x = execvp(args[0], args);
			if ( x < 0) {     
				printf("ERROR: Exec failed\n");
				exec_status = 1;
			}
			exit(1);
		}
		else {

			//////I AM THE FATHER//////
			
			while ((pid = wait(&status)) != -1)
			//Any value other than 0 means error
			if(WEXITSTATUS(status)){									
				printf("exit status = %d\n", WEXITSTATUS(status));
				exec_status = 1;
			}
			
		}
		
	}
	
	return exec_status;
}

//Executes two simple piped commands without redirection
int execute_pipe(char **args1, char **args2){
	
	int exec_status = 0;
	pid_t pid1, pid2;
	int status;
	int x1, x2;
	int fd[2];
	
	pipe(fd);
	
	pid1 = fork();
	
	if(pid1 < 0){
		printf("ERROR: Forking child process failed\n");
		exec_status = 1;
		exit(1);
	}
	else if(pid1 == 0) {

		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);

		x1 = execvp(args1[0], args1);
		if ( x1 < 0) {     
			printf("ERROR: Exec failed\n");
			exec_status = 1;
		}
		exit(1);
		
	}
	
	pid2 = fork();
	
	if(pid2 < 0){
		printf("ERROR: Forking child process failed\n");
		exec_status = 1;
		exit(1);
	}
	else if(pid2 == 0) {

		dup2(fd[0], STDIN_FILENO);
		close(fd[1]);

		x2 = execvp(args2[0], args2);
		if ( x2 < 0) {     
			printf("ERROR: Exec failed\n");
			exec_status = 1;
		}
		exit(1);
		
	}
	
	close(fd[0]);
	close(fd[1]);
	
	waitpid(pid1,&status,0);
	waitpid(pid2,&status,0);
	
	return exec_status;
	
}