#include "parse.c"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h> //for isdigit()

/*
** A very simple main program that re-prints the line after it's been scanned and parsed.
*/


int exit_command(char **argv) {
    int length = strlen(argv[1]);
    if (length > 10) {
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


    	for(i=0; i < cmdLine.numCommands; i++)
    	{
        // this is just to stop at the first command right now
        if (i > 0) {
          continue;
        }
          // beginning of each command, so pipe and fork should have happened here.

          int startCommandIndex =cmdLine.cmdStart[i];

          // checking for local commands that do not need a fork
          if(strcmp(cmdLine.argv[startCommandIndex],"exit") == 0) {
            return exit_command(cmdLine.argv);
          } else if (strcmp(cmdLine.argv[startCommandIndex],"cd") == 0) {
            cd_command(cmdLine.argv);
            continue;
          }


          pid_t pid = fork();
          if (pid < 0) {
            perror("fork");
          }

          if (pid == 0) { // child process
            if(execvp(cmdLine.argv[startCommandIndex], cmdLine.argv) == -1) {
              // perror(cmdLine.argv[j]); // not sure if I should print this message
              printf("nsh: %s: command not found\n", cmdLine.argv[startCommandIndex]); // this seems more appropriate err message
            }
            exit(EXIT_FAILURE); // if program arrives at this point it means that an error has been prompted
          } else { // parent process
            wait(NULL);
          }

          // HERE IS THE END OF 1 COMMAND
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
