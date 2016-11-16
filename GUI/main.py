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

OPTIONS = [
    "COM1",
    "COM2",
    "COM3",
    "COM4"
]

def readserial(listselection):
    ser = serial.Serial(port=listselection, baudrate=9600, timeout=1)
    b = ser.readline()
    gui_temp.set(calculate_temp(temperature))
    gui_distance.set(distance)
    gui_light.set(light)
    if b.strip():
        # Function to set variables
        handle_arduino_input(b.decode('utf-8'))
    root.after(1000, readserial)

Connected_Ports = []

def Hook_To_Arduino():
    global Connected_Ports
    for a in OPTIONS:
        try:
            ser = serial.Serial(port=a, baudrate=9600, timeout=1)
            ser.readline()
            print("Arduino found on port : " + str(a))
            Connected_Ports.append(a)
        except serial.serialutil.SerialException:
            print("Kon geen arduino vinden op poort : " + str(a))
    add_ports_to_list()

#Simple GUI to show the variables updating live
root = Tk()
root.title("A simple GUI")

gui_temp = IntVar()
gui_distance = IntVar()
gui_light = IntVar()

root.label = Label(root, text="My Gui")
root.label.pack()

listbox = Listbox(root)
listbox.bind('<<ListboxSelect>>', readserial)
listbox.pack()

def add_ports_to_list():
    for item in set(Connected_Ports):
        listbox.insert(END, item)
        listbox.update_idletasks()


root.label1 = Label(root, text="dog", textvariable=gui_temp)
root.label1.pack()

root.label2 = Label(root, textvariable=gui_distance)
root.label2.pack()

root.label3 = Label(root, textvariable=gui_light)
root.label3.pack()

root.close_button = Button(root, text="Close", command=root.quit)
root.close_button.pack()

root.close_button = Button(root, text="Hook", command=Hook_To_Arduino)
root.close_button.pack()

root.close_button = Button(root, text="Read", command=readserial)
root.close_button.pack()

#Start GUI and Serial
# root.after(5, readserial)

root.mainloop()