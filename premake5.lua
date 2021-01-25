workspace "PelicanEngine"
    architecture "x86_64"
    startproject "Sandbox"

    configurations
    {
        "Debug",
        "Release"
    }

    flags
    {
        "MultiProcessorCompile",
        "FatalWarnings"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Pelican/dependencies/GLFW/include"
IncludeDir["Glm"] = "%{wks.location}/Pelican/dependencies/glm"
IncludeDir["Logtools"] = "%{wks.location}/Pelican/dependencies/logtools/logtools/src"
IncludeDir["stb"] = "%{wks.location}/Pelican/dependencies/stb"
IncludeDir["json_hpp"] = "%{wks.location}/Pelican/dependencies/json_hpp/include"
IncludeDir["tiny_gltf"] = "%{wks.location}/Pelican/dependencies/tiny_gltf/include"
IncludeDir["Vulkan"] = "%VULKAN_SDK%/Include"
IncludeDir["ImGui"] = "%{wks.location}/Pelican/dependencies/imgui"
IncludeDir["entt"] = "%{wks.location}/Pelican/dependencies/entt/include"

LibDir = {}
LibDir["Vulkan"] = "%VULKAN_SDK%/Lib"

group "Dependencies"
    include "Pelican/dependencies/glfw"
    include "Pelican/dependencies/imgui"
group ""

group "Engine"
    include "Pelican"
    include "PelicanEd"
group ""

include "Sandbox"
