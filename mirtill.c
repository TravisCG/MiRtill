#include <stdio.h>
#include <stdlib.h>
#include <hiredis/hiredis.h>
#include "params.h"
#include "levenshtein.h"

#define MAX_DISTANCE 4

void store(Params params, redisContext *redis){
   FILE *input;
   char *key = NULL;
   redisReply *replay;
   ssize_t len;
   size_t store;

   input = fopen(params.filename, "r");
   while( (len = getline(&key, &store, input)) != -1){
      if(key[0] == '>') continue; /* skip fasta header */
      key[len-1] = '\0'; /* remove newline */

      /* Store key in redis database */
      replay = redisCommand(redis, "incr raw:%s", key);
      freeReplyObject(replay);
   }
   free(key);
   fclose(input);
}

void clustering(redisContext *redis){
   redisReply *r;
   unsigned int i,j,dist;

   r = redisCommand(redis, "keys raw:*");
   printf("element number: %u\n", (unsigned int)r->elements);
   /* TODO too slow */
   for(i = 0; i < r->elements-1; i++){
      for(j = i+1; j < r->elements; j++){
         dist = levenshtein(r->element[i]->str, r->element[j]->str);
         if(dist < MAX_DISTANCE){
            /* FIXME check the return value */
            redisCommand(redis, "sadd conn:%s %s", r->element[i]->str, r->element[j]->str);
            printf("%u %s %s %d\n", i, r->element[i]->str, r->element[j]->str, dist);
         }
      }
   }
   freeReplyObject(r);
}

int main(int argc, char **argv){
   redisContext *redis;
   Params params;
   parseParams(argc, argv, &params);

   /* Connecting to database */
   redis  = redisConnect("127.0.0.1", params.port);
   if(redis->err){
      printf("Error in connection: %s\n", redis->errstr);
      return(0);
   }

   /* Store sequences in redis database */
   printf("Storing reads to database\n");
   store(params, redis);

   /* Clustering */
   printf("Start clustering\n");
   clustering(redis);

   /* Free resources*/
   redisFree(redis);
   return(0);
}
