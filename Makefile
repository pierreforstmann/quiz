#
# quiz Makefile
#
#
all: insert select check
	chmod u+x create.sh
insert: insert.c
	$(CC) -std=gnu99 -Wall -o $@ -lsqlite3 $<
select: select.c
	$(CC) -std=gnu99 -Wall -o $@ -lsqlite3 $<
check: check.c
	$(CC) -std=gnu99 -Wall -o $@ -lsqlite3 $<
clean: 
	rm -f insert select check


