#!/usr/bin/env python3

""" A simple continuous receiver class. """

# Copyright 2015 Mayer Analytics Ltd.
#
# This file is part of pySX127x.
#
# pySX127x is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
# License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# pySX127x is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
# details.
#
# You can be released from the requirements of the license by obtaining a commercial license. Such a license is
# mandatory as soon as you develop commercial activities involving pySX127x without disclosing the source code of your
# own applications, or shipping pySX127x with a closed source product.
#
# You should have received a copy of the GNU General Public License along with pySX127.  If not, see
# <http://www.gnu.org/licenses/>.


from time import sleep
from SX127x.LoRa import *
from SX127x.LoRaArgumentParser import LoRaArgumentParser
from SX127x.board_config import BOARD
import json
import urllib.request

BOARD.setup()

parser = LoRaArgumentParser("Continous LoRa receiver.")
message = []
api_key = ''
sender_id = 0
ts_key = ''

def format_message_new(str):
    global message
    global api_key

    values = str.split(';')

    sender_id = values[0]
    api_key = values[1]
    message = []

    for value in range(2, len(values)):
        message.append(values[value])

    print('Message received - Node ID: {} - API Key: {} - Values: {}'.format(sender_id, api_key, message))


def send_thingspeak_new():
    #url = "https://api.thingspeak.com/update?api_key=KEY&fisender_ideld1={}&field2={}".format(data['temp'], data['hum'])
    url = "https://api.thingspeak.com/update?api_key={}".format(api_key)
    i = 1
    for value in message:
        url += '&field{}={}'.format(i, value)
        i += 1
    response = urllib.request.urlopen(url).read()
    print("\nHTTP Request: {}".format(url))
    print("\nHTTP Response: {}".format(response.decode()))

class LoRaRcvCont(LoRa):
    def __init__(self, verbose=False):
        super(LoRaRcvCont, self).__init__(verbose)
        self.set_mode(MODE.SLEEP)
        self.set_dio_mapping([0] * 6)



    def on_rx_done(self):
        BOARD.led_on()
        print("\nRxDone")
        self.clear_irq_flags(RxDone=1)
        payload = self.read_payload(nocheck=True)

        #Get RSSI and SNR values
        rssi = self.get_pkt_rssi_value()
        snr = self.get_pkt_snr_value()
        print('RSSI: {} / SNR: {}'.format(rssi, snr))
        
        #Discard header bytes from message
        str = ''.join(chr(i) for i in bytes(payload)[4:-1])
        print('Raw message: {}'.format(str))
        format_message_new(str)
        send_thingspeak_new()

        #Set to TX mode
        self.set_mode(MODE.STDBY)
        self.set_dio_mapping([1,0,0,0,0,0])
        print('sending')
        self.write_payload([0, 16, 1, 8, 111, 107, 0])
        self.set_mode(MODE.TX)
        
    def on_tx_done(self):
        print("\nTxDone")
        self.set_mode(MODE.STDBY)
        print(self.get_irq_flags())
        sys.stdout.flush()
        self.clear_irq_flags(TxDone=1)
        self.set_mode(MODE.SLEEP)
        self.set_dio_mapping([0] * 6)
        self.reset_ptr_rx()
        #back to RX mode on TX end
        self.set_mode(MODE.RXCONT)

    def on_cad_done(self):
        print("\non_CadDone")
        print(self.get_irq_flags())

    def on_rx_timeout(self):
        print("\non_RxTimeout")
        print(self.get_irq_flags())

    def on_valid_header(self):
        print("\non_ValidHeader")
        print(self.get_irq_flags())

    def on_payload_crc_error(self):
        print("\non_PayloadCrcError")
        print(self.get_irq_flags())

    def on_fhss_change_channel(self):
        print("\non_FhssChangeChannel")
        print(self.get_irq_flags())

    def start(self):

        self.reset_ptr_rx()
        self.set_mode(MODE.RXCONT)
        while True:
            sleep(.5)
            rssi_value = self.get_rssi_value()
            status = self.get_modem_status()
            sys.stdout.flush()
            sys.stdout.write("\r%d %d %d" % (rssi_value, status['rx_ongoing'], status['modem_clear']))

lora = LoRaRcvCont(verbose=False)
args = parser.parse_args(lora)

lora.set_freq(915)
lora.set_coding_rate(CODING_RATE.CR4_5)
lora.set_bw(BW.BW125)
lora.set_spreading_factor(10)

lora.set_mode(MODE.STDBY)
lora.set_pa_config(pa_select=1)

print(lora)
assert(lora.get_agc_auto_on() == 1)

try: input("Press enter to start...")
except: pass

try:
    lora.start()
except KeyboardInterrupt:
    sys.stdout.flush()
    print("")
    sys.stderr.write("KeyboardInterrupt\n")
finally:
    sys.stdout.flush()
    print("")
    lora.set_mode(MODE.SLEEP)
    print(lora)
    BOARD.teardown()
