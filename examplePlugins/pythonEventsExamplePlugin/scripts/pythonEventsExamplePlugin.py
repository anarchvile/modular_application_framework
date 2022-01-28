import plugin
import eventPython

class PythonEventsExamplePlugin(plugin.Plugin):
    def initialize(self, identifier):
        print("pythonEventsExamplePlugin successfully loaded!")
        eventPython.eventTestFunc()

    def release(self):
        print("pythonEventsExamplePlugin successfully unloaded!")

    def start(self):
        print("pythonEventsExamplePlugin successfully started!")

    def stop(self):
        print("pythonEventsExamplePlugin successfully stopped!")