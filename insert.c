/*
 * insert.c
 *
 * quiz: insert questions and answers
 *
 * Copyright Pierre Forstmann 2022
 */
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

#define QUESTION_MAX_LENGTH	80
#define	ANSWER_MAX_LENGTH	80
#define MAX_ANSWER_NB		3

static void begin_transaction(sqlite3 *db)
{
    int rc;

    rc = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "BEGIN TRANSACTION failed : %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1) ;
    }
}

static void rollback_transaction(char *message, sqlite3 *db)
{
    int rc;

    fprintf(stderr, "%s %s \n", message, sqlite3_errmsg(db));
    rc = sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "ROLLBACK failed : %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1) ;
    }
}


int main(int argc, char **argv) {
    
    sqlite3 *db;
    int rc;
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
    rc = sqlite3_open(dbname, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    
    rc = sqlite3_prepare_v2(db, "SELECT SQLITE_VERSION()", -1, &res, 0);    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to check SQLite version: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }    
    
    rc = sqlite3_step(res);
    if (rc == SQLITE_ROW) {
        printf("Connected to database: %s SQLite version %s\n", dbname, sqlite3_column_text(res, 0));
    } else {
        fprintf(stderr, "Failed to get SQLite version: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
	return 1;
    }
    
   /*
    * insert question
    */ 

restart:
    begin_transaction(db);

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
	rollback_transaction("Failed to prepare INSERT INTO q", db);
	goto restart;
	}
	
    rc = sqlite3_bind_text(res, 1, col1, -1, SQLITE_STATIC); 
    if (rc != SQLITE_OK) {
        rollback_transaction("Failed to bind INSERT param. 1", db);
	goto restart;
	}

    rc = sqlite3_bind_int(res, 2, col2); 
    if (rc != SQLITE_OK) {
        rollback_transaction("Failed to bind INSERT param. 2", db);
	goto restart;
	}

    rc = sqlite3_step(res);
    if (rc != SQLITE_DONE) {
         rollback_transaction("Failed to INSERT INTO q", db);
	goto restart;
    }
    last_id = sqlite3_last_insert_rowid(db);
    printf("The last question id. is: %d\n", last_id);

    rc = sqlite3_finalize(res);
    if (rc != SQLITE_OK) {
         rollback_transaction("Failed to delete prepared statement INSERT INTO q", db);
	goto restart;
	}

    /*
     * insert answers
     */

    col3 = (char *)malloc(ANSWER_MAX_LENGTH);
    for (i = 1 ; i <= MAX_ANSWER_NB; i++)
    {
        rc = sqlite3_prepare_v2(db, "INSERT INTO a(id, no, answer) VALUES(?1, ?2, ?3)", 
			             -1, &res, &tail);
        if (rc != SQLITE_OK) {
          rollback_transaction("Failed to prepare INSERT INTO a", db);
	  goto restart;
	}

        rc = sqlite3_bind_int(res, 1, last_id); 
        if (rc != SQLITE_OK) {
          rollback_transaction("Failed to bind INSERT param. 1", db);
	  goto restart;
	}

        rc = sqlite3_bind_int(res, 2, i); 
        if (rc != SQLITE_OK) {
         rollback_transaction("Failed to bind INSERT param. 2", db);
	 goto restart;
	}

    	printf("Enter answser:");
        scanf("%s", col3);
        rc = sqlite3_bind_text(res, 3, col3, -1, SQLITE_STATIC); 
        if (rc != SQLITE_OK) {
         rollback_transaction("Failed to bind INSERT param. 3", db);
	 goto restart;
	}

        rc = sqlite3_step(res);
        if (rc != SQLITE_DONE) {
         rollback_transaction("Failed to INSERT INTO a", db);
	  goto restart;
        }
    }
    rc  = sqlite3_finalize(res);
    if (rc != SQLITE_OK) {
         rollback_transaction("Failed to delete prepared statement INSERT INTO a", db);
	goto restart;
	}


    rc = sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        rollback_transaction("COMMIT failed", db);
        sqlite3_close(db);
        return 1;
    }
    printf("Transaction committed \n") ;


    /*
     * exit
     */

    sqlite3_close(db);
    
    return 0;
}
