# This plugin showcases how python-binded C++ plugins can be utilized within python, including how
# the python-binded runner/input plugin can work in this context. We are able to either directly add
# python-defined functions or exposed functions of other python-binded plugins to runner/input descriptors, 
# OR have those functions be subscribed to the corresponding events for runner/input (which were defined in C++).

# 3 python plugins in total. One is pythonRunner, which has inputPython and a custom pyRunner. Two is runnerInput, which
# has inputPython and runnerPython (basically this plugin). Three is the commented-out stuff in this plugin. Basically
# separate pythonExamplePlugin into 2 separate plugins.

# TODO
# -Push a python-defined function directly to a python-binded updating plugin (runner).
# -Push a python-binded plugin's function directly to a python-binded updating plugin (runner).
# -Subscribe a python-defined function to the runner event defined in C++ through the runnerPython binding.
# -Subscribe a python-binded plugin's function to the runner event defined in C++ through the runnerPython binding.

import plugin
import runnerPython
import helloWorldPython
import goodbyeWorldPython
import inputPython
import concurrent.futures
import time

def update_func_runner(dt):
#    print("PyExPlg1 update func: " + str(dt))
#    print(helloWorldPython.print_hello_world() + " - executed inside pyExPlg1.py")
#    print(helloWorld.test_func() + " - executed inside pyExPlg1.py")
#    print(goodbyeWorldPython.print_goodbye_world() + " - executed inside pyExPlg1.py")
#    if pep.counter_1 == 30:
#       #runnerPython.stop()
#        pep.stop()
#        pep.counter_1 = 0
#    else:
#        pep.counter_1 = pep.counter_1 + 1
    time.sleep(1)
    print("PyExPlg1 runner update func: " + str(dt) + ", " + str(pep.counter_1))
    if pep.counter_1 == 4:
        #runnerPython.stop()
        #pep.stop()
        pep.counter_1 = 0
    else:
        pep.counter_1 = pep.counter_1 + 1

def update_func_input(event):
    pep.counter_2 = pep.counter_2 + 1
    # if is_async = true
    if pep.counter_2 >= 100:
    # if is_async = false
    #if pep.counter_2 >= 5:
        pep.stop()
    print("PyExPlg1 input update func: " + str(pep.counter_2))

class PyExPlg1(plugin.Plugin):

    counter_1 = 0
    counter_2 = 0

    def initialize(self, identifier):
        #runnerPython.load(identifier)
        #helloWorldPython.load(identifier)
        #goodbyeWorldPython.load(identifier)
        #runnerPython.push("update_func_runner", 0, update_func_runner)

        inputPython.load(identifier)
        runnerPython.load(identifier)
        inputPython.push("update_func_input", 0, 0, False, update_func_input)
        runnerPython.push("update_func_runner", 0, update_func_runner)
        print("PyExPlg1 successfully loaded!")

    def release(self):
        #helloWorldPython.unload()
        #goodbyeWorldPython.unload()
        #runnerPython.unload()

        inputPython.unload()
        runnerPython.unload()
        print("PyExPlg1 successfully unloaded!")

    def start(self):
        #helloWorldPython.start()
        #goodbyeWorldPython.start()
        #with concurrent.futures.ThreadPoolExecutor(max_workers = 5) as executor:
        #    executor.submit(runnerPython.start)

        # is_async = true
        with concurrent.futures.ThreadPoolExecutor(max_workers = 5) as executor:
            executor.submit(inputPython.start)
            executor.submit(runnerPython.start)

        # is_async = false
        #inputPython.start()
        #with concurrent.futures.ThreadPoolExecutor(max_workers = 5) as executor:
        #    executor.submit(runnerPython.start)

        #g_id1 = eventPython.subscribe("runner", update_func_runner)
        #g_id2 = eventPython.subscribe("input", update_func_input)

    def stop(self):
        #helloWorldPython.stop()
        #goodbyeWorldPython.stop()
        #with concurrent.futures.ThreadPoolExecutor(max_workers = 5) as executor:
        #    executor.submit(runnerPython.pop, "update_func_runner", 0, update_func_runner)
        #    executor.submit(runnerPython.stop)

        # is_async = true
        with concurrent.futures.ThreadPoolExecutor(max_workers = 5) as executor:
            executor.submit(inputPython.pop, "update_func_input", 0, 0, False, update_func_input)
            executor.submit(inputPython.stop)
            executor.submit(runnerPython.pop, "update_func_runner", 0, update_func_runner)
            executor.submit(runnerPython.stop)

        # is_async = false
        #inputPython.pop("update_func_input", 0, 0, False, update_func_input)
        #inputPython.stop()
        #with concurrent.futures.ThreadPoolExecutor(max_workers = 5) as executor:
        #    executor.submit(runnerPython.pop, "update_func_runner", 0, update_func_runner)
        #    executor.submit(runnerPython.stop)

        #eventPython.unsubscribe("runner", g_id1)
        #eventPython.subscribe("input", g_id2)


pep = PyExPlg1()