# Generate debugging symbols.
add_compile_options(-g3)

# Enable warnings.
add_compile_options(-Wall -Wextra -pedantic)

# Put everything in separate sections to improve stripping.
add_compile_options(-ffunction-sections -fdata-sections)
add_link_options("$<$<CONFIG:Release>:-Wl,--gc-sections>")

# Configure for a 32-bit build.
add_compile_options(-m32)
add_link_options(-m32)

# Disable all default libraries.
add_compile_options(-nostdlib -nostartfiles -static)
add_link_options(-nostdlib -nostartfiles -static -Wl,--build-id=none)

# Assume that we're compiling the whole program (which improves optimizations).
add_compile_options(-fwhole-program)

# Disable position-independent code, since we have no lib deps.
add_compile_options(-fno-pic)

# Disable stack protectors, since this requires some library code.
add_compile_options(-fno-stack-protector)

# Drop frame pointers in release builds since this generates extra instructions.
add_compile_options("$<$<CONFIG:Release>:-fomit-frame-pointer>")

# Automatically include the bootstrapping header.
add_compile_options(-e _start -include "${CMAKE_CURRENT_SOURCE_DIR}/src/core/start.h")

# Use the custom linker script.
add_link_options("$<$<CONFIG:Release>:-T${CMAKE_CURRENT_SOURCE_DIR}/src/core/link.ld>")
