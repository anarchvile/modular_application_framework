# runnerPython + helloWorldPython + goodbyeWorldPython + eventPython

# This plugin showcases how python-binded C++ plugins can be utilized within python, including how
# the python-binded runner plugin can work in this context. We are able to either directly add
# python-defined functions or exposed functions of other python-binded plugins to runner descriptors, 
# OR have those functions be subscribed to the corresponding events for runner (which were defined in C++)
# via eventPython.

# Also note that we can't use callAsync in python plugins - need to implement multithreading on python end if the programmer
# wishes to have such functionality. An example of how this might be done is included as part of the implementation in both
# this python plugin and pyExPlg2.py

# Have a global macro to switch between synchronous and multithreaded execution for this plugin.
DEFINE_SYNCHRONOUS = True

import plugin
import runnerPython
import helloWorldPython
import goodbyeWorldPython
import eventPython
import concurrent.futures
import time
import os
from enum import Enum

# Have a global macro to subscribe handlers to the runner event either one-at-a-time or simultaneously
class Sub(Enum):
    multiple_list = 1
    multiple_args = 2

DEFINE_SUBSCRIPTION = Sub.multiple_list

class PyExPlg1(plugin.Plugin):

    def __init__(self):
        self.cnt_1 = 0
        self.cnt_2 = 0

        self.g_ids = []

        # EventStreamPythonDouble (espd)
        self.espd = 0

        # Rather than simply creating member methods of the class to then push/subscribe to plugins/events, we define the functions
        # independently (but still inside) of the class, create class-member python function objects, and set the latter equal to the 
        # former. We do this because there doesn't seem to be an easy way to push/subscribe a class-member method to the internal list
        # of handlers for updating without also passing a reference to the self pointer of the class, and I wasn't able to find a 
        # successful way of passing that information along. So, we decouple the function from the class by defining it independently and
        # just pass a corresponding function object defined in the class to the plugin/event we wish to push/subscribe to, without the need
        # for also passing a self pointer. We also define these functions inside the class so as to give them access to any class member
        # variables/functions we might want to modify.

        # A python-defined function that'll be pushed directly to runnerPython for updating on the tick. Note that it increments
        # a counter once every second; once that counter hits either 1 or 5 (depending on what line is commented out), we call stop() 
        # on the entire plugin.
        def update_func_runner_direct(dt):
            time.sleep(1)
            print("PyExPlg1 python update function directly subscribed to runnerPython: " + str(dt) + ", " + str(self.cnt_1))
            #if self.cnt_1 == 5:
            if self.cnt_1 == 1:
                self.cnt_1 = 0
                # Note that we stop all iterations when cnt_1 reaches 5, so cnt_2 will also be capped off at
                # 1 or 5 rather than reaching its max value of 7.
                self.stop(self)
            else:
                self.cnt_1 = self.cnt_1 + 1

        # A python-defined function that'll be subscribed to the C++ runner event via eventPython.
        def update_func_runner_event(dt):
            time.sleep(1)
            print("PyExPlg1 python update function subscribed to runnerPython's event: " + str(dt) + ", " + str(self.cnt_1))
            if self.cnt_2 == 7:
                self.cnt_2 = 0
            else:
                self.cnt_2 = self.cnt_2 + 1
        
        # As was mentioned before, we create class member function objects that copy the methods defined above.
        self.update_func_runner_direct = update_func_runner_direct
        self.update_func_runner_event = update_func_runner_event

    def initialize(self, identifier):
        # Load each plugin into the current environment.
        runnerPython.load(identifier)
        helloWorldPython.load(identifier)
        goodbyeWorldPython.load(identifier)

        # Pass this script's path to the EventStream creation function.
        pypath = os.path.dirname(os.path.realpath(__file__))
        self.espd = eventPython.EventStreamPythonDouble(pypath)

        print("PyExPlg1 successfully loaded!")

    def release(self):
        # Unload each plugin from the curent environment.
        helloWorldPython.unload()
        goodbyeWorldPython.unload()
        runnerPython.unload()
        self.espd.request_delete()

        # TODO: Destroy self.espd as well?
        print("PyExPlg1 successfully unloaded!")

    def start(self):
        if DEFINE_SYNCHRONOUS:
            # Push a python-defined function (self.update_func_runner_direct) directly to a python-binded updating plugin (runnerPython).
            runnerPython.push("update_func_runner_direct", 0, self.update_func_runner_direct)
            # Push a python-binded plugin's function (helloWorldPython.print_hello_world) directly to a python-binded updating plugin 
            # (runnerPython).
            runnerPython.push("print_hello_world", 1, helloWorldPython.print_hello_world)
            # Subscribe a python-defined function (self.update_func_runner_event) and a python-binded plugin's function 
            # (goodbyeWorldPython.print_goodbye_world) to the runner event defined in C++ through the eventPython binding.
            if DEFINE_SUBSCRIPTION == Sub.multiple_list:
                self.g_ids = self.espd.subscribe("runner", [self.update_func_runner_event, goodbyeWorldPython.print_goodbye_world])
            elif DEFINE_SUBSCRIPTION == Sub.multiple_args:
                self.g_ids = self.espd.subscribe("runner", self.update_func_runner_event, goodbyeWorldPython.print_goodbye_world)
            # helloWorldPython.start() subscribes some C++ functions to a C++ runner directly, 
            # which continues printing messages until stop is called (which in this example plugin 
            # occurs in update_func_runner_direct).
            helloWorldPython.start()
            # goodbyeWorldPython.start() subscribes some C++ functions to a C++ runner's event, 
            # which continues printing messages until stop is called (which in this example plugin 
            # occurs in update_func_runner_direct).
            goodbyeWorldPython.start()
            # runnerPython.start() simply starts the runner plugin.
            runnerPython.start()
        else:
            # The start functions below perform the same tasks as those in the previous if-block. The only difference is that
            # they are called asynchronously in multiple threads.
            with concurrent.futures.ThreadPoolExecutor(max_workers = 5) as executor:
                executor.submit(runnerPython.push, "update_func_runner_direct", 0, self.update_func_runner_direct)
                executor.submit(runnerPython.push, "print_hello_world", 1, helloWorldPython.print_hello_world)
                if DEFINE_SUBSCRIPTION == Sub.multiple_list:
                    executor.submit(self.espd.subscribe, self.g_ids, "runner", [self.update_func_runner_event, goodbyeWorldPython.print_goodbye_world])
                elif DEFINE_SUBSCRIPTION == Sub.multiple_args:
                    executor.submit(eventPython.subscribe, self.g_ids, "runner", self.update_func_runner_event, goodbyeWorldPython.print_goodbye_world)
                executor.submit(helloWorldPython.start)
                executor.submit(goodbyeWorldPython.start)
                executor.submit(runnerPython.start)

    def stop(self):
        if DEFINE_SYNCHRONOUS:
            # Unsubscribe the function associated with self.g_ids (i.e. self.update_func_runner_event and goodbyeWorldPython.print_goodbye_world)
            # from the "runner" event.
            self.espd.unsubscribe("runner", self.g_ids)
            # helloWorldPython.stop() pops off the C++ functions it originally pushed directly to the runner plugin, so that they are
            # no longer called/updated by runner.
            helloWorldPython.stop()
            # goodbyeWorldPython.stop() unsubscribes the C++ functions it originally subscribed to the c++ runner event, so that they
            # are no longer called/updated by said runner event.
            goodbyeWorldPython.stop()
            # Pop off the functions we initially pushed to the python-binded runner plugin.
            runnerPython.pop("update_func_runner_direct", 0, self.update_func_runner_direct)
            runnerPython.pop("print_hello_world", 1, helloWorldPython.print_hello_world)
            # Stop the runner.
            runnerPython.stop()
        else:
            # Performs the same tasks as the previous if-block asynchronously/in a multi-threaded environment.
            with concurrent.futures.ThreadPoolExecutor(max_workers = 5) as executor:
                executor.submit(self.espd.unsubscribe, "runner", self.g_ids)
                executor.submit(helloWorldPython.stop)
                executor.submit(goodbyeWorldPython.stop)
                executor.submit(runnerPython.pop, "update_func_runner_direct", 0, self.update_func_runner_direct)
                executor.submit(runnerPython.pop, "print_hello_world", 1, helloWorldPython.print_hello_world)
                executor.submit(runnerPython.stop)
