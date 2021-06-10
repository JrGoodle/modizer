#!/usr/bin/env bash

set -x
set -euo pipefail
cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

svn co svn://asma.scene.pl/asma/trunk
