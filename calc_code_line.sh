# !/bin/bash

find . \( -path "./deployer/lib" -o -path "./deployer/ref" \) -prune -o -name "*.[c|h|py]" | xargs wc -l
