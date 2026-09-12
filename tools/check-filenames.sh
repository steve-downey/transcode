#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Reject filenames that look like command-line options.
#
# A tracked file named `-` sat in the repository root holding a stale wording
# snapshot, committed by accident: a hand-run `specgen render ... -o -`, where
# `-` was meant as "write to stdout" and was taken as a literal filename.
#
# Nothing in the checked-in scripts can produce it -- generate.sh writes to
# `-o "$ir"` and `-o /dev/null` -- so there is no bug to fix, only a mistake
# that is easy to repeat and almost invisible in `git status`, where the entry
# is one character wide.
#
# A leading dash is also a hazard in its own right: every tool that takes the
# file as an argument sees an option instead.

set -euo pipefail

status=0

for path in "$@"; do
    name=${path##*/}
    if [[ $name == -* ]]; then
        echo "check-filenames: '$path' begins with a dash." >&2
        status=1
    fi
done

if [[ $status -ne 0 ]]; then
    echo "" >&2
    echo "A filename starting with '-' is read as an option by most tools." >&2
    echo "This usually means a '-o -' or a redirection captured stdout into a" >&2
    echo "file instead of writing to it.  Rename or remove the file." >&2
fi

exit "$status"
