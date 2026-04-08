build_dir := "build/debug"

src_dirs := "src/core src/db"
test_src_dirs := "tests"
rust_dir := "src/crypto"

docs_out   := "docs"
docs_input := "README.md src"
project_name := "Password Manager"

trash_files := ""

build-install: config build install

config:
    cmake --preset release

config-dev coverage="0" lint="0" asan="0" valgrind="0":
    cmake --preset debug \
          -DENABLE_COVERAGE={{ if coverage == "1" { "ON" } else { "OFF" } }} \
          -DENABLE_LINT={{ if lint == "1" { "ON" } else { "OFF" } }} \
          -DENABLE_ASAN={{ if asan == "1" { "ON" } else { "OFF" } }} \
          -DENABLE_VALGRIND={{ if valgrind == "1" { "ON" } else { "OFF" } }}

build:
    cmake --build --preset build-release

build-dev:
    cmake --build --preset build-debug

test:
    ctest --preset test-release

test-dev:
    ctest --preset test-debug

install:
    cmake --install --preset install-release

install-dev:
    cmake --install --preset install-debug

clean:
    @echo "Cleaning project files..."  
    {{ if os() == "windows" { \
        "powershell -Command \"Remove-Item -Recurse -Force build, install, docs, src/crypto/Cargo.lock, src/crypto/target -ErrorAction SilentlyContinue\"" \
    } else { \
        "rm -rf build install docs src/crypto/Cargo.lock src/crypto/target" \
    } }}

format: format-cpp format-rust
    @echo "All the code has been formated!"

format-cpp:
    @echo "Formating C++ in: {{src_dirs}}..."
    {{ if os() == "windows" { \
        "powershell -Command \"foreach ($dir in '" + src_dirs + " " + test_src_dirs + "'.Split(' ')) { Get-ChildItem -Path ./$dir -Include *.c,*.cpp,*.c++,*.h,*.hpp,*.hh,*.cc -Recurse -ErrorAction SilentlyContinue | ForEach-Object { clang-format -i $_.FullName } }\"" \
    } else { \
        "find " + src_dirs + " " + test_src_dirs + " -type f -regextype posix-extended -regex '.*\\.(c|cpp|c\\+\\+|h|hpp|hh|cc)$' -exec clang-format -i {} +" \
    } }}

format-rust:
    @echo "Formating Rust in: {{rust_dir}}..."
    @cd {{rust_dir}} && cargo fmt

docs:
    @echo "Verifing tools..."
    {{ if os() == "windows" { \
        "where doxygen >$null 2>&1 || (echo Error: Doxygen not found && exit 1)" \
    } else { \
        "command -v doxygen >/dev/null 2>&1 || { echo >&2 'Error: Doxygen not fount'; exit 1; }" \
    } }}

    @echo "Preparing directories..."
    {{ if os() == "windows" { \
        "powershell -Command \"if (!(Test-Path " + docs_out + ")) { New-Item -ItemType Directory -Force -Path " + docs_out + " }\"" \
    } else { \
        "mkdir -p " + docs_out \
    } }}
    
    @echo "Generatins temporary config file for Doxygen..."
    @doxygen -g - > Doxyfile_temp
    
    @echo "PROJECT_NAME = {{project_name}}" >> Doxyfile_temp
    @echo "OUTPUT_DIRECTORY = {{docs_out}}" >> Doxyfile_temp
    @echo "INPUT = {{docs_input}}" >> Doxyfile_temp
    @echo "RECURSIVE = YES" >> Doxyfile_temp
    @echo "GENERATE_HTML = YES" >> Doxyfile_temp
    @echo "HTML_OUTPUT = html" >> Doxyfile_temp
    @echo "USE_MDFILE_AS_MAINPAGE = README.md" >> Doxyfile_temp
    
    @{{ if os() == "windows" { \
        "where dot >$null 2>&1 && (echo HAVE_DOT = YES >> Doxyfile_temp && echo CALL_GRAPH = YES >> Doxyfile_temp && echo INTERACTIVE_SVG = YES >> Doxyfile_temp) || echo 'Warning: Graphviz not found'" \
    } else { \
        "command -v dot >/dev/null 2>&1 && { echo 'HAVE_DOT = YES'; echo 'CALL_GRAPH = YES'; echo 'INTERACTIVE_SVG = YES'; } >> Doxyfile_temp || echo 'Warning: Graphviz not found'" \
    } }}

    @echo "Executing Doxygen..."
    @doxygen Doxyfile_temp
    {{ if os() == "windows" { "rm Doxyfile_temp" } else { "rm Doxyfile_temp" } }}
    @echo "Docs generated in: {{docs_out}}/html/index.html"
