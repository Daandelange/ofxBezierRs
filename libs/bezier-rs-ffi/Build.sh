
# Build lib
cargo build --release

# Copy files to OF plugin structure
mv ./target/release/libbezier_rs_ffi.dylib ./lib/osx/libbezier_rs_ffi.dylib
mv ./target/release/libbezier_rs_ffi.a ./lib/osx/libbezier_rs_ffi.a

# Generate header file
cbindgen --config ./cbindgen.toml --crate bezier-rs-ffi --output include/bezier-rs-ffi.h
