#!/usr/bin/env python

# Display images from the MSP430<->OV7670 project

import pygame
import sys
import serial
import time
import re

def parsergb565(byte1, byte2):
    byte12 = byte1 << 8 | byte2

    red = byte12 >> 8+3
    green = (byte12 >> 5) & 0x3f
    blue = byte12 & 0x1f
    
    red *= 8
    green *= 4
    blue *= 8

    return (red, green, blue)

def camera_capture():
    l = ''
    while l != 'OK':
        writeslow('cap\r')
        time.sleep(1)
        l = ser.readline().strip()

def camera_rrst():
    l = ''
    while l != 'OK':
        writeslow('rrst\r')
        time.sleep(0.1)
        l = ser.readline().strip()

def readimage():
    imagebuf = [None] * image_height
    print 'Requesting new image'
    camera_capture()
    print 'New image taken, resetting read pointer'
    camera_rrst()
    print 'Transferring buffer via serial'
    l = ''
    for y in range(0, image_height):
        sys.stdout.write('\rReading line %d/%d [%d%%]' % \
            (y + 1, image_height,
            int((y + 1) / float(image_height) * 100.0)))
        sys.stdout.flush()
        writeslow('read %d\r' % ((image_width * 2),))
        l = ser.read(image_width * 2)
        if len(l) != (image_width * 2):
            print '\nonly got %d bytes!' % (len(l),)
            sys.exit(1)
        imagebuf[y] = l
    print
    return imagebuf

# the software uart on the ov7670 project is not perfect...
def writeslow(s):
    for c in s:
        ser.write(c)
        time.sleep(0.01)

def _drawimage():
    for y in range(0, image_height):
        i = 0
        for x in range(0, image_width):
            color = parsergb565(
                ord(buf[y][i]),
                ord(buf[y][i + 1]))
            i += 2

            screen.set_at((2 * x, 2 * y), color)
            screen.set_at((2 * x + 1, 2 * y), color)
            screen.set_at((2 * x, 2 * y + 1), color)
            screen.set_at((2 * x + 1, 2 * y + 1), color)

def drawimage():
    for y in range(0, image_height):
        i = 0
        for x in range(0, image_width):
            color = parsergb565(
                ord(buf[y][i]),
                ord(buf[y][i + 1]))
            i += 2

            screen.set_at((x, y), color)
            #screen.set_at((2 * x + 1, 2 * y), color)
            #screen.set_at((2 * x, 2 * y + 1), color)
            #screen.set_at((2 * x + 1, 2 * y + 1), color)

if __name__ == '__main__':
    #image_width = 160
    #image_height = 120

    image_width = 320
    image_height = 240

    ser = serial.Serial(
            port = 5,
            baudrate = 115200,
            parity = serial.PARITY_NONE,
            stopbits = serial.STOPBITS_ONE,
            bytesize = serial.EIGHTBITS,
            timeout = 1
        )
    ser.close()
    ser.open()
    ser.isOpen()
    print 'Serial port open'

    print 'Reading image from camera...'
    starttime = time.time()
    buf = readimage()
    print 'Read complete in %.3f seconds' % (time.time() - starttime)

    print 'Opening window'
    width = image_width
    height = image_height
    screen = pygame.display.set_mode((width, height))
    clock = pygame.time.Clock()

    running = True
    while running:
        drawimage()
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif (event.type == pygame.KEYDOWN):
                if (event.key == pygame.K_SPACE):
                    #buf = readimage()
                    pass
        pygame.display.flip()

        clock.tick(240)

