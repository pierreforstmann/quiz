/*
 * select.c
 *
 * quiz: read questions and check answer
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
	    } else {
		    buf[strlen(buf) - 1 ] = '\0';
		    return buf;
	    }

	} else	return NULL;
}


int main(int argc, char **argv) {
    
    sqlite3 *db;
    int rc;
    sqlite3_stmt *res;
    char *dbname; 
    const char	*tail;
    char buf[3];
    char anrbuf[3] ;
    int	anr;
    int i;
    char *endptr;
    int qnr = 1;
    int expected_anr = 1;

    if (argc != 3) {
        fprintf(stderr, "usage: select  <database name> <question id>\n");
	exit(1);
    }
    dbname = argv[1];
    qnr = atoi(argv[2]);

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
    * start transaction
    */ 

restart_before_transaction:
    begin_transaction(db);
restart_after_transaction:

   /*
    * retrieve question
    */ 

    rc = sqlite3_prepare_v2(db, "SELECT question, solution FROM q WHERE id = ?1", -1, &res, &tail);
    if (rc != SQLITE_OK) {
	rollback_transaction("Failed to prepare SELECT ... FROM q", db);
	goto restart_before_transaction;
    }
   
     rc = sqlite3_bind_int(res, 1, qnr);
     if (rc != SQLITE_OK) {
        rollback_transaction("Failed to bind SELECT ... FROM q param. 1", db);
        goto restart_before_transaction;
        }

     while (1) {
       rc = sqlite3_step(res);
       if (rc == SQLITE_ROW) {
	   printf("Question: %s \n", sqlite3_column_text(res, 0));
	   expected_anr = sqlite3_column_int(res, 1);
       }
       else if (rc != SQLITE_DONE) {
           rollback_transaction("Failed to retrieve rows from table q", db);
           goto restart_before_transaction;
        }
       else break;
     }	

    /*
     * retrieve answers
     */

    rc = sqlite3_prepare_v2(db, "SELECT no, answer FROM a WHERE id = ?1", -1, &res, &tail);
    if (rc != SQLITE_OK) {
	rollback_transaction("Failed to prepare SELECT ... FROM a", db);
	goto restart_before_transaction;
    }

    rc = sqlite3_bind_int(res, 1, qnr);
    if (rc != SQLITE_OK) {
        rollback_transaction("Failed to bind SELECT ... FROM a param. 1", db);
        goto restart_before_transaction;
    }

      while (1) {
       rc = sqlite3_step(res);
       if (rc == SQLITE_ROW) {
           printf("Answer %d: %s \n", sqlite3_column_int(res, 0), sqlite3_column_text(res,1));
       }
       else if (rc != SQLITE_DONE) {
           rollback_transaction("Failed to retrieve rows from table a", db);
           goto restart_before_transaction;
        }
       else break;
     }



    /* 
     * read answer
     */
    printf("Enter answer number:");
    if (fgets_with_newline(anrbuf, 3) == NULL) {
	    fprintf(stderr, "fgets anrbuf failed");
	    goto restart_after_transaction;
    }
    errno = 0;
    endptr = buf;
    anr = strtol(anrbuf, &endptr, 10);
    if ((errno == ERANGE && (anr == LONG_MAX || anr == LONG_MIN))
               || (errno != 0 && anr == 0)) {
	 fprintf(stderr, "answer number is not a valid number (errno)\n");
         goto restart_after_transaction;
    }
    if (anrbuf == endptr || *endptr != '\0') {
	 fprintf(stderr, "answer number is not a valid number (endptr)\n");
         goto restart_after_transaction;
    }
    if ( anr <= 0) {
	 fprintf(stderr, "answer number must be >= 1 \n");
         goto restart_after_transaction;
    }
    if ( anr > MAX_ANSWER_NB) {
	 fprintf(stderr, "answer number must be <= %d \n", MAX_ANSWER_NB);
         goto restart_after_transaction;
    }

    if (anr == expected_anr)
	 printf("=> Good answer. \n");
    else 
         printf("=> Bad answer: right answer is: %d.\n", expected_anr);
    /*
     * end transaction 
     */

    rc  = sqlite3_finalize(res);
    if (rc != SQLITE_OK) {
         rollback_transaction("Failed to delete prepared statement SELECT ...", db);
	 goto restart_before_transaction;
	}


    rc = sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        rollback_transaction("COMMIT failed", db);
        sqlite3_close(db);
        return 1;
    }
    printf("Transaction ended.\n") ;


    /*
     * exit
     */

    sqlite3_close(db);
    printf("Disconneced from database.\n") ;
    
    return 0;
}
