#!/usr/bin/python

import sys
import logging
from datetime import datetime
from arduino import Arduino

# Logging
_name = "Bargraph"
_debug = logging.WARN

log = logging.getLogger(_name)
if not len(log.handlers):
    log.setLevel(_debug)

    longFormatter = logging.Formatter("%(asctime)s [%(threadName)-12.12s] [%(levelname)-10.10s]  %(message)s")
    shortFormatter = logging.Formatter("[%(levelname)-8.8s]  %(message)s")

    fileHandler = logging.FileHandler("logs/{0}/{1}.log".format("./", _name))
    fileHandler.setFormatter(longFormatter)
    log.addHandler(fileHandler)

    consoleHandler = logging.StreamHandler()
    consoleHandler.setFormatter(shortFormatter)
    log.addHandler(consoleHandler)


class Bargraph(object):
    formats = [
        "default",
        "red",
        "yellow",
        "green",
        "rainbow",
        "delta"
    ]

    def __init__(self, name, api, options=None):
        """Initialize pin with parameters"""
        # Set core attributes
        self.device = None
        self._arduino = None

        self.name = name
        self.api = api
        
        # Pre-set extra attributes
        self._type = "delta"
        self._max = 100
        self._showdelta = 60*1000
        
        # Override defaults with passed values
        if options:
            for key in options:
                setattr(self, "_" + key, options[key])
        
        # Set ephemeral values
        self.value = 0
        self._lastvalue = 0
        
        self._lastupdate = datetime.now()
        self._delta = datetime.now() - self._lastupdate
        
        self.red = [False] * 24
        self.green = [False] * 24
        self._lastred = [False] * 24
        self._lastgreen = [False] * 24
        
        for i in range(self._max, 0):
            self.set(i)
        
        # Run initial update
        self.update()

    def _color(self, character, color):
        try:
            from termcolor import colored
            return colored(character, color)
        except ImportError:
            return character

    def attach(self, arduino, device):
        self._arduino = arduino
        self.device = device

    def detatch(self):
        self._arduino = None
        self.device = None

    def set(self, value, new_max=None):
        log.debug("Setting bargraph " + self.name + " " + str(value) + "/" + str(new_max))

        if new_max is not None:
            self._max = new_max

        try:
            value = int(value)
        except ValueError:
            print "Unexpected value: " + value + ". Ignoring."

        self._lastvalue = self.value
        self._delta = datetime.now() - self._lastupdate
        self._lastupdate = datetime.now()
        self.value = value
        self.update()

    def update(self):
        self.format()
        self.write()
        
    def printout(self):
        print "Bargraph " + self.name + " (Type = " + self._type + ")"
        print self.toString()
        
    def toString(self):
        bar = "["
        char = "|"
        
        for i in range(0, 24):
            if self.red[i] and self.green[i]:
                c = self._color(char, "yellow")
            elif self.red[i]:
                c = self._color(char, "red")
            elif self.green[i]:
                c = self._color(char, "green")
            else:
                c = " "
            
            bar += c
        
        bar += "]"
        return bar
        
    def format(self):
        def clear():
            self._lastred = self.red[:]
            self._lastgreen = self.green[:]
            
            for i in range(0, 24):
                self.red[i] = False
                self.green[i] = False
        
        def default():
            percent = float(self.value) * 100 / self._max
            
            for i in range(0, min(24, int(24*percent/100))):
                if percent > 50:
                    self.green[i] = True
                elif percent > 20:
                    self.green[i] = True
                    self.red[i] = True
                else:
                    self.green[i] = False
                    self.red[i] = True
        
        def rainbow():
            percent = float(self.value) * 100 / self._max
            
            for i in range(0, min(24, int(24*percent/100))):
                if i > 12:
                    self.green[i] = True
                elif i > 4:
                    self.green[i] = True
                    self.red[i] = True
                else:
                    self.green[i] = False
                    self.red[i] = True
        
        def red():
            percent = float(self.value) * 100 / self._max
            
            for i in range(0, min(24, int(24*percent/100))):
                self.red[i] = True
                
        def green():
            percent = float(self.value) * 100 / self._max
            
            for i in range(0, min(24, int(24*percent/100))):
                self.green[i] = True
                
        def yellow():
            percent = float(self.value) * 100 / self._max
            
            for i in range(0, min(24, int(24*percent/100))):
                self.red[i] = True
                self.green[i] = True
        
        def delta():
            green()
            
            percent = float(self.value) * 100 / self._max
            changepermilli = float(self.value-self._lastvalue)/(self._delta.total_seconds()*1000)
            projectedchange = changepermilli * self._showdelta
            percentchange = projectedchange / self._max
            
            if percentchange > 0:
                for i in range(min(24, int(24*percent/100)), min(24, int(24*(percent+percentchange)/100))):
                    self.red[i] = True
                    self.green[i] = True
            elif percentchange < 0:
                for i in range(max(0, (int(24*(percent+percentchange)/100)-1)), min(24, int(24*percent/100))):
                    self.red[i] = True
                    self.green[i] = False
            
        clear()
        
        if self.value < 0 or self._max < 0:
            self._arduino.bargraphWrite(self.device, self.red, self.green)
            return
        
        f = locals().get(self._type)
        if not f:
            log.error("Unknown type: " + self._type)
            return

        try:
            f()
        except ZeroDivisionError:
            pass

    def write(self):
        if cmp(self.red, self._lastred) == 0 and cmp(self.green, self._lastgreen) == 0:
            pass
        else:
            self._arduino.bargraphWrite(self.device, self.red, self.green)

# #####################################
# ########## Testing Methods ##########
# #####################################


def breakpoint():
    """Python debug breakpoint."""
    
    from code import InteractiveConsole
    from inspect import currentframe
    try:
        import readline
    except ImportError:
        pass

    caller = currentframe().f_back

    env = {}
    env.update(caller.f_globals)
    env.update(caller.f_locals)

    shell = InteractiveConsole(env)
    shell.interact(
        '* Break: {} ::: Line {}\n'
        '* Continue with Ctrl+D...'.format(
            caller.f_code.co_filename, caller.f_lineno
        )
    )


def main():
    a = Arduino()
    
    bar0 = Bargraph(a, "Test", "test", 0)
    bar1 = Bargraph(a, "Test", "test", 1)
    bar2 = Bargraph(a, "Test", "test", 2)
    bar3 = Bargraph(a, "Test", "test", 3)
    bar4 = Bargraph(a, "Test", "test", 4)
    
    bar0.update()
    bar1.update()
    bar2.update()
    bar3.update()
    bar4.update()
    
    bar0.printout()
    
    import time
    
    for i in range(0, 101):
        bar0.set(i)
        bar1.set(i)
        bar2.set(i)
        bar3.set(i)
        bar4.set(i)
        
        print bar0.toString()
        time.sleep(.25)
        
    for i in range(101, -1, -1):
        bar0.set(i)
        bar1.set(i)
        bar2.set(i)
        bar3.set(i)
        bar4.set(i)
        
        print bar0.toString()
        time.sleep(.25)
        

if __name__ == "__main__":    
    try:
        main()
    except KeyboardInterrupt:
        sys.exit(0)
    except EOFError:
        sys.exit(0)
    # except:
    #     sys.exit(0)
