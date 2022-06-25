#
# quiz Makefile
#
#
all: insert 
	chmod u+x create.sh
insert: insert.c
	$(CC) -std=gnu99 -Wall -o $@ -lsqlite3 $<
clean: 
	rm -f insert 


