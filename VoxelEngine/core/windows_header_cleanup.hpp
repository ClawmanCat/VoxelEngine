// Note: #pragma once omitted intentionally.


// This header should be included after any header that includes Windows.h to undef Windows' macros that break stuff.
// Note that CMakeLists.txt already defines some macros to reduce the amount of macros Windows spits out.
#undef near
#undef far
#undef NEAR
#undef FAR
#undef TRUE
#undef FALSE
#undef IN
#undef OUT
#undef OPTIONAL
#undef CONST