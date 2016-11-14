from tkinter import *
import serial

temperature = 0
light = 0
distance = 0

def calculate_temp(a):
    voltage = float(a) * 5.0
    voltage /= 1024.0
    temperature = (voltage - 0.5) * 100
    return temperature

def find_between( s, first, last ):
    try:
        start = s.index( first ) + len( first )
        end = s.index( last, start )
        return s[start:end]
    except ValueError:
        return ""

def handle_arduino_input(text):
    global temperature
    global light
    global distance

    string = find_between(text, "|", "|")
    identifier = string[0]

    if identifier == "C":
        temperature = string[1:]
    elif identifier == "L":
       light = string[1:]
    elif identifier == "D":
       distance = string[1:]


def readserial():
    ser = serial.Serial(port='COM4', baudrate=9600, timeout=1)

    while 1:
        b = ser.readline()
        print("Light : " + str(light))
        print("Distance : " + str(distance))
        print("Temp : " + str(calculate_temp(temperature)))
        if b.strip():
            # Function to set variables
            handle_arduino_input(b.decode('utf-8'))

#Simple GUI to show the variables updating live
root = Tk()
root.title("A simple GUI")

gui_var1 = IntVar()
gui_var1.set(temperature)

gui_var2 = IntVar()
gui_var2.set(distance)

gui_var3 = IntVar()
gui_var3.set(light)

root.label = Label(root, text="My Gui")
root.label.pack()

root.label1 = Label(root, textvariable=gui_var1)
root.label1.pack()

root.label2 = Label(root, textvariable=gui_var2)
root.label2.pack()

root.label3 = Label(root, textvariable=gui_var3)
root.label3.pack()

root.close_button = Button(root, text="Close", command=root.quit)
root.close_button.pack()

#Start GUI and Serial
root.mainloop()
readserial()