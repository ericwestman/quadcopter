import usb.core
import time
HELLO = 0
SET_VALS = 1
GET_VALS = 2
PRINT_VALS = 3

class USBServo:
    def __init__(self):
        self.dev = usb.core.find(idVendor = 0x6666, idProduct = 0x0003)
        if self.dev is None:
            raise ValueError('no USB device found matching idVendor = 0x6666 and idProduct = 0x0003')
        self.dev.set_configuration()

    def close(self):
        self.dev = None

    def hello(self):
        try:
            self.dev.ctrl_transfer(0x40, HELLO)
        except usb.core.USBError:
            print "Could not send HELLO vendor request."

    def set_vals(self, val1, val2):
        try:
            self.dev.ctrl_transfer(0x40, SET_VALS, int(val1), int(val2))
        except usb.core.USBError:
            print "Could not send SET_VALS vendor request."

    def get_vals(self):
        try:
            ret = self.dev.ctrl_transfer(0xC0, GET_VALS, 0, 0, 8)
        except usb.core.USBError:
            print "Could not send GET_VALS vendor request."
        else:
            return [int(ret[0])+int(ret[1])*256, int(ret[2])+int(ret[3])*256, int(ret[4])+int(ret[5])*256, int(ret[6])+int(ret[7])*256]

    def print_vals(self):
        try:
            self.dev.ctrl_transfer(0x40, PRINT_VALS)
        except usb.core.USBError:
            print "Could not send PRINT_VALS vendor request."


if __name__ == '__main__':
    dev = USBServo()
    time.sleep(1)
    while (True):
        time.sleep(0.01)
        # print "Current: %s, Current Counter: %s, Button: %s, Button Counter: %s"%(dev.get_vals()[0], dev.get_vals()[1], dev.get_vals()[2], dev.get_vals()[3])
        print "Button: %s, Button Counter: %s"%(dev.get_vals()[2], dev.get_vals()[3])
        #print "%s, %s, %s, %s"%(dev.get_vals()[0], dev.get_vals()[1], dev.get_vals()[2], dev.get_vals()[3])

