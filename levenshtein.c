#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "levenshtein.h"

double min3(double a, double b, double c){
   if (a < b){
      if (a < c) return(a); else return(c);
   }
   else{
      if (b < c) return(b); else return(c);
   }
}

int levenshtein(char *s, char *t){

   int    *d;
   int    n,m;
   int    i,j;
   int    cost, res;

   n = strlen(s);
   m = strlen(t);

   if ((n==0) || (m==0)) return(0);

   d = malloc(sizeof(int) * n * m);


   for(i=0; i<n; i++){
      d[i] = i;
   }

   for(j=0; j<m; j++){
      d[n*j] = j;
   }

   for(i=1; i<n; i++)
      for(j=1; j<m; j++) {

      if (s[i-1] == t[j-1]) cost = 0; else cost = 1;
      d[n*j + i] = min3(d[n*j + i - 1]+1,
                        d[n*(j-1) + i]+1,
                        d[n*(j-1) + i - 1] + cost);

   }

   res = d[n*m-1];
   free(d);

   return(res);
}
