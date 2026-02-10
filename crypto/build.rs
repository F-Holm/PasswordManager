extern crate cbindgen;

use std::env;
use std::path::PathBuf;
use cbindgen::{Config, Language, LineEndingStyle};

fn main() {
    let profile = env::var("PROFILE").unwrap();

    let crate_dir = env::var("CARGO_MANIFEST_DIR").unwrap();

    let mut config = Config::default();
    config.language = Language::Cxx;
    config.pragma_once = true;
    config.cpp_compat = true;
    config.line_endings = LineEndingStyle::LF;
    config.usize_is_size_t = true;

    let target_dir = PathBuf::from(&crate_dir).join("target").join(profile).join("rust");
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
