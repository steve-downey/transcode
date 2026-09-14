#!/bin/sh
# papers/wording/inputs-check.sh                                      -*-sh-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Compare the hashes of the headers the wording is generated from against the
# hashes recorded when it was last generated, and decide whether the difference
# is one this repository is willing to merge.
#
# This is the cheap half of the drift gate.  It says the fragments were
# generated from different headers than these; it cannot say the wording is
# wrong.  `make wording-check` answers that, and needs a specgen.
#
# The pending list is why this is a script and not four lines of `diff`.  The
# check over-reports by construction -- an edit that changes no wording still
# moves a hash -- and regenerating needs a tool that links LLVM's Clang front
# end.  Requiring that for every header edit makes the gate expensive enough
# that the honest response is to switch it off.  So a header named in PENDING
# may be ahead of the fragments, and one that is not may not.  The exemption is
# per-file, it is in the diff where a reviewer sees it, and it expires: an entry
# that is no longer needed fails just as loudly as a missing one.
#
# Usage:
#   papers/wording/inputs-check.sh [--committed FILE] [--current FILE]
#                                  [--pending FILE]
#
#   --committed FILE  hashes recorded at generation time
#                     (default: papers/wording/inputs.sha256)
#   --current FILE    hashes of the tree as it stands.  Default: computed by
#                     running `generate.sh --inputs`.  Passing a file is how
#                     the tests drive this without a repository.
#   --pending FILE    the reviewed exemption list
#                     (default: papers/wording/PENDING)
#
# Exit status:
#   0  no drift, or all drift is listed in PENDING
#   1  a header changed without being listed, or a listed header did not change
#   2  usage error, or the current hashes could not be computed

set -eu

# `sort` and `comm` must use the same bytewise ordering.  In particular, some
# UTF-8 locales order punctuation differently, which can make a C-sorted path
# list look unsorted to `comm` and turn an allowed PENDING entry into a false
# failure.
LC_ALL=C
export LC_ALL

usage() {
    sed -n '4,37p' "$0" | sed 's/^# \{0,1\}//'
}

script_dir=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)

committed_file=$script_dir/inputs.sha256
pending_file=$script_dir/PENDING
current_file=

while [ $# -gt 0 ]; do
    case $1 in
    --committed | --current | --pending)
        [ $# -ge 2 ] || {
            echo "inputs-check.sh: $1 needs a file" >&2
            exit 2
        }
        case $1 in
        --committed) committed_file=$2 ;;
        --current) current_file=$2 ;;
        --pending) pending_file=$2 ;;
        esac
        shift
        ;;
    -h | --help)
        usage
        exit 0
        ;;
    *)
        echo "inputs-check.sh: unknown argument '$1'" >&2
        exit 2
        ;;
    esac
    shift
done

work_dir=$(mktemp -d)
trap 'rm -rf "$work_dir"' EXIT INT TERM

if [ -z "$current_file" ]; then
    current_file=$work_dir/current
    "$script_dir/generate.sh" --inputs >"$current_file" || {
        echo "inputs-check.sh: generate.sh --inputs failed; see above" >&2
        exit 2
    }
fi

[ -f "$committed_file" ] || {
    echo "inputs-check.sh: no such file: $committed_file" >&2
    exit 2
}

# The paths whose recorded hash and current hash disagree, plus any path that
# appears in only one of the two lists.  Each line is `<hash>  <path>`, so a
# line present in exactly one file names a path that moved, was added, or was
# removed -- and the path is the last field either way.
sort "$committed_file" >"$work_dir/committed.sorted"
sort "$current_file" >"$work_dir/current.sorted"
comm -3 "$work_dir/committed.sorted" "$work_dir/current.sorted" |
    awk 'NF { print $NF }' | LC_ALL=C sort -u >"$work_dir/changed"

# The pending list, comments and blank lines dropped, trailing space trimmed.
if [ -f "$pending_file" ]; then
    sed -e 's/#.*//' -e 's/[[:space:]]*$//' "$pending_file" |
        awk 'NF' | LC_ALL=C sort -u >"$work_dir/pending"
else
    : >"$work_dir/pending"
fi

comm -23 "$work_dir/changed" "$work_dir/pending" >"$work_dir/unlisted"
comm -13 "$work_dir/changed" "$work_dir/pending" >"$work_dir/stale"
comm -12 "$work_dir/changed" "$work_dir/pending" >"$work_dir/allowed"

status=0

if [ -s "$work_dir/unlisted" ]; then
    {
        echo ""
        echo "A header the wording is generated from has changed, and the"
        echo "committed fragments were generated from the older one:"
        echo ""
        sed 's/^/    /' "$work_dir/unlisted"
        echo ""
        echo "Run 'make wording' and commit the result -- that regenerates the"
        echo "fragments and this file together.  It needs a specgen on PATH;"
        echo "see papers/wording/README.md."
        echo ""
        echo "If the wording is meant to be regenerated later in a series of"
        echo "changes, add the path to papers/wording/PENDING instead.  That is"
        echo "a reviewed exemption, not a way to skip the regeneration: the"
        echo "entry has to come back out before the paper goes out, and"
        echo "'make wording-check' does not honour it."
        echo ""
        echo "This check does not read the fragments.  It says the inputs"
        echo "moved, not that the wording is wrong -- an edit that changes no"
        echo "wording still needs a regeneration to say so."
    } >&2
    status=1
fi

if [ -s "$work_dir/stale" ]; then
    {
        echo ""
        echo "papers/wording/PENDING lists a header that has not changed since"
        echo "the fragments were generated:"
        echo ""
        sed 's/^/    /' "$work_dir/stale"
        echo ""
        echo "The entry is doing nothing, and an exemption nobody notices is"
        echo "how this gate stops working.  Remove it, or -- if the wording was"
        echo "regenerated -- note that 'make wording' empties the list for you."
    } >&2
    status=1
fi

if [ -s "$work_dir/allowed" ]; then
    echo "wording inputs: regeneration pending for these headers, as listed in"
    echo "papers/wording/PENDING:"
    sed 's/^/    /' "$work_dir/allowed"
    echo "Run 'make wording' before the next paper revision."
fi

if [ "$status" -eq 0 ] && [ ! -s "$work_dir/allowed" ]; then
    echo "wording inputs are unchanged since the fragments were generated"
fi

exit "$status"
