from __future__ import print_function
from __future__ import absolute_import
from __future__ import division
import json
import struct
import os
import sys
import base64
import math
import shutil
import time
import pickle

import yaml

import maya.cmds
import maya.OpenMaya as OpenMaya
try:
    from PySide.QtGui import QImage, QColor, qRed, qGreen, qBlue, QImageWriter
    from PySide.QtCore import QByteArray
except ImportError:
    from PySide2.QtGui import QImage, QColor, qRed, qGreen, qBlue, QImageWriter
    from PySide2.QtCore import QByteArray

def timeit(method):

    def timed(*args, **kw):
        ts = time.time()
        result = method(*args, **kw)
        te = time.time()

        print('%r (%r, %r) %2.2f sec' % \
              (method.__name__, args, kw, te-ts))
        return result

    return timed

class ClassPropertyDescriptor(object):

    def __init__(self, fget):
        self.fget = fget

    def __get__(self, obj, klass=None):
        if klass is None:
            klass = type(obj)
        return self.fget.__get__(obj, klass)()

    def __set__(self, obj, value):
        raise AttributeError("can't set attribute")

def classproperty(func):
    if not isinstance(func, (classmethod, staticmethod)):
        func = classmethod(func)
    return ClassPropertyDescriptor(func)

class ExportSettings(object):
    file_format = 'vtmesh'
    out_file = ''
    out_dir = ''

    @classmethod
    def SetDefaults(cls):
        cls.file_format = 'vtmesh'

    @classproperty
    def OutDir(cls):
        cls._out_dir = os.path.dirname(cls.out_file)
        return cls._out_dir

class VoltExporter(object):
    def __init__(self, file_path):
        ExportSettings.SetDefaults()
        ExportSettings.out_file = file_path

    def run(self):
        if not ExportSettings.out_file:
            ExportSettings.out_file = maya.cmds.fileDialog2(caption = "Specify a name for the file to export!", filemode = 0)[0]

        basename, ext = os.path.splitext(ExportSettings.out_file)
        if not ext in ['.vtmesh']:
            raise Exception("Output file must have vtmesh extension!")
        ExportSettings.file_format = ext[1:]

        if not os.path.exists(ExportSettings.OutDir):
            os.makedirs(ExportSettings.OutDir)

        scene = Scene()

        if not scene.nodes:
            raise RuntimeError('Scene is empty.  No file will be exported.')

        vertices = []
        indices = []
        meshCount = 0

        for node in scene.nodes:
            if not node.mesh:
                continue

            indices.extend(node.mesh.indices)

            for idx, pos in enumerate(node.mesh.positions):
                vertex = Vertex()
                vertex.position = pos
                vertex.normal = node.mesh.normals[idx]
                vertex.texCoords = node.mesh.uvs[idx]
                vertices.append(vertex)

            meshCount += 1

        bin_out = bytearray()

        # submesh count
        bin_out.extend(struct.pack('<I', meshCount))
        
        # material id
        bin_out.extend(struct.pack('<Q', 0))
        
        # vertex count
        bin_out.extend(struct.pack('<I', len(vertices)))

        # vertices
        for vert in vertices:

            # position
            bin_out.extend(struct.pack('<f', vert.position[0]))
            bin_out.extend(struct.pack('<f', vert.position[1]))
            bin_out.extend(struct.pack('<f', vert.position[2]))

            # normal
            bin_out.extend(struct.pack('<f', vert.normal[0]))
            bin_out.extend(struct.pack('<f', vert.normal[1]))
            bin_out.extend(struct.pack('<f', vert.normal[2]))

            # tangent
            bin_out.extend(struct.pack('<f', vert.tangent[0]))
            bin_out.extend(struct.pack('<f', vert.tangent[1]))
            bin_out.extend(struct.pack('<f', vert.tangent[2]))

            # bitangent
            bin_out.extend(struct.pack('<f', vert.bitangent[0]))
            bin_out.extend(struct.pack('<f', vert.bitangent[1]))
            bin_out.extend(struct.pack('<f', vert.bitangent[2]))

            # texture coords
            bin_out.extend(struct.pack('<f', vert.texCoords[0]))
            bin_out.extend(struct.pack('<f', vert.texCoords[1]))

        bin_out.extend(struct.pack('<I', len(indices)))

        for index in indices:
            bin_out.extend(struct.pack('<I', index))

        bin_out.extend(struct.pack('<f', 0))
        bin_out.extend(struct.pack('<f', 0))
        bin_out.extend(struct.pack('<f', 0))
        bin_out.extend(struct.pack('<f', 0))

        vertexOffset = 0
        indexOffset = 0

        for node in scene.nodes:
            if not node.mesh:
                continue

            bin_out.extend(struct.pack('<I', node.mesh.material.index))
            bin_out.extend(struct.pack('<I', len(node.mesh.indices)))
            bin_out.extend(struct.pack('<I', vertexOffset))
            bin_out.extend(struct.pack('<I', indexOffset))

            vertexOffset += len(node.mesh.positions)
            indexOffset += len(node.mesh.indices)

        with open(ExportSettings.out_file, 'wb') as outfile:
            outfile.write(bin_out)

def export(file_path = None, selection = False):
    VoltExporter(file_path).run()

class Vertex:
    position = [ 0.0 ] * 3
    normal = [ 0.0 ] * 3
    tangent = [ 0.0 ] * 3
    bitangent = [ 0.0 ] * 3
    texCoords = [ 0.0 ] * 2

class ExportItem(object):
    def __init__(self, name=None):
        self.name = name

class Scene(ExportItem):
    instances = []
    maya_nodes = None

    @classmethod
    def SetDefaults(cls):
        cls.instances = []

    def __init__(self, name="defaultScene", maya_nodes=None):
        super(Scene, self).__init__(name=name)

        self.index = len(Scene.instances)
        Scene.instances.append(self)
        self.nodes = []

        if maya_nodes:
            self.maya_nodes = maya_nodes
        else:
            self.maya_nodes = maya.cmds.ls(assemblies=True, long=True)
        
        for transform in self.maya_nodes:
                self.nodes.append(Node(transform))

class Node(ExportItem):
    instances = []
    maya_node = None
    matrix = None
    translation = None
    rotation = None
    mesh = None

    @classmethod
    def SetDefaults(cls):
        cls.instances = []

    def __init__(self, maya_node):
        self.maya_node = maya_node
        name = maya.cmds.ls(maya_node, shortNames = True)[0]
        super(Node, self).__init__(name = name)
        self.index = len(Node.instances)
        Node.instances.append(self)

        self.children = []
        self.translation = maya.cmds.getAttr(self.maya_node + '.translate')[0]
        self.rotation = self._get_rotation_quaternion()
        self.scale = maya.cmds.getAttr(self.maya_node + '.scale')[0]

        maya_children = maya.cmds.listRelatives(self.maya_node, children = True, fullPath = True)
        if maya_children:
            for child in maya_children:
                childType = maya.cmds.objectType(child)

                if childType == 'mesh' and not maya.cmds.getAttr(child + ".intermediateObject"):
                    mesh = Mesh(child)
                    self.mesh = mesh

    def _get_rotation_quaternion(self):
        obj=OpenMaya.MObject()
        #make a object of type MSelectionList
        sel_list=OpenMaya.MSelectionList()
        #add something to it
        #you could retrieve this from function or the user selection
        sel_list.add(self.maya_node)
        #fill in the MObject
        sel_list.getDependNode(0,obj)
        #check if its a transform
        if (obj.hasFn(OpenMaya.MFn.kTransform)):
            quat = OpenMaya.MQuaternion()
            #then we can add it to transfrom Fn
            #Fn is basically the collection of functions for given objects
            xform=OpenMaya.MFnTransform(obj)
            xform.getRotation(quat)
            # glTF requires normalize quat
            quat.normalizeIt()
        
        py_quat = [quat[x] for x in range(4)]
        return py_quat 

class Mesh(ExportItem):
    instances = []
    maya_node = None
    material = None
    
    positions = None
    normals = None
    colors = None
    uvs = None
    indices = None

    @classmethod
    def SetDefaults(cls):
        cls.instances = []

    def __init__(self, maya_node):
        self.maya_node = maya_node
        name = maya.cmds.ls(maya_node, shortNames = True)[0]
        super(Mesh, self).__init__(name = name)

        self.index = len(Mesh.instances)
        Mesh.instances.append(self)

        self._getMeshData()
        self._getMaterial()

    def _getMaterial(self):
        shadingGrps = maya.cmds.listConnections(self.maya_node,type='shadingEngine')
        # We currently only support one materical per mesh, so we'll just grab the first one.
        # TODO: support facegroups as glTF primitivies to support one material per facegroup
        shader = maya.cmds.ls(maya.cmds.listConnections(shadingGrps),materials=True)[0]
        self.material = Material(shader)

    @timeit
    def _getMeshData(self):
        maya.cmds.select(self.maya_node)
        selList = OpenMaya.MSelectionList()
        OpenMaya.MGlobal.getActiveSelectionList(selList)

        meshPath = OpenMaya.MDagPath()
        selList.getDagPath(0, meshPath)

        meshIt = OpenMaya.MItMeshPolygon(meshPath)
        meshFn = OpenMaya.MFnMesh(meshPath)
        dagFn = OpenMaya.MFnDagNode(meshPath)

        boundingBox = dagFn.boundingBox()
        
        do_color = False
        if meshFn.numColorSets():
            do_color = True
        
        self.indices = []
        self.positions = [None]*meshFn.numVertices()
        self.normals = [None]*meshFn.numVertices()
        self.colors = [None]*meshFn.numVertices()
        self.uvs = [None]*meshFn.numVertices()

        ids = OpenMaya.MIntArray()
        points = OpenMaya.MPointArray()

        if do_color:
            vertexColorList = OpenMaya.MColorArray()
            meshFn.getFaceVertexColors(vertexColorList)

        normal = OpenMaya.MVector()
        face_verts = OpenMaya.MIntArray()
        polyNormals = OpenMaya.MFloatVectorArray()
        meshFn.getNormals(polyNormals)
        
        uv_util = OpenMaya.MScriptUtil()
        uv_util.createFromList([0, 0], 2)
        uv_ptr = uv_util.asFloat2Ptr()

        while not meshIt.isDone():
            meshIt.getTriangles(points, ids)
            meshIt.getVertices(face_verts)

            face_vertices = list(face_verts)

            for point, vertex_index in zip(points, ids):
                self.indices.append(vertex_index)
                pos = (point.x, point.y, point.z)
                face_vert_id = face_vertices.index(vertex_index)
                
                norm_id = meshIt.normalIndex(face_vert_id)
                norm = polyNormals[norm_id]
                norm = (norm.x, norm.y, norm.z)

                meshIt.getUV(face_vert_id, uv_ptr, meshFn.currentUVSetName())

                u = uv_util.getFloat2ArrayItem(uv_ptr, 0, 0)
                v = uv_util.getFloat2ArrayItem(uv_ptr, 0, 1)
                uv = (u, v)

                if not self.positions[vertex_index]:
                    self.positions[vertex_index] = pos
                    self.normals[vertex_index] = norm
                    self.uvs[vertex_index] = uv

                elif not (self.positions[vertex_index] == pos and
                            self.normals[vertex_index] == norm and
                            self.uvs[vertex_index] == uv):
                    self.positions.append(pos)
                    self.normals.append(norm)
                    self.uvs.append(uv)
                    self.indices[-1] = len(self.positions) - 1

                if do_color:
                    color = vertexColorList[vertex_index]
                    self.colors[vertex_index] = (color.r, color.g, color.b)
            next(meshIt)

class Material(ExportItem):
    instances = []
    maya_node = None
    default_material_id = None

    @classmethod
    def set_defaults(cls):
        cls.instances = []
        cls.default_material_id = None

    def __new__(cls, maya_node, *args, **kwargs):
        if maya_node:
            name = maya.cmds.ls(maya_node, shortNames=True)[0]
            matches = [mat for mat in Material.instances if mat.name == name]
            if matches:
                return matches[0]
            
        return super(Material, cls).__new__(cls, *args, **kwargs)

    def __init__(self, maya_node):
        if hasattr(self, 'index'):
            return

        if maya_node is None:
            name = 'Material'
            super(Material, self).__init__(name = name)
            self.index = len(Material.instances)
            self.__class__.default_material_id = self.index

            Material.instances.append(self)
            return

        self.maya_node = maya_node
        name = maya.cmds.ls(maya_node, shortNames=True)[0]
        super(Material, self).__init__(name=name)
        
        self.index = len(Material.instances)
        Material.instances.append(self)


    @classmethod
    def _get_default_material(cls):
        if cls.default_material_id:
            return Material.instances[cls.default_material_id]
        else:
            return Material(None)