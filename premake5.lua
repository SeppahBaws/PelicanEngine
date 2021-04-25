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

LibDir = {}
LibDir["Vulkan"] = (vulkanDir .. "/Lib")

group "Dependencies"
    include "Pelican/dependencies/glfw"
    include "Pelican/dependencies/imgui"
group ""

group "Engine"
    include "Pelican"
    include "PelicanEd"
group ""

include "Sandbox"
