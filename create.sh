#!/bin/bash
#
# create.sh
#
# create sqlite3 quiz database
#
# -------------------------------------------
if [ $# -ne 1 ]
then
  echo "Usage: create.sh <database name>"
  exit 1
fi
#
if [ -f $1 ]
then
  echo "$1 already exists."
  exit 1
fi
#
sqlite3 -init initrc.sql $1 < create.sql
CR=$?
#
echo ""
if [ $CR -eq 0 ]
then
  echo "Database $1 created."
else
  echo "Database creation $1 failed."
fi
