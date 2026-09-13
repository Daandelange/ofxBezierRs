
# Build lib
#cargo build --release
echo "Building Library..."
RUSTFLAGS='-C target-feature=+crt-static' cargo build --release

# on binary : install_name_tool -change /Developer/of_v0.11.2/addons/ofxBezierRs/libs/bezier-rs-ffi/target/release/deps/libbezier_rs_ffi.dylib @executable_path//libbezier_rs_ffi.dylib example-simple
# install_name_tool -change @rpath/libbezier_rs_ffi.dylib @executable_path/libbezier_rs_ffi.dylib exe
# Change lib name accoringly
#- `install_name_tool -id @executable_path/libbezier_rs_ffi.dylib /Developer/of_v0.11.2/addons/ofxBezierRs/example-simple/bin/example-simple.app/Contents/MacOS/libbezier_rs_ffi.dylib`
#- Verify with `otool -l exe` and `otool -D exe.dylib`

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