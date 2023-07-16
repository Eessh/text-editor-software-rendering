workspace("text-editor-software-rendering")
	configurations({ "Debug", "Release" })
	defines { "LOG_BOII__COLORED_LOGS", "LOG_BOII__HIGHLIGHT_WARN_ERROR_FATAL_STRINGS" }
	filter("configurations:Debug")
		defines({ "DEBUG" })
		symbols("On")
	filter("configurations:Release")
		defines({ "NDEBUG", "O3" })
		optimize("On")
	filter({})
	targetdir("bin/%{cfg.buildcfg}/")

	-- Project
	project("text-editor-software-rendering")
		filter("configurations:Debug")
			kind("ConsoleApp")
		filter("configurations:Release")
			kind("WindowedApp")
		filter({})
		language("C++")
		includedirs({
			"include",
			"log-boii",
			"toml++",
			"cpp-tokenizer",
			"SDL2-2.26.5/x86_64-w64-mingw32/include/SDL2",
			"cairo-windows-1.17.2/include",
			"freetype"
		})
		files({
			"src/*.cpp",
			"log-boii/*.c",
			"cpp-tokenizer/*.cpp",
			"res.res"
		})
		filter({ "system:windows" })
			links({
				"SDL2main",
				"SDL2",
				"cairo",
				"freetype",
				"mingw32",
				"comdlg32",
				"ole32",
				"gdi32"
			})
			libdirs({ "SDL2-2.26.5/x86_64-w64-mingw32/lib", "cairo-windows-1.17.2/lib/x64", "freetype/lib/x86_64" })
		filter({ "system:linux" })
			links({ "SDL2main", "SDL2", "cairo", "freetype" })
		filter({ "system:macos" })
			links({ "SDL2main", "SDL2", "cairo", "freetype" })
		filter({})
