files_to_clean := "build install package docs src/rust_*/Cargo.lock src/rust_*/target"

docs_out   := "docs"
docs_input := "README.md src"
project_name := "Password Manager"

build-install: config build install package

build-install-ci: config build test install package

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

package:
    cmake --package --preset package-release

package-dev:
    cmake --package --preset package-debug

clean:
    @echo "Cleaning project files..."  
    {{ if os() == "windows" { \
        "powershell -Command \"Get-Item " + files_to_clean + " -ErrorAction SilentlyContinue | Remove-Item -Recurse -Force\"" \
    } else { \
        "rm -rf " + files_to_clean \
    } }}

format: format-cpp format-rust
    @echo "All the code has been formated!"

format-cpp:
    @echo "Formating C++ in: src tests..."
    {{ if os() == "windows" { \
        "powershell -Command \"foreach ($dir in 'src tests'.Split(' ')) { Get-ChildItem -Path ./$dir -Include *.c,*.cpp,*.c++,*.h,*.hpp,*.hh,*.cc -Recurse -ErrorAction SilentlyContinue | ForEach-Object { clang-format -i $_.FullName } }\"" \
    } else { \
        "find src tests -type f -regextype posix-extended -regex '.*\\.(c|cpp|c\\+\\+|h|hpp|hh|cc)$' -exec clang-format -i {} +" \
    } }}

format-rust:
    @echo "Formating Rust projects in src/rust_*..."
    {{ if os() == "windows" { \
        "powershell -Command \"Get-ChildItem -Path ./src/rust_* -Directory | ForEach-Object { cd $_.FullName; cargo fmt; cd ..\\.. }\"" \
    } else { \
        "for dir in src/rust_*; do [ -d \"$dir\" ] && context=$(pwd) && cd \"$dir\" && cargo fmt && cd \"$context\"; done" \
    } }}

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
    @echo "EXTRACT_ALL = YES" >> Doxyfile_temp
    @echo "EXTRACT_STATIC = YES" >> Doxyfile_temp
    @echo "SHOW_FILES = YES" >> Doxyfile_temp
    
    @{{ if os() == "windows" { \
        "where dot >$null 2>&1 && (echo HAVE_DOT = YES >> Doxyfile_temp && echo CALL_GRAPH = YES >> Doxyfile_temp && echo INTERACTIVE_SVG = YES >> Doxyfile_temp) || echo 'Warning: Graphviz not found'" \
    } else { \
        "command -v dot >/dev/null 2>&1 && { echo 'HAVE_DOT = YES'; echo 'CALL_GRAPH = YES'; echo 'INTERACTIVE_SVG = YES'; } >> Doxyfile_temp || echo 'Warning: Graphviz not found'" \
    } }}

    @echo "Executing Doxygen..."
    @doxygen Doxyfile_temp
    {{ if os() == "windows" { "rm Doxyfile_temp" } else { "rm Doxyfile_temp" } }}
    @echo "Docs generated in: {{docs_out}}/html/index.html"
