# ofxBezierRs
An OpenFrameworks wrapper for [bezier-rs](https://github.com/GraphiteEditor/Graphite/tree/master/libraries/bezier-rs) providing a set geometrical functions on cartesian planar Beziér paths or shapes.

## Early state
Bezier-rs provides [plenty of functions](https://graphite.rs/libraries/bezier-rs/), not all are implemented yet.  
Right now, see this addon as a proof-of-concept for using lib bezier-rs with C++ code.
Only tested on osx 10.12 + OF 0.11.2, the binaries are not yet bundled.

Functions :
- [x] Shape offset 
- [x] Shape outline
- [x] Shape rotation
- [x] Reversing winding direction

## Shapes
The shape object is close to the underlying one used in bezier-rs.  
Their concept is quite simple :
- A point has a `x` and `y`.
- A bezier handle is a set of 3 points : 1 anchor and 2 (absolute) in/out bezier handles.
- A shape is a set of multiple bezier handles and can be closed (shape) or not (path).  
  Note: Shapes work better if handles are winded clockwise.

## Implementation notes
Bezier-rs is a library written in Rust which can build a C compatible library.

ofxBezierRs relies on 3 essential components :
- `bezier-rs-ffi` : A rust crate to build a library that provides API for communicating with C (an `ffi` in Rust's terms).
- `bezier-rs-ffi.h` : C++ bindings for the compiled library, generated from the rust crate.
- `ofxBezierRs.h` : Some glue to make it work better with OF.

## Usage
```cpp
#include "ofxBezierRs.h"
int main(){

	// Build an array-compatible list of beziers handles.
	// Must remain accessible when calling bez_rs_shape_create.
	std::vector<bezrsBezierHandle> bezierHandles = { ... }; // <-- Fill this with your vector drawing
	// Make raw handle
	bezrsShapeRaw bezRsShapeInput = { bezierHandles.data(), bezierHandles.size(), true };
	// Build internal/opaque shape from raw input
	bezrsShape* bezRsShape = bezrs_shape_create( &bezRsShapeInput, true);
	// Transform the shape
	bezrs_cubic_bezier_offset(bezRsShape, 10, bezrs_join_type::Round, 0);
	// Retrieve resulting shape
	bezrsShapeRaw offsetShapeRaw = bezrs_shape_return_handle_data(bezRsShape);
	// Use result
	size_t bhi = 0;
	for (const bezrsBezierHandle* bh = offsetShapeRaw.data; bhi < offsetShapeRaw.len; bh++){
		ofDrawCircle(bh->pos.x, bh->pos.y, 5.f); // draw
		// or copy to own variable

		bhi++;
	}
	// Destroy shape handle
	bezrs_shape_destroy(bezRsShape);

	return 0;
}
```

There's a set of ImGui helpers available, to opt-in, define `OFXBEZRS_DEFINE_IMGUI_HELPERS`.

## Development
To build a new library binary for your platform, make sure that you have [Rust](https://www.rust-lang.org/tools/install) installed.
- `curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh`

Build the library :
- `cd ./bezier-rs-ffi/`
- `cargo build --release`
- Copy files to `ofxBezierRs/libs/bezier-rs-ffi/...`

Generate bindings :
- Install or update cbindgen : `cargo install --force cbindgen`
- `cd ./bezier-rs-ffi/`
- Run : `cbindgen --config ./cbindgen.toml --crate bezier-rs-ffi --output include/bezier-rs-ffi.h`.
- Copy `bezier-rs-ffi.h` to `libs/bezier-rs-ffi/include`.

You can run the above instructions automatically :
- `cd ./libs/bezier-rs-ffi && ./Build.sh`


## License
The [bezier-rs crate](https://crates.io/crates/bezier-rs) is licensed [MIT](https://github.com/GraphiteEditor/Graphite/blob/master/libraries/bezier-rs/LICENSE-MIT) or [Apache-2.0](https://github.com/GraphiteEditor/Graphite/blob/master/libraries/bezier-rs/LICENSE-APACHE). The bezier-rs crate is made by the team behind [Graphite.rs](https://editor.graphite.rs).
`ofxBezierRs` and `bezier-rs-ffi` are [MIT](https://github.com/Daandelange/ofxBezierRs/blob/main/LICENSE) and made by [Daan de Lange](https://daandelange.com/).
