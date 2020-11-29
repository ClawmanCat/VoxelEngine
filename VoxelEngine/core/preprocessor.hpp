#pragma once


// ve_init_order controls static initialization order.
// Prefer using methods returning a local static variable to avoid using this.
// Allowed values are in [0, 65535]. Lower numbers will be initialized first.
#define ve_init_order(Priority) __attribute__((init_priority(Priority + 101)))


// Useful for passing macro arguments that contain a comma.
#define VE_COMMA ,

// Useful as a replacement for BOOST_PP_EMPTY when a non-empty parameter list will be appended.
#define VE_EMPTY(...)