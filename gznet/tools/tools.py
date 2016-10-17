import os

from SCons.Script import *

Env = None
my_root = ''
BuildOptions = {}


def GetCurrentDir():
    sconscript = File('Sconscript')
    fn = sconscript.rfile()
    path = os.path.dirname(fn.abspath)
    return path


def SrcRemove(src, remove):
    if isinstance(src[0], str):
        for item in src:
            if os.path.basename(item) in remove:
                src.remove(item)
        return

    for item in src:
        if os.path.basename(item.rstr()) in remove:
            src.remove(item)


def GetDepend(depend):
    building = True
    if isinstance(depend, str):
        if (not depend in BuildOptions) or (BuildOptions[depend] == 0):
            building = False
        elif BuildOptions[depend] != '':
            return BuildOptions[depend]

        return building

    for item in depend:
        if item != '':
            if (not item in BuildOptions) or (BuildOptions[item] == 0):
                building = False

    return building


def DefineGroup(name, src, depend, **param):
    global Env
    if not GetDepend(depend):
        return []

    group = param

    group['name'] = name

    if isinstance(src, list):
        group['src'] = File(src)
    else:
        group['src'] = src

    if 'CPPPATH' in group:
        Env.Append(CPPPATH=group['CPPPATH'])
    if 'CCFLAGS' in group:
        Env.Append(CCFLAGS=group['CCFLAGS'])
    if 'CPPDEFINES' in group:
        Env.Append(CPPDEFINES=group['CPPDEFINES'])
    if 'LINKFLAGS' in group:
        Env.Append(LINKFLAGS=group['LINKFLAGS'])

    objs = Env.Object(group['src'])

    if 'LIBRARY' in group:
        objs = Env.Library(name, objs)

    # print(Env['CPPPATH'])
    return objs


def PrepareBuilding(env, root_directory):
    import SCons.cpp
    # import config

    global Env
    global my_root

    Env = env
    my_root = root_directory

    Repository(my_root)

    Env.Append(CPPPATH=my_root)

    objs = []
    return objs
