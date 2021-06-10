#!/usr/bin/env bash

set -euo pipefail
cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

find . -name "*.sap" -exec md5 {} \; > $1.txt
