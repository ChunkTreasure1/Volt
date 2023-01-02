from re import sub
import sys
import subprocess
import os

from SetupPython import Python

Python.CheckPython()

import colorama

from colorama import Fore
from SetupPremake import Premake

colorama.init()

Premake.CheckPremake()

sys.stdout.write(Fore.WHITE)