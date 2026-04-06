# Password Manager

## Dependencies

- `clang`
- `cmake`
- `ninja`

---

## Dev Dependencies

[Dockerfile](.devcontainer/Dockerfile)

---

## Just commands

| Command | Description |
| :--- | :--- |
| `just build-install` | Full pipeline: Configures, builds, and installs the **Release** version. |
| `just config` | Configures the project with the **Release** preset. |
| `just config-dev` | Configures the project with the **Debug** preset. |
| `just build` | Compiles the project in **Release** mode. |
| `just build-dev` | Compiles the project in **Debug** mode. |
| `just install` | Installs the **Release** binaries. |
| `just install-dev` | Installs the **Debug** binaries. |
| `just clean` | Removes `build`, `install`, `docs`, and Rust target folders. |
| `just test` | Runs the test suite in **Release** mode. |
| `just test-dev` | Runs the test suite in **Debug** mode. |
| `just format` | Formats both C++ and Rust source code. |
| `just format-cpp` | Runs `clang-format` on `core` and `tests` directories. |
| `just format-rust` | Runs `cargo fmt` inside the `crypto` directory. |
| `just docs` | Generates HTML documentation via **Doxygen** (includes Graphviz support if found). |

---

## Advanced Debugging Flags

When running `just config-dev`, you can enable specific analysis tools by passing parameters (`1` to enable, `0` to disable).

Order: `coverage` `lint` `asan` `valgrind`

**Example: lint & asan**
```bash
just config-dev 0 1 1 0
```
