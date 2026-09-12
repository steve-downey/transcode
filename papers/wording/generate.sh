#!/bin/sh
# papers/wording/generate.sh                                          -*-sh-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Regenerate the paper's wording fragments from the marked-up headers.
#
# This is the only place beman.specgen is invoked from (see
# docs/plans/phase5-index.md, "Standing conventions").  Everything it writes
# into this directory is generated output: the mpark/wg21 fragments and the
# wording.mk that lists them in document order for papers/Makefile.  Do not
# edit those by hand; edit the markup in the headers and run this again.
#
# Usage:
#   papers/wording/generate.sh [--out DIR] [--validate]
#
#   --out DIR   write the fragments and wording.mk here instead of into
#               papers/wording (used by `make wording-check`, which
#               regenerates into a scratch directory and diffs)
#   --validate  additionally run specgen's wording validators over each
#               header and report the findings.  Findings are printed, not
#               applied: this exits non-zero when any of them is an error, so
#               it is a reporting target and not part of `make wording`.
#
# Environment:
#   SPECGEN                       the specgen executable (default: `specgen`)
#   BEMAN_TRANSCODE_BUILD_INCLUDE the build tree's include directory, which is
#                                 where CMake writes config_generated.hpp.
#                                 Added to the parse only when it exists; the
#                                 umbrella header needs it, the others do not.
#   SPECGEN_GCC_TOOLCHAIN         a GCC installation prefix to parse against.
#                                 Unset by default: Clang selects the newest
#                                 installed GCC on its own, which is GCC 16 on
#                                 a machine set up to build this project.

set -eu

usage() {
    sed -n '3,32p' "$0" | sed 's/^# \{0,1\}//'
}

out_dir=
validate=0
inputs_only=0
authored_only=0
while [ $# -gt 0 ]; do
    case $1 in
    --out)
        [ $# -ge 2 ] || {
            echo "generate.sh: --out needs a directory" >&2
            exit 2
        }
        out_dir=$2
        shift
        ;;
    --validate) validate=1 ;;
    --inputs) inputs_only=1 ;;
    --authored) authored_only=1 ;;
    -h | --help)
        usage
        exit 0
        ;;
    *)
        echo "generate.sh: unknown argument '$1'" >&2
        exit 2
        ;;
    esac
    shift
done

# The files in this directory that are *not* output.  Two things need to know:
# the cleanup below, which would otherwise delete them, and `make
# wording-check`, which diffs this directory against a scratch directory that
# only ever holds output and would otherwise report them missing.  One list, so
# a third authored file breaks neither.  `--authored` is how the Makefile reads
# it.
authored_files() {
    cat <<'FILES'
generate.sh
inputs-check.sh
README.md
specgen-ref
inputs.sha256
PENDING
FILES
}

# PENDING's header, which survives the truncation below.  A file whose only
# content is a list of paths says nothing about why the paths are there, and
# this one needs to say what putting a path in it means.
pending_header() {
    cat <<'HEADER'
# Headers that are knowingly ahead of the committed wording fragments.
#
# One repository-relative path per line.  `#` starts a comment.
#
# A path here tells `make wording-inputs-check` that this header changed on
# purpose and the regeneration is coming later in the series.  A path that is
# not here and has changed anyway fails the check, which is the drift this gate
# is for.
#
# An entry expires.  If a listed header turns out not to have changed, the
# check fails on that too -- an exemption nobody notices is how a gate stops
# working.  `make wording` empties this list, because after a regeneration
# nothing is pending.
#
# This does not defer correctness, only the staleness report.
# `make wording-check` regenerates and diffs, and ignores this file entirely,
# so a paper revision still cannot go out against stale wording.
HEADER
}

if [ "$authored_only" -eq 1 ]; then
    authored_files
    exit 0
fi

script_dir=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH='' cd -- "$script_dir/../.." && pwd)
# Whether this run is the real regeneration or `wording-check`'s scratch one.
# Only the real one may empty PENDING: the scratch run is a comparison, and a
# comparison that mutates the thing it is comparing against is not one.
regenerating_in_place=0
[ -n "$out_dir" ] || regenerating_in_place=1
[ -n "$out_dir" ] || out_dir=$script_dir
mkdir -p "$out_dir"
out_dir=$(CDPATH='' cd -- "$out_dir" && pwd)
out_parent=$(dirname -- "$out_dir")
out_name=$(basename -- "$out_dir")

# The document extent, as specgen defines it (its docs/decisions/document-extent
# ADR): a document is its root file plus the headers `#include`d *inside* the
# gathered `.syn` region.  An include outside the region is implementation and
# cannot reach the wording, so it is not part of the extent and a change to it
# cannot make the fragments stale.
#
# Printing the list is what makes a staleness check possible without the tool:
# `--inputs` hashes exactly the files the wording is generated from.
document_files() {
    while IFS='|' read -r header root; do
        [ -n "$header" ] || continue
        printf '%s\n' "$header"
        awk -v root="$root" '
            index($0, "\\rSec") && index($0, "[" root "]") { inside = 1; next }
            index($0, "END [" root "]") { inside = 0 }
            inside && match($0, /#include <beman\/transcode\/[^>]*>/) {
                inc = substr($0, RSTART, RLENGTH)
                sub(/^#include </, "", inc)
                sub(/>$/, "", inc)
                print "include/" inc
            }
        ' "$repo_root/$header"
    done <<HEADERS
$(spec_headers)
HEADERS
}

# The hashes of everything the wording is generated from, sorted in the C
# locale so the file is stable across filesystems *and* machines -- a collating
# order that ignores punctuation puts codec_concepts.hpp before codec.hpp, and
# the C locale puts `.` before `_`, so an unpinned sort makes this file depend
# on the developer's environment and fail in a container that has none.  Committed as papers/wording/inputs.sha256, which
# is what `make wording-inputs-check` compares against -- a gate that says "the
# fragments were generated from different headers than these", which is the
# question CI can answer without a specgen to answer the stronger one.
wording_input_hashes() {
    (
        cd "$repo_root" || exit 1
        document_files | LC_ALL=C sort -u | xargs --no-run-if-empty sha256sum
    )
}

# The spec-facing headers, one specgen document each, in the order their
# clauses appear in the paper.  Each line is
#
#     <header path relative to the repository root>|<root fragment name>
#
# where the root fragment holds whatever is outside every \rSec section --
# the header synopsis.  Adding a clause means adding markup to a header, and
# adding a header means adding a line here.
# The paper's own stable-name roots.  Every clause under one of them is a clause
# this paper *adds*, so its name is not in the srefs database mpark's `.sref`
# looks up: rendering it as an sref warns once per clause at paper-build time
# and links every cross-reference in the published text to a c++draft page that
# does not exist.  `--new-root` drops the class for a whole subtree at any
# depth, and is given once per root because a document cross-references the
# other header's clauses -- `[transcode.reqs]` cites `[null.term.adaptor]`.
#
# A citation of a clause that *is* in the draft is under neither root, keeps
# its `.sref`, and still resolves.  That is the whole reason this is a list of
# roots rather than a blanket strip.
paper_roots() {
    cat <<'ROOTS'
transcode
null.term
ROOTS
}

spec_headers() {
    cat <<'HEADERS'
include/beman/transcode/transcode.hpp|transcode.syn
include/beman/transcode/null_term.hpp|null.term.syn
HEADERS
}

if [ "$inputs_only" -eq 1 ]; then
    wording_input_hashes
    exit 0
fi

specgen=${SPECGEN:-specgen}
command -v "$specgen" >/dev/null 2>&1 || {
    echo "generate.sh: '$specgen' not found on PATH." >&2
    echo "  beman.specgen builds it; see ~/src/specgen/main/docs/building.md." >&2
    echo "  Set SPECGEN=<path> to use a build tree copy." >&2
    exit 1
}

# The parse tail, in one place.  --no-compile-commands keeps generation
# independent of whatever build database happens to be lying around: the
# wording must not change because someone reconfigured their build tree.
clang_args="-std=c++2c -I $repo_root/include"
build_include=${BEMAN_TRANSCODE_BUILD_INCLUDE:-$repo_root/.build/build-system/include}
[ -d "$build_include" ] && clang_args="$clang_args -I $build_include"
[ -z "${SPECGEN_GCC_TOOLCHAIN:-}" ] || clang_args="$clang_args --gcc-toolchain=$SPECGEN_GCC_TOOLCHAIN"


new_root_args=""
for paper_root in $(paper_roots); do
    new_root_args="$new_root_args --new-root $paper_root"
done

ir_dir=$(mktemp -d)
trap 'rm -rf "$ir_dir"' EXIT INT TERM

# Clear out the previous run's fragments, so a clause that stops being
# generated stops being committed.  Not a blanket `*.md`: the authored files
# live here too, and a wildcard that eats one leaves the directory
# undocumented and the deletion buried in a diff full of regenerated files.
# The script's own arguments are consumed by now, so `set --` is free.
set --
for authored in $(authored_files); do
    set -- "$@" ! -name "$authored"
done
find "$out_dir" -maxdepth 1 -name '*.md' "$@" -delete
rm -f "$out_dir/wording.mk"

manifest=$ir_dir/manifest
: >"$manifest"
findings=0

while IFS='|' read -r header root; do
    [ -n "$header" ] || continue
    ir=$ir_dir/$(echo "$root" | tr '.' '_').json
    # shellcheck disable=SC2086 # clang_args is a deliberate argument list
    "$specgen" generate --emit-ir "$repo_root/$header" --no-compile-commands \
        -o "$ir" -- $clang_args
    (
        cd "$out_parent" || exit 1
        # shellcheck disable=SC2086 # new_root_args is a deliberate argument list
        "$specgen" render --from-ir "$ir" --backend mpark \
            --split "$out_name" --root "$root" $new_root_args
    ) >>"$manifest"
    if [ "$validate" -eq 1 ]; then
        echo "== $header" >&2
        "$specgen" render --from-ir "$ir" --backend mpark --validate \
            -o /dev/null || findings=1
    fi
done <<HEADERS
$(spec_headers)
HEADERS

wording_input_hashes >"$out_dir/inputs.sha256"

# The manifest is document order, and document order is the order pandoc has
# to concatenate the fragments in, so it is what papers/Makefile consumes.
{
    echo "# Generated by papers/wording/generate.sh -- do not edit."
    echo "# The paper's wording fragments, in document order."
    echo "WORDING_MD := \\"
    sed -e 's/^/\t/' -e 's/$/ \\/' -e '$ s/ \\$//' "$manifest"
} >"$out_dir/wording.mk"

# The fragments are now current, so nothing is pending.  Emptying the list here
# rather than asking a contributor to remember is what keeps an exemption from
# outliving the reason for it.
if [ "$regenerating_in_place" -eq 1 ]; then
    pending_header >"$script_dir/PENDING"
fi

if [ "$validate" -eq 1 ] && [ "$findings" -ne 0 ]; then
    echo "generate.sh: validation reported errors (see above)" >&2
    exit 1
fi
