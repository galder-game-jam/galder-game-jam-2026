// Forwarding shim so FakeIt uses the bundled Catch2 v2 single header.
// fakeit.hpp probes `__has_include("catch2/catch.hpp")` first; without this shim it falls
// through to a system-installed Catch2 v3 (/usr/include/catch2), which clashes with catch.hpp.
#include "../../../../external_libs/catch.hpp"
