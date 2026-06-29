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
