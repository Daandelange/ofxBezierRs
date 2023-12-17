
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
	// Ptr to std::vec<bezrsBezierHandle> (if c++ owned) or Vec<bezrsBezierHandle> (if rust owned)
    data: *const bezrsBezierHandle, 
    /// count of data items
    len: usize,
    /// if true, behave as shape, otherwise behave as path.
    closed: bool,
}

// C++ : Opaque pointer to internal data handle
// Rust : Internal data object holding the subpath
/// Opaque internal shape data handle
/// (use only as pointer! allocated on rust side, needs to be freed properly)
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
/// Create a shape instance in rust memory : needs to be freed afterwards.
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
            let safe_closed : bool = closed && (beziers_raw.len >= 2);
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
			// Return empty path/shape
			let boxed_shape = Box::new(bezrsShape::new(outline));

			// Return raw pointer to the allocated memory
    		return Box::into_raw(boxed_shape);
    	}
	}
	
	return std::ptr::null_mut();
}
