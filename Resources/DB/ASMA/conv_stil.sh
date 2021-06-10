#!/usr/bin/env bash

set -x
set -euo pipefail
cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

awk -f stil.awk trunk/asma/Docs/STIL.txt > stilconverted_asma
