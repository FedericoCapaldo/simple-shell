#include "parse.h"
#include <assert.h>
#include <stdio.h>

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
    }
    else
    {
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

	if(line[strlen(line)-1] == '\n')
	    line[strlen(line)-1] = '\0';   /* zap the newline */

	Parse(line, &cmdLine);

	printf("%d: ", cmdLine.numCommands);

	if(cmdLine.infile)
	    printf("< '%s' ", cmdLine.infile);

	for(i=0; i < cmdLine.numCommands; i++)
	{
	    int j;
	    for(j=cmdLine.cmdStart[i]; cmdLine.argv[j] != NULL; j++)
		printf("'%s' ", cmdLine.argv[j]);
	    if(i < cmdLine.numCommands - 1)
		printf("| ");
	}

	if(cmdLine.append)
	{
	    /* verify that if we're appending there should be an outfile! */
	    assert(cmdLine.outfile);
	    printf(">");
	}
	if(cmdLine.outfile)
	    printf(">'%s'", cmdLine.outfile);

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
