# Contributing Guide

Thank you for your interest in contributing to this project.

This document focuses **exclusively** on how to write commit messages. A clean, consistent commit history is a hard requirement for this repository.

---

## Commit Message Convention

This project **strictly follows** the **Conventional Commits** specification.

* 📘 Specification: [Conventional Commits](https://www.conventionalcommits.org/)
* 🎯 Goal: readable history, automation-ready, zero ambiguity

If a commit does not follow this convention, it **may be rejected**.

---

## Commit Message Format

```text
<type>(<scope>): <short description>
```

### Examples

```text
feat(auth): add AES-256-GCM encryption
fix(crypto): prevent IV reuse in GCM mode
refactor(core): split storage from crypto layer
build(cmake): add LibreSSL detection
```

---

## Parts Explained

### 1. Type (required)

The **type** describes the *kind of change* introduced by the commit.

| Type       | Meaning       | When to Use It                      |
| ---------- | ------------- | ----------------------------------- |
| `feat`     | Feature       | New functionality or capability     |
| `fix`      | Bug Fix       | Correcting incorrect behavior       |
| `refactor` | Refactor      | Code change without behavior change |
| `perf`     | Performance   | Performance improvements            |
| `test`     | Tests         | Adding or modifying tests           |
| `docs`     | Documentation | README, comments, guides            |
| `build`    | Build System  | CMake, dependencies, toolchains     |
| `ci`       | CI/CD         | GitHub Actions, pipelines           |
| `style`    | Formatting    | Whitespace, formatting, warnings    |
| `chore`    | Maintenance   | Cleanup, scripts, tooling           |

> ⚠️ Do **not** invent new types without discussion.

---

### 2. Scope (optional but strongly recommended)

The **scope** indicates *where* the change applies.

Typical scopes:

* `core`
* `crypto`
* `auth`
* `storage`
* `ui`
* `cmake`
* `docs`

Examples:

```text
fix(storage): handle corrupted database header
feat(ui): add password strength indicator
```

Rules:

* Lowercase
* Short and specific
* One scope only

---

### 3. Short Description (required)

A concise summary of **what the commit does**.

Rules:

* Imperative mood ("add", not "added")
* Lowercase first letter
* No trailing period
* Maximum **72 characters**

❌ Bad examples:

```text
fixed encryption bug.
Added new feature for passwords
```

✅ Good examples:

```text
fix(crypto): validate tag length before decrypt
feat(auth): support multiple key derivation rounds
```

---

## Commit Body (optional but encouraged)

Use the commit body to explain **why** and **how**, not *what*.

```text
feat(storage): encrypt site identifiers at rest

Site names are now encrypted using the master key
with a distinct nonce per entry to avoid correlation
and information leakage.
```

Guidelines:

* Wrap lines at ~72 characters
* Explain motivation and reasoning
* Mention trade-offs if relevant

---

## Breaking Changes

Breaking changes **must be explicit**.

### Syntax

```text
<type>(<scope>)!: <description>
```

### Example

```text
refactor(api)!: remove legacy password format

BREAKING CHANGE: the old password database format
is no longer supported and must be migrated.
```

Rules:

* Use `!` after type or scope
* Add a `BREAKING CHANGE:` section in the body

---

## Reverts

Revert commits should use the `revert` type.

```text
revert: feat(auth): enable biometric unlock
```

The body should explain *why* the revert happened.

---

## What Not to Do

❌ Do not:

* Mix unrelated changes in one commit
* Use vague messages like "fix stuff" or "update"
* Commit generated files unless explicitly required
* Change formatting and logic in the same commit

Each commit should represent **one logical change**.

---

## Commit Granularity

Good commits are:

* Small
* Focused
* Bisectable

If you need to use "and" in the description, the commit
is probably doing too much.

---

## Automation & Tooling

This convention enables:

* 📦 Automatic changelog generation
* 🔢 Semantic versioning
* 🤖 CI validation of commit messages

Future tooling may reject commits that do not comply.

---

## Summary

✔ Follow Conventional Commits
✔ Be explicit and concise
✔ One logical change per commit
✔ When in doubt, add context in the body

A clean commit history is part of the code quality.

---

If you are unsure how to classify a change, open an issue or ask before committing.
