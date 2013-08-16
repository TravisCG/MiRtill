#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>
#include "params.h"
#include "levenshtein.h"

#define MAX_DISTANCE 4

void store(Params params, redisContext *redis, int minlength, int maxlength){
   FILE *input;
   char *key = NULL;
   redisReply *replay;
   ssize_t len;
   size_t store;

   input = fopen(params.filename, "r");
   while( (len = getline(&key, &store, input)) != -1){
      if(key[0] == '>') continue; /* skip fasta header */
      key[len-1] = '\0'; /* remove newline */
      if(len < minlength || len > maxlength) continue;

      /* Store key in redis database */
      replay = redisCommand(redis, "incr raw:%s", key);
      freeReplyObject(replay);
   }
   free(key);
   fclose(input);
}

/*
   Clustering sequences. To calculate distance Levenshtein was used
*/
void clustering_slow(redisContext *redis){
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
            redisCommand(redis, "sadd conn:%s %s", (r->element[i]->str)+4, (r->element[j]->str)+4); /* remove raw: */
         }
      }
      printf("...%u", i);
   }
   printf("\n");
   freeReplyObject(r);
}

void clustering(redisContext *redis){
   redisReply *r, *s;
   unsigned int i,j,k,length,hashlen,dist;
   char *seq, *subseq;

   r = redisCommand(redis, "keys raw:*");
   printf("number of elements: %u\n", (unsigned int)r->elements);
   for(i = 0; i < r->elements; i++){
      seq = (r->element[i]->str)+4;
      length = strlen(seq);
      hashlen = length / MAX_DISTANCE;
      for(j = 0; j < length-hashlen; j+=hashlen){
         subseq = malloc(hashlen);
         for(k = 0; k < hashlen; k++) subseq[k] = seq[k+j];
         subseq[hashlen] = '\0';
         s = redisCommand(redis, "keys raw:*%s*", subseq);
         for(k = 0; k < s->elements; k++){
            if(!strcmp(r->element[i]->str, s->element[k]->str)) continue;
            dist = levenshtein(r->element[i]->str, s->element[k]->str);
            if(dist < MAX_DISTANCE){
               redisCommand(redis, "sadd conn:%s %s", (r->element[i]->str)+4, (s->element[k]->str)+4);
            }
         }
         free(subseq);
         freeReplyObject(s);
      }
      printf("...%u",i);
   }
   printf("\n");
   freeReplyObject(r);
}

/* Recursive cluster walking */
int checksiblings(redisContext *redis, char *parent, char *firstparent){
   redisReply *r, *s;
   unsigned int i;
   int ret = 0;

   r = redisCommand(redis, "smembers conn:%s", parent);

   if(r->elements == 0){
      /* leaf of the graph */
      s = redisCommand(redis, "sismember conn:%s %s", firstparent, parent);
      ret = s->integer;
      freeReplyObject(s);
   }
   else{
      for(i = 0; i < r->elements; i++){
         ret = checksiblings(redis, r->element[i]->str, firstparent);
         if(ret == 0){
            redisCommand(redis, "del raw:%s", r->element[i]->str);
         }
         redisCommand(redis, "del conn:%s", r->element[i]->str); /* Remove complexity */
      }
   }

   freeReplyObject(r);
   return(ret);
}

/* Check triangle inequality. If one of the child cluster contains
   other elements than the parent, it has inequality */
void triangleinequality(redisContext *redis){
   redisReply *r;
   unsigned int i;
   int j;

   r = redisCommand(redis, "keys conn:*");
   printf("  element number:%u", (unsigned int)r->elements);
   for(i = 0; i < r->elements; i++){
      j = checksiblings(redis, (r->element[i]->str)+5, (r->element[i]->str)+5); /* remove conn:*/
      if(j == 0){
         redisCommand(redis, "del %s",r->element[i]->str);
         redisCommand(redis, "del raw:%s", (r->element[i]->str)+5);
      }
      printf("...%u",i);
   }
   printf("\n");
   freeReplyObject(r);
}

void filter(redisContext *redis, int min_abu){
   redisReply *r, *get;
   unsigned int i;

   r = redisCommand(redis, "keys raw:*");

   for(i = 0; i < r->elements; i++){
      get = redisCommand(redis, "get %s", r->element[i]->str);
      if(atoi(get->str) < min_abu){
         printf("filter out %s\n", r->element[i]->str);
         redisCommand(redis, "del %s", r->element[i]->str);
      }
      freeReplyObject(get);
   }
   freeReplyObject(r);
}

int main(int argc, char **argv){
   redisContext *redis;
   Params params;
   parseParams(argc, argv, &params);

   /* Connecting to database */
   redis  = redisConnect("127.0.0.1", params.port); /*FIXME host shoud be a parameter*/
   if(redis->err){
      printf("Error in connection: %s\n", redis->errstr);
      return(0);
   }

   /* Store sequences in redis database */
   printf("Storing reads to database\n");
   store(params, redis, 18, 25); /*FIXME this number shoud be parameters */

   /* Filtering sequences with small abundance */
   filter(redis, params.min_abundance);

   /* Clustering */
   printf("Start clustering\n");
   clustering(redis);

   /* Remove sRNA and tRNA */
   printf("Remove s/tRNAs\n");
   triangleinequality(redis);

   /* Free resources*/
   redisFree(redis);
   return(0);
}
