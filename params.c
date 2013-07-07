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
   exit(0);
}

/**
 * Parse command line options
 */
int parseParams(int argc, char **argv, Params *params){
   int i;

   for(i = 1; i < argc; i++){
      if(!strcmp(argv[i], "-f")){
         /* Store filename*/
      }
      if(!strcmp(argv[i], "-d")){
         params->database = atoi(argv[i+1]);
      }
      if(!strcmp(argv[i], "-h")){
         printHelp();
      }
   }

   return(0);
}
