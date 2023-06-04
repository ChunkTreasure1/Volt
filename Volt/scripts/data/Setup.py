from re import sub
import sys
import subprocess
import os

from SetupPython import Python

def GenerateProjects():
        subprocess.call("Win-GenProjects-vs2022.bat")


Python.CheckPython()

import colorama

from colorama import Fore
from SetupPremake import Premake
from SetupVulkan import Vulkan

colorama.init()

Vulkan.CheckVulkan()
print("")
Premake.CheckPremake()

os.chdir('../../Engine/Setup')
subprocess.call("VoltSetup.exe")
os.chdir('../../Volt/scripts')

sys.stdout.write(Fore.WHITE)
GenerateProjects()