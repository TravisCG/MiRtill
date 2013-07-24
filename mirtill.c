#include <stdio.h>
#include <stdlib.h>
#include <hiredis/hiredis.h>
#include "params.h"

int main(int argc, char **argv){
   redisContext *redis;
   redisReply *replay;
   Params params;
   FILE *input;
   char *key = NULL;
   ssize_t len;
   size_t store;

   parseParams(argc, argv, &params);

   /* Connecting to database */
   redis  = redisConnect("127.0.0.1", params.port);
   if(redis->err){
      printf("Error in connection: %s\n", redis->errstr);
      return(0);
   }


   /* Store sequences in redis database */
   input = fopen(params.filename, "r");
   while( (len = getline(&key, &store, input)) != -1){
      if(key[0] == '>') continue; /* skip fasta header */
      key[len-1] = '\0'; /* remove newline */

      /* Store key in redis database */
      replay = redisCommand(redis, "incr %s", key);
      freeReplyObject(replay);
   }
   free(key);
   fclose(input);

   /* Free resources*/
   redisFree(redis);
   return(0);
}
