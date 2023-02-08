from multiprocessing import shared_memory
from pathlib import Path, WindowsPath

SHARED_MEMORY_NAME = "RF2_ConsoleHWControl"
PLUGIN_NAME = "RF2_ConsoleHWControl"
PLUGIN_LOCATION = r'E:\Steam\steamapps\common\rFactor 2\Bin64\Plugins'
PLUGIN_PATH = Path(PLUGIN_LOCATION) / f'{PLUGIN_NAME}.dll'


def call_c_dll_sm(message):
    # open shared memory
    try:
        shm = shared_memory.SharedMemory(SHARED_MEMORY_NAME, create=False)
    except FileNotFoundError:
        return

    mem_bytes = message.encode('cp1252')

    # write to shared memory
    shm.buf[:len(mem_bytes)] = mem_bytes

    shm.close()



if __name__ == '__main__':
    c_message = "PitRequest"
    call_c_dll_sm(c_message)
