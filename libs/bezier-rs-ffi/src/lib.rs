
// This is some kind of rust FFI that creates an adapter for communicating between rust and c++ code.
// Written in rust, it transpiles (using cbindgen) to c++ headers, the ones used for the resulting lib.

// Principle : Create some common/shared data structures (bezierShape and bezierPoint/Handle), then let rust do operations on them.

// Useful ressources :
// - Using a rust lib from within C++ : https://docs.rust-embedded.org/book/interoperability/rust-with-c.html
// - FFI principles : https://doc.rust-lang.org/nomicon/ffi.html
// - FFI mistakes explained : https://rust-unofficial.github.io/patterns/idioms/ffi/errors.html
// - The Rust FFI Omnibus : http://jakegoulding.com/rust-ffi-omnibus/objects/
// - CBindgen docs : https://docs.rs/crate/cbindgen/latest
// - A WASM FFI for bezier-rs : https://github.com/GraphiteEditor/Graphite/blob/master/website/other/bezier-rs-demos/wasm/src/subpath.rs
// - RUST FII with complex data types : http://kmdouglass.github.io/posts/complex-data-types-and-the-rust-ffi/

// Note:
// Comments that start with 3 slashes (///) end up in the generated C header file.

// Note: bezier-rs uses the following terms for describing vector shape data:
// - Subpath = shape or path holding a list of bezier handles.
// - ManipulatorGroup = Bezier handle composed of 3 positions : 1 anchor + 2 bezier controls.
// - Bezier = Segment of a Shape (used internally).

use std::slice;
use std::ptr;
use bezier_rs::SubpathTValue; // Warns unused, but doesn't compile without this import !
//use bezier_rs::TValue;

// For accessing/sharing c++ std::vectors
use bezier_rs::{Subpath, ManipulatorGroup};
use bezier_rs::{Cap, Join};

// Included for conversions
use glam::f64::DVec2; // point class, already defined repr(C)

/// Join type enum
#[repr(C)]
pub enum bezrsJoinType {
	Bevel,
	Mitter,
	Round,
}

/// Cap type enum
#[repr(C)]
pub enum bezrsCapType {
	Butt,
	Round,
	Square,
}

pub fn parse_join(join: bezrsJoinType, miter_limit: Option<f64>) -> Join {
	match join {
		bezrsJoinType::Bevel => Join::Bevel,
		bezrsJoinType::Mitter => Join::Miter( Some(miter_limit.unwrap_or(0.0)) ),
		bezrsJoinType::Round => Join::Round,
		// Default value
		//_ => Join::Bevel,
	}
}

pub fn parse_cap(cap: bezrsCapType) -> Cap {
	match cap {
		bezrsCapType::Butt => Cap::Butt,
		bezrsCapType::Round => Cap::Round,
		bezrsCapType::Square => Cap::Square,
		// Default value
		//_ => Cap::Butt,
	}
}

/// A simple position wrapper (x, y)
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct bezrsPos {
    pub x: f64,
    pub y: f64,
}

impl bezrsPos {

	pub fn new(_x : f64, _y : f64) -> Self {
		bezrsPos {
			x: _x,
			y: _y,
		}
	}

	pub(crate) fn from_dvec2(_pos : &DVec2) -> Self {
    	bezrsPos {
	        x : _pos.x,
	        y : _pos.y,
        }
    }

    // pub(crate) fn from_dvec2_opt(_pos : &Option<DVec2>) -> Self {
    // 	if let Some(valid_pos) = _pos {
    // 		return bezrsPos {
	   //      	x : valid_pos.x,
	   //      	y : valid_pos.y,
    //     	}
    //     }
    //     else {
    //     	return bezrsPos {
	   //      	x : 0.0,
	   //      	y : 0.0,
    //     	}
    //     }
    // }

    pub(crate) fn to_dvec2(&self) -> DVec2 {
    	DVec2 {
	        x : self.x,
	        y : self.y,
        }
    }
}

// Rust type casting using traits : cannot get this working
// impl Into<DVec2> for bezrsPos {
//     fn into(self) -> Dvec2 {
//         Dvec2 {
//         	x : self.x,
// 	        y : self.y,
//         }
//     }
// }

// Dummy ID
//#[repr(C)]
#[derive(Clone, PartialEq, Hash)]
pub(crate) struct EmptyId;
impl bezier_rs::Identifier for EmptyId {
	fn new() -> Self {
		Self
	}
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct bezrsBezierHandle {
    pub pos : bezrsPos,
    pub in_bez : bezrsPos,
    pub out_bez : bezrsPos,
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct bezrsRect {
    pub pos : bezrsPos,
    pub size : bezrsPos,
}

impl bezrsBezierHandle {

	pub fn new(x: f64, y: f64, inx : f64, iny : f64, outx : f64, outy : f64) -> Self {
    	bezrsBezierHandle{
	        pos : bezrsPos::new(x, y),
	        in_bez : bezrsPos::new(inx, iny),
        	out_bez : bezrsPos::new(outx, outy),
        }
    }

    pub fn from_dvec2(_pos : &DVec2, _in_bez : &DVec2, _out_bez : &DVec2) -> Self {
    	bezrsBezierHandle{
	        pos : bezrsPos::from_dvec2(_pos),
	        in_bez : bezrsPos::from_dvec2(_in_bez),
        	out_bez : bezrsPos::from_dvec2(_out_bez),
        }
    }

    // #[no_mangle]
    // pub extern "C" fn bez_rs_handle_from_pos(_pos : &bezrsPos, _in_bez : &bezrsPos, _out_bez : &bezrsPos) -> Self {
    // 	bezrsBezierHandle{
	   //      pos : *_pos,
	   //      in_bez : *_in_bez,
    //     	out_bez : *_out_bez,
    //     }
    // }

   	// Converts to internal format
   	// todo: no unsafe ?
	pub(crate) fn to_internal(&self) -> ManipulatorGroup<EmptyId> {
		ManipulatorGroup {
			anchor: self.pos.to_dvec2(),
			in_handle: Some(self.in_bez.to_dvec2()),
			out_handle: Some(self.out_bez.to_dvec2()),
			id: EmptyId//ManipulatorGroupId::new(),
		}
	}

	// Constructs from internal format
    // pub fn from_internal(internal: ManipulatorGroup<EmptyId>) -> Self {
    //     bezrs_point::new(v.x, v.y)
    // }

   //  #[no_mangle]
   //  pub extern "C" fn hasOutBez(&self) -> bool {
   //  	return self.out_bez.x != self.pos.x && self.out_bez.y != self.pos.y;
  	// }

   //  #[no_mangle]
  	// pub extern "C" fn hasInBez(&self) -> bool {
   //  	return self.out_bez.x != self.pos.x && self.out_bez.y != self.pos.y;
  	// }
}

pub(crate) fn sub_path_to_vec(sub_path : &Subpath<EmptyId>) -> Vec<bezrsBezierHandle> {
	sub_path
		.manipulator_groups()
		.into_iter()
		.map(|mg| bezrsBezierHandle {
			pos: bezrsPos::from_dvec2(&mg.anchor),
			//in_bez: bezrsPos::from_dvec2_opt(&mg.in_handle),
			//out_bez: bezrsPos::from_dvec2_opt(&mg.out_handle),
			in_bez: bezrsPos::from_dvec2(&mg.in_handle.unwrap_or(mg.anchor)),
			out_bez: bezrsPos::from_dvec2(&mg.out_handle.unwrap_or(mg.anchor)),
		})
		.collect()
}

/// Raw vector handle representing a bezier shape
/// Used for sending owned data from Rust to C++ in both directions.
#[repr(C)]
pub struct bezrsShapeRaw {
	/// Ptr to std::vec<bezrsBezierHandle> (if c++ owned) or Vec<bezrsBezierHandle> (if rust owned)
    data: *const bezrsBezierHandle,
    /// count of data items
    len: usize,
    /// if true, behave as shape, otherwise behave as path.
    closed: bool,
}

/// Raw vector of floats
/// Used for sending owned data from Rust to C++ in both directions.
#[repr(C)]
pub struct bezrsFloatsRaw {
	/// Ptr to std::vec<float> (if c++ owned) or Vec<f64> (if rust owned)
    data: *const f64,
    /// count of data items
    len: usize,
}

// C++ : Opaque pointer to internal data handle
// Rust : Internal data object holding the subpath
/// Opaque internal shape data handle
/// (use only as pointer! allocated on rust side, needs to be freed properly)
// Todo: rename this bezrsShapeInternal for c++ clarity ??
#[derive(Debug)]
pub struct bezrsShape {
	pub(crate) sub_path : Subpath<EmptyId>, // Internal data object
	pub(crate) beziers : Vec<bezrsBezierHandle>, // Mirrored beziers for returning the data to c++
}

impl bezrsShape {

	pub(crate) fn new(_sub_path : Subpath<EmptyId>) -> Self {
		bezrsShape {
			beziers : sub_path_to_vec(&_sub_path),
			sub_path : _sub_path, // Check : need .clone() here ?
		}
	}
}

#[no_mangle]
// note : Option is for allowing nullptr from c++
/// Create a shape instance in rust memory : needs to be freed afterwards. Also, `beziers_opt` needs to remain valid (and static) until freed.
pub extern "C" fn bezrs_shape_create(beziers_opt: Option<&bezrsShapeRaw>, closed: bool) -> *mut bezrsShape {

    if let Some(beziers_raw) = beziers_opt {
        if !beziers_raw.data.is_null() {
            // Convert the raw pointer and length to a slice
            let beziers_slice = unsafe {
                slice::from_raw_parts(beziers_raw.data as *const bezrsBezierHandle, beziers_raw.len)
            };

            // Convert `Vec<bezrsBezierHandle>` to `Vec<ManipulatorGroup<EmptyId>>`
		    let manipulator_groups = beziers_slice
		    	.to_vec()
				.into_iter()
				.map(|bez_handle| ManipulatorGroup {
					anchor: bez_handle.pos.clone().to_dvec2(),
					in_handle: Some(bez_handle.in_bez.clone().to_dvec2()),
					out_handle: Some(bez_handle.out_bez.clone().to_dvec2()),
					id: EmptyId,
				})
				.collect();

            // Create a Shape from the handles
            // Note : Bezier-rs panics when < 2 subpath items and closed = false
            let safe_closed : bool = closed && (beziers_raw.len > 1);
	        let shape = bezrsShape {
	            sub_path: Subpath::<EmptyId>::new(manipulator_groups, safe_closed),
	            beziers: beziers_slice.to_vec(),
	        };

	        // Put instance on heap to get a stable memory address.
	        // Box is similar to std::unique_ptr
	        let boxed_shape = Box::new(shape);

			// Return raw pointer to the allocated memory
    		return Box::into_raw(boxed_shape)
        } else {
            // The pointer is null
        }
    } else {
        // No option given !
    }

	// Return empty path/shape
	let boxed_shape = Box::new(bezrsShape::new(Subpath::new(Vec::<ManipulatorGroup<EmptyId>>::new(), false)));

	// Return raw pointer to the allocated memory
    return Box::into_raw(boxed_shape)
}

#[no_mangle]
/// To destroy an internal shape handle when you don't need it anymore.
pub extern "C" fn bezrs_shape_destroy(_bezier: *mut bezrsShape) {
    if _bezier.is_null() {
        return;
    }
    unsafe {
        let _ = Box::from_raw(_bezier);
    }
}

#[no_mangle]
/// Inserts a bezier to the shape at a given position
pub extern "C" fn bezrs_shape_insert_bezier(_shape: *mut bezrsShape, _bez : bezrsBezierHandle, _pos : usize) {
    let shape = unsafe {
        assert!(!_shape.is_null());
        &mut *_shape
    };
    shape.sub_path.insert_manipulator_group(_pos, _bez.to_internal());
}

#[no_mangle]
/// Appends a bezier to the shape
pub extern "C" fn bezrs_shape_append_bezier(_shape: *mut bezrsShape, _bez : bezrsBezierHandle) {
    let shape = unsafe {
        assert!(!_shape.is_null());
        &mut *_shape
    };
    let pos = shape.sub_path.len();
    shape.sub_path.insert_manipulator_group(pos, _bez.to_internal());
}

#[no_mangle]
/// Reverses the winding order of bezier handles
pub extern "C" fn bezrs_shape_reverse_winding(_shape: *mut bezrsShape) {
    let shape = unsafe {
        assert!(!_shape.is_null());
        &mut *_shape
    };
    shape.sub_path = shape.sub_path.reverse();
}

// Retrieve shape data
#[no_mangle]
/// To retrieve the data of an internal shape handle.
pub extern "C" fn bezrs_shape_return_handle_data(_shape: *mut bezrsShape) -> bezrsShapeRaw {
    let shape = unsafe {
        assert!(!_shape.is_null());
        &mut *_shape
    };

    // Todo: add a dirty flag to prevent recomputing for nothing ?
    shape.beziers = sub_path_to_vec(&shape.sub_path);

    return bezrsShapeRaw {
    	// data: shape.sub_path.manipulator_groups().as_mut_ptr(),
    	// len: shape.sub_path.manipulator_groups().len(),
    	data: shape.beziers.as_mut_ptr(),
    	len: shape.beziers.len(),
    	closed: shape.sub_path.closed(),
    };
}



// Returning a vec to c++ : https://www.reddit.com/r/rust/comments/aca3do/ffi_how_do_you_pass_a_vec_to_c/
#[no_mangle]
/// Offset a shape. When the shape is winded clockwise : positive offset goes inside, negative is outside.
// Todo : Rename this to bezrs_shape_offset
pub extern "C" fn bezrs_cubic_bezier_offset(_shape: *mut bezrsShape, offset : f64, join_type : bezrsJoinType, join_mitter : f64 ) {
	let shape = unsafe {
        assert!(!_shape.is_null());
        &mut *_shape
    };

	// Offset real object
	shape.sub_path = shape.sub_path.offset(offset, parse_join(join_type, Some(join_mitter))); // Bevel, Round, Mitter(limit:f64)
}

#[no_mangle]
/// Rotates the whole shape
pub extern "C" fn bezrs_shape_rotate(_shape: *mut bezrsShape, _angle: f64, _center_point : *mut bezrsPos ) {
    let shape = unsafe {
        assert!(!_shape.is_null());
        &mut *_shape
    };

    let center_point = if _center_point.is_null() { DVec2::new(0.0,0.0) } else { unsafe { _center_point.as_ref().unwrap().to_dvec2() } };
    shape.sub_path = shape.sub_path.rotate_about_point(_angle, center_point);
}

#[no_mangle]
/// Outlines a shape or path.
/// Important: Closed shapes will return a new shape instance, to be destroyed correctly.
pub extern "C" fn bezrs_shape_outline(_shape: *mut bezrsShape, distance: f64, join: bezrsJoinType, cap: bezrsCapType, miter_limit: f64) -> *mut bezrsShape {
	let shape = unsafe {
        assert!(!_shape.is_null());
        &mut *_shape
    };

    // Convert arguments
	let join = parse_join(join, Some(miter_limit));
	let cap = parse_cap(cap);

	// Note : A path outline returns 1 subpath. If it was a shape (closed), a 2nd one is received.
	let (outline_piece1, outline_piece2) = shape.sub_path.outline(distance, join, cap);

	// Update 1st result as usual
	shape.sub_path = outline_piece1;

	// Return 2nd result as a shape
	if shape.sub_path.closed() {
		if let Some(outline) = outline_piece2 {
			// Allocate return path/shape
			let boxed_shape = Box::new(bezrsShape::new(outline));

			// Return raw pointer to the allocated memory
    		return Box::into_raw(boxed_shape);
    	}
	}

	return std::ptr::null_mut();
}

#[no_mangle]
/// Returns the bounding box of the shape
pub extern "C" fn bezrs_shape_boundingbox(_shape: *mut bezrsShape) -> bezrsRect {
	let shape = unsafe {
        assert!(!_shape.is_null());
        &mut *_shape
    };

    if let Some(bb) = shape.sub_path.bounding_box() {
		let _size = bb[1]-bb[0];
		let ret = bezrsRect { pos: bezrsPos::from_dvec2(&bb[0]), size: bezrsPos::from_dvec2(&_size)};
		return ret; // Todo: is it memory-safe to return it like this ? (copied, but is the ownership transferred correctly ?)
	}

	return bezrsRect {pos:bezrsPos::new(0.,0.), size: bezrsPos::new(0.,0.)};
}

#[no_mangle]
/// Returns the inflection points on a shape
pub extern "C" fn bezrs_shape_inflections(_shape: *mut bezrsShape) -> bezrsFloatsRaw {
	let shape = unsafe {
        assert!(!_shape.is_null());
        &mut *_shape
    };

    let inflections = shape.sub_path.inflections();
	if inflections.len() > 0 {
		let ret = bezrsFloatsRaw { data: inflections.as_ptr(), len: inflections.len() };
		return ret; // Todo: is it memory-safe to return it like this ? (copied, but is the ownership transferred correctly ?)
	}

	return bezrsFloatsRaw {data: ptr::null(), len: 0};
}

#[no_mangle]
/// Returns if a point is contained within a shape
pub extern "C" fn bezrs_shape_containspoint(_shape: *mut bezrsShape, _pos : bezrsPos) -> bool {
	let shape = unsafe {
        assert!(!_shape.is_null());
        &mut *_shape
    };

    let contained = shape.sub_path.contains_point(_pos.to_dvec2());
	return contained; // Todo: is it memory-safe to return it like this ? (copied, but is the ownership transferred correctly ?)
}

#[no_mangle]
/// Returns positions where the shape self intersects
pub extern "C" fn bezrs_shape_selfintersections(_shape: *mut bezrsShape, _error_treshold : f64, _min_dist : f64) -> bezrsFloatsRaw {
	let shape = unsafe {
        assert!(!_shape.is_null());
        &mut *_shape
    };

	let si = shape.sub_path.self_intersections(None, None);
	if si.len() > 0 {
		let flattened : Vec<f64> = si.iter().map(|(_x,f)| *f).collect();
		let ret = bezrsFloatsRaw { data: flattened.as_ptr(), len: flattened.len() };
		return ret; // Todo: is it memory-safe to return it like this ? (copied, but is the ownership transferred correctly ?)
	}

	return bezrsFloatsRaw {data: ptr::null(), len: 0};
}

#[no_mangle]
/// Returns the position on the shape from a t-value (0->1) using `evaluate()`.
pub extern "C" fn bezrs_shape_posfromtvalue(_shape: *mut bezrsShape, _t : f64) -> bezrsPos {
	let shape = unsafe {
        assert!(!_shape.is_null());
        &mut *_shape
    };

	let pos = shape.sub_path.evaluate( SubpathTValue::GlobalParametric(_t) );
	// let pos = shape.sub_path.evaluate( TValue::Parametric(_t) );
	return bezrsPos::from_dvec2(&pos);
}

#[no_mangle]
/// Returns the position on the shape from a t-value (0->1) using `evaluate()`.
pub extern "C" fn bezrs_shape_posfromtvalue_subpath(_shape: *mut bezrsShape, _i : usize, _t : f64) -> bezrsPos {
	let shape = unsafe {
        assert!(!_shape.is_null());
        &mut *_shape
    };

	let pos = shape.sub_path.evaluate( SubpathTValue::Parametric{ segment_index: _i, t:_t} );
	// let pos = shape.sub_path.evaluate( TValue::Parametric(_t) );
	return bezrsPos::from_dvec2(&pos);
}

#[no_mangle]
/// Returns the normal on the shape from a t-value (0->1).
pub extern "C" fn bezrs_shape_normalfromtvalue(_shape: *mut bezrsShape, _t : f64) -> bezrsPos {
	let shape = unsafe {
        assert!(!_shape.is_null());
        &mut *_shape
    };

	let normal = shape.sub_path.normal( SubpathTValue::GlobalParametric(_t) );
	return bezrsPos::from_dvec2(&normal);
}

#[no_mangle]
/// Returns the tangent on the shape from a t-value (0->1).
pub extern "C" fn bezrs_shape_tangentfromtvalue(_shape: *mut bezrsShape, _t : f64) -> bezrsPos {
	let shape = unsafe {
        assert!(!_shape.is_null());
        &mut *_shape
    };

	let tangent = shape.sub_path.tangent( SubpathTValue::GlobalParametric(_t) );
	return bezrsPos::from_dvec2(&tangent);
}

#[no_mangle]
/// Returns the curvature on the shape from a t-value (0->1).
pub extern "C" fn bezrs_shape_curvaturefromtvalue(_shape: *mut bezrsShape, _t : f64) -> f64 {
	let shape = unsafe {
        assert!(!_shape.is_null());
        &mut *_shape
    };

	let curvature = shape.sub_path.curvature( SubpathTValue::GlobalParametric(_t) );
	return curvature;
}

#[no_mangle]
/// Returns t-value of the projection of a position on the shape from a t-value (0->1). (finds closest point on shape)
pub extern "C" fn bezrs_shape_project_pos(_shape: *mut bezrsShape, _pos : bezrsPos) -> bezrsPos {
	let shape = unsafe {
        assert!(!_shape.is_null());
        &mut *_shape
    };

	if let Some((_path_index, t_value)) = shape.sub_path.project( _pos.to_dvec2(), None ) {
		return bezrsPos::from_dvec2(&shape.sub_path.evaluate(SubpathTValue::Parametric { segment_index: _path_index, t: t_value }));
	}

	return bezrsPos {x:0., y:0.}; // Todo : set this to infinity or something to mark the fail
}

