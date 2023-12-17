/* Lib bezier-rs-ffi C++ bindings. */
/* Copyright Daan de Lange 2023-(current-year). */


#pragma once

/* Generated with cbindgen:0.26.0 */

/* Warning : this file is autogenerated by cbindgen. Don't modify this manually. */

#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>

/// Join type enum
enum class bezrsJoinType {
  Bevel,
  Mitter,
  Round,
};

/// Opaque internal shape data handle
/// (use only as pointer! allocated on rust side, needs to be freed properly)
struct bezrsShape;

/// A simple position wrapper (x, y)
struct bezrsPos {
  double x;
  double y;
};

struct bezrsBezierHandle {
  bezrsPos pos;
  bezrsPos in_bez;
  bezrsPos out_bez;
  bool hasOutBez(){
      return out_bez.x != pos.x && out_bez.y != pos.y;
  }
  bool hasInBez(){
      return out_bez.x != pos.x && out_bez.y != pos.y;
  }
};

/// Raw vector handle representing a bezier shape
/// Used for sending owned data from Rust to C++ in both directions.
struct bezrsShapeRaw {
  const bezrsBezierHandle *data;
  /// count of data items
  uintptr_t len;
  /// if true, behave as shape, otherwise behave as path.
  bool closed;
};

extern "C" {

/// Create a shape instance in rust memory : needs to be freed afterwards.
bezrsShape *bezrs_shape_create(const bezrsShapeRaw *beziers_opt, bool closed);

/// To destroy an internal shape handle when you don't need it anymore.
void bezrs_shape_destroy(bezrsShape *ptr);

/// Not implemented yet !
void bezrs_shape_add_handle(bezrsShape *ptr);

/// To retrieve the data of an internal shape handle.
bezrsShapeRaw bezrs_shape_return_handle_data(bezrsShape *ptr);

/// Offset a shape. When the shape is winded clockwise : positive offset goes inside, negative is outside.
bool bezrs_cubic_bezier_offset(bezrsShape *shape,
                               double offset,
                               bezrsJoinType join_type,
                               double join_mitter);

} // extern "C"
