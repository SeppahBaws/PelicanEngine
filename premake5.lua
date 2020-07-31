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
IncludeDir["GLFW"] = "Pelican/dependencies/GLFW/include"
IncludeDir["Glm"] = "Pelican/dependencies/glm"
IncludeDir["Vulkan"] = "C:/VulkanSDK/1.2.141.2/Include"

LibDir = {}
LibDir["Vulkan"] = "C:/VulkanSDK/1.2.141.2/Lib"

group "Dependencies"
    include "Pelican/dependencies/glfw"
group ""

project "Pelican"
    location "Pelican"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    warnings "extra"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "PelicanPCH.h"
    pchsource "Pelican/src/PelicanPCH.cpp"

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    defines
    {
        "GLFW_INCLUDE_NONE"
    }

    includedirs
    {
        "%{prj.name}/src",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glm}",
        "%{IncludeDir.Vulkan}"
    }

    libdirs
    {
        "%{LibDir.Vulkan}"
    }

    links
    {
        "GLFW",
        "vulkan-1"
    }

    filter "system:windows"
        systemversion "latest"
        system "Windows"

        defines
        {
            "PLATFORM_WINDOWS"
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

        defines
        {
            "PELICAN_DEBUG"
        }

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

        defines
        {
            "PELICAN_RELEASE"
        }

project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    warnings "extra"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs
    {
        "Pelican/src",
        "Pelican/vendor",
        "%{IncludeDir.Glm}",
        "%{IncludeDir.Vulkan}"
    }

    links
    {
        "Pelican"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"