#include<stdio.h>
#include<stdlib.h>
#include<string.h>

// Colors 
#define RED  "\x1B[31m"
#define GREEN  "\x1B[32m"
#define YELLOW  "\x1B[33m"
#define BLUE  "\x1B[34m"
#define MAGENTA  "\x1B[35m"
#define CYAN  "\x1B[36m"

//Displaying starting message
void welcome_message(){
    printf("\n");
    printf("\t\t\t\t\t\t\t  Welcome to your PHONE DIRECTORY ! \n\n");
    printf("\t\t\t\t\t\t\t    Please see codes given below\n");
    printf("\n");
    printf("%s\t\t\t\t -------------------------------------------------------------------------------------- \n",CYAN);
    printf("%s\t\t\t\t|                            |                            |                            |\n",CYAN);
    printf("%s\t\t\t\t|",CYAN);
    printf("%s          ADD NO.           ",MAGENTA);
    printf("%s|",CYAN);
    printf("%s         DELETE NO.         ",MAGENTA);
    printf("%s|",CYAN);
    printf("%s         SEARCH NO.         ",MAGENTA);
    printf("%s|\n",CYAN);
    printf("%s\t\t\t\t|                            |                            |                            |\n",CYAN);
    printf("%s\t\t\t\t|--------------------------------------------------------------------------------------|\n",CYAN);
    printf("%s\t\t\t\t|                            |                            |                            |\n",CYAN);
    printf("%s\t\t\t\t|",CYAN);
    printf("%s             1              ",MAGENTA);
    printf("%s|",CYAN);
    printf("%s             2              ",MAGENTA);
    printf("%s|",CYAN);
    printf("%s             3              ",MAGENTA);
    printf("%s|\n",CYAN);
    printf("%s\t\t\t\t|                            |                            |                            |\n",CYAN);
    printf("%s\t\t\t\t|--------------------------------------------------------------------------------------|\n",CYAN);
    printf("%s\t\t\t\t|--------------------------------------------------------------------------------------|\n",CYAN);
    printf("%s\t\t\t\t|                            |                            |                            |\n",CYAN);
    printf("%s\t\t\t\t|",CYAN);
    printf("%s      SEE ALL CONTACTS      ",MAGENTA);
    printf("%s|",CYAN);
    printf("%s      SORT ALL CONTACTS     ",MAGENTA);
    printf("%s|",CYAN);
    printf("%s           CALL             ",MAGENTA);
    printf("%s|\n",CYAN);
    printf("%s\t\t\t\t|                            |                            |                            |\n",CYAN);
    printf("%s\t\t\t\t|--------------------------------------------------------------------------------------|\n",CYAN);
    printf("%s\t\t\t\t|                            |                            |                            |\n",CYAN);
    printf("%s\t\t\t\t|",CYAN);
    printf("%s             4              ",MAGENTA);
    printf("%s|",CYAN);
    printf("%s             5              ",MAGENTA);
    printf("%s|",CYAN);
    printf("%s             6              ",MAGENTA);
    printf("%s|\n",CYAN);
    printf("%s\t\t\t\t|                            |                            |                            |\n",CYAN);
    printf("%s\t\t\t\t|--------------------------------------------------------------------------------------|\n",CYAN);
    printf("%s\t\t\t\t|--------------------------------------------------------------------------------------|\n",CYAN);
    printf("%s\t\t\t\t|                            |                            |                            |\n",CYAN);
    printf("%s\t\t\t\t|",CYAN);
    printf("%s          MESSAGE           ",MAGENTA);
    printf("%s|",CYAN);
    printf("%s        FREQUENCY LIST      ",MAGENTA);
    printf("%s|",CYAN);
    printf("%s           EXIT             ",MAGENTA);
    printf("%s|\n",CYAN);
    printf("%s\t\t\t\t|                            |                            |                            |\n",CYAN);
    printf("%s\t\t\t\t|--------------------------------------------------------------------------------------|\n",CYAN);
    printf("%s\t\t\t\t|                            |                            |                            |\n",CYAN);
    printf("%s\t\t\t\t|",CYAN);
    printf("%s             7              ",MAGENTA);
    printf("%s|",CYAN);
    printf("%s             8              ",MAGENTA);
    printf("%s|",CYAN);
    printf("%s             9              ",MAGENTA);
    printf("%s|\n",CYAN);
    printf("%s\t\t\t\t|                            |                            |                            |\n",CYAN);
    printf("%s\t\t\t\t -------------------------------------------------------------------------------------- \n",CYAN);
    printf("\n");
}

//Main function
int main(){
    welcome_message();
    return 0;
}