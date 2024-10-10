find_path(
        metal_irconverter_INCLUDE_DIRS
        NAMES
        metal_irconverter/ir_root_descriptor_flags.h
        metal_irconverter/ir_root_signature_flags.h
        metal_irconverter/ir_comparison_function.h
        metal_irconverter/ir_shader_visibility.h
        metal_irconverter/ir_descriptor_range_flags.h
        metal_irconverter/ir_tessellator_domain.h
        metal_irconverter/ir_filter.h
        metal_irconverter/ir_tessellator_output_primitive.h
        metal_irconverter/ir_format.h
        metal_irconverter/ir_tessellator_partitioning.h
        metal_irconverter/ir_input_classification.h
        metal_irconverter/ir_texture_address_mode.h
        metal_irconverter/ir_input_primitive.h
        metal_irconverter/ir_version.h
        metal_irconverter/ir_input_topology.h
        metal_irconverter/metal_irconverter.h
        PATHS /usr/local/include/
        PATHS /usr/include/
)

find_path(
        metal_irconverter_runtime_INCLUDE_DIRS
        NAMES
        metal_irconverter_runtime/ir_tessellator_tables.h
        metal_irconverter_runtime/metal_irconverter_runtime.h
        metal_irconverter_runtime/ir_raytracing.h
        PATHS /usr/local/include
        PATHS /usr/include/
)

find_library(
        metal_irconverter_LIBRARIES
        NAMES metalirconverter
        PATHS /usr/local/lib/
        PATHS /usr/lib64/
        PATHS /usr/lib/
)

if (metal_irconverter_INCLUDE_DIRS AND metal_irconverter_LIBRARIES)
else ()
    message(FATAL_ERROR "Metal converter is required to build this project, please download/install it online. (At the time of this writing: https://developer.apple.com/download/all/?q=shader%20converter)")
endif ()