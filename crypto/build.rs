extern crate cbindgen;

use cbindgen::{Config, Language, LineEndingStyle};
use std::env;
use std::path::PathBuf;

fn main() {
    let crate_dir = env::var("CARGO_MANIFEST_DIR").unwrap();

    let config = Config {
        language: Language::Cxx,
        pragma_once: true,
        cpp_compat: true,
        line_endings: LineEndingStyle::LF,
        usize_is_size_t: true,
        ..Default::default()
    };

    let include_dir = PathBuf::from(&crate_dir)
        .join("include")
        .join("rust_crypto");
    if !include_dir.exists() {
        std::fs::create_dir_all(&include_dir).unwrap();
    }
    let output_file = include_dir.join("rust_crypto.h");

    cbindgen::Builder::new()
        .with_crate(crate_dir)
        .with_config(config)
        .generate()
        .expect("Unable to generate bindings")
        .write_to_file(output_file);
}
