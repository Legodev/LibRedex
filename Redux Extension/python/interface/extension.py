import ast
from time import sleep

from cffi import FFI


class Extension:
    buffersize = 4096
    callbackstore = dict()
    ffi = FFI()
    header = '''
    void RVExtension(char *output, int outputSize, const char *function);
    void RVExtensionArgs(char *output, int outputSize, const char *function, const char ** args, int argsCnt);
    void RVExtensionRegisterCallback(void(*callbackProc)(char const *name, char const *function, char const *data));
    '''

    def __init__(self, dlpath):
        self.ffi.cdef(self.header)

        # open libredex
        self.lib = self.ffi.dlopen(dlpath, self.ffi.RTLD_NOW | self.ffi.RTLD_GLOBAL)
        print('Loaded lib {0}'.format(self.lib))

        # create callback and store it as variable
        @self.ffi.callback("void(char[], char[], char[])")
        def callback(c_name, c_function, c_data):
            name = self.ffi.string(c_name).decode('ascii')
            function = self.ffi.string(c_function).decode('ascii')
            data = self.ffi.string(c_data).decode('ascii')
            print("\tCALLBACK: name =", name, "function =", function, "data =", data)

            if name == "libredex_callback":
                self.callbackstore[function] = data

        self.callback = callback
        self.lib.RVExtensionRegisterCallback(self.callback)

        print('Registered callback {0}'.format(self.callback))

    def RegisterCallback(self, callback):
        self.lib.RVExtensionRegisterCallback(callback)

    def ExecuteAndPrint(self, functions, arguments=[]):
        cbuffer = self.ffi.new("char[]", self.buffersize)
        cbuffersize = self.ffi.cast("int", self.buffersize)

        function = self.ffi.new("char[]", functions.encode('utf-8'))
        arglist = []
        for arg in arguments:
            arglist.append(self.ffi.new("char[]", arg.encode('utf-8')))

        args = self.ffi.new("char*[]", arglist)

        argsCnt = self.ffi.cast("int", arglist.__len__())

        print('SENDING: [%s, %s]' % (functions, arguments))
        self.lib.RVExtensionArgs(cbuffer, cbuffersize, function, args, argsCnt)
        mybytes = self.ffi.string(cbuffer).decode('utf-8')
        print('OUTPUT: %s' % mybytes)

        try:
            mybyteseval = ast.literal_eval(mybytes.replace('""', '\\"'))
            #             print ('EVALUATED: %s' % mybyteseval)

            if (mybyteseval[0] == "MSG"):
                return mybytes

            if (mybyteseval[0] == "CALLBACK"):
                print("waiting for callback")
                uuid = mybyteseval[1]
                while not (uuid in self.callbackstore):
                    pass
                data = self.callbackstore[uuid]
                print("got callback")
                return data

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
                sleep(0.2)

                output = self.ExecuteAndPrint("chkasmsg", ["msguuid", mybyteseval[1]])

                if (output == "MESSAGE DOES EXIST"):
                    break


            return self.ExecuteAndPrint("rcvasmsg", ["msguuid", mybyteseval[1]])

        return mybyteseval[2]
