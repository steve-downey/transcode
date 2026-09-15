# Dependabot auto-merge

This local composite Action verifies Dependabot's metadata, applies an update
policy, checks that the repository is safe for native auto-merge, and enables
auto-merge on eligible pull requests. GitHub performs the merge only after the
target branch's required checks pass.

The default policy allows patch and minor updates. A grouped pull request is
classified by the highest semantic-version change in the group, so a group
containing a major update is not eligible.

## Repository prerequisites

Before enabling the workflow:

1. In **Settings > General > Pull Requests**, enable **Allow auto-merge** and
   ensure the selected merge method is allowed.
2. Add a branch protection rule or ruleset for the default branch that requires
   these status checks to pass before merging:

   - `Required CI`
   - `Test System Compiler with Makefile`
   - `dependency-review`
   - `pre-commit / Pre-Commit check on PR`
   - `Analyze (actions)`
   - `Analyze (c-cpp)`
   - `CodeQL`

The Action deliberately fails without both settings. Without required status
checks, GitHub can merge a pull request immediately when auto-merge is enabled.
If branch protection also requires a review, the pull request remains queued
until a reviewer approves it; this Action does not auto-approve changes.

## Local usage

The caller must use `pull_request_target`: Dependabot workflows triggered by
`pull_request` receive a read-only token. Do not check out or execute pull
request code in this privileged workflow.

```yaml
permissions: {}

jobs:
  auto-merge:
    if: github.event.pull_request.user.login == 'dependabot[bot]'
    permissions:
      contents: write
      pull-requests: write
    runs-on: ubuntu-latest
    steps:
      - uses: $/.github/actions/dependabot-auto-merge
        with:
          github-token: ${{ github.token }}
          allowed-update-types: >-
            version-update:semver-patch,version-update:semver-minor
          merge-method: merge
```

## Extraction path

The directory is already a standalone composite Action. To publish it for
reuse, move it to its own repository, add tests and release automation, tag an
immutable release, and replace the local `uses:` value with
`owner/dependabot-auto-merge@<full-commit-sha>`.
