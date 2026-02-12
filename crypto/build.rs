extern crate cbindgen;

use cbindgen::{Config, Language, LineEndingStyle};
use std::env;
use std::path::PathBuf;

fn main() {
    let profile = env::var("PROFILE").unwrap();
    let crate_dir = env::var("CARGO_MANIFEST_DIR").unwrap();

    let config = Config {
        language: Language::Cxx,
        pragma_once: true,
        cpp_compat: true,
        line_endings: LineEndingStyle::LF,
        usize_is_size_t: true,
        ..Default::default()
    };

    let target_dir = PathBuf::from(&crate_dir).join("target").join(profile);
    if !target_dir.exists() {
        std::fs::create_dir_all(&target_dir).unwrap();
    }
    let output_file = target_dir.join("rust_crypto.h");

    cbindgen::Builder::new()
        .with_crate(crate_dir)
        .with_config(config)
        .generate()
        .expect("Unable to generate bindings")
        .write_to_file(output_file);
}
