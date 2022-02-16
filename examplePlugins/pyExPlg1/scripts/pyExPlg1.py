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
# -Have a global macro to switch between synchronous and multithreaded execution for this plugin.

DEFINE_SYNCHRONOUS = True

import plugin
import runnerPython
import helloWorldPython
import goodbyeWorldPython
import eventPython
import concurrent.futures
import time

class PyExPlg1(plugin.Plugin):

    def __init__(self):
        self.cnt_1 = 0
        self.cnt_2 = 0

        self.g_id1 = 0
        self.g_id2 = 0

        def update_func_runner_direct(dt):
            time.sleep(1)
            print("PyExPlg1 python update function directly subscribed to runnerPython: " + str(dt) + ", " + str(self.cnt_1))
            if self.cnt_1 == 5:
                self.cnt_1 = 0
                self.stop(self)
            else:
                self.cnt_1 = self.cnt_1 + 1

        def update_func_runner_event(dt):
            time.sleep(1)
            print("PyExPlg1 python update function subscribed to runnerPython's event: " + str(dt) + ", " + str(self.cnt_1))
            if self.cnt_2 == 7:
                self.cnt_2 = 0
            else:
                self.cnt_2 = self.cnt_2 + 1
        
        self.update_func_runner_direct = update_func_runner_direct
        self.update_func_runner_event = update_func_runner_event

    def initialize(self, identifier):
        runnerPython.load(identifier)
        helloWorldPython.load(identifier)
        goodbyeWorldPython.load(identifier)
        runnerPython.push("update_func_runner_direct", 0, self.update_func_runner_direct)
        runnerPython.push("print_hello_world", 1, helloWorldPython.print_hello_world)

        print("PyExPlg1 successfully loaded!")

    def release(self):
        helloWorldPython.unload()
        goodbyeWorldPython.unload()
        runnerPython.unload()

        print("PyExPlg1 successfully unloaded!")

    def start(self):
        self.g_id1 = eventPython.subscribe("runner", self.update_func_runner_event)
        self.g_id2 = eventPython.subscribe("runner", goodbyeWorldPython.print_goodbye_world)
        if DEFINE_SYNCHRONOUS:
            # helloWorldPython.start() subscribes some C++ functions to a C++ runner directly, 
            # which continues printing messages until stop is called (which in this example plugin 
            # occurs in update_func_runner).
            helloWorldPython.start()
            # goodbyeWorldPython.start() subscribes some C++ functions to a C++ runner's event, 
            # which continues printing messages until stop is called (which in this example plugin 
            # occurs in update_func_runner).
            goodbyeWorldPython.start()
            runnerPython.start()
        else:
            with concurrent.futures.ThreadPoolExecutor(max_workers = 5) as executor:
                executor.submit(helloWorldPython.start)
                executor.submit(goodbyeWorldPython.start)
                executor.submit(runnerPython.start)

    def stop(self):
        if DEFINE_SYNCHRONOUS:
            eventPython.unsubscribe("runner", self.g_id1)
            eventPython.unsubscribe("runner", self.g_id2)
            helloWorldPython.stop()
            goodbyeWorldPython.stop()
            runnerPython.pop("update_func_runner_direct", 0, self.update_func_runner_direct)
            runnerPython.pop("print_hello_world", 1, helloWorldPython.print_hello_world)
            runnerPython.stop()
        else:
            with concurrent.futures.ThreadPoolExecutor(max_workers = 5) as executor:
                executor.submit(eventPython.unsubscribe, "runner", self.g_id1)
                executor.submit(eventPython.unsubscribe, "runner", self.g_id2)
                executor.submit(helloWorldPython.stop)
                executor.submit(goodbyeWorldPython.stop)
                executor.submit(runnerPython.pop, "update_func_runner_direct", 0, self.update_func_runner_direct)
                executor.submit(runnerPython.pop, "print_hello_world", 1, self.print_hello_world)
                executor.submit(runnerPython.stop)
