#!/usr/bin/env bash

set -x
set -euo pipefail
cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

find trunk/asma -name "*.sap" -exec md5 {} \; > "Asma.txt"
