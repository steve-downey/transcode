#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Assemble the documentation site that GitHub Pages publishes.

The site is one directory::

    index.html        landing page: docs/site/index.md, then generated sections
                      linking everything below
    doxygen/          the Doxygen rendering of the API      (--doxygen DIR)
    mrdocs/           the MrDocs rendering of the API       (--mrdocs DIR)
    paper/            the rendered paper, HTML and PDF      (--paper DIR)
    README.html       rendered from README.md; likewise CONTRIBUTING.html
    docs/**/*.html    rendered from docs/**/*.md, with the data files beside
    papers/**/*.html  rendered from the Markdown that is not the paper itself
    site.css          the stylesheet of the rendered pages

Every Markdown document is rendered with pandoc.  A relative link from one
document to another becomes a link between the rendered pages, and a relative
link to a repository file that is not on the site goes to that file on GitHub,
so nothing a document points at is a dead end.

The three generated trees are optional so that the Markdown part of the site
can be built on a machine without Doxygen or MrDocs; the index lists only what
was given.  The workflow gives all three.
"""

from __future__ import annotations

import argparse
import json
import posixpath
import re
import shutil
import subprocess
import sys
import tempfile
from dataclasses import dataclass, field
from pathlib import Path, PurePosixPath
from typing import Any

#: Markdown at the repository root that is rendered.
RENDERED_ROOT_FILES = ("README.md", "CONTRIBUTING.md")

#: Directory trees whose Markdown is rendered and whose data files are copied.
RENDERED_TREES = ("docs", "papers")

#: Subtrees left out: vendored frameworks, build output, and the site's own
#: sources, which are consumed by this tool rather than published as pages.
EXCLUDED_DIRS = (
    "docs/site",
    "papers/generated",
    "papers/wg21",
    "papers/wg21org",
)

#: Markdown that is input to something else and renders wrongly on its own.
#: The paper and its tail are built by the mpark/wg21 framework and published
#: under paper/; the Doxygen main page is published under doxygen/.
EXCLUDED_FILES = frozenset(
    {
        "docs/mainpage.md",
        "papers/TEST.md",
        "papers/parts/tail.md",
        "papers/transcode-view.md",
    }
)

#: The generated wording fragments are mpark/wg21 Markdown that only reads
#: correctly inside the paper, where they are.  README.md there is prose.
WORDING_DIR = "papers/wording"

#: Non-Markdown files in the rendered trees that are copied beside the pages,
#: so that the source data the documents describe is reachable too.
COPIED_SUFFIXES = frozenset({".bib", ".js", ".json", ".pdf", ".png", ".svg", ".txt"})

#: What the paper build leaves in its output directory.
PAPER_HTML = "transcode-view.html"
PAPER_PDF = "transcode-view.pdf"
PAPER_SOURCE = "papers/transcode-view.md"

#: Repository-relative source files that are published under another name,
#: so a link to them lands on the rendered thing rather than on GitHub.
RELOCATED_SOURCES = {
    PAPER_SOURCE: f"paper/{PAPER_HTML}",
    "docs/mainpage.md": "doxygen/index.html",
}

LANDING_SOURCE = "docs/site/index.md"
STYLESHEET_SOURCE = "docs/site/site.css"
STYLESHEET = "site.css"

DEFAULT_REPO_URL = "https://github.com/steve-downey/transcode"
DEFAULT_BRANCH = "main"

#: Marker that an output directory was made by this tool and may be replaced.
MARKER = ".nojekyll"

_SCHEME = re.compile(r"^[A-Za-z][A-Za-z0-9+.-]*:")
_HEADING = re.compile(r"^#\s+(?P<title>.+?)\s*(?:\{#[^}]*\})?\s*$")
_FRONT_MATTER_FIELD = re.compile(r"^(?P<key>[A-Za-z_-]+):\s*(?P<value>.+?)\s*$")


class SiteError(Exception):
    """A condition the tool reports instead of raising a traceback for."""


# --------------------------------------------------------------------------
# Sources


def is_excluded_path(relative: str) -> bool:
    """Whether a repository-relative path lies in an excluded subtree."""
    return any(relative == d or relative.startswith(d + "/") for d in EXCLUDED_DIRS)


def is_rendered_source(relative: str) -> bool:
    """Whether the repository-relative Markdown file becomes a page."""
    if not relative.endswith(".md"):
        return False
    if relative in RENDERED_ROOT_FILES:
        return True
    if "/" not in relative:
        return False
    if is_excluded_path(relative) or relative in EXCLUDED_FILES:
        return False
    if relative.startswith(WORDING_DIR + "/"):
        return relative == WORDING_DIR + "/README.md"
    return relative.split("/", 1)[0] in RENDERED_TREES


def is_copied_source(relative: str) -> bool:
    """Whether the repository-relative file is copied beside the pages."""
    if "/" not in relative or is_excluded_path(relative):
        return False
    if relative.split("/", 1)[0] not in RENDERED_TREES:
        return False
    return PurePosixPath(relative).suffix in COPIED_SUFFIXES


@dataclass
class Sources:
    """The repository files that go on the site, repository-relative."""

    pages: list[str] = field(default_factory=list)
    copied: list[str] = field(default_factory=list)


def collect_sources(root: Path) -> Sources:
    """Find every file under *root* that is rendered or copied."""
    sources = Sources()
    for name in RENDERED_ROOT_FILES:
        if (root / name).is_file():
            sources.pages.append(name)
    for tree in RENDERED_TREES:
        top = root / tree
        if not top.is_dir():
            continue
        for path in sorted(top.rglob("*")):
            if not path.is_file():
                continue
            relative = path.relative_to(root).as_posix()
            if is_rendered_source(relative):
                sources.pages.append(relative)
            elif is_copied_source(relative):
                sources.copied.append(relative)
    return sources


def page_output(relative: str) -> str:
    """The site-relative path of the page rendered from a Markdown source."""
    return relative[: -len(".md")] + ".html"


def page_title(text: str, fallback: str) -> str:
    """The first level-one heading of a Markdown document, or *fallback*.

    A trailing Doxygen/pandoc attribute such as ``{#mainpage}`` is dropped.
    A pandoc YAML front matter ``title:`` counts when there is no heading.
    """
    front_matter_title: str | None = None
    lines = text.splitlines()
    if lines and lines[0].strip() == "---":
        for line in lines[1:]:
            if line.strip() == "---":
                break
            m = _FRONT_MATTER_FIELD.match(line)
            if m and m.group("key") == "title":
                front_matter_title = m.group("value").strip("\"'")
    for line in lines:
        m = _HEADING.match(line)
        if m:
            return m.group("title")
    return front_matter_title or fallback


def front_matter_field(text: str, key: str) -> str | None:
    """A scalar field of a pandoc YAML front matter block, if present."""
    lines = text.splitlines()
    if not lines or lines[0].strip() != "---":
        return None
    for line in lines[1:]:
        if line.strip() == "---":
            return None
        m = _FRONT_MATTER_FIELD.match(line)
        if m and m.group("key") == key:
            return m.group("value").strip("\"'")
    return None


# --------------------------------------------------------------------------
# Links


def split_target(target: str) -> tuple[str, str]:
    """Split a link target into its path and its ``#fragment`` or ``?query``."""
    for i, ch in enumerate(target):
        if ch in "#?":
            return target[:i], target[i:]
    return target, ""


def resolve_relative(page_dir: str, target_path: str) -> str | None:
    """A relative link's repository-relative path, or None if it leaves it."""
    joined = posixpath.normpath(posixpath.join(page_dir, target_path))
    if joined == "." or joined.startswith("../") or joined == "..":
        return None
    return joined


def relative_href(from_dir: str, to_path: str) -> str:
    """The href from a page in site directory *from_dir* to site file *to_path*."""
    return posixpath.relpath(to_path, from_dir or ".")


@dataclass(frozen=True)
class LinkContext:
    """What rewriting a page's links needs to know."""

    page_dir: str
    """Repository-relative directory of the page's source ('' at the root)."""
    out_dir: str
    """Site-relative directory of the rendered page ('' at the root)."""
    site_files: dict[str, str]
    """Repository-relative source -> site-relative published path."""
    root: Path
    """The repository, for deciding whether an unpublished target exists."""
    repo_url: str
    branch: str


def rewrite_target(target: str, ctx: LinkContext) -> str:
    """The href a link target should carry on the rendered page.

    External links, in-page anchors and absolute paths are left alone.  A
    relative link to a published source becomes a relative link to the
    published page or file.  A relative link to a repository file that is not
    published goes to that file on GitHub.  Anything else is left as it is.
    """
    if not target or _SCHEME.match(target) or target[0] in "#/?":
        return target
    path, suffix = split_target(target)
    if not path:
        return target
    resolved = resolve_relative(ctx.page_dir, path)
    if resolved is None:
        return target
    published = ctx.site_files.get(resolved)
    if published is not None:
        return relative_href(ctx.out_dir, published) + suffix
    candidate = ctx.root / resolved
    if candidate.is_dir():
        return f"{ctx.repo_url}/tree/{ctx.branch}/{resolved}{suffix}"
    if candidate.is_file():
        return f"{ctx.repo_url}/blob/{ctx.branch}/{resolved}{suffix}"
    return target


def rewrite_links(node: Any, ctx: LinkContext) -> None:
    """Rewrite every Link and Image target in a pandoc JSON AST, in place."""
    if isinstance(node, dict):
        if node.get("t") in ("Link", "Image"):
            contents = node["c"]
            target_pair = contents[2]
            target_pair[0] = rewrite_target(target_pair[0], ctx)
        for value in node.values():
            rewrite_links(value, ctx)
    elif isinstance(node, list):
        for item in node:
            rewrite_links(item, ctx)


# --------------------------------------------------------------------------
# Rendering


def run_pandoc(pandoc: str, args: list[str], stdin: str) -> str:
    result = subprocess.run(
        [pandoc, *args],
        input=stdin,
        capture_output=True,
        text=True,
        check=False,
    )
    if result.returncode != 0:
        raise SiteError(f"pandoc {' '.join(args)} failed:\n{result.stderr}")
    return result.stdout


def markdown_to_ast(pandoc: str, markdown: str) -> Any:
    return json.loads(run_pandoc(pandoc, ["-f", "gfm", "-t", "json"], markdown))


def html_title(title: str) -> str:
    """The browser-tab title of a page: its heading, then the project."""
    if title == "beman.transcode":
        return title
    return f"{title} - beman.transcode"


def nav_html(root_href: str, parts: dict[str, str], source: str | None) -> str:
    """The bar at the top of every rendered page.

    *parts* maps a label to a site-relative path, for the generated trees that
    are present.  *source* is the repository path the page was rendered from.
    """
    links = [f'<a href="{root_href}index.html">beman.transcode</a>']
    for label, path in parts.items():
        links.append(f'<a href="{root_href}{path}">{label}</a>')
    where = f'<span class="site-nav-path">{source}</span>' if source else ""
    return f'<nav class="site-nav">{" ".join(links)} {where}</nav>\n'


def footer_html(source: str | None, repo_url: str, branch: str, revision: str) -> str:
    rendered = (
        f'Rendered from <a href="{repo_url}/blob/{branch}/{source}">{source}</a>'
        if source
        else "Generated"
    )
    at = f" at {revision}" if revision else ""
    return f'<footer class="site-footer">{rendered}{at}.</footer>\n'


def render_page(
    pandoc: str,
    markdown: str,
    *,
    ctx: LinkContext,
    title: str,
    nav: str,
    footer: str,
    toc: bool,
    workdir: Path,
) -> str:
    """Render one Markdown document to a standalone HTML page."""
    ast = markdown_to_ast(pandoc, markdown)
    rewrite_links(ast, ctx)
    nav_file = workdir / "nav.html"
    footer_file = workdir / "footer.html"
    nav_file.write_text(nav, encoding="utf-8")
    footer_file.write_text(footer, encoding="utf-8")
    args = [
        "-f",
        "json",
        "-t",
        "html5",
        "--standalone",
        "--css",
        relative_href(ctx.out_dir, STYLESHEET),
        "--metadata",
        f"pagetitle={title}",
        "--include-before-body",
        str(nav_file),
        "--include-after-body",
        str(footer_file),
    ]
    if toc:
        args += ["--toc", "--toc-depth=2"]
    return run_pandoc(pandoc, args, json.dumps(ast))


# --------------------------------------------------------------------------
# The index


@dataclass
class GeneratedParts:
    """The generated trees given on the command line, if any."""

    doxygen: Path | None = None
    mrdocs: Path | None = None
    paper: Path | None = None

    def nav_parts(self) -> dict[str, str]:
        parts: dict[str, str] = {}
        if self.doxygen is not None:
            parts["Doxygen"] = "doxygen/index.html"
        if self.mrdocs is not None:
            parts["MrDocs"] = "mrdocs/index.html"
        if self.paper is not None:
            parts["Paper"] = f"paper/{PAPER_HTML}"
        return parts


def group_by_directory(relatives: list[str]) -> dict[str, list[str]]:
    groups: dict[str, list[str]] = {}
    for relative in relatives:
        directory = posixpath.dirname(relative)
        groups.setdefault(directory, []).append(relative)
    return groups


def index_markdown(
    intro: str,
    parts: GeneratedParts,
    pages: dict[str, str],
    copied: list[str],
    paper_title: str,
) -> str:
    """The landing page: the authored intro, then the generated sections.

    *pages* maps each rendered source to its title.  Links are written as
    repository paths; the renderer maps them to the published pages, the same
    way it does for every other document.
    """
    out = [intro.rstrip(), ""]

    out += ["## API reference", ""]
    if parts.doxygen is None and parts.mrdocs is None:
        out.append("The API reference was not built in this run.")
    if parts.doxygen is not None:
        out.append(
            "- [Doxygen](doxygen/index.html): the headers and examples, with "
            "class and file indexes"
        )
    if parts.mrdocs is not None:
        out.append(
            "- [MrDocs](mrdocs/index.html): the same headers rendered by "
            "MrDocs, one page per symbol"
        )
    out.append("")

    out += ["## Paper", ""]
    if parts.paper is None:
        out.append("The paper was not built in this run.")
    else:
        out.append(f"- [{paper_title}](paper/{PAPER_HTML}) ([PDF](paper/{PAPER_PDF}))")
    out.append("")

    out += ["## Documents", ""]
    groups = group_by_directory(list(pages))
    copied_groups = group_by_directory(copied)
    for directory in sorted(set(groups) | set(copied_groups), key=_directory_key):
        heading = f"`{directory}/`" if directory else "Repository root"
        out += [f"### {heading}", ""]
        for relative in sorted(groups.get(directory, []), key=_page_key):
            out.append(f"- [{pages[relative]}]({relative})")
        for relative in sorted(copied_groups.get(directory, [])):
            out.append(f"- [`{posixpath.basename(relative)}`]({relative})")
        out.append("")
    return "\n".join(out)


def _directory_key(directory: str) -> tuple[int, str]:
    return (0 if not directory else 1, directory)


def _page_key(relative: str) -> tuple[int, str]:
    """README first within a directory, then the rest alphabetically."""
    name = posixpath.basename(relative)
    return (0 if name == "README.md" else 1, name.lower())


# --------------------------------------------------------------------------
# Assembly


def prepare_output(out: Path) -> None:
    """Make *out* an empty directory, replacing only a previous site."""
    if out.exists():
        if not out.is_dir():
            raise SiteError(f"{out} exists and is not a directory")
        if any(out.iterdir()) and not (out / MARKER).exists():
            raise SiteError(
                f"{out} is not empty and was not made by this tool "
                f"(no {MARKER}); refusing to replace it"
            )
        shutil.rmtree(out)
    out.mkdir(parents=True)


def copy_tree(src: Path, dst: Path, what: str) -> None:
    if not src.is_dir():
        raise SiteError(f"{what}: {src} is not a directory")
    if not (src / "index.html").is_file():
        raise SiteError(f"{what}: {src} holds no index.html")
    shutil.copytree(src, dst)


def copy_paper(src: Path, dst: Path) -> None:
    if not src.is_dir():
        raise SiteError(f"paper: {src} is not a directory")
    dst.mkdir()
    for name in (PAPER_HTML, PAPER_PDF):
        if not (src / name).is_file():
            raise SiteError(f"paper: {src / name} is missing")
        shutil.copy2(src / name, dst / name)


def build_site(
    root: Path,
    out: Path,
    parts: GeneratedParts,
    *,
    pandoc: str = "pandoc",
    repo_url: str = DEFAULT_REPO_URL,
    branch: str = DEFAULT_BRANCH,
    revision: str = "",
) -> Sources:
    """Assemble the site under *out* from the repository at *root*."""
    landing = root / LANDING_SOURCE
    stylesheet = root / STYLESHEET_SOURCE
    for required in (landing, stylesheet):
        if not required.is_file():
            raise SiteError(f"{required} is missing")

    sources = collect_sources(root)
    site_files: dict[str, str] = {}
    for relative in sources.pages:
        site_files[relative] = page_output(relative)
    for relative in sources.copied:
        site_files[relative] = relative
    if parts.paper is not None:
        site_files[PAPER_SOURCE] = RELOCATED_SOURCES[PAPER_SOURCE]
    if parts.doxygen is not None:
        site_files["docs/mainpage.md"] = RELOCATED_SOURCES["docs/mainpage.md"]
    site_files[LANDING_SOURCE] = "index.html"

    prepare_output(out)
    (out / MARKER).touch()
    shutil.copy2(stylesheet, out / STYLESHEET)
    if parts.doxygen is not None:
        copy_tree(parts.doxygen, out / "doxygen", "doxygen")
    if parts.mrdocs is not None:
        copy_tree(parts.mrdocs, out / "mrdocs", "mrdocs")
    if parts.paper is not None:
        copy_paper(parts.paper, out / "paper")
    for relative in sources.copied:
        target = out / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(root / relative, target)

    nav_parts = parts.nav_parts()
    titles: dict[str, str] = {}
    with tempfile.TemporaryDirectory() as tmp:
        workdir = Path(tmp)
        for relative in sources.pages:
            text = (root / relative).read_text(encoding="utf-8")
            title = page_title(text, posixpath.basename(relative))
            titles[relative] = title
            published = site_files[relative]
            out_dir = posixpath.dirname(published)
            ctx = LinkContext(
                page_dir=posixpath.dirname(relative),
                out_dir=out_dir,
                site_files=site_files,
                root=root,
                repo_url=repo_url,
                branch=branch,
            )
            root_href = relative_href(out_dir, ".")
            root_href = "" if root_href == "." else root_href + "/"
            html = render_page(
                pandoc,
                text,
                ctx=ctx,
                title=html_title(title),
                nav=nav_html(root_href, nav_parts, relative),
                footer=footer_html(relative, repo_url, branch, revision),
                toc=True,
                workdir=workdir,
            )
            target = out / published
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_text(html, encoding="utf-8")

        paper_title = "Transcoding Text Views"
        paper_source = root / PAPER_SOURCE
        if paper_source.is_file():
            paper_text = paper_source.read_text(encoding="utf-8")
            document = front_matter_field(paper_text, "document")
            paper_title = front_matter_field(paper_text, "title") or paper_title
            if document:
                paper_title = f"{document}: {paper_title}"
        intro = landing.read_text(encoding="utf-8")
        markdown = index_markdown(intro, parts, titles, sources.copied, paper_title)
        ctx = LinkContext(
            page_dir="",
            out_dir="",
            site_files=site_files,
            root=root,
            repo_url=repo_url,
            branch=branch,
        )
        html = render_page(
            pandoc,
            markdown,
            ctx=ctx,
            title="beman.transcode documentation",
            nav=nav_html("", nav_parts, None),
            footer=footer_html(None, repo_url, branch, revision),
            toc=False,
            workdir=workdir,
        )
        (out / "index.html").write_text(html, encoding="utf-8")
    return sources


# --------------------------------------------------------------------------
# Command line


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Assemble the documentation site for GitHub Pages.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument(
        "--root",
        type=Path,
        default=Path(__file__).resolve().parent.parent,
        help="repository root (default: the checkout this script is in)",
    )
    parser.add_argument(
        "--out",
        type=Path,
        required=True,
        help="site directory to write; replaced if this tool made it",
    )
    parser.add_argument(
        "--doxygen", type=Path, help="Doxygen HTML output to publish as doxygen/"
    )
    parser.add_argument(
        "--mrdocs", type=Path, help="MrDocs HTML output to publish as mrdocs/"
    )
    parser.add_argument(
        "--paper",
        type=Path,
        help=f"directory holding {PAPER_HTML} and {PAPER_PDF}, published as paper/",
    )
    parser.add_argument("--pandoc", default="pandoc", help="pandoc executable")
    parser.add_argument(
        "--repo-url",
        default=DEFAULT_REPO_URL,
        help="repository on GitHub, for links to unpublished files",
    )
    parser.add_argument(
        "--branch", default=DEFAULT_BRANCH, help="branch those links point at"
    )
    parser.add_argument(
        "--revision", default="", help="revision named in the page footers"
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(sys.argv[1:] if argv is None else argv)
    if shutil.which(args.pandoc) is None:
        print(f"build_site: {args.pandoc} not found", file=sys.stderr)
        return 2
    parts = GeneratedParts(doxygen=args.doxygen, mrdocs=args.mrdocs, paper=args.paper)
    try:
        sources = build_site(
            args.root.resolve(),
            args.out,
            parts,
            pandoc=args.pandoc,
            repo_url=args.repo_url.rstrip("/"),
            branch=args.branch,
            revision=args.revision,
        )
    except SiteError as e:
        print(f"build_site: {e}", file=sys.stderr)
        return 1
    print(
        f"build_site: {len(sources.pages)} pages and {len(sources.copied)} files "
        f"written to {args.out}"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
