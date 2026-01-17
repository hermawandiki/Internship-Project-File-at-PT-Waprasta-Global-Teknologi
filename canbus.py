import tkinter as tk
from tkinter import ttk, messagebox
import serial
import serial.tools.list_ports
import threading
import time

root = tk.Tk()
root.title("IF40NAVI Simulator")
root.geometry("480x320")
root.iconbitmap("canbusico.ico")

ser = None
running = False
indicator_blinking = False
indicator_state = False
indicator_stop_time = 0

# Frame COM port
frame_com = tk.Frame(root)
frame_com.pack(pady=10)

tk.Label(frame_com, text="COM Port:").pack(side=tk.LEFT, padx=5)
combo_com = ttk.Combobox(frame_com, values=[], width=15)
combo_com.pack(side=tk.LEFT, padx=5)

btn_connect = tk.Button(frame_com, text="Connect")
btn_connect.pack(side=tk.LEFT, padx=10)

# Frame LED dan tombol
frame_led = tk.Frame(root)
frame_led.pack(pady=10)

leds = []
buttons = []

def create_led(frame, size=50, color="gray"):
    canvas = tk.Canvas(frame, width=size, height=size, highlightthickness=0)
    oval = canvas.create_oval(2, 2, size-2, size-2, fill=color, outline="black", width=1)
    canvas.pack(pady=5)
    return canvas, oval

def create_button(frame, size=50, text="", button_index=None):
    canvas = tk.Canvas(frame, width=size, height=size, highlightthickness=0)
    oval = canvas.create_oval(2, 2, size-2, size-2, fill="lightgray", outline="black", width=1)
    label = canvas.create_text(size//2, size//2, text=text, font=("Arial", 12, "bold"))
    canvas.pack(pady=5)

    def on_press(event):
        canvas.move(oval, 1, 1)
        canvas.move(label, 1, 1)
        canvas.itemconfig(oval, fill="yellow")
        if ser and ser.is_open:
            msg = f"0x100 0x11 0x{button_index+1:02X}\n"
            try:
                ser.write(msg.encode())
            except:
                pass

    def on_release(event):
        canvas.move(oval, -1, -1)
        canvas.move(label, -1, -1)
        canvas.itemconfig(oval, fill="lightgray")
        if ser and ser.is_open:
            msg = f"0x100 0x12 0x{button_index+1:02X}\n"
            try:
                ser.write(msg.encode())
            except:
                pass

    canvas.bind("<ButtonPress-1>", on_press)
    canvas.bind("<ButtonRelease-1>", on_release)
    return canvas, oval

# Buat LED dan tombol
for i in range(5):
    col_frame = tk.Frame(frame_led)
    col_frame.pack(side=tk.LEFT, padx=15)
    led_canvas, led_oval = create_led(col_frame)
    leds.append((led_canvas, led_oval))
    btn_canvas, btn_oval = create_button(col_frame, text=f"{i+1}", button_index=i)
    buttons.append((btn_canvas, btn_oval))

# LED indikator bawah tengah (lebih besar)
frame_indicator = tk.Frame(root)
frame_indicator.pack(pady=5)
indicator_label = tk.Label(frame_indicator, text="NOT COMPLETED", font=("Arial", 12, "bold"), fg="red", bg="yellow")
indicator_label.pack(pady=5)
indicator_canvas, indicator_oval = create_led(frame_indicator, size=70, color="gray")

def refresh_com_ports():
    ports = [p.device for p in serial.tools.list_ports.comports()]
    combo_com['values'] = ports
#    if ports:
#        combo_com.current(0)
#    else:
#        combo_com.set("")

def connect_com():
    global ser, running
    if ser and ser.is_open:
        running = False
        ser.close()
        btn_connect.config(text="Connect")
        messagebox.showinfo("Serial", "Disconnected")
    else:
        port = combo_com.get()
        if not port:
            messagebox.showwarning("Serial", "Select a COM port first!")
            return
        try:
            ser = serial.Serial(port, 115200, timeout=0.1)
            btn_connect.config(text="Disconnect")
            messagebox.showinfo("Serial", f"Connected to {port}")
            running = True
            threading.Thread(target=serial_receive_thread, daemon=True).start()
        except Exception as e:
            messagebox.showerror("Serial", f"Failed to connect: {e}")

btn_connect.config(command=connect_com)
refresh_com_ports()

def blink_indicator():
    global indicator_state, indicator_blinking, indicator_stop_time
    if indicator_blinking:
        current_time = time.time()
        if current_time >= indicator_stop_time:
            indicator_blinking = False
            indicator_canvas.itemconfig(indicator_oval, fill="gray")
            indicator_label.config(text="NOT COMPLETED")
            return
        indicator_state = not indicator_state
        color = "green" if indicator_state else "gray"
        indicator_canvas.itemconfig(indicator_oval, fill=color)
        indicator_label.config(text="COMPLETED")
        root.after(50, blink_indicator)

def serial_receive_thread():
    global running, indicator_blinking, indicator_state, indicator_stop_time
    while running:
        try:
            if ser.in_waiting:
                line = ser.readline().decode(errors='ignore').strip()

                # LED utama
                if "DATA=" in line and "ID=0x001" in line:
                    data_str = line.split("DATA=")[1].strip()
                    bytes_str = data_str.split()
                    if len(bytes_str) >= 2:
                        cmd = int(bytes_str[0], 16)
                        led_mask = int(bytes_str[1], 16)
                        led_map = [0x01, 0x02, 0x04, 0x08, 0x10]
                        for i, mask in enumerate(led_map):
                            canvas, oval = leds[i]
                            if led_mask & mask:
                                canvas.itemconfig(oval, fill="red" if cmd==0x01 else "gray")

                # Tombol GUI sesuai RX
                if "ID=0x100" in line and "DATA=" in line:
                    data_str = line.split("DATA=")[1].strip()
                    bytes_str = data_str.split()
                    if len(bytes_str) >= 2:
                        cmd = int(bytes_str[0], 16)
                        btn_index = int(bytes_str[1], 16) - 1
                        if 0 <= btn_index < len(buttons):
                            canvas, oval = buttons[btn_index]
                            if cmd == 0x11:
                                canvas.itemconfig(oval, fill="yellow")
                            elif cmd == 0x12:
                                canvas.itemconfig(oval, fill="lightgray")

                # Indikator blink 3 detik
                if "ID=0x0FF" in line and "DATA=0x28" in line:
                    indicator_blinking = True
                    indicator_stop_time = time.time() + 3
                    blink_indicator()

        except:
            pass
        time.sleep(0.05)

root.mainloop()
