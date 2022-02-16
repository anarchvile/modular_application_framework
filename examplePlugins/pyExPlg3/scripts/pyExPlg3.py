# This plugin showcases how python-binded C++ plugins can be utilized within python, including how
# we can create custom events in python. We are able to subscribe python-defined functions or exposed functions 
# of other python-binded plugins to these python-defined events, and these python events will act in the same
# way as their C++ counterparts (i.e. both can be created, destroyed, call all their handlers, have functions
# be subscribed/unsubscribed, etc.). In this example we create a pyRunner event (basically the pure-Python version
# of the C++ runner) and subscribe to it some python functions and python-binded plugin functions.

import plugin
import runnerPython
import helloWorldPython
import goodbyeWorldPython
import inputPython
import concurrent.futures
import time
import eventPython

class PyExPlg3(plugin.Plugin):

    def __init__(self):
        self.g_id1 = 0
        self.g_id2 = -2
        self.g_id3 = -2
        self.cnt_1 = 0
        self.cnt_2 = 0

        # TODO: Explain why we create nested functions here, streamline structure/function calls (especially multithreading portions)
        def update_func_pyRunner(dt):
            time.sleep(1)
            print("PyExPlg3 pyRunner update func: " + str(dt) + ", " + str(self.cnt_1))
            if self.cnt_1 == 4:
                self.cnt_1 = 0
            else:
                self.cnt_1 = self.cnt_1 + 1

        def update_func_input(input_data):
            self.cnt_2 = self.cnt_2 + 1
            if self.cnt_2 >= 100:
                self.stop(self)
            print("PyExPlg3 input update func: " + str(self.cnt_2))

        def pyRunner_start(func, name, idx):
            while idx < 10:
                func(name, idx)
                idx = idx + 1

        self.update_func_pyRunner = update_func_pyRunner
        self.update_func_input = update_func_input
        self.pyRunner_start = pyRunner_start

    def initialize(self, identifier):
        helloWorldPython.load(identifier)
        goodbyeWorldPython.load(identifier)
        inputPython.load(identifier)
        eventPython.create("pyRunner")
        # Push a python-defined function (self.update_func_input) directly to a python-binded updating plugin (inputPython).
        inputPython.push("update_func_input", 0, 0, False, self.update_func_input)
        # Push a python-binded plugin's (helloWorldPython) function (print_hello_world) directly to a python-binded updating plugin (inputPython).
        inputPython.push("print_hello_world", 1, 1, True, helloWorldPython.print_hello_world)

        # TODO:
        # -Expose input's is-async option to python.
        # -Subscribe a python-defined function to the input event defined in C++ through the inputPython binding.
        # -Subscribe a python-binded plugin's function to the input event defined in C++ through the inputPython binding.

        print("PyExPlg3 successfully loaded!")

    def release(self):
        helloWorldPython.unload()
        goodbyeWorldPython.unload()
        inputPython.unload()
        eventPython.destroy("pyRunner")
        print("PyExPlg3 successfully unloaded!")

    def start(self):
        idx = 0
        # Subscribe a python-defined function (self.update_func_pyRunner) to an event we created purely in python (pyRunner).
        self.g_id2 = eventPython.subscribe("pyRunner", self.update_func_pyRunner)
        # Subscribe a python-binded plugin's (goodbyeWorldPython) function (print_goodbye_world) to an event we created purely in python (pyRunner).
        self.g_id3 = eventPython.subscribe("pyRunner", goodbyeWorldPython.print_goodbye_world)

        with concurrent.futures.ThreadPoolExecutor(max_workers = 5) as executor:
            executor.submit(inputPython.start)
            executor.submit(self.pyRunner_start, eventPython.call, "pyRunner", idx)

    def stop(self):
        with concurrent.futures.ThreadPoolExecutor(max_workers = 5) as executor:
            executor.submit(inputPython.pop, "update_func_input", 0, 0, False, self.update_func_input)
            executor.submit(inputPython.pop, "print_hello_world", 1, 1, True, helloWorldPython.print_hello_world)
            executor.submit(inputPython.stop)
            executor.submit(eventPython.unsubscribe, "pyRunner", self.g_id2)
            executor.submit(eventPython.unsubscribe, "pyRunner", self.g_id3)
