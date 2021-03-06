
/**
 * Params stores the user defined parameters
 */
typedef struct _Params {
   char *filename;
   int database;
   int port;
   int min_abundance;
}Params;

int parseParams(int argc, char **argv, Params *params);
