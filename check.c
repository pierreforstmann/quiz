/*
 * check.c
 *
 * quiz: check database
 *
 * Copyright Pierre Forstmann 2022
 */
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

static int row_count = 0;

static void exit_ko(void) 
{
    fprintf(stderr, "Database check: KO \n");
    exit(1);
}

int callback(void *p_arg, int argc, char **argv, char **col_names) {

    int i;

    for(i = 0; i < argc; i++) {
      printf("%s: %s\n", col_names[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    row_count++;
    return 0;
}

int main(int argc, char **argv) {
    
    sqlite3 *db;
    int rc;
    const char *tail;
    sqlite3_stmt *res;
    char *dbname, *errmsg;

    if (argc != 2) {
        fprintf(stderr, "usage: check  <database name> \n");
	exit_ko();
    }
    dbname = argv[1];

    /*
     * connect to database 
     */
    rc = sqlite3_open(dbname, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        exit_ko();
    }
    
    rc = sqlite3_prepare_v2(db, "SELECT SQLITE_VERSION()", -1, &res, 0);    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to check SQLite version: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit_ko();
    }    
    
    rc = sqlite3_step(res);
    if (rc == SQLITE_ROW) {
        printf("Connected to database: %s SQLite version %s\n", dbname, sqlite3_column_text(res, 0));
    } else {
        fprintf(stderr, "Failed to get SQLite version: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
	exit_ko();
    }
   
   /*
    * check q structure
    */ 

    rc = sqlite3_prepare_v2(db, "SELECT id, question FROM q WHERE 1=0", -1, &res, &tail);
    if (rc != SQLITE_OK) {
	fprintf(stderr, "Failed to prepare SELECT ... FROM q \n");
	exit_ko();
    }
    printf("=> table q: structure OK\n"); 
   
    /*
     * check a  structure
     */

    rc = sqlite3_prepare_v2(db, "SELECT id, no, answer, solution FROM a WHERE 1 = 0", -1, &res, &tail);
    if (rc != SQLITE_OK) {
	fprintf(stderr, "Failed to prepare SELECT ... FROM a \n");
	 exit_ko();
    }

    rc  = sqlite3_finalize(res);
    if (rc != SQLITE_OK) {
         fprintf(stderr, "Failed to delete prepared statement SELECT ...\n" );
    }
    printf("=> table a: structure OK\n"); 

    /*
     * check q data
     */
    printf("=> table q: checking rows ...\n"); 
    row_count = 0; 
    rc =  sqlite3_exec(db, "select id, question from q" , 
		            callback , NULL, &errmsg);
    if (rc != SQLITE_OK) {
       printf("Error executing query: %s \n", sqlite3_errstr(rc));
       printf("Error executing query: %s \n", errmsg);
       sqlite3_free(errmsg);
       exit_ko();
    }
    printf("\n=> ... table q: %d rows checked OK \n\n", row_count); 

    /*
     * check a data
     */
    printf("=> table a: checking rows ...\n"); 
    row_count = 0; 
    rc =  sqlite3_exec(db, "select id, no, answer, solution from a" , 
		            callback , NULL, &errmsg);
    if (rc != SQLITE_OK) {
       printf("Error executing query: %s \n", sqlite3_errstr(rc));
       printf("Error executing query: %s \n", errmsg);
       sqlite3_free(errmsg);
       exit_ko();
    }
    printf("=> ... table a: %d rows checked OK \n\n", row_count); 
    /*
     * exit
     */

    sqlite3_close(db);

    printf("Database check: OK\n");
    
    exit(0);
}
