from cffi import FFI
import ast
from time import sleep
import string
import random

def stringgenerator(size=6, chars=string.ascii_uppercase):
    return ''.join(random.choice(chars) for _ in range(size))

class Extension:
    buffersize = 4096
    ffi = FFI()
    header = '''
    void RVExtension(char *output, int outputSize, const char *function);
    void RVExtensionArgs(char *output, int outputSize, const char *function, const char ** args, int argsCnt);
    '''

    def __init__(self):
        self.ffi.cdef(self.header)

        self.lib = self.ffi.dlopen('../../Binaries/libredex_x64.so', self.ffi.RTLD_NOW | self.ffi.RTLD_GLOBAL)
        print('Loaded lib {0}'.format(self.lib))
        
    def ExecuteAndPrint(self, functions, arguments=[]):
        cbuffer = self.ffi.new("char[]", self.buffersize)
        cbuffersize = self.ffi.cast("int", self.buffersize)
        
        function = self.ffi.new("char[]", functions.encode('utf-8'))
        arglist = []
        for arg in arguments:
            arglist.append(self.ffi.new("char[]", arg.encode('utf-8')))

        args = self.ffi.new("char*[]", arglist)
        
        argsCnt = self.ffi.cast("int", arglist.__len__())
        
        print ('SENDING: %s - %s' % (functions, arguments))
        self.lib.RVExtensionArgs(cbuffer, cbuffersize, function, args, argsCnt)
        mybytes = self.ffi.string(cbuffer).decode('utf-8')
        print ('OUTPUT: %s' % mybytes)
        
        try:
            mybyteseval = ast.literal_eval(mybytes.replace('""', '\\"'))
#             print ('EVALUATED: %s' % mybyteseval)
            
            if (mybyteseval[0] == "MSG"):
                return mybytes
            
            elif (mybyteseval[0] == "MPMSG"):
                return self.ContinueGet(mybyteseval)
            
            elif (mybyteseval[0] == "ASYNC"):
                return self.ContinueGet(mybyteseval)
            
            elif (mybyteseval[0] == "ERROR"):
                return mybytes
            
            else:
                return "NOT CATCHED type: " + mybytes
        except:
            return mybytes
    
    def ContinueGet(self, mybyteseval):
        if (mybyteseval[0] == "MPMSG"):
            concatinated = mybyteseval[2]
            while True:
                output = self.ExecuteAndPrint("rcvmsg", ["msguuid", mybyteseval[1]])
                
                if (output == "DONE GETTING CONTENT"):
                    break
                
                concatinated += output
            
            return concatinated
        
        elif (mybyteseval[0] == "ASYNC"):
            output = ""
            while True:
                output = self.ExecuteAndPrint("chkasmsg", ["msguuid", mybyteseval[1]])
                
                if (output == "MESSAGE DOES EXIST"):
                    break
                
                sleep(0.05)

            return self.ExecuteAndPrint("rcvasmsg", ["msguuid", mybyteseval[1]])
        
        return mybyteseval[2]
            
instance = Extension()
print(instance.ExecuteAndPrint("echo", [
                               "echostring",
                               "The quick brown fox jumps over the lazy dog. " \
                               "The quick brown fox jumps over the lazy dog. "\
                               "The quick brown fox jumps over the lazy dog. "\
                               "The quick brown fox jumps over the lazy dog. "\
                               "The quick brown fox jumps over the lazy dog. "\
                               "The quick brown fox jumps over the lazy dog. "\
                               "The quick brown fox jumps over the lazy dog. "\
                               "The quick brown fox jumps over the lazy dog."]))

print("\n\n")
out= instance.ExecuteAndPrint("GetInitOrder", [])
print(out)
print("\n\n")
out= instance.ExecuteAndPrint("GetCfgFile", ["configfiles",'["PluginManager","DesoDB","Desolation","DSZombz","ObjectMovement","ActionSystem","Realism","AntiSideChat","Jump","TimeManagement","SpookyWeather","EarPlugs","Holster","GlitchPunisher","ChernarusMap","CUPExpansion","OPA2Expansion"]'])
print(out)
print("\n\n")
out= instance.ExecuteAndPrint("getRandomNumberList", ["type","int","amount","20","start","-10","end","10"])
print(out)
print("\n\n")
out= instance.ExecuteAndPrint("getEpochTime", [])
print(out)
print("\n\n")
out= instance.ExecuteAndPrint("getDateTimeArray", [])
print(out)


out= instance.ExecuteAndPrint("initdb", [
                               "poolsize", "4",
                               "worlduuid", "11e66ac33a4ccd1c82c510bf48883ace"])
print(out)
# sleep(1)
#  
# print("\n\n")
# out= instance.ExecuteAndPrint("loadPlayer", [
#                                "nickname", "Lego",
#                                "steamid", "76561198025362180"])
# print(out)
# compiledout = ast.literal_eval(out.replace('false', '"false"'))
# playeruuid = compiledout[1][0]
#  
#   
# print("\n\n")
# out= instance.ExecuteAndPrint("loadAvChars", [
#                                "playeruuid", playeruuid])
# print(out)
#   
# print("\n\n")
# out= instance.ExecuteAndPrint("loadChar", [
#                                "playeruuid", playeruuid])
# print(out)
# 
# if out =='["MSG",[]]':
#     print("\n\n")
#     print("need to create char")
#     out= instance.ExecuteAndPrint("createChar", [
#                                   "playeruuid", playeruuid, 
#                                   "animationstate", "VAR_ANIMATIONSTATE", 
#                                   "direction", "23.5", 
#                                   "positiontype", "0", 
#                                   "positionx", "21.42", 
#                                   "positiony", "666.9", 
#                                   "positionz", "133.7", 
#                                   "classname", "sampleclass", 
#                                   "hitpoints", "[]", 
#                                   "variables", "[]", 
#                                   "persistentvariables", "[]", 
#                                   "textures", "[]", 
#                                   "inventoryuniform", "[]", 
#                                   "inventoryvest", "[]", 
#                                   "inventorybackpack", "[]", 
#                                   "uniform", "someuniform", 
#                                   "vest", "somevest", 
#                                   "backpack", "somebackpack", 
#                                   "headgear", "someheadgear", 
#                                   "googles", "somegoogles", 
#                                   "primaryweapon", "[\"someprimaryweapon\", [\"someattachment\"]]", 
#                                   "secondaryweapon", "[\"somesecondaryweapon\", [\"someattachment\"]]", 
#                                   "handgun", "[\"somehandgunweapon\", [\"someattachment\"]]", 
#                                   "tools", "[]", 
#                                   "currentweapon", "someprimaryweapon"])
#     print(out)
#     
#     print("\n\n")
#     out= instance.ExecuteAndPrint("loadChar", [
#                                    "playeruuid", playeruuid])
# 
# compiledout = ast.literal_eval(out)
# charuuid = compiledout[1][0]
# 
# print("\n\n")
# out= instance.ExecuteAndPrint("updateChar", [
#                               "charuuid", charuuid, 
#                               "animationstate", "VAR_ANIMATIONSTATE", 
#                               "direction", "23.5", 
#                               "positiontype", "0", 
#                               "positionx", "21.42", 
#                               "positiony", "666.9", 
#                               "positionz", "133.7", 
#                               "classname", "sampleclass", 
#                               "hitpoints", "[]", 
#                               "variables", "[]", 
#                               "persistentvariables", "[]", 
#                               "textures", "[]", 
#                               "inventoryuniform", "[]", 
#                               "inventoryvest", "[]", 
#                               "inventorybackpack", "[]", 
#                               "uniform", "someuniform", 
#                               "vest", "somevest", 
#                               "backpack", "somebackpack", 
#                               "headgear", "someheadgear", 
#                               "googles", "somegoogles", 
#                               "primaryweapon", "[\"someprimaryweapon\", [\"someattachment\"]]", 
#                               "secondaryweapon", "[\"somesecondaryweapon\", [\"someattachment\"]]", 
#                               "handgun", "[\"somehandgunweapon\", [\"someattachment\"]]", 
#                               "tools", "[]", 
#                               "currentweapon", "someprimaryweapon"])
# print(out)
# 
# print("\n\n")
# out= instance.ExecuteAndPrint("getUUID", [])
# print(out)
# compiledout = ast.literal_eval(out)
# objectuuid = compiledout[1]
# 
# print("\n\n")
# out= instance.ExecuteAndPrint("qcreateObject", [
#                               "objectuuid", objectuuid,
#                               "classname", stringgenerator(8),
#                               "priority", "2", 
#                               "visible", "1", 
#                               "accesscode", "", 
#                               "locked", "0", 
#                               "playeruuid", playeruuid,
#                               "hitpoints", "[]",  
#                               "damage", "0.1", 
#                               "fuel", "0.9", 
#                               "fuelcargo", "0.0", 
#                               "repaircargo", "0.0",
#                               "items", "[]", 
#                               "magazinesturret", "[]", 
#                               "variables", "[]",
#                               "animationstate", "[]", 
#                               "textures", "[]", 
#                               "direction", "23.5", 
#                               "positiontype", "0", 
#                               "positionx", "21.42", 
#                               "positiony", "666.9", 
#                               "positionz", "133.7",
#                               "positionadvanced", "[[1,2,3]]", 
#                               "reservedone", "[]", 
#                               "reservedtwo", "[]"])
# print(out)
# 
# print("\n\n")
# out= instance.ExecuteAndPrint("createObject", [
#                               "classname", stringgenerator(8),
#                               "priority", "2", 
#                               "visible", "1", 
#                               "accesscode", "", 
#                               "locked", "0", 
#                               "playeruuid", playeruuid,
#                               "hitpoints", "[]",  
#                               "damage", "0.1", 
#                               "fuel", "0.9", 
#                               "fuelcargo", "0.0", 
#                               "repaircargo", "0.0",
#                               "items", "[]", 
#                               "magazinesturret", "[]", 
#                               "variables", "[]",
#                               "animationstate", "[]", 
#                               "textures", "[]", 
#                               "direction", "23.5", 
#                               "positiontype", "0", 
#                               "positionx", "21.42", 
#                               "positiony", "666.9", 
#                               "positionz", "133.7",
#                               "positionadvanced", "[[1,2,3]]", 
#                               "reservedone", "[]", 
#                               "reservedtwo", "[]"])
# print(out)
# compiledout = ast.literal_eval(out)
# objectuuid = compiledout[1]
# 
# print("\n\n")
# out= instance.ExecuteAndPrint("updateObject", [
#                               "objectuuid", objectuuid,
#                               "classname", stringgenerator(8),
#                               "priority", "2", 
#                               "visible", "1", 
#                               "accesscode", "", 
#                               "locked", "0", 
#                               "playeruuid", playeruuid,
#                               "hitpoints", "[]",  
#                               "damage", "0.1", 
#                               "fuel", "0.9", 
#                               "fuelcargo", "0.0", 
#                               "repaircargo", "0.0",
#                               "items", "[]", 
#                               "magazinesturret", "[]", 
#                               "variables", "[]",
#                               "animationstate", "[]", 
#                               "textures", "[]", 
#                               "direction", "23.5", 
#                               "positiontype", "0", 
#                               "positionx", "21.42", 
#                               "positiony", "666.9", 
#                               "positionz", "133.7",
#                               "positionadvanced", "[[1,2,3]]", 
#                               "reservedone", "[]", 
#                               "reservedtwo", "[]"])
# print(out)
# 
# print("\n\n")
# out= instance.ExecuteAndPrint("dumpObjects", [])
# print(out)

