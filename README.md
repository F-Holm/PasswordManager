# Password Manager

## Build and Install

Run the following command from the project root to build and install the application:

Note: Ensure all [Build Dependencies](#build-dependencies) are installed before running this command.

```bash
just build-install
```

## Build Dependencies

- `Clang`
- `CMake`
- `Ninja-Build`
- `Just`
- `Rustup`

---

## Dev Dependencies

- `Clang`
- `Clangd`
- `LLDB`
- `LLD`
- `LLVM`
- `Clang-Format`
- `Clang-Tidy`
- `CMake`
- `Ninja-Build`
- `Cppcheck`
- `Ccache`
- `Doxygen`
- `Graphviz`
- `lcov`
- `Just`
- `Rustup`
- `Rustfmt`
- `Rust Clippy`

---

## Just commands

| Command | Description |
| :--- | :--- |
| `just build-install` | Full pipeline without tests: Configures, builds, and installs the **Release** version. |
| `just build-install-ci` | Full pipeline: Configures, builds, test, and installs the **Release** version. |
| `just config` | Configures the project with the **Release** preset. |
| `just config-dev` | Configures the project with the **Debug** preset. |
| `just config-dev [FLAGS...]` | Configures the project with the **Debug** preset and optional instrumentation. |
| `just build` | Compiles the project in **Release** mode. |
| `just build-dev` | Compiles the project in **Debug** mode. |
| `just install` | Installs the **Release** binaries in the `install` folder. |
| `just install-dev` | Installs the **Debug** binaries in the `install` folder. |
| `just package` | Generates the **Release** installers in the `package` folder. |
| `just package-dev` | Generates the **Debug** installers in the `package` folder. |
| `just clean` | Removes `build`, `install`, `docs`, and Rust target folders. |
| `just test` | Runs the test suite in **Release** mode. |
| `just test-dev` | Runs the test suite in **Debug** mode. |
| `just format` | Formats both C++ and Rust source code. |
| `just format-cpp` | Runs `clang-format` on `core` and `tests` directories. |
| `just format-rust` | Runs `cargo fmt` inside the `crypto` directory. |
| `just docs` | Generates HTML documentation via **Doxygen** (includes Graphviz support if found). |

---

## Advanced Debugging Flags

When running `just config-dev`, you can enable code coverage, static analysis (linting), and multiple dynamic analysis tools simultaneously.

### Usage
```bash
just config-dev [FLAGS...]
```
- `FLAGS...`: A space-separated list of flags in **UPPERCASE** (can receive 0, 1, or multiple). Each flag `X` maps directly to CMake's `-DENABLE_X=ON`; a flag that isn't passed stays at its default (`OFF`).

### Available Flags

| Category | Flags |
| :--- | :--- |
| Instrumentation | `LINT`, `COVERAGE` |
| Sanitizer | `ASAN` (AddressSanitizer) |

> ⚠️ **Important:**
> - Passing an invalid or misspelled flag name will trigger a Warning during the CMake configuration step.
> - Flags aren't reset by omission on an existing build directory: once enabled, a flag stays `ON` in the CMake cache until you explicitly reconfigure with a fresh build directory or pass it again some other way.

### Examples

**1. Default configuration (No analysis):**
```bash
just config-dev
```

**2. Enable Linting and AddressSanitizer:**
```bash
just config-dev LINT ASAN
```
