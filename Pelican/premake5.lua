project "Pelican"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    warnings "extra"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "PelicanPCH.h"
    pchsource "src/PelicanPCH.cpp"

    files
    {
        "src/**.h",
        "src/**.cpp"
    }

    defines
    {
        "GLFW_INCLUDE_NONE"
    }

    includedirs
    {
        "src",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glm}",
        "%{IncludeDir.Logtools}",
        "%{IncludeDir.stb}",
        "%{IncludeDir.json_hpp}",
        "%{IncludeDir.tiny_gltf}",
        "%{IncludeDir.Vulkan}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.assimpBuild}",
    }

    libdirs
    {
        "%{LibDir.Vulkan}"
    }

    links
    {
        "GLFW",
        "ImGui",
        "Assimp",
        "vulkan-1"
    }

    filter "system:windows"
        systemversion "latest"
        system "Windows"

        defines
        {
            "PELICAN_WINDOWS"
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
