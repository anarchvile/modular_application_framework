import plugin

def test_func():
    return "Testing function in helloWorld.py that is not a member of the HelloWorldClass"

class HelloWorld(plugin.Plugin):
    def hello_world_py():
        return "A HelloWorld class member function that is located in helloWorld.py (i.e. the Python plugin component of HelloWorld)"

    def initialize(self, identifier):
        print("initialize() called from helloWorld.py")

    def release(self):
        print("release() called from helloWorld.py")

    def start(self):
        print("start() called from helloWorld.py")

    def stop(self):
        print("stop() called from helloWorld.py")
