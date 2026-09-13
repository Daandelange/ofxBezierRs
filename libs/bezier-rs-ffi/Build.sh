
# Build lib
#cargo build --release
echo "Building Library..."
RUSTFLAGS='-C target-feature=+crt-static' cargo build --release

echo "Copying Library Files..."
mkdir -p ./lib/osx
mkdir -p ./lib/linux64

# Copy files to OF plugin structure
mv ./target/release/libbezier_rs_ffi.dylib ./lib/osx/libbezier_rs_ffi.dylib 2>/dev/null || true
mv ./target/release/libbezier_rs_ffi.a ./lib/osx/libbezier_rs_ffi.a 2>/dev/null || true
mv ./target/release/libbezier_rs_ffi.so ./lib/linux64/libbezier_rs_ffi.so 2>/dev/null || true

# Generate header file
echo "Generating C Header File..."
cbindgen --config ./cbindgen.toml --crate bezier-rs-ffi --output ./include/bezier-rs-ffi.h

echo "Done! :)"