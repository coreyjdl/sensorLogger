#!/usr/bin/python3

import serial, json, csv
from datetime import datetime

def parse_data(data):
    return json.loads(data.decode('utf-8').strip())

def main():
    date =  datetime.now().strftime('%m_%d_%y-%H_%M')
    filename = f'SENSLOG-{date}.txt'
    writer = None 

    ser = serial.Serial("/dev/ttyACM0", 9600)

    with open(filename, 'a') as out_file:
        
        while(True):
            incoming = parse_data(ser.readline())

            if not writer:
                fields = ['timestamp', *incoming.keys()]
                writer = csv.DictWriter(out_file, fieldnames=fields)
                writer.writeheader()

            out = {"timestamp": datetime.now(), **incoming}
            writer.writerow(out)
            out_file.flush()


if __name__ == '__main__':
    main()