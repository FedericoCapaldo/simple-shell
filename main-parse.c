#include "parse.c"
#include <assert.h>
#include <stdio.h>
#include <unistd.h> //dup2
#include <string.h> // strlen
#include <sys/wait.h>
#include <ctype.h> //for isdigit()

/*
** A very simple main program that re-prints the line after it's been scanned and parsed.
*/
// NO AMOUNT OF WORRY WILL MAKE THE FUTURE BETTER. NO AMOUNT OF WORRY WILL MAKE YOU ACHIEVE ANYTHING. FOCUS.
void sigintCHandler(int sig)
{
    signal(SIGINT, sigintCHandler);
    printf("\n? ");
    fflush(stdout);
}

int exit_command(char **argv) {
    if(argv[1] == NULL) {
      return 0;
    }

    int length = strlen(argv[1]);
    if (length > 10 || length == 0) {
      // too big exit status so exit(0)
      return 0;
    }

    int statusNumber = 0;
    for(int i=0; i<length; ++i) {
      if(!isdigit(argv[1][i])) {
        return 0;
      }
      int x = argv[1][i] - 48; // convert ascii char digit to int
      statusNumber =  (statusNumber * 10) + x;
    }
    return statusNumber;
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
    while(fgets(line, sizeof(line), input))
    {
    	int i;
    	struct commandLine cmdLine;

    	if(line[strlen(line)-1] == '\n') {
    	   line[strlen(line)-1] = '\0';   /* zap the newline */
      }

    	Parse(line, &cmdLine);

      int index = 0;
    	// for(i=0; i < cmdLine.numCommands; i++)
    	// {
        // this is just to stop at the first command right now
        // if (i > 0) {
        //   continue;
        // }

          // beginning of each command, so pipe and fork should have happened here.
          int firstCommandIndex =cmdLine.cmdStart[index];

          // check if user input is empty to avoid crash
          if(cmdLine.argv[firstCommandIndex] == NULL) {
            printf("? ");
            continue;
          }

          // checking for local commands that do not need a fork
          if(strcmp(cmdLine.argv[firstCommandIndex],"exit") == 0) {
            return exit_command(cmdLine.argv);
          } else if (strcmp(cmdLine.argv[firstCommandIndex],"cd") == 0) {
            cd_command(cmdLine.argv);
            printf("? ");
            continue;
          }

          if (cmdLine.numCommands == 1) {
            pid_t pid;
            pid = fork();
            if (pid < 0) {
              perror("fork");
            }

            if (pid == 0) { // child process
              printf("%s\n", "hanging in the SINGLE child");

              if(execvp(cmdLine.argv[firstCommandIndex], &cmdLine.argv[firstCommandIndex]) == -1) {
                // perror(cmdLine.argv[firstCommandIndex]); // not sure if I should print this message
                printf("nsh: %s: command not found\n", cmdLine.argv[firstCommandIndex]); // this seems more appropriate err message
                exit(EXIT_FAILURE); // if program arrives at this point it means that an error has been prompted
              }
              printf("%s\n", "just before return in child 1" );
              return 1;
            } else { // parent process
              wait(NULL);
              // waitpid(pid1, &status, WUNTRACED);
              // do {
              //   waitpid(pid1, &status, WUNTRACED);
              // } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            }
          } else if (cmdLine.numCommands == 2) {
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

              if(execvp(cmdLine.argv[firstCommandIndex], &cmdLine.argv[firstCommandIndex]) == -1) {
                // perror(cmdLine.argv[firstCommandIndex]); // not sure if I should print this message
                printf("nsh: %s: command not found\n", cmdLine.argv[firstCommandIndex]); // this seems more appropriate err message
                exit(EXIT_FAILURE); // if program arrives at this point it means that an error has been prompted
              }
              printf("%s\n", "just before return in child 1" );
              return 1;
            } else { // parent process
              // wait(NULL);
              // waitpid(pid1, &status, WUNTRACED);
              // do {
              //   waitpid(pid1, &status, WUNTRACED);
              // } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            }

            // HERE IS THE END OF 1 COMMAND
      	// } //end of commented out for loop

            ++index; // increase index of array that holds indexeses of commands
            int secondCommandIndex =cmdLine.cmdStart[index];

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
              if(execvp(cmdLine.argv[secondCommandIndex], &cmdLine.argv[secondCommandIndex]) == -1) {
                // perror(cmdLine.argv[firstCommandIndex]);
                printf("nsh: %s: command not found\n", cmdLine.argv[secondCommandIndex]); // this seems more appropriate err message
                exit(EXIT_FAILURE);
              }
              printf("%s\n", "just before return in child 2" );
              return 1;
            } else { // parent process
              // wait(NULL);
              // waitpid(pid2, &status, WUNTRACED);
              // do {
              //   waitpid(pid2, &status, WUNTRACED);
              // } while (!WIFEXITED(status) && !WIFSIGNALED(status));
              // printf("%d\n", getpid());
              // printf("%s\n", "sure is the parent only?");
            }


            // if you don't close the fd, code breaks.
            close(pipefd[0]);
            close(pipefd[1]);

            // waitpid(pid1);
            // waitpid(pid2);
            // wait(NULL);
            printf("%s\n", "does it get here?");
            waitpid(pid1, &status, WUNTRACED);
            waitpid(pid2, &status, WUNTRACED);
            printf("%s\n", "supposedly terminated children");
            // return 0;
          }
    	if(cmdLine.append)
    	{
    	    /* verify that if we're appending there should be an outfile! */
  	    assert(cmdLine.outfile);
  	    printf(">");
    	}
    	if(cmdLine.outfile) {
    	   printf(">'%s'", cmdLine.outfile);
      }
    	/* Print any other relevant info here, such as '&' if you implement
    	 * background execution, etc.
    	 */

      // seems not to be needed anymore
    	// printf("\n");

    	if(input == stdin)
    	{
  	    printf("? ");
  	    fflush(stdout);
    	}
    }

    return 0;
}
