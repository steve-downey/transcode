# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

if(BEMAN_TRANSCODE_USE_MODULES)
    target_sources(
        beman.transcode
        PUBLIC
            FILE_SET CXX_MODULES FILES transcode.cppm
            FILE_SET HEADERS
                FILES
                    config.hpp
                    transcode.hpp
                    todo.hpp
                    "${PROJECT_BINARY_DIR}/include/beman/transcode/config_generated.hpp"
    )
else()
    target_sources(
        beman.transcode
        PUBLIC
            FILE_SET HEADERS
                FILES
                    config.hpp
                    transcode.hpp
                    todo.hpp
                    detail/concepts.hpp
                    detail/error.hpp
                    detail/null_term.hpp
                    whatwg_decode_view.hpp
                    "${PROJECT_BINARY_DIR}/include/beman/transcode/config_generated.hpp"
    )
endif()
