import ctypes
from multiprocessing import shared_memory
from pathlib import Path, WindowsPath

SHARED_MEMORY_NAME = "RF2_ConsoleHWControl"
PLUGIN_NAME = "RF2_ConsoleHWControl"
PLUGIN_LOCATION = r'E:\Steam\steamapps\common\rFactor 2\Bin64\Plugins'
PLUGIN_PATH = Path(PLUGIN_LOCATION) / f'{PLUGIN_NAME}.dll'

def call_c_dll(message: str):
    if not PLUGIN_PATH.exists() and not PLUGIN_PATH.is_file():
        print("Failed")
        return

    # Load dll
    plugin_dll = ctypes.cdll.LoadLibrary(str(WindowsPath(PLUGIN_PATH)))

    log_msg = plugin_dll.Log_Extern
    log_msg.argtypes = [ctypes.c_char_p]
    log_msg.restype = ctypes.c_bool

    mem_bytes  = message.encode('cp1252')
    result = log_msg(ctypes.c_char_p(mem_bytes))
    print(result)

def call_c_dll_sm(message):
    # open shared memory
    try:
        shm = shared_memory.SharedMemory(SHARED_MEMORY_NAME, create=False)
    except FileNotFoundError:
        return

    # make sure message does not exceed message size
    mem_bytes = message.encode('cp1252')

    # write to shared memory
    shm.buf[:len(mem_bytes)] = mem_bytes

    # close shared memory handle
    shm.close()



if __name__ == '__main__':
    c_message = "PitRequest"
    #call_c_dll(c_message)
    call_c_dll_sm(c_message)
