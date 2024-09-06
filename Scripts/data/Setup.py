from re import sub
import sys
import subprocess
import os

from SetupPython import Python

def GenerateProjects(filename):
        os.chdir('../')
        if (filename is not None and filename != ""):
                escaped_filename = filename.replace('\\', '\\\\')
                
                subprocess.call(['GenerateProjects.bat', '/project(\'' + escaped_filename + '\')'])
        else:
                subprocess.call(['GenerateProjects.bat'])


Python.CheckPython()

import colorama

from colorama import Fore
from SetupSharpmake import Sharpmake
from SetupVulkan import Vulkan

os.chdir("Scripts")

colorama.init()

Vulkan.CheckVulkan()
print("")
Sharpmake.CheckSharpmake()

os.chdir('../Engine/Setup')
subprocess.call("VoltSetup.exe")
os.chdir('../../Scripts')

sys.stdout.write(Fore.WHITE)

from argparse import ArgumentParser

parser = ArgumentParser()
parser.add_argument('-p', '--project')

args = parser.parse_args()

GenerateProjects(args.project)