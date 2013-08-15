#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "params.h"

/**
 *  Very simple help messages
 */
void printHelp(){
   printf("Mirtill\n");
   printf("-f: fasta file name\n");
   printf("-d: Redis database number\n");
   printf("-m: minimal abundance\n");
   exit(0);
}

/**
 * Parse command line options
 */
int parseParams(int argc, char **argv, Params *params){
   int i;

   /* Default settings */
   params->port          = 6379;
   params->database      = 0;
   params->min_abundance = 2;

   for(i = 1; i < argc; i++){
      if(!strcmp(argv[i], "-f")){
         /* Store filename*/
         params->filename = malloc(strlen(argv[i+1])+1);
         strcpy(params->filename, argv[i+1]);
      }
      if(!strcmp(argv[i], "-d")){
         params->database = atoi(argv[i+1]);
      }
      if(!strcmp(argv[i], "-h")){
         printHelp();
      }
      if(!strcmp(argv[i], "-m")){
         params->min_abundance = atoi(argv[i+1]);
      }
   }

   return(0);
}
