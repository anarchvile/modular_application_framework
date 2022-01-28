class Plugin:
    def initialize(self, identifier):
        return NotImplementedError

    def release(self):
        return NotImplementedError

    def start(self):
        return NotImplementedError

    def stop(self):
        return NotImplementedError