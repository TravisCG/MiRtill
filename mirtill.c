#include <stdio.h>
#include <stdlib.h>
#include <hiredis/hiredis.h>

int main(int argc, char **argv){
   redisContext *redis;
   redisReply *replay;
   int i;

   /* Connecting to database */
   redis  = redisConnect("127.0.0.1", 6379);
   if(redis->err){
      printf("Error in connection: %s\n", redis->errstr);
      return(0);
   }

   /* Send a command */
   replay = redisCommand(redis, "keys *");
   if(replay == NULL){
      printf("Error during the command: %s\n", redis->errstr);
      return(0);
   }
   for(i = 0; i < replay->elements; i++){
      printf("%s\n", replay->element[i]->str);
   }
   freeReplyObject(replay);

   /* Free resources*/
   redisFree(redis);
   return(0);
}
