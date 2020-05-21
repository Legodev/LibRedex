from time import sleep, time
from ctypes import CFUNCTYPE, c_int, c_char_p
from interface.extension import Extension
from interface.helper import getCompiledOutput

from cffi import FFI

instance = Extension(dlpath='../Binaries/libredex_x64.so')

print("---- asking for version using sync")
out = instance.ExecuteAndPrint("version", [])
print("")
"""
print("---- asking for version using async")
out = instance.ExecuteAndPrint("aversion", [])
print("")

print("---- asking for version using callback")
out = instance.ExecuteAndPrint("cversion", [])
print("")

out = instance.ExecuteAndPrint("initdb", [
    "poolsize", "4",
    "worlduuid", "11E71379043C8906833008626681DE3D"])

print("--------------------------------------")

ndump_start = time()
out = instance.ExecuteAndPrint("dumpObjects", [])
ndump_time = time() - ndump_start

print("--------------------------------------")

cdump_start = time()
out = instance.ExecuteAndPrint("cdumpObjects", [])
cdump_time = time() - cdump_start

print("normal dumpObjects", ndump_time, "callback dumpObjects", cdump_time)
"""

out = instance.ExecuteAndPrint("startrestserver", ["restaddress", "127.0.0.1", "restport", "8000"])
print("")

sleep(30)