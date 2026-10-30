# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Tests for tools/build_site.py."""

import shutil
import sys
from pathlib import Path
from typing import Any

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from build_site import (
    GeneratedParts,
    LinkContext,
    SiteError,
    Sources,
    build_site,
    collect_sources,
    front_matter_field,
    html_title,
    index_markdown,
    is_copied_source,
    is_rendered_source,
    page_output,
    page_title,
    prepare_output,
    rewrite_links,
    rewrite_target,
)

ROOT = Path(__file__).parent.parent.parent


# --------------------------------------------------------------------------
# Which files go on the site


@pytest.mark.parametrize(
    "relative",
    [
        "README.md",
        "CONTRIBUTING.md",
        "docs/iconv-testing.md",
        "docs/plans/phase6-index.md",
        "docs/benchmarks/README.md",
        "papers/README.md",
        "papers/wording/README.md",
    ],
)
def test_rendered_sources(relative: str) -> None:
    assert is_rendered_source(relative)


@pytest.mark.parametrize(
    "relative",
    [
        "CLAUDE.md",  # root Markdown that is not listed
        "docs/mainpage.md",  # the Doxygen main page
        "docs/site/index.md",  # the landing page's own source
        "papers/transcode-view.md",  # the paper: built by mpark/wg21
        "papers/parts/tail.md",
        "papers/TEST.md",
        "papers/wording/transcode.syn.md",  # a generated wording fragment
        "papers/wg21/README.md",  # vendored framework
        "papers/wg21org/AUTHORING.md",
        "papers/generated/transcode-view.md",
        "docs/Doxyfile",  # not Markdown
        "include/beman/transcode/transcode.hpp",
    ],
)
def test_not_rendered_sources(relative: str) -> None:
    assert not is_rendered_source(relative)


@pytest.mark.parametrize(
    "relative,copied",
    [
        ("docs/whatwg/index-big5.txt", True),
        ("docs/whatwg/encodings.json", True),
        ("docs/wpt/textdecoder-eof.any.js", True),
        ("docs/prior-art.bib", True),
        ("docs/Doxyfile", False),
        ("docs/mrdocs.yml", False),
        ("docs/doxygen-awesome.css", False),
        ("docs/site/site.css", False),
        ("papers/wg21/data/favicon.ico", False),
        ("papers/wording/generate.sh", False),
        ("README.md", False),
        ("data/tables/big5.bin", False),
    ],
)
def test_copied_sources(relative: str, copied: bool) -> None:
    assert is_copied_source(relative) is copied


def test_collect_sources_in_this_repository() -> None:
    sources = collect_sources(ROOT)
    assert "README.md" in sources.pages
    assert "docs/wording-outline.md" in sources.pages
    assert "papers/transcode-view.md" not in sources.pages
    assert all(is_rendered_source(p) for p in sources.pages)
    assert all(is_copied_source(p) for p in sources.copied)
    assert sources.pages == sorted(sources.pages, key=_collect_order)


def _collect_order(relative: str) -> tuple[int, str]:
    # Root files first, in their fixed order; then the trees, sorted.
    roots = ["README.md", "CONTRIBUTING.md"]
    return (roots.index(relative), "") if relative in roots else (len(roots), relative)


def test_page_output() -> None:
    assert page_output("docs/plans/phase6-index.md") == "docs/plans/phase6-index.html"
    assert page_output("README.md") == "README.html"


# --------------------------------------------------------------------------
# Titles


def test_page_title_from_heading() -> None:
    text = "<!-- licence -->\n\nSome preamble.\n\n# The Title  \n\n## Not this\n"
    assert page_title(text, "fallback") == "The Title"


def test_page_title_drops_attribute() -> None:
    assert page_title("# beman.transcode {#mainpage}\n", "x") == "beman.transcode"


def test_page_title_from_front_matter() -> None:
    text = (
        '---\ntitle: "Transcoding Text Views"\ndocument: D4246R1\n---\n\n## Abstract\n'
    )
    assert page_title(text, "x") == "Transcoding Text Views"
    assert front_matter_field(text, "document") == "D4246R1"
    assert front_matter_field(text, "missing") is None
    assert front_matter_field("# no front matter\n", "title") is None


def test_page_title_fallback() -> None:
    assert page_title("## only a level-two heading\n", "name.md") == "name.md"


def test_html_title() -> None:
    assert html_title("Wording outline") == "Wording outline - beman.transcode"
    assert html_title("beman.transcode") == "beman.transcode"


# --------------------------------------------------------------------------
# Links


def make_context(tmp_path: Path, page_dir: str, out_dir: str) -> LinkContext:
    (tmp_path / "include/beman").mkdir(parents=True)
    (tmp_path / "include/beman/x.hpp").write_text("")
    (tmp_path / "docs").mkdir()
    (tmp_path / "docs/a.md").write_text("# A\n")
    (tmp_path / "docs/b.md").write_text("# B\n")
    (tmp_path / "docs/data.txt").write_text("")
    (tmp_path / "papers").mkdir()
    (tmp_path / "papers/transcode-view.md").write_text("# P\n")
    site_files = {
        "docs/a.md": "docs/a.html",
        "docs/b.md": "docs/b.html",
        "docs/data.txt": "docs/data.txt",
        "README.md": "README.html",
        "papers/transcode-view.md": "paper/transcode-view.html",
    }
    return LinkContext(
        page_dir=page_dir,
        out_dir=out_dir,
        site_files=site_files,
        root=tmp_path,
        repo_url="https://example.invalid/repo",
        branch="main",
    )


@pytest.mark.parametrize(
    "target,expected",
    [
        ("https://encoding.spec.whatwg.org/", "https://encoding.spec.whatwg.org/"),
        ("mailto:someone@example.invalid", "mailto:someone@example.invalid"),
        ("#ratios", "#ratios"),
        ("/absolute", "/absolute"),
        ("", ""),
        ("b.md", "b.html"),
        ("b.md#section", "b.html#section"),
        ("./b.md", "b.html"),
        ("../README.md", "../README.html"),
        ("data.txt", "data.txt"),
        ("../papers/transcode-view.md", "../paper/transcode-view.html"),
        (
            "../include/beman/x.hpp",
            "https://example.invalid/repo/blob/main/include/beman/x.hpp",
        ),
        ("../include", "https://example.invalid/repo/tree/main/include"),
        ("missing.md", "missing.md"),
        ("../../outside.md", "../../outside.md"),
    ],
)
def test_rewrite_target_from_docs(tmp_path: Path, target: str, expected: str) -> None:
    ctx = make_context(tmp_path, page_dir="docs", out_dir="docs")
    assert rewrite_target(target, ctx) == expected


def test_rewrite_target_from_root(tmp_path: Path) -> None:
    ctx = make_context(tmp_path, page_dir="", out_dir="")
    assert rewrite_target("docs/a.md", ctx) == "docs/a.html"
    assert (
        rewrite_target("papers/transcode-view.md", ctx) == "paper/transcode-view.html"
    )


def test_rewrite_target_relocated_page(tmp_path: Path) -> None:
    # A page published somewhere other than beside its source: links from it
    # are relative to where it is published, not to where its source is.
    ctx = make_context(tmp_path, page_dir="papers", out_dir="paper")
    assert rewrite_target("../docs/a.md", ctx) == "../docs/a.html"


def test_rewrite_links_walks_the_ast(tmp_path: Path) -> None:
    ctx = make_context(tmp_path, page_dir="docs", out_dir="docs")
    ast: dict[str, Any] = {
        "blocks": [
            {
                "t": "Para",
                "c": [
                    {"t": "Link", "c": [["", [], []], [], ["b.md#x", ""]]},
                    {"t": "Image", "c": [["", [], []], [], ["data.txt", "t"]]},
                    {"t": "Str", "c": "text"},
                ],
            },
            {
                "t": "BulletList",
                "c": [
                    [
                        {
                            "t": "Plain",
                            "c": [
                                {
                                    "t": "Link",
                                    "c": [["", [], []], [], ["https://x.invalid/", ""]],
                                }
                            ],
                        }
                    ]
                ],
            },
        ]
    }
    rewrite_links(ast, ctx)
    para = ast["blocks"][0]["c"]
    assert para[0]["c"][2] == ["b.html#x", ""]
    assert para[1]["c"][2] == ["data.txt", "t"]
    assert ast["blocks"][1]["c"][0][0]["c"][0]["c"][2][0] == "https://x.invalid/"


# --------------------------------------------------------------------------
# The index


def test_index_markdown_lists_everything() -> None:
    parts = GeneratedParts(doxygen=Path("d"), mrdocs=Path("m"), paper=Path("p"))
    pages = {
        "README.md": "beman.transcode",
        "docs/b.md": "Bee",
        "docs/a.md": "Ay",
        "docs/plans/README.md": "Plans",
        "docs/plans/x.md": "Ex",
    }
    md = index_markdown("# Intro\n", parts, pages, ["docs/plans/data.txt"], "D1: Paper")
    assert md.startswith("# Intro\n")
    assert "- [Doxygen](doxygen/index.html)" in md
    assert "- [MrDocs](mrdocs/index.html)" in md
    assert (
        "- [D1: Paper](paper/transcode-view.html) ([PDF](paper/transcode-view.pdf))"
        in md
    )
    assert "### Repository root" in md
    assert "### `docs/`" in md
    assert "### `docs/plans/`" in md
    assert (
        md.index("### Repository root")
        < md.index("### `docs/`")
        < md.index("### `docs/plans/`")
    )
    # Alphabetical within a directory, README first.
    assert md.index("[Ay](docs/a.md)") < md.index("[Bee](docs/b.md)")
    assert md.index("[Plans](docs/plans/README.md)") < md.index("[Ex](docs/plans/x.md)")
    assert "- [`data.txt`](docs/plans/data.txt)" in md


def test_index_markdown_says_what_was_not_built() -> None:
    md = index_markdown("intro", GeneratedParts(), {}, [], "Paper")
    assert "The API reference was not built in this run." in md
    assert "The paper was not built in this run." in md
    assert "doxygen/index.html" not in md
    assert "paper/transcode-view.html" not in md


def test_index_markdown_lists_only_what_was_built() -> None:
    md = index_markdown("intro", GeneratedParts(mrdocs=Path("m")), {}, [], "Paper")
    assert "- [MrDocs](mrdocs/index.html)" in md
    assert "Doxygen" not in md
    assert "not built" in md  # the paper


# --------------------------------------------------------------------------
# The output directory


def test_prepare_output_refuses_a_foreign_directory(tmp_path: Path) -> None:
    out = tmp_path / "out"
    out.mkdir()
    (out / "precious").write_text("")
    with pytest.raises(SiteError, match="refusing"):
        prepare_output(out)
    assert (out / "precious").exists()


def test_prepare_output_replaces_a_previous_site(tmp_path: Path) -> None:
    out = tmp_path / "out"
    out.mkdir()
    (out / ".nojekyll").touch()
    (out / "stale.html").write_text("")
    prepare_output(out)
    assert out.is_dir()
    assert not any(out.iterdir())


def test_prepare_output_creates_nested(tmp_path: Path) -> None:
    out = tmp_path / "a" / "b"
    prepare_output(out)
    assert out.is_dir()


# --------------------------------------------------------------------------
# End to end, with pandoc

pandoc_available = pytest.mark.skipif(
    shutil.which("pandoc") is None, reason="pandoc is not installed"
)


def make_repository(root: Path) -> None:
    (root / "docs/site").mkdir(parents=True)
    (root / "docs/sub").mkdir()
    (root / "papers/wording").mkdir(parents=True)
    (root / "papers/wg21").mkdir()
    (root / "include").mkdir()
    (root / "README.md").write_text(
        "# beman.transcode\n\nSee [the guide](docs/guide.md#start) and "
        "[the paper](papers/transcode-view.md).\n"
    )
    (root / "CONTRIBUTING.md").write_text("# Contributing\n\nText.\n")
    (root / "docs/site/index.md").write_text(
        "# Welcome\n\nRead the [README](README.md).\n"
    )
    (root / "docs/site/site.css").write_text("body {}\n")
    (root / "docs/mainpage.md").write_text("# beman.transcode {#mainpage}\n")
    (root / "docs/guide.md").write_text(
        "# The Guide\n\n## Start\n\n[Data](sub/data.txt), [sub](sub/README.md), "
        "[header](../include/x.hpp), [Doxygen main page](mainpage.md).\n"
    )
    (root / "docs/sub/README.md").write_text(
        "# Sub\n\nBack to [the guide](../guide.md).\n"
    )
    (root / "docs/sub/data.txt").write_text("data\n")
    (root / "include/x.hpp").write_text("")
    (root / "papers/transcode-view.md").write_text(
        '---\ntitle: "The Paper"\ndocument: D0000R0\n---\n\n## Abstract\n'
    )
    (root / "papers/README.md").write_text("# Papers\n")
    (root / "papers/wording/README.md").write_text("# Wording\n")
    (root / "papers/wording/clause.md").write_text("::: wording\n")
    (root / "papers/wg21/README.md").write_text("# Framework\n")


def make_generated(root: Path) -> GeneratedParts:
    doxygen = root / "gen/doxygen"
    mrdocs = root / "gen/mrdocs"
    paper = root / "gen/paper"
    for d in (doxygen, mrdocs, paper):
        d.mkdir(parents=True)
    (doxygen / "index.html").write_text("<html>doxygen</html>")
    (doxygen / "x.js").write_text("")
    (mrdocs / "index.html").write_text("<html>mrdocs</html>")
    (paper / "transcode-view.html").write_text("<html>paper</html>")
    (paper / "transcode-view.pdf").write_bytes(b"%PDF-")
    return GeneratedParts(doxygen=doxygen, mrdocs=mrdocs, paper=paper)


@pandoc_available
def test_build_site_end_to_end(tmp_path: Path) -> None:
    root = tmp_path / "repo"
    make_repository(root)
    parts = make_generated(root)
    out = tmp_path / "site"

    sources = build_site(
        root, out, parts, repo_url="https://example.invalid/r", revision="abc"
    )

    assert sources == Sources(
        pages=[
            "README.md",
            "CONTRIBUTING.md",
            "docs/guide.md",
            "docs/sub/README.md",
            "papers/README.md",
            "papers/wording/README.md",
        ],
        copied=["docs/sub/data.txt"],
    )
    assert (out / ".nojekyll").exists()
    assert (out / "site.css").read_text() == "body {}\n"
    assert (out / "doxygen/index.html").exists()
    assert (out / "doxygen/x.js").exists()
    assert (out / "mrdocs/index.html").exists()
    assert (out / "paper/transcode-view.html").exists()
    assert (out / "paper/transcode-view.pdf").exists()
    assert (out / "docs/sub/data.txt").read_text() == "data\n"
    assert not (out / "docs/mainpage.html").exists()
    assert not (out / "papers/wording/clause.html").exists()
    assert not (out / "papers/wg21").exists()

    index = (out / "index.html").read_text()
    assert "<title>beman.transcode documentation</title>" in index
    assert 'href="README.html"' in index
    assert 'href="doxygen/index.html"' in index
    assert 'href="mrdocs/index.html"' in index
    assert ">D0000R0: The Paper</a>" in index
    assert 'href="paper/transcode-view.html"' in index
    assert 'href="paper/transcode-view.pdf"' in index
    assert 'href="docs/guide.html"' in index
    assert 'href="docs/sub/README.html"' in index
    assert 'href="docs/sub/data.txt"' in index
    assert 'href="papers/wording/README.html"' in index
    assert 'href="site.css"' in index

    readme = (out / "README.html").read_text()
    assert "<title>beman.transcode</title>" in readme
    assert 'href="docs/guide.html#start"' in readme
    assert 'href="paper/transcode-view.html"' in readme
    assert 'class="site-nav"' in readme
    assert "https://example.invalid/r/blob/main/README.md" in readme
    assert " at abc." in readme

    guide = (out / "docs/guide.html").read_text()
    assert "<title>The Guide - beman.transcode</title>" in guide
    assert 'href="../site.css"' in guide
    assert 'href="../index.html"' in guide
    assert 'href="sub/data.txt"' in guide
    assert 'href="sub/README.html"' in guide
    assert 'href="https://example.invalid/r/blob/main/include/x.hpp"' in guide
    assert 'href="../doxygen/index.html"' in guide

    sub = (out / "docs/sub/README.html").read_text()
    assert 'href="../guide.html"' in sub
    assert 'href="../../site.css"' in sub


@pandoc_available
def test_build_site_without_generated_parts(tmp_path: Path) -> None:
    root = tmp_path / "repo"
    make_repository(root)
    out = tmp_path / "site"

    build_site(root, out, GeneratedParts())

    index = (out / "index.html").read_text()
    assert "not built in this run" in index
    assert not (out / "doxygen").exists()
    readme = (out / "README.html").read_text()
    # With no paper on the site, a link to its source goes to GitHub.
    assert "/blob/main/papers/transcode-view.md" in readme
    guide = (out / "docs/guide.html").read_text()
    assert "/blob/main/docs/mainpage.md" in guide


@pandoc_available
def test_build_site_rejects_incomplete_generated_parts(tmp_path: Path) -> None:
    root = tmp_path / "repo"
    make_repository(root)
    empty = tmp_path / "empty"
    empty.mkdir()
    with pytest.raises(SiteError, match="index.html"):
        build_site(root, tmp_path / "site1", GeneratedParts(doxygen=empty))
    with pytest.raises(SiteError, match="missing"):
        build_site(root, tmp_path / "site2", GeneratedParts(paper=empty))


@pandoc_available
def test_build_site_rebuilds_in_place(tmp_path: Path) -> None:
    root = tmp_path / "repo"
    make_repository(root)
    out = tmp_path / "site"
    build_site(root, out, GeneratedParts())
    (out / "stale.html").write_text("")
    build_site(root, out, GeneratedParts())
    assert not (out / "stale.html").exists()
    assert (out / "index.html").exists()


def test_build_site_needs_the_landing_page(tmp_path: Path) -> None:
    root = tmp_path / "repo"
    (root / "docs").mkdir(parents=True)
    with pytest.raises(SiteError, match="index.md"):
        build_site(root, tmp_path / "site", GeneratedParts())
