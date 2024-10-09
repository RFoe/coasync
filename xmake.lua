project("coasync")
    version("0.0.1")

    add_requires("c++20")

    includes("include")

    add_cxxflags("-fcoroutines -fconcepts -freport-bug -Wno-interference-size")

    local is_master_project = os.getcwd() == os.projectdir()

    if is_master_project then
        if os.is("windows") then
            add_cxxflags("/W4")
        else
            add_cxxflags("-Wall -Wextra -Wdeprecated -Wdocumentation")
        end
    end

    if is_master_project and _OPTIONS["tests"] == "on" then
        add_subdirs("tests")
    end

    package("find")
        set_name("coasync")
        set_version(project.version())
        set_desc("A header-only async library")
        set_includedirs("include", {public = true})
        set_links("coasync")

rule("install")
    on("postbuild", function (target)
        os.cp("include/*", os.getenv("prefix") .. "/include")
        os.cp("cmake/coasync-config.cmake", os.getenv("prefix") .. "/lib/cmake/coasync")
        os.cp("coasync-config-version.cmake", os.getenv("prefix") .. "/lib/cmake/coasync")
    end)

    add_rules("install")
