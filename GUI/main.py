'''
Created on 14 Nov 2016

@author: Rob
'''
import tkinter as Tkinter
import serial  # @UnresolvedImport
from time import sleep


# Gui class
class GUI(Tkinter.Tk):
    def __init__(self, parent):
        Tkinter.Tk.__init__(self, parent)
        self.parent = parent
        self.WIDTH = 900
        self.HEIGHT = self.WIDTH / 16 * 9

        # variables for calculations
        self.rec_temperature = []
        self.rec_light = []
        self.canvases = []
        self.str_vars = []

        self.initialize(self.WIDTH, self.HEIGHT)

    def show_frame1(self):
        self.sensor_frame1.grid()
        self.sensor_frame2.grid_forget()

    def show_frame2(self):
        self.sensor_frame2.grid()
        self.sensor_frame1.grid_forget()

    # initialize the widgets
    def initialize(self, x, y):
        self.resizable(0, 0)

        # label vars
        inf_bg = 'lightblue'
        label_font = 'sans-serif'

        # create sensor frames
        self.sensor_frame1 = Tkinter.Frame(
            self,
            height=y,
            width=x,
            bg='white'
        )

        self.sensor_frame2 = Tkinter.Frame(
            self,
            height=y,
            width=x,
            bg='white'
        )

        # initiate first frame
        self.sensor_frame1.grid()

        # lists used for adressing
        frames = [self.sensor_frame1, self.sensor_frame2]
        self.widgets = [[], []]
        self.input_list = [[], []]

        # var for adressing
        self.widget_num = 0

        # add frames to hold widgets
        for frame in frames:
            # create container frames
            graph = Tkinter.Frame(
                frame,
                height=y,
                width=x / 2,
                bg=inf_bg
            )

            variables = Tkinter.Frame(
                frame,
                height=y / 2,
                width=x / 3,
                bg=inf_bg
            )

            information = Tkinter.Frame(
                frame,
                height=y / 2,
                width=x / 3,
                bg=inf_bg
            )

            # place container frames in main frame
            graph.grid(row=0, column=1, rowspan=2, sticky='EW')
            variables.grid(row=1, column=0, sticky='EW')
            information.grid(row=0, column=0, sticky='EW')

            # frames stay same size even with widgets
            graph.grid_propagate(False)
            variables.grid_propagate(False)
            information.grid_propagate(False)

            # list with text used in forloop because it will not change
            labeltekst = ["sensor waarde:", "gemiddeld laatste minuut:", "minimum laatste minuut:",
                          "maximum laatste minuut:"]
            var_labeltekst = ["oprollen bij een waarde van:", "laten zakken bij een waarde van:",
                              "maximale uitrol afstand:", "minimale uitrol afstand:"]

            # add labels
            for i in range(4):
                # text label, text will not change (information frame)
                label = Tkinter.Label(
                    information,
                    text=labeltekst[i],
                    fg='black',
                    bg=inf_bg,
                    font=label_font
                )

                # create variable to change display
                var = Tkinter.StringVar()
                var.set(0)

                # variable label, text changes (information frame)
                val_label = Tkinter.Label(
                    information,
                    textvariable=var,
                    fg='black',
                    bg=inf_bg,
                    font=label_font
                )

                # adding widgets to list for adressing
                self.widgets[self.widget_num].append(var)

                # place labels in the frame
                label.place(x=10, y=40 * (i + 1))
                val_label.place(x=200, y=40 * (i + 1))

                # str_var gebruiken om ingevoerde waarde te lezen
                self.str_var = Tkinter.StringVar(variables, value=0)
                self.str_vars.append(self.str_var)

                # text label, text will not change (variables frame)
                label = Tkinter.Label(
                    variables,
                    text=var_labeltekst[i],
                    fg='black',
                    bg=inf_bg,
                    font=
                    label_font
                )

                # variable entry, text changes to current value (variable frame)
                var_entry = Tkinter.Entry(
                    variables,
                    textvariable=self.str_var
                )

                # buttons to set the values
                button = Tkinter.Button(
                    variables,
                    text="submit",
                    #command=lambda: self.str_var.set(int(var_entry.get()))
                    command=send_min_rollout
                )

                # place the widgets
                label.place(x=10, y=50 * (i) + 20)
                var_entry.place(x=13, y=50 * (i) + 46)
                button.place(x=150, y=50 * (i) + 43)

            # de mooie lijntjes
            labelframe = Tkinter.LabelFrame(information, width=250, height=2)
            labelframe.place(x=10, y=50 * 4 + 35)
            labelframe2 = Tkinter.LabelFrame(information, width=2, height=200)
            labelframe2.place(x=50 * 5 + 47, y=10)
            labelframe2 = Tkinter.LabelFrame(variables, width=2, height=200)
            labelframe2.place(x=50 * 5 + 47, y=10)

            # canvas
            self.canvas = Tkinter.Canvas(graph, width=x / 2 - 25, height=y - 25, bg='white')
            self.canvas.place(x=10, y=10)
            self.canvases.append(self.canvas)

            # create_line(x1,y1,x2,y2)
            self.canvas.create_line(10, y - 35, 10, 10, width=2)
            self.canvas.create_line(10, y - 35, x / 2 - 35, y - 35, width=2)

            # create roster lines
            for q in range(12):
                self.canvas.create_line(10, y - 35 - q * ((x / 2 - 45) / 10), x / 2 - 35,
                                        y - 35 - q * ((x / 2 - 45) / 10))

            for q in range(9):
                self.canvas.create_line(10 + 50 * q, y - 35, 10 + 50 * q, 10)

            self.widget_num += 1

        # add a menu
        self.menu = Tkinter.Menu(self)

        # add a button to menu to show the menu
        self.menu.add_command(label="Hook 1", command=Hook_To_Arduino)
        self.menu.add_command(label="Hook 2", command=Hook_To_Arduino)
        self.menu.add_command(label="Read", command=readserial)
        self.menu.add_command(label="temperature", command=self.show_frame1)
        self.menu.add_command(label="light", command=self.show_frame2)
        self.config(menu=self.menu)
        # End of initialize

    # calculate the average temperature
    def avg(self, list):
        sum = 0
        for i in range(len(list)):
            sum += list[i]
        avg = sum / len(list)
        return round(avg,3)

    def draw(self, width, height):
        x = width / 2 - 20
        y = height - 35

        for i in range(2):
            self.canvases[i].delete('temp')

        temp_y1 = self.rec_temperature[0]
        xcount = 1
        for value in self.rec_temperature:
            self.canvases[0].create_line(
                (x / 60) * xcount,
                y - temp_y1,
                (x / 60) * (xcount + 1),
                y - value,
                fill='red',
                tags='temp'
            )
            temp_y1 = value
            xcount += 1

        temp_y1 = self.rec_light[0]
        xcount = 1
        for value in self.rec_light:
            self.canvases[1].create_line(
                (x / 60) * xcount,
                y - temp_y1, (x / 60) * (xcount + 1),
                y - value,
                fill='red',
                tags='temp'
            )
            temp_y1 = value
            xcount += 1

    def set_vars(self, temperature, light):
        # [0][0-3] = eerste sensor
        # [1][0-3] = tweede sensor

        # add new measurement into list
        self.rec_light.append(light)
        self.rec_temperature.append(temperature)

        # check for values outside a minute
        if len(self.rec_light) > 60:
            del self.rec_light[0]

        if len(self.rec_temperature) > 60:
            del self.rec_temperature[0]

        # set the latest measured values
        self.widgets[0][0].set(temperature)
        self.widgets[1][0].set(light)

        # set the average values
        self.widgets[0][1].set(self.avg(self.rec_temperature))
        self.widgets[1][1].set(self.avg(self.rec_light))

        # set the min values
        self.widgets[0][2].set(min(self.rec_temperature))
        self.widgets[1][2].set(min(self.rec_light))

        # set the maximum values
        self.widgets[0][3].set(max(self.rec_temperature))
        self.widgets[1][3].set(max(self.rec_light))

        self.draw(self.WIDTH, self.HEIGHT)

temperature = 0
light = 0

def calculate_temp(a):
    voltage = float(a) * 5.0
    voltage /= 1024.0
    temperature = (voltage - 0.5) * 100
    return temperature

def find_between(s, first, last ):
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

OPTIONS = ["COM1","COM2","COM3","COM4"]

Connected_Ports = []

ser = serial.Serial(baudrate=9600, timeout=1)

def set_serial(a):
    ser.port = a
    ser.open()

def readserial():
    b = ser.readline()
    gui.set_vars(round(calculate_temp(temperature), 3), round(int(light)/5, 3))
    if b.strip():
        # Function to set variables
        handle_arduino_input(b.decode('utf-8'))
    gui.after(1000, readserial)

def send_min_rollout():
    if ser.isOpen():
        ser.close()
    set = serial.Serial(port="COM4", baudrate=9600, timeout=1)
    set.open()
    # write data
    set.write(6)
    sleep(3)

def Hook_To_Arduino():
    global Connected_Ports
    for a in OPTIONS:
        try:
            ser = serial.Serial(port=a, baudrate=9600, timeout=1)
            print("Arduino found on port : " + str(a))
            Connected_Ports.append(a)
            ser.close()
            set_serial(a)
        except serial.serialutil.SerialException:
            print("Kon geen arduino vinden op poort : " + str(a))

if __name__ == '__main__':
    gui = GUI(None)
    gui.title("Software Centrale")
    gui.mainloop()
