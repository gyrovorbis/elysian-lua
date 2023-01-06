# ElysianLua #
ElysianLua is a Lua binding API for interoping between the C++ and the Lua languages. It is the core Lua framework powering the Elysian Shadows scripting engine within its tech stack. The API is written in C++20, making heavy usage of variadic and metatemplates as well as leveraging the new concepts and constraints additions to the language. Furthermore, it is specifically targeting the latest Lua version 5.4.

## Goals ##
As this library is being used in a game engine running on the Sega Dreamcast, it attepts to be extremely performance-minded by going to great lengths to exploit fast paths for table lookups, avoiding using registry storage except for persistent data, primarily operating on the stack, favoring compile-time polymorphism, and providing mechanisms for resource management and validation, despite still providing a higher-level C++-based API.

## Status ##
ElysianLua is still under development and is not quite feature-complete. I have lately been working on other areas of the ES tech stack, but I will absolutely be returning to finish the remaining feature support as this is the core of our scripting engine. In the meantime, it's more than usable and still has quite a lot to offer; however, it will not suffice as your only binding API in its current state.

## Features ##
* C++ wrapper API around the Lua stack C API
    * variadically templated pushes/pulls
    * push/pull pairs, tuples, and aggregate types
    * optimized table accesses for integral keys
    * optimized table accesses for global fields
* Extendible stack interop layer 
    * handles all builtin types
    * extended via custom type traits
    * extension points for push, pop, and check stack type operations
* Collection of various statically typed different references + generic concept types
    * implicit global lua_State vs explicit member lua_State storage
    * registry references vs stack references 
    * fixed vs relative stack indexed references
    * references to table fields
    * move semantics for reusing references
* Generic callable objects 
    * functions or tables/userdata with "call" metamethod
    * supports custom error message handlers 
    * regular and protected call variants
* Generic table-style operations for compatible static types
    * getting/setting fields via metamethods + raw versions
    * iterators, getLength(), getMetatable()
    * generic concatenation via overloaded operators
    * generic multi-level lookups through chaining of the index operator (ie: object["a"]["b"] = true)
* Generic LuaVariant type which can represent any valid C++ or Lua compatible type
    * templated accessors and conversion operators 
    * beautiful std::initializer of pairs declarative initializaiton of arbitrary tables
    * reference semantics for Lua functions, strings, and tables
    * value semantics for C++ and Lua primitive value types 
    * move semantics, weak references
* Dynamic byte array/string-style API built around luaL_Buffers
* Lua allocation tracking and statistics
* Resource Monitors and Scope Guards
    * ensures # of items remains balanced within a given scope
    * supports monitoring stack depth and reference counts
    * manual and RAII style interfaces for managing their scope
    * captures surrounding C++ source code context of surrounding scope
    * raises appropriate Lua warnings/errors upon unexpected remaining item counts 

## Notes ##
This library makes use of the pointer returned by lua_extraspace() to store its own internal state information, so if this is already in-use, there will be a conflict.

## Deficiences ##
ElysianLua's core functionality was pulled from our monolithic engine which had a hard requirement on the OOLua binding API. As we were already using (a heavily modified version of) it for our userdata binding, I did not see fit to implement userdata binding until the end. If you wish to use ElysianLua in a project, you will still need a binding API for binding your core classes to the Lua runtime. The best option available for this is currently the Sol3 project.

## Testing ##
Note that the testing folder utilizes a custom external testing framework with a dependency on Qt's QTest. This dependency has been removed from our tech stack, which now has its own cross-platform test framework written from the ground-up. These tests need to be ported to the new model, but yes, they were passing before we broke them. 
