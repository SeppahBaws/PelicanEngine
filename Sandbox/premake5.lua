local targetDir = ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
local objDir = ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

project "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    warnings "extra"

    targetdir(targetDir)
    objdir(objDir)

    files
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
        "%{wks.location}/Pelican/src",
        "%{wks.location}/Pelican/vendor",
        "%{IncludeDir.Glm}",
        "%{IncludeDir.Vulkan}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.ImGui}"
    }

    links
    {
        "Pelican"
    }

    if (_OPTIONS["use-vld"]) then
        defines { "SANDBOX_USE_VLD" }
        includedirs
        {
            (VLDLocation .. "/include")
        }
        libdirs
        {
            (VLDLocation .. "/lib/Win64")
        }
        links
        {
            "vld"
        }
    end

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

        postbuildcommands
        {
            "{COPYDIR} \"%{wks.location}/Pelican/dependencies/assimp/build/bin/Debug\" \"%{cfg.targetdir}\""
        }

    filter "configurations:Release"
        runtime "Release"
        optimize "on"

        postbuildcommands
        {
            "{COPYDIR} \"%{wks.location}/Pelican/dependencies/assimp/build/bin/Release\" \"%{cfg.targetdir}\""
        }
