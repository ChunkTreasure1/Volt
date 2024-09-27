from re import sub
import sys
import subprocess
import os

from SetupPython import Python

def GenerateProjects():
        os.chdir('../')
        subprocess.call(['GenerateProjects.bat'])


Python.CheckPython()

import colorama

from colorama import Fore
from SetupSharpmake import Sharpmake

os.chdir("Scripts")

colorama.init()

print("")
Sharpmake.CheckSharpmake()

sys.stdout.write(Fore.WHITE)

GenerateProjects()