#
# quiz Makefile
#
#
all: insert 
insert: insert.c
	$(CC) -std=gnu99 -Wall -o $@ -lsqlite3 $<
clean: 
	rm -f insert 


