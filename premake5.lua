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

local vulkanDir = os.getenv("VULKAN_SDK")
print("Vulkan is installed at " .. vulkanDir)

newoption {
    trigger = "use-vld",
    description = "Enable the use of VLD to check for memory leaks."
}

if (_OPTIONS["use-vld"]) then
    print("VLD was enabled, memory leaks will get detected.")
end

VLDLocation = "C:/Program Files (x86)/Visual Leak Detector"

IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Pelican/dependencies/GLFW/include"
IncludeDir["Glm"] = "%{wks.location}/Pelican/dependencies/glm"
IncludeDir["Logtools"] = "%{wks.location}/Pelican/dependencies/logtools/logtools/src"
IncludeDir["stb"] = "%{wks.location}/Pelican/dependencies/stb"
IncludeDir["json_hpp"] = "%{wks.location}/Pelican/dependencies/json_hpp/include"
IncludeDir["tiny_gltf"] = "%{wks.location}/Pelican/dependencies/tiny_gltf/include"
IncludeDir["Vulkan"] = (vulkanDir .. "/Include")
IncludeDir["ImGui"] = "%{wks.location}/Pelican/dependencies/imgui"
IncludeDir["entt"] = "%{wks.location}/Pelican/dependencies/entt/include"
IncludeDir["assimp"] = "%{wks.location}/Pelican/dependencies/assimp/include"
IncludeDir["assimpBuild"] = "%{wks.location}/Pelican/dependencies/assimp/build/include"

LibDir = {}
LibDir["Vulkan"] = (vulkanDir .. "/Lib")


group "Dependencies/Assimp"
    externalproject "zlibstatic"
        location "Pelican/dependencies/assimp/build/contrib/zlib"
        uuid "0B10AC6D-B1BF-3E69-B188-19E0BA552B8A"
        kind "StaticLib"
        language "C++"
    externalproject "assimp"
        location "Pelican/dependencies/assimp/build/code"
        uuid "BF5998C7-CFC7-349F-9213-B65DC300AEF6"
        kind "SharedLib"
        language "C++"

group "Dependencies"
    include "Pelican/dependencies/glfw"
    include "Pelican/dependencies/imgui"
group ""

group "Engine"
    include "Pelican"
    include "PelicanEd"
group ""

include "Sandbox"
