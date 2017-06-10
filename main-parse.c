#include "parse.c"
#include <assert.h>
#include <stdio.h>
#include <unistd.h> //dup2
#include <string.h> // strlen
#include <sys/wait.h>
#include <ctype.h> //for isdigit()

void sigintCHandler(int sig)
{
    signal(SIGINT, sigintCHandler);
    printf("\n? ");
    fflush(stdout);
}

int exit_command(char **argv) {
    if(argv[1] == NULL) {
      exit(0);
    }

    int length = strlen(argv[1]);
    if (length > 10 || length == 0) {
      // too big exit status so exit(0)
      exit(0);
    }

    int statusNumber = 0;
    for(int i=0; i<length; ++i) {
      if(!isdigit(argv[1][i])) {
        // return non-zero as exit with error, when the code provided is not a valid number
        exit(1);
      }
      int x = argv[1][i] - 48; // convert ascii char digit to int
      statusNumber =  (statusNumber * 10) + x;
    }
    exit(statusNumber);
}

void cd_command(char **argv) {
    char *cdArgument = argv[1];
    if (cdArgument == NULL) {
      if(chdir(getenv("HOME")) != 0) {
        perror(getenv("HOME"));
      }
    } else {
      if(chdir(cdArgument) != 0) {
        perror(cdArgument);
      };
    }
}

void single_child(int commandIndex, char *argv[]) {
  pid_t pid;
  pid = fork();
  if (pid < 0) {
    perror("fork");
  }

  if (pid == 0) { // child process
    printf("%s\n", "hanging in the SINGLE child");

    if(execvp(argv[commandIndex], &argv[commandIndex]) == -1) {
      // print or perror
      printf("nsh: %s: command not found\n", argv[commandIndex]);
    }
    // exec gets here only in case of error
    exit(EXIT_FAILURE);
  } else { // parent process
    wait(NULL);
  }
}

void double_child(int firstCommandIndex, int secondCommandIndex, char *argv[]) {
  pid_t pid1, pid2;
  int status;
  int pipefd[2];

  pipe(pipefd);

  pid1 = fork();
  if (pid1 < 0) {
    perror("fork");
  }

  if (pid1 == 0) { // child process
    close(pipefd[0]); // close the reading end in first child
    printf("%s\n", "hanging in the FIRST child");
    if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
        perror("dup2");
    } // connect stdout to writing end of the pipe

    if(execvp(argv[firstCommandIndex], &argv[firstCommandIndex]) == -1) {
      printf("nsh: %s: command not found\n", argv[firstCommandIndex]);
    }
    exit(EXIT_FAILURE);
  }

  pid2 = fork();
  if (pid2 < 0) {
    perror("fork");
  }

  if (pid2 == 0) { // child process
    close(pipefd[1]); // close the writing end in second child
    if(dup2(pipefd[0], STDIN_FILENO) == -1) {
      perror("dup2");
    } // connect stdin to reading end of the pipe
    printf("%s\n", "hanging in the SECOND child");
    if(execvp(argv[secondCommandIndex], &argv[secondCommandIndex]) == -1) {
      printf("nsh: %s: command not found\n", argv[secondCommandIndex]);
    }
    exit(EXIT_FAILURE);
  }

  close(pipefd[0]);
  close(pipefd[1]);
  waitpid(pid1, &status, WUNTRACED);
  waitpid(pid2, &status, WUNTRACED);
  printf("%s\n", "supposedly terminated children");
}

int main(int argc, char *argv[])
{
   // Set the SIGINT (Ctrl-C) signal handler
   signal(SIGINT, sigintCHandler);

   FILE *input;
   char line[MAX_LINE];

    if(argc == 2)
    {
    	input = fopen(argv[1], "r");
    	if(input == NULL)
    	{
  	    perror(argv[1]);
  	    exit(1);
    	}
    } else {
    	assert(argc == 1);
    	input = stdin;
    	printf("? ");
    	/* By default, printf will not "flush" the output buffer until
    	* a newline appears.  Since the prompt does not contain a newline
    	* at the end, we have to explicitly flush the output buffer using
    	* fflush.
    	*/
    	fflush(stdout);
    }

    setlinebuf(input);
    while(fgets(line, sizeof(line), input)) {
      int i;
    	struct commandLine cmdLine;

    	if(line[strlen(line)-1] == '\n') {
    	   line[strlen(line)-1] = '\0';   /* zap the newline */
      }

    	Parse(line, &cmdLine);

      int index = 0;
      int firstCommandIndex = cmdLine.cmdStart[index];

      // check if user input is empty to avoid unwanted behaviours
      if(cmdLine.argv[firstCommandIndex] == NULL) {
        printf("? ");
        continue;
      }

      // checking for local commands that do not need a fork
      if(strcmp(cmdLine.argv[firstCommandIndex],"exit") == 0) {
        exit_command(cmdLine.argv);
      } else if (strcmp(cmdLine.argv[firstCommandIndex],"cd") == 0) {
        cd_command(cmdLine.argv);
        printf("? ");
        continue;
      }

      if (cmdLine.numCommands == 1) {
        single_child(firstCommandIndex, cmdLine.argv);
      } else if (cmdLine.numCommands == 2) {
        ++index; // increase index of array that holds indexeses of commands
        int secondCommandIndex = cmdLine.cmdStart[index];
        double_child(firstCommandIndex, secondCommandIndex, cmdLine.argv);
      }

      // if(cmdLine.append)
    	// {
    	//     /* verify that if we're appending there should be an outfile! */
  	  //   assert(cmdLine.outfile);
  	  //   printf(">");
    	// }
    	// if(cmdLine.outfile) {
    	//    printf(">'%s'", cmdLine.outfile);
      // }

    	if(input == stdin)
    	{
  	    printf("? ");
  	    fflush(stdout);
    	}
    }

    return 0;
}
