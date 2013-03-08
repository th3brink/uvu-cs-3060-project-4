// Student: Kory Kehl, James Brinkerhoff, Bryson Murray
// Instructor: John Jolly
// Class: CS 3060-001
// Project: 4
// Date: Feb 26, 2013
// Description: Simulates the more command

/* stdio for printf, fprintf, perror, fopen, feof and fgets */
#include <stdio.h>
/* stdlib for exit */
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <termios.h>

/* Declare my buffer size constant */
#define BUFFER_SIZE (100)
#define O_RDONLY (100)
// Number of lines to print
#define PAGELEN (23)
// Length of each line to print
#define LINELEN (512)

// do_more function
// Description: Prints 23 lines to display at a time or 1 the calls see_more
//              for a reply on how many more lines to print
// Return: None
// Paramaters: Takes File *
void do_more(FILE *);

// do_more function
// Description: Prints 23 lines to display at a time or 1 the calls see_more
//              for a reply on how many more lines to print
// Return: None
// Paramaters: Takes File *
void do_more_stdio(FILE *);


// see_more function
// Description: Prints in reverse video percentage of file displayed
// Return: int
// Paramaters: Takes File *, char *
int see_more(FILE *, char*);

// find_num_of_lines function
// Description: Finds the number of lines in file
// Return: None
// Paramaters: Takes File *
void find_num_of_lines(FILE *);

// deleteSpace function
// Description: Deletes what was printed in reverse video
// Return: None
// Paramaters: Takes int
void deleteSpace(int);

// ctrl_c_handler function
// Description: provides ctrl+c to exit correctly
// Return: None
// Paramaters: Takes int
void ctrl_c_handler (int);

// Total number of lines
int numLines;
// Total number of lines displayed
int linesDisplayed;
// File name
char *fileName;


//main function
int main (int argc, char * argv[])
{
    FILE *fp;
	
	tty_mode(0);
	set_cr_noecho_mode();
	signal(SIGINT, ctrl_c_handler);
	signal(SIGQUIT, SIG_IGN); 
    
    // More then 1 argument, so quit
    if(argc > 2)
    {
        tty_mode(1);
        exit(1);
    }
    // No file name passed in
    if(argc == 1)
    {
        printf("Type ^c to quit\n");
		fp = stdin;
        do_more_stdin(fp);
    }
    // One file name is passed in
    else
    {
        if((fp = fopen(argv[1], "r")) != NULL)
        {
            linesDisplayed = 0;
            find_num_of_lines(fp);
            fileName = argv[1];
            do_more(fp);
            fclose(fp);
        }
    }

	//return response;
	tty_mode(1);
    return 0;
}

// Prints each increment of lines
void do_more(FILE *fp)
{
    // Stores each line
    char line[LINELEN];
    // Store infor to print in reverse video
    char toPrint[50];
    // Increment of lines printed
    int num_of_lines = 0;
    // Percent of page printed
    int pagePercent;
    int see_more(FILE *, char*), reply;
    FILE *fp_tty;
    
    fp_tty = fopen("/dev/tty", "r");
    if(fp_tty == NULL)
    {
        tty_mode(1);
        exit(1);
    }
    
    while(fgets(line, LINELEN, fp))
    {
        if(num_of_lines == PAGELEN)
        {
            pagePercent = (linesDisplayed * 100) / numLines;
            if(linesDisplayed == PAGELEN)
            {
                // Stores what to print in reverse video
                // File Name and Percentage
                sprintf(toPrint, "%s %%%d", fileName, pagePercent);
                reply = see_more(fp_tty, toPrint);
            }
            if(linesDisplayed > PAGELEN)
            {
                // Stores what to print in reverse video
                // Percentage
                sprintf(toPrint, "%%%d", pagePercent);
                reply = see_more(fp_tty, toPrint);
            }
            if(reply == 0)
            {
                break;
            }
            num_of_lines -= reply;
        }
        if(fputs(line, stdout) == EOF)
        {
            tty_mode(1);
            exit(1);
        }
        num_of_lines++;
        linesDisplayed++;
    }
}

// Prints each increment of lines
void do_more_stdin(FILE *fp)
{
    char buffer[BUFFER_SIZE];
    char *numByte;
    int wordSize;
    int toDelete;
    int linesDone = 0;
    /* Keep going until the end of file is reached */
    while(!feof(fp))
    {
        /* Get a line of text from the file. If there is no text, or
         an error occured, then don't print anything */
        if(fgets(buffer, BUFFER_SIZE, fp))
        {
            if(linesDone > 0)
            {
                deleteSpace(toDelete + 3);
            }
            numByte = buffer;
            wordSize = ((int)strlen(numByte) * sizeof(numByte) - 8) / 8;
            printf("%s", buffer);
            sprintf(buffer, "bytes %d", wordSize);
            numByte = buffer;
            printf("\033[7m %s \033[m", numByte);
            toDelete = ((int)strlen(numByte) * sizeof(numByte) - 8) / 8;
            memset(&buffer[0], 0, sizeof(buffer));
            linesDone++;
        }
    }
}

// Prints percentage and waits for command to print more
int see_more(FILE *cmd, char *buf)
{
    int c;
    
    // Prints revers video
    printf("\033[7m %s \033[m", buf);
    while((c = getc(cmd)) != EOF)
    {
        if(c == 'q')
        {
            // Call to delete what printed in reverse video
            deleteSpace((int)strlen(buf) + 2);
            return 0;
        }
        if(c == ' ')
        {
            // Call to delete what printed in reverse video
            deleteSpace((int)strlen(buf) + 2);
            return PAGELEN;
        }
        if(c == '\n')
        {
            // Call to delete what printed in reverse video
            deleteSpace((int)strlen(buf) + 2);
            return 1;
        }
    }
    return 0;
}

// Finds how many lines the document has
void find_num_of_lines(FILE *fp)
{
    char line[LINELEN];
    int lines = 0;
    while(fgets(line, LINELEN, fp))
    {
        lines++;
    }
    numLines = lines;
    clearerr(fp);
    fseek(fp, 0, SEEK_SET);
}

// Delete characters from reverse video print with percentage
void deleteSpace(int n)
{
    int i = 0;
    for(i = 0; i < n; i++)
    {
        printf("\b \b");
    }
}

// turns off echo
set_cr_noecho_mode()
{
	struct termios ttystate;
	tcgetattr(0,&ttystate);
	ttystate.c_lflag &= ~ICANON;
	ttystate.c_lflag &= ~ECHO;
	tcsetattr(0,TCSANOW, &ttystate);
}

//saves flags and resets falgs so that echo works when 0 is passed.
tty_mode(int how) {
	static struct termios original_mode;
	static int original_flags;
	static int stored = 0;
	
	if (how == 0) {
		tcgetattr(0, &original_mode);
		original_flags = fcntl(0, F_GETFL);
		stored = 1;
	
	} else if (stored) { 
		tcsetattr(0, TCSANOW, &original_mode);
		fcntl(0, F_SETFL, original_flags);
	}
}

void ctrl_c_handler (int signum) {
	tty_mode(1);
	exit(1);

}


//Notes
	//To erase a character from the screen, display a backspace character (\b), a blank, and another backspace character.
	//To display a percent (%) sign using printf, use two % symbols in a row.
	//For reverse video, print \033[7m to start reverse video, and then \033[m to end reverse video. See the code on page 22 of the 
	// Molay book for an example.
	//You can use the reset command to restore your terminal settings if your program doesn't.
