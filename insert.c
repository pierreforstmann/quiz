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
#include <string.h>
#include <errno.h>
#include <limits.h>

/*
 * maximum length include '\0'
 */
#define QUESTION_MAX_LENGTH	60	
#define	ANSWER_MAX_LENGTH	60	
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

static char *fgets_with_newline(char *buf, int size)
{
	char c;

    	/* 
	 * fgets(s, n, stdin) reads at most n-1 characters.
	 *
	 * Reading until newline character is found
	 * and ignoring corresponding data if input data has more
	 * than n-1 characters. This is needed so that multiple fgets 
	 * work correctly (i.e. each fgets processes its newline).
	 */
	if (fgets(buf, size, stdin) != NULL) {
	    if (buf[strlen(buf) - 1] != '\n') {
	         do {
			c = fgetc(stdin);
		 } while (c != '\n');
	    	return buf;
	    } else return buf;

	} else	return NULL;
}


int main(int argc, char **argv) {
    
    sqlite3 *db;
    int rc;
    sqlite3_stmt *res;
    char *dbname; 
    const char	*tail;
    char *col1; 
    char icol2[2] ;
    int	col2;
    int	last_id = 0;
    int i;
    char *col3;
    char *endptr;

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

    begin_transaction(db);
restart:

    printf("Enter question:");
    col1 = (char *)malloc(QUESTION_MAX_LENGTH);
    if (col1 == NULL) {
	    fprintf(stderr, "malloc failed");
	    goto restart;
    }
    if (fgets_with_newline(col1, QUESTION_MAX_LENGTH) == NULL) {
	    fprintf(stderr, "fgets col1 failed");
	    goto restart;
    }
    printf("Enter solution number:");
    if (fgets_with_newline(icol2, 3) == NULL) {
	    fprintf(stderr, "fgets icol2 failed");
	    goto restart;
    }
    errno = 0;
    col2 = strtol(icol2, &endptr, 10);
    if ((errno == ERANGE && (col2 == LONG_MAX || col2 == LONG_MIN))
               || (errno != 0 && col2 == 0)) {
	 fprintf(stderr, "solution number is not a valid number \n");
         goto restart;
    }
    if (icol2 == endptr || *endptr != '\0') {
	 fprintf(stderr, "solution number is not a valid number \n");
         goto restart;
    }
    if ( col2 <= 0) {
	 fprintf(stderr, "solution number must be >= 1 \n");
         goto restart;
    }
    if ( col2 > MAX_ANSWER_NB) {
	 fprintf(stderr, "solution number too big \n");
         goto restart;
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
    printf("=> the last question id. is: %d\n", last_id);

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
        rc = sqlite3_prepare_v2(db, "INSERT INTO a(id, no, answer) VALUES(?1, ?2, ?3)", -1, &res, &tail);
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

    	printf("Enter answer number %d:", i);
	fgets_with_newline(col3, ANSWER_MAX_LENGTH);
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
