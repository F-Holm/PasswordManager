files_to_clean := "build install package docs src/rust_*/Cargo.lock src/rust_*/target"

docs_out   := "docs"
docs_input := "README.md src"
project_name := "Password Manager"

build-install: config build install package

build-install-ci: config build test install package

config:
    cmake --preset release

config-dev *flags:
    cmake --preset debug \
          {{ if flags == "" { "" } else { "-DENABLE_" + replace(flags, " ", "=ON -DENABLE_") + "=ON" } }}

build:
    cmake --build --preset build-release --parallel {{ num_cpus() }}

build-dev:
    cmake --build --preset build-debug --parallel {{ num_cpus() }}

test:
    ctest --preset test-release -j{{ num_cpus() }}

test-dev:
    ctest --preset test-debug -j{{ num_cpus() }}

# Build and test one sanitizer group in build/debug-<group>: asan (ASan+LSan+UBSan) or tsan
sanitize group:
    cmake --preset debug-{{ group }}
    cmake --build --preset build-debug-{{ group }} --parallel {{ num_cpus() }}
    ctest --preset test-debug-{{ group }} -j{{ num_cpus() }}

# Runs every sanitizer group supported by the current platform.
[unix]
sanitize-all: (sanitize "asan") (sanitize "tsan")

[windows]
sanitize-all:
    @echo "Sanitizers are not supported on Windows"

install:
    cmake --install --preset install-release

install-dev:
    cmake --install --preset install-debug

package:
    cmake --package --preset package-release

package-dev:
    cmake --package --preset package-debug

[unix]
clean:
    @echo "Cleaning project files..."
    rm -rf {{ files_to_clean }}

[windows]
clean:
    @echo "Cleaning project files..."
    powershell -Command "Get-Item {{ files_to_clean }} -ErrorAction SilentlyContinue | Remove-Item -Recurse -Force"

format: format-cpp format-rust
    @echo "All the code has been formated!"

[unix]
format-cpp:
    @echo "Formating C++ in: src tests..."
    find src tests -type f -regextype posix-extended -regex '.*\.(c|cpp|c\+\+|h|hpp|hh|cc)$' -exec clang-format -i {} +

[windows]
format-cpp:
    @echo "Formating C++ in: src tests..."
    powershell -Command "foreach ($dir in 'src tests'.Split(' ')) { Get-ChildItem -Path ./$dir -Include *.c,*.cpp,*.c++,*.h,*.hpp,*.hh,*.cc -Recurse -ErrorAction SilentlyContinue | ForEach-Object { clang-format -i $_.FullName } }"

[unix]
format-rust:
    @echo "Formating Rust projects in src/rust_*..."
    for dir in src/rust_*; do [ -d "$dir" ] && context=$(pwd) && cd "$dir" && cargo fmt && cd "$context"; done

[windows]
format-rust:
    @echo "Formating Rust projects in src/rust_*..."
    powershell -Command "Get-ChildItem -Path ./src/rust_* -Directory | ForEach-Object { cd $_.FullName; cargo fmt; cd ..\.. }"

[unix]
docs:
    @echo "Verifing tools..."
    command -v doxygen >/dev/null 2>&1 || { echo >&2 'Error: Doxygen not fount'; exit 1; }

    @echo "Preparing directories..."
    mkdir -p {{docs_out}}

    @echo "Generatins temporary config file for Doxygen..."
    @doxygen -g - > Doxyfile_temp

    @echo "PROJECT_NAME = {{project_name}}" >> Doxyfile_temp
    @echo "OUTPUT_DIRECTORY = {{docs_out}}" >> Doxyfile_temp
    @echo "INPUT = {{docs_input}}" >> Doxyfile_temp
    @echo "RECURSIVE = YES" >> Doxyfile_temp
    @echo "GENERATE_HTML = YES" >> Doxyfile_temp
    @echo "HTML_OUTPUT = html" >> Doxyfile_temp
    @echo "USE_MDFILE_AS_MAINPAGE = README.md" >> Doxyfile_temp
    @echo "EXTRACT_ALL = YES" >> Doxyfile_temp
    @echo "EXTRACT_STATIC = YES" >> Doxyfile_temp
    @echo "SHOW_FILES = YES" >> Doxyfile_temp

    @command -v dot >/dev/null 2>&1 && { echo 'HAVE_DOT = YES'; echo 'CALL_GRAPH = YES'; echo 'INTERACTIVE_SVG = YES'; } >> Doxyfile_temp || echo 'Warning: Graphviz not found'

    @echo "Executing Doxygen..."
    @doxygen Doxyfile_temp
    rm Doxyfile_temp
    @echo "Docs generated in: {{docs_out}}/html/index.html"

[windows]
docs:
    @echo "Verifing tools..."
    where doxygen >$null 2>&1 || (echo Error: Doxygen not found && exit 1)

    @echo "Preparing directories..."
    powershell -Command "if (!(Test-Path {{docs_out}})) { New-Item -ItemType Directory -Force -Path {{docs_out}} }"

    @echo "Generatins temporary config file for Doxygen..."
    @doxygen -g - > Doxyfile_temp

    @echo "PROJECT_NAME = {{project_name}}" >> Doxyfile_temp
    @echo "OUTPUT_DIRECTORY = {{docs_out}}" >> Doxyfile_temp
    @echo "INPUT = {{docs_input}}" >> Doxyfile_temp
    @echo "RECURSIVE = YES" >> Doxyfile_temp
    @echo "GENERATE_HTML = YES" >> Doxyfile_temp
    @echo "HTML_OUTPUT = html" >> Doxyfile_temp
    @echo "USE_MDFILE_AS_MAINPAGE = README.md" >> Doxyfile_temp
    @echo "EXTRACT_ALL = YES" >> Doxyfile_temp
    @echo "EXTRACT_STATIC = YES" >> Doxyfile_temp
    @echo "SHOW_FILES = YES" >> Doxyfile_temp

    @where dot >$null 2>&1 && (echo HAVE_DOT = YES >> Doxyfile_temp && echo CALL_GRAPH = YES >> Doxyfile_temp && echo INTERACTIVE_SVG = YES >> Doxyfile_temp) || echo Warning: Graphviz not found

    @echo "Executing Doxygen..."
    @doxygen Doxyfile_temp
    rm Doxyfile_temp
    @echo "Docs generated in: {{docs_out}}/html/index.html"
