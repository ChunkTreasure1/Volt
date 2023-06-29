# Building

## Windows
#### Prerequisites
* Git
* Git-LFS
* Python 3
* Visual Studio 2022
* .NET Framework 4.7.2

#### Fetching
To fetch all the files used by Volt use
```git clone --recursive https://github.com/ChunkTreasure1/Volt```

#### Generating Solution Files
To generate the solution and project files go to ```Volt/scripts``` and run Setup.bat. During the setup project, you will be asked to install Vulkan. To be able to build the Debug configuration you must also install the Shader Debugging tools 64-bit.
After you've run Setup.bat you should do it again for everything to be setup correctly.

#### Running
To run Volt, open Volt.sln and build your favorite configuration. After the build is complete you will be able to run the Sandbox project to access the editor 
