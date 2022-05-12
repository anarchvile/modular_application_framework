# pyRunner + inputPython + eventPython

# This plugin showcases how python-binded C++ plugins can be utilized within python, including how
# we can create custom events in python. We are able to subscribe python-defined functions or exposed functions 
# of other python-binded plugins to these python-defined events, and these python events will act in the same
# way as their C++ counterparts (i.e. both can be created, destroyed, call all their handlers, have functions
# be subscribed/unsubscribed, etc.). In this example we create a pyRunner event (basically the pure-Python version
# of the C++ runner) and subscribe to it some python functions and python-binded plugin functions. We also show how
# methods can be either directly pushed to inputPython for updating, or subscribed to the input event 
# (either input_keyboard or input_mouse depending on which system one wishes to use to call a function) via eventPython.

# Have a global macro to switch between synchronous and multithreaded execution for this plugin.
DEFINE_SYNCHRONOUS = False

import plugin
import helloWorldPython
import goodbyeWorldPython
import inputPython
import concurrent.futures
import time
import eventPython
import os
from enum import Enum

# Have a global macro to subscribe handlers to the runner and input events either one-at-a-time or simultaneously
class Sub(Enum):
    multiple_list = 1
    multiple_args = 2

DEFINE_SUBSCRIPTION = Sub.multiple_list

class PyExPlg2(plugin.Plugin):

    def __init__(self):
        self.cnt_1 = 0
        self.cnt_2 = 0
        self.cnt_3 = 0

        self.g_ids_pyrunner = []
        self.g_ids_input_keyboard = []
        self.g_ids_input_mouse = []

        # EventStreamPythonDouble (espd)
        self.espd = 0
        
        self.dt = 0
        self.counter = [0]

        # A wrapper function to kickstart our custom pyRunner. Will be useful when attempting to start pyRunner in its own thread.
        def pyRunner_start(func, name, idx):
            while idx < 10:
                func(name, idx)
                idx += 1
                self.counter[0] = self.counter[0] + 1
        
        # A python-defined function that'll be subscribed to our custom python-defined event.
        def update_func_pyRunner(dt):
            time.sleep(1)
            print("PyExPlg2 pyRunner update func: " + str(dt) + ", " + str(self.cnt_1))
            if self.cnt_1 == 4:
                self.cnt_1 = 0
            else:
                self.cnt_1 = self.cnt_1 + 1

        # A python-defined function that'll be pushed directly to the C++ input plugin via inputPython; it'll be called
        # every time a mouse action is taken. Note that every time this function is called, a counter is incremented; once
        # this counter hits 100, stop() is called on the entire plugin.
        def update_func_input_direct_mouse(input_data):
            print("PyExPlg2 python update function directly subscribed to inputPython's mouse: " + str(self.cnt_2))
            if self.cnt_2 >= 100:
                self.cnt_2 = 0
                self.stop(self)
            else:
                self.cnt_2 = self.cnt_2 + 1

        # A python-defined function that'll be subscribed to the C++ input_keyboard event via eventPython. When the internal counter
        # in this function reaches 20, stop() is called on the entire plugin.
        # Note that in this example, if the keyboard event is used to call stop() on the plugin, the entire process gets halted because
        # the keyboard input cycle doesn't complete (the function used to call stop() is subscribed to the keyboard input event, which
        # gets called first before the directly-pushed print function gets a chance to be called). ThIS WORKS AS INTENDED - the plugin 
        # developer is responsible for ensuring that their stop() function works correctly.
        def update_func_input_event_keyboard(input_data):
            print("PyExPlg2 python update function subscribed to inputPython's keyboard event: " + str(self.cnt_3))
            if self.cnt_3 == 20:
                self.cnt_3 = 0
                self.stop(self)
            else:
                self.cnt_3 = self.cnt_3 + 1

        self.pyRunner_start = pyRunner_start
        self.update_func_pyRunner = update_func_pyRunner
        self.update_func_input_direct_mouse = update_func_input_direct_mouse
        self.update_func_input_event_keyboard = update_func_input_event_keyboard

    def initialize(self, identifier):
        # Load each plugin into the current environment.
        helloWorldPython.load(identifier)
        goodbyeWorldPython.load(identifier)
        inputPython.load(identifier)
        
        # Pass this script's path to the EventStream creation function.
        pypath = os.path.dirname(os.path.realpath(__file__))
        self.espd = eventPython.EventStreamPythonDouble(pypath)
        # Create a custom, python-defined event called pyRunner using the eventPython bindings. pyRunner is a pythonic equivalent
        # to the C++ runner event present in the runner plugin.
        self.espd.create("pyRunner")

        print("PyExPlg2 successfully loaded!")

    def release(self):
        # Unload each plugin from the curent environment.
        helloWorldPython.unload()
        goodbyeWorldPython.unload()
        inputPython.unload()
        # Destroy the custom pyRunner event using eventPython bindings.
        self.espd.destroy("pyRunner")
        self.espd.request_delete()

        print("PyExPlg2 successfully unloaded!")

    def start(self):
        if DEFINE_SYNCHRONOUS:
            # Push a python-defined function (self.update_func_input_direct_mouse) directly to a python-binded updating plugin (inputPython's mouse).
            inputPython.push("update_func_input_direct_mouse", 0, 0, False, self.update_func_input_direct_mouse)
            # Push a python-binded plugin's (helloWorldPython) function (print_hello_world) directly to a python-binded updating plugin 
            # (inputPython's keyboard).
            inputPython.push("print_hello_world", 1, 1, True, helloWorldPython.print_hello_world)
            
            # Subscribe a python-defined function (self.update_func_pyRunner) and a python-binded plugin's (goodbyeWorldPython) function 
            # (print_goodbye_world) to an event we created purely in python (pyRunner).            
            if DEFINE_SUBSCRIPTION == Sub.multiple_list:
                self.g_ids_pyrunner = self.espd.subscribe("pyRunner", [self.update_func_pyRunner, goodbyeWorldPython.print_goodbye_world])
            elif DEFINE_SUBSCRIPTION == Sub.multiple_args:
                self.g_ids_pyrunner = self.espd.subscribe("pyRunner", self.update_func_pyRunner, goodbyeWorldPython.print_goodbye_world)

            # Subscribe a python-defined function (self.update_func_input_event_keyboard) to the keyboard input event defined in C++ 
            # through the eventPython binding. Also subscribe a python-binded plugin's (goodbyeWorldPython) function (another_goodbye_world_func)
            # to the mouse input event defined in C++ through the eventPython binding.
            if DEFINE_SUBSCRIPTION == Sub.multiple_list:
                self.g_ids_input_keyboard = self.espd.subscribe("input_keyboard", [self.update_func_input_event_keyboard])
                self.g_ids_input_mouse = self.espd.subscribe("input_mouse", [goodbyeWorldPython.another_goodbye_world_func])
            elif DEFINE_SUBSCRIPTION == Sub.multiple_args:
                self.g_ids_input_keyboard = self.espd.subscribe("input_keyboard", self.update_func_input_event_keyboard)
                self.g_ids_input_mouse = self.espd.subscribe("input_mouse", goodbyeWorldPython.another_goodbye_world_func)
        
            # helloWorldPython.start() subscribes some C++ functions to a C++ runner/input plugin directly, 
            # which continues printing messages until stop is called (which in this example plugin 
            # occurs in update_func_runner_direct).
            #helloWorldPython.start()
            # goodbyeWorldPython.start() subscribes some C++ functions to a C++ runner's event, 
            # which continues printing messages until stop is called (which in this example plugin 
            # occurs in update_func_runner_direct).
            #goodbyeWorldPython.start()
            # inputPython.start() simply starts the input plugin.
            self.pyRunner_start(self.espd.call, "pyRunner", self.dt)
            inputPython.start()

        else:
            # The start functions below perform the same tasks as those in the previous if-block. The only difference is that
            # they are called asynchronously in multiple threads.
            with concurrent.futures.ThreadPoolExecutor(max_workers = 5) as executor:
                executor.submit(inputPython.push, "update_func_input_direct_mouse", 0, 0, False, self.update_func_input_direct_mouse)
                executor.submit(inputPython.push, "print_hello_world", 1, 1, True, helloWorldPython.print_hello_world)
                if DEFINE_SUBSCRIPTION == Sub.multiple_list:
                    executor.submit(self.espd.subscribe, self.g_ids_pyrunner, "pyRunner", [self.update_func_pyRunner, goodbyeWorldPython.print_goodbye_world])
                    executor.submit(self.espd.subscribe, self.g_ids_input_keyboard, "input_keyboard", [self.update_func_input_event_keyboard])
                    executor.submit(self.espd.subscribe, self.g_ids_input_mouse, "input_mouse", [goodbyeWorldPython.another_goodbye_world_func])
                elif DEFINE_SUBSCRIPTION == Sub.multiple_args:
                    executor.submit(self.espd.subscribe, self.g_ids_pyrunner, "pyRunner", self.update_func_pyRunner, goodbyeWorldPython.print_goodbye_world)
                    executor.submit(self.espd.subscribe, self.g_ids_input_keyboard, "input_keyboard", self.update_func_input_event_keyboard)
                    executor.submit(self.espd.subscribe, self.g_ids_input_mouse, "input_mouse", goodbyeWorldPython.another_goodbye_world_func)
                #executor.submit(helloWorldPython.start)
                #executor.submit(goodbyeWorldPython.start)
                executor.submit(inputPython.start)
                executor.submit(self.pyRunner_start, self.espd.call, "pyRunner", self.dt)

    # Why is stop() getting executed twice? Because we call stop both in self.update_func_input_direct_mouse/self.update_func_input_event_keyboard
    # (to stop the infinite loop from running) and in main.
    def stop(self):
        while self.counter[0] < 10:
            continue

        if DEFINE_SYNCHRONOUS:
            # Unsubscribe the function(s) associated with self.g_ids_pyrunner (i.e. self.update_func_pyRunner and goodbyeWorldPython.print_goodbye_world)
            # from the "pyRunner" event.
            self.espd.unsubscribe("pyRunner", self.g_ids_pyrunner)
            # Unsubscribe the function(s) associated with self.g_ids_input_keyboard (i.e. self.update_func_input_event_keyboard)
            # from the "input_keyboard" event.
            self.espd.unsubscribe("input_keyboard", self.g_ids_input_keyboard)    
            # Unsubscribe the function(s) associated with self.g_ids_input_mouse (i.e. goodbyeWorldPython.another_goodbye_world_func)
            # from the "input_keyboard" event.
            self.espd.unsubscribe("input_mouse", self.g_ids_input_mouse)
            # helloWorldPython.stop() pops off the C++ functions it originally pushed directly to the runner/input plugin, so that they are
            # no longer called/updated by runner/input.
            #helloWorldPython.stop()
            # goodbyeWorldPython.stop() unsubscribes the C++ functions it originally subscribed to the C++ runner event, so that they
            # are no longer called/updated by said runner event.
            #goodbyeWorldPython.stop()
            # Pop off the functions we initially pushed to the python-binded input plugin.
            inputPython.pop("update_func_input_direct_mouse", 0, 0, False, self.update_func_input_direct_mouse)
            inputPython.pop("print_hello_world", 1, 1, True, helloWorldPython.print_hello_world)
            # Stop the input plugin.
            inputPython.stop()
        else:
            # Performs the same tasks as the previous if-block asynchronously/in a multi-threaded environment.
            with concurrent.futures.ThreadPoolExecutor(max_workers = 5) as executor:
                executor.submit(self.espd.unsubscribe, "pyRunner", self.g_ids_pyrunner)
                executor.submit(self.espd.unsubscribe, "input_keyboard", self.g_ids_input_keyboard)
                executor.submit(self.espd.unsubscribe, "input_mouse", self.g_ids_input_mouse)
                #executor.submit(helloWorldPython.stop)
                #executor.submit(goodbyeWorldPython.stop)
                executor.submit(inputPython.pop, "update_func_input_direct_mouse", 0, 0, False, self.update_func_input_direct_mouse)
                executor.submit(inputPython.pop, "print_hello_world", 1, 1, True, helloWorldPython.print_hello_world)
                executor.submit(inputPython.stop)