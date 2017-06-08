#include "parse.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

/*
** A very simple main program that re-prints the line after it's been scanned and parsed.
*/
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


      // from here the cmdLine struct is filled up.
    	// printf("%d: ", cmdLine.numCommands);
      //
    	// if(cmdLine.infile) {
    	//     printf("< '%s' ", cmdLine.infile);
      // }


    	for(i=0; i < cmdLine.numCommands; i++)
    	{
          // beginning of each command, so pipe and fork should have happened here.
          pid_t pid = fork();

          if (pid == 0) { // child process
            for(int j=cmdLine.cmdStart[i]; cmdLine.argv[j] != NULL; j++) {
              execvp(cmdLine.argv[j], cmdLine.argv);
            }
          } else { // parent process
            wait(NULL);
          }
          // beginning of every word of each command

          // HERE IS THE END OF 1 COMMAND

          // if(i < cmdLine.numCommands - 1) {
          //   printf("| ");
          // }
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

    	printf("\n");

    	if(input == stdin)
    	{
  	    printf("? ");
  	    fflush(stdout);
    	}
    }

    return 0;
}
