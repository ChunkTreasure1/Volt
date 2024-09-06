import os
import sys
import maya.OpenMayaMPx as OpenMayaMPx
import voltExport

PLUGIN_NAME = "Volt Export"
PLUGIN_COMPANY = "Volt"
FILE_EXT = 'vtmesh'

class VoltTranslator(OpenMayaMPx.MPxFileTranslator):
    def __init__(self):
        OpenMayaMPx.MPxFileTranslator.__init__(self)
        self.kwargs = {}

    def haveWriteMethod(self):
        return True

    def haveReadMethod(self):
        return False

    def filter(self):
        return "*.{}".format(FILE_EXT)

    def defaultExtension(self):
        return FILE_EXT

    def writer(self, file_obj, opt_string, access_mode):
        fullName = file_obj.fullName()
        try:
            if access_mode == OpenMayaMPx.MPxFileTranslator.kExportAccessMode:
                self._parse_args(opt_string)
                voltExport.export(file_obj.fullName(), **self.kwargs)
            
            elif access_mode == OpenMayaMPx.MPxFileTranslator.kExportActiveAccessMode:
                self._parse_args(opt_string)
                raise NotImplementedError("Not implemented!")

        except:
            sys.stderr.write("Failed to write file information\n")
            raise
    
    def _parse_args(self, opt_string):
        return

    def reader(self, fileObject, optionString, accessMode):
        return NotImplementedError()

    def identifyFile(file_obj, buffer, size):
        basename, ext = os.path.splitext(file_obj.fullName())
        if ext not in ['.vtmesh']:
            return OpenMayaMPx.MPxFileTranslator.kNotMyFileType

        return OpenMayaMPx.MPxFileTranslator.kIsMyFileType

def TranslatorCreator():
    return OpenMayaMPx.asMPxPtr(VoltTranslator())

def initializePlugin(mobject):
    mplugin = OpenMayaMPx.MFnPlugin(mobject, PLUGIN_COMPANY, '1.0', "Any")
    try:
        mplugin.registerFileTranslator(PLUGIN_NAME, None, TranslatorCreator, "voltTranslatorOpts", "createMaterial=1;")
    except:
        sys.stderr.write("Failed to register translator: %s" % PLUGIN_NAME)
        raise

def uninitializePlugin(mobject):
    mplugin = OpenMayaMPx.MFnPlugin(mobject)
    try:
        mplugin.deregisterFileTranslator(PLUGIN_NAME)
    except:
        sys.stderr.write("Failed to deregister translator: %s" % PLUGIN_NAME)
        raise