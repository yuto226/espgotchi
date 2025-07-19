idf_component_register(SRCS "tamalib.c"
                              "cpu.c" 
                              "hw.c"
                    INCLUDE_DIRS ".")

# Disable maybe-uninitialized warning for this component
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-maybe-uninitialized)