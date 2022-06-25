#!/bin/bash
#
# create.sh
#
# create sqlite3 quiz database
#
sqlite3 -init initrc.sql quiz.db < quiz.sql
if [ $? -eq 0 ]
then
  echo "Database quiz.db created"
else
  echo "Database creation quiz.db failed"
fi
