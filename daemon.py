import time
import os
from gps3 import gps3
from sakuraio.hardware.rpi import SakuraIOSMBus

# see here
# http://python-sakuraio.readthedocs.io/en/latest/hardware/api.html#transmit
# http://www.stuffaboutcode.com/2013/09/raspberry-pi-gps-setup-and-python.html

class Gpsrunner:
    def __init__(self):
        self.sakura = SakuraIOSMBus()
        self.gps_socket  = gps3.GPSDSocket()
        self.data_stream = gps3.DataStream()


    def run(self):
        self.gps_socket.connect()
        self.gps_socket.watch()
        for new_data in self.gps_socket:
            if new_data:
                self.data_stream.unpack(new_data)
                msg = self.data_stream.TPV['lat'] + self.data_stream.TPV['lon']
                print(msg)
                self.send_message(msg)


    def send_message(self, msg):
        self.sakura.send_immediate(0, msg)
            


if __name__ == '__main__':
    gps_runner = Gpsrunner()
    gps_runner.run()
