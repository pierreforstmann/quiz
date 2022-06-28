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

static void exit_ko(void) 
{
    fprintf(stderr, "Database check: KO \n");
    exit(1);
}

int main(int argc, char **argv) {
    
    sqlite3 *db;
    int rc;
    const char *tail;
    sqlite3_stmt *res;
    char *dbname; 

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
    * check q
    */ 

    rc = sqlite3_prepare_v2(db, "SELECT id, question FROM q WHERE 1=0", -1, &res, &tail);
    if (rc != SQLITE_OK) {
	fprintf(stderr, "Failed to prepare SELECT ... FROM q \n");
	exit_ko();
    }
   
    /*
     * check a 
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


    /*
     * exit
     */

    sqlite3_close(db);

    printf("Database check: OK\n");
    
    exit(0);
}
