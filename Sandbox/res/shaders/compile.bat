%VULKAN_SDK%/Bin32/glslc shader.vert -o vert.spv
%VULKAN_SDK%/Bin32/glslc shader.frag -o frag.spv
%VULKAN_SDK%/Bin32/glslc compute-test.comp -o compute-test.spv
@REM %VULKAN_SDK%/Bin32/glslc unlit.vert -o unlit_vert.spv
@REM %VULKAN_SDK%/Bin32/glslc unlit.frag -o unlit_frag.spv
pause