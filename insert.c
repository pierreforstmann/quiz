/*
 * insert.c
 *
 * insert questions and answers
 *
 * Copyright Pierre Forstmann 2022
 */
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

#define QUESTION_MAX_LENGTH	80
#define	ANSWER_MAX_LENGTH	80
#define MAX_ANSWER_NB		3

int main(int argc, char **argv) {
    
    sqlite3 *db;
    sqlite3_stmt *res;
    char *dbname; 
    const char	*tail;
    char *col1; 
    int	col2;
    int	last_id = 0;
    int i;
    char *col3;

    if (argc != 2) {
        fprintf(stderr, "usage: insert <database name> \n");
	exit(1);
    }
    dbname = argv[1];

    /*
     * connect to database 
     */
    int rc = sqlite3_open(dbname, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
    
    rc = sqlite3_prepare_v2(db, "SELECT SQLITE_VERSION()", -1, &res, 0);    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }    
    
    rc = sqlite3_step(res);
    if (rc == SQLITE_ROW) {
        printf("Connected to database %s version %s\n", dbname, sqlite3_column_text(res, 0));
    }
    
   /*
    * insert question
    */ 
    printf("Enter question:");
    col1 = (char *)malloc(QUESTION_MAX_LENGTH);
    scanf("%s", col1);
    printf("Enter solution no:");
    scanf("%d", &col2);
    if ( col2 > MAX_ANSWER_NB) {
	    fprintf(stderr, "Solution no cannot be greater than %d \n", MAX_ANSWER_NB);
	    return 1;
    }

    rc = sqlite3_prepare_v2(db, "INSERT INTO q(question, solution) VALUES(?1, ?2)", -1, &res, &tail);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare INSERT INTO q: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
	}
	
    rc = sqlite3_bind_text(res, 1, col1, -1, SQLITE_STATIC); 
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to bind INSERT param. 1: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
	}

    rc = sqlite3_bind_int(res, 2, col2); 
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to bind INSERT param. 2: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
	}

    rc = sqlite3_step(res);
    if (rc == SQLITE_OK) {
        printf("INSERT INTO q: OK\n");
    }
    last_id = sqlite3_last_insert_rowid(db);
    printf("The last id. of the inserted row is %d\n", last_id);

    /*
     * insert answers
     */
    col3 = (char *)malloc(ANSWER_MAX_LENGTH);
    for (i = 1 ; i <= MAX_ANSWER_NB; i++)
    {
        rc = sqlite3_prepare_v2(db, "INSERT INTO a(id, no, answer) VALUES(?1, ?2, ?3)", 
			             -1, &res, &tail);
        if (rc != SQLITE_OK) {
          fprintf(stderr, "Failed to prepare INSERT INTO a: %s\n", sqlite3_errmsg(db));
          sqlite3_close(db);
          return 1;
	}

        rc = sqlite3_bind_int(res, 1, last_id); 
        if (rc != SQLITE_OK) {
         fprintf(stderr, "Failed to bind INSERT param. 1: %s\n", sqlite3_errmsg(db));
         sqlite3_close(db);
         return 1;
	}

        rc = sqlite3_bind_int(res, 2, i); 
        if (rc != SQLITE_OK) {
         fprintf(stderr, "Failed to bind INSERT param. 2: %s\n", sqlite3_errmsg(db));
         sqlite3_close(db);
         return 1;
	}

    	printf("Enter answser:");
        scanf("%s", col3);
        rc = sqlite3_bind_text(res, 3, col3, -1, SQLITE_STATIC); 
        if (rc != SQLITE_OK) {
         fprintf(stderr, "Failed to bind INSERT param. 3: %s\n", sqlite3_errmsg(db));
         sqlite3_close(db);
         return 1;
	}

        rc = sqlite3_step(res);
        if (rc == SQLITE_OK) {
          printf("INSERT INTO q: OK\n");
        }
    }

    /*
     * exit
     */

    sqlite3_finalize(res);
    sqlite3_close(db);
    
    return 0;
}
