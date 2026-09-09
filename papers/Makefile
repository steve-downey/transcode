# Build the WG21 paper via the mpark/wg21 framework (vendored as a git subtree
# under wg21/). This is a flat layout: transcode-view.md lives here next to
# wg21/, and outputs are written under generated/. See wg21/MANUAL.md,
# "Project Layouts".
#
# Refresh the framework with:
#   git subtree pull --prefix=papers/wg21 https://github.com/mpark/wg21.git master --squash

# Corporate TLS-intercepting networks: point Python / requests / pip at the
# system CA bundle (which carries the intercepting root CA) so the framework's
# wg21.link reference fetch and the venv install validate certificates. Keeping
# these here, in the downstream Makefile, lets the wg21 subtree stay pristine
# instead of patching its sources (which would conflict on every subtree pull).
export SSL_CERT_FILE := /etc/ssl/certs/ca-certificates.crt
export REQUESTS_CA_BUNDLE := /etc/ssl/certs/ca-certificates.crt
export PIP_CERT := /etc/ssl/certs/ca-certificates.crt

include wg21/flat.mk

# The wording clauses are generated from the marked-up headers by
# papers/wording/generate.sh, which also writes wording/wording.mk listing the
# fragments in document order.  base.mk builds its pandoc command from
# `$(filter %.md, $^)`, so naming them as ordered prerequisites is what
# transcludes them: pandoc concatenates the paper and its wording in
# prerequisite order.  These rules are explicit rather than relying on the
# pattern rules in flat.mk, so that the order is stated here and not left to
# make's prerequisite merging.
-include wording/wording.mk

# Everything that has to read *after* the wording lives in parts/tail.md:
# Acknowledgements, Prior Art, Relationship to Other Proposals, References and
# the explicit `::: {#refs}` div citeproc fills.  Prerequisite order is
# document order, so a section cannot be both authored in transcode-view.md and
# printed after the generated clauses -- it has to be in a later file.
# `flat.mk` globs `papers/*.md` and would build any sibling .md as a paper of
# its own, which is why this one is in a subdirectory.
PAPER_TAIL := parts/tail.md

PAPER_SRC := transcode-view.md $(WORDING_MD) $(PAPER_TAIL)

$(OUTDIR)/transcode-view.html: $(PAPER_SRC) $(DEPS) | $(OUTDIR)
	$(PANDOC)

$(OUTDIR)/transcode-view.latex: $(PAPER_SRC) $(DEPS) | $(OUTDIR)
	$(PANDOC)

$(OUTDIR)/transcode-view.pdf: $(PAPER_SRC) $(DEPS) | $(OUTDIR)
	$(PANDOC)
