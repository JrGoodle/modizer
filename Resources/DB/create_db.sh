#!/usr/bin/env bash

set -euo pipefail
cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

rm -f database.modizerdb
rm -f databaseMAIN.modizerdb
sqlite3 databaseMAIN.modizerdb ".read 'create_dbMAIN.sql.txt'"
sqlite3 database.modizerdb ".read 'create_dbUSER.sql.txt'"
