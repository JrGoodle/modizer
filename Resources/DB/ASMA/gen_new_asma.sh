#!/usr/bin/env bash

set -euo pipefail
cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd trunk/asma
../../gen_asma_md5.sh ../../$1
cd ../..
