local function getAssemblyFiles(directory, is_windows)
	if is_windows then
		handle = io.popen("dir " .. directory .. " /B /A-D")
	end

	t = {}
	for f in handle:lines() do
		if path.hasextension(f, ".dll") then
			if string.find(f, "System.") then
				table.insert(t, f)
			end
		end
	end

	handle:close()
	return t
end

function linkAppReferences()
	local voltDir = os.getenv("VOLT_PATH")
	local monoLibsPath = path.join(voltDir, "Scripts", "mono", "lib", "mono", "4.5")
	local is_windows = os.istarget('windows')

	if is_windows then
		monoLibsPath = monoLibsPath:gsub("/", "\\")
	end

	libdirs { monoLibsPath, monoLibsPath .. "/Facades" }
	links { "Volt-ScriptCore" }

	for k, v in ipairs(getAssemblyFiles(monoLibsPath, is_windows)) do
		print("Adding reference to: " .. v)
		links { v }
	end

	for k, v in ipairs(getAssemblyFiles(monoLibsPath .. "/Facades", is_windows)) do
        print("Adding reference to: " .. v)
        links { v }
    end
end