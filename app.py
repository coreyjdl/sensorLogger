#!/usr/bin/python3

import serial, json, csv
from datetime import datetime

def parse_data(data):
    return json.loads(data.decode('utf-8').strip())

def main():

    # the thumb drive has been troublesome
    #file_path = '/media/pi/A4F8-E1DB/'
    file_path = './'
    date =  datetime.now().strftime('%m_%d_%y-%H_%M')
    filename = f'SENSLOG-{date}.txt'

    ser = serial.Serial("/dev/ttyACM0", 9600)

    data = parse_data(ser.readline())  
    fields = ['timestamp', *data.keys()]

    with open(filename, 'w') as out_file:
        writer = csv.DictWriter(out_file, fieldnames=fields)
        writer.writeheader()

    while(True):
        with open(filename, 'a') as out_file:
            incoming = ser.readline()
            out = {"timestamp": datetime.now(), **parse_data(incoming)}
            writer.writerow(out)

if __name__ == '__main__':
    main()