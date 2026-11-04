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
| `just sanitize <group>` | Configures, builds and tests a sanitizer group (`asan` or `tsan`) in its own `build/debug-<group>` directory. |
| `just sanitize-all` | Runs every sanitizer group supported by the platform, one build after another. |
| `just format` | Formats both C++ and Rust source code. |
| `just format-cpp` | Runs `clang-format` on the `src` and `tests` directories. |
| `just format-rust` | Runs `cargo fmt` inside every `src/rust_*` crate. |
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
| Sanitizers | `ASAN` (AddressSanitizer), `LSAN` (LeakSanitizer), `UBSAN` (UndefinedBehaviorSanitizer), `TSAN` (ThreadSanitizer) |

> ⚠️ **Important:**
> - Passing an invalid or misspelled flag name will trigger a Warning during the CMake configuration step.
> - Incompatible sanitizers (see below) make the CMake configuration step fail.
> - Flags aren't reset by omission on an existing build directory: once enabled, a flag stays `ON` in the CMake cache until you explicitly reconfigure with a fresh build directory or pass it again some other way.

### Sanitizers

The big sanitizers use the program's memory in incompatible ways, so at most one of
ASan and TSan can be enabled in a build. LSan and UBSan are lighter and can be added
on top of some of them:

|       | ASan | LSan | UBSan | TSan |
| :---- | :--: | :--: | :---: | :--: |
| ASan  |  —   | ✅ (part of ASan) | ✅ | ❌ |
| LSan  |  ✅  |  —   |  ✅   |  ❌  |
| UBSan |  ✅  |  ✅  |   —   |  ✅  |
| TSan  |  ❌  |  ❌  |  ✅   |  —   |

That is why there are two sanitizer builds, each in its own directory:

| Group | Sanitizers | Preset | Directory |
| :--- | :--- | :--- | :--- |
| `asan` | ASan + LSan + UBSan | `debug-asan` | `build/debug-asan` |
| `tsan` | TSan | `debug-tsan` | `build/debug-tsan` |

`just sanitize-all` builds and tests both groups one after the other.

> ⚠️ **Notes:**
> - Sanitizers are only applied to **Debug** builds and are disabled on Windows.
> - LSan is not available on macOS; it is skipped there with a warning.
> - UBSan also enables Clang's `integer` and `nullability` checks; well-defined but
>   suspicious patterns in third-party code and system headers are silenced in
>   `cmake/ubsan_ignorelist.txt`.
> - SQLite's lock-free WAL index protocol looks like a data race to TSan; those
>   functions (and only those) are listed in `cmake/tsan_suppressions.txt`.
> - `rust_crypto` is built by Cargo and is not instrumented.
> - MemorySanitizer (MSan) is not supported yet: it needs every library, including
>   the C++ standard library, to be instrumented.

### Examples

**1. Default configuration (No analysis):**
```bash
just config-dev
```

**2. Enable Linting and AddressSanitizer:**
```bash
just config-dev LINT ASAN
```

**3. Run the ASan + LSan + UBSan and TSan builds:**
```bash
just sanitize-all
```
