import tkinter as tk
from tkinter import ttk, messagebox
import serial
import threading
import queue
from datetime import datetime

class CANUI:
    def __init__(self, root):
        self.root = root
        self.root.title("CAN Bus Interface")
        self.root.geometry("800x600")
        
        # Serial connections
        self.tx_serial = None
        self.rx_serial = None
        self.message_queue = queue.Queue()
        self.running = False
        
        self.setup_ui()
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        
    def setup_ui(self):
        # Connection Frame
        conn_frame = ttk.LabelFrame(self.root, text="Connection Settings", padding=10)
        conn_frame.pack(fill=tk.X, padx=10, pady=5)
        
        ttk.Label(conn_frame, text="TX Port (ESP32):").grid(row=0, column=0, sticky=tk.W)
        self.tx_port = ttk.Entry(conn_frame, width=15)
        self.tx_port.insert(0, "COM3")
        self.tx_port.grid(row=0, column=1, padx=5)
        
        ttk.Label(conn_frame, text="RX Port (Arduino):").grid(row=0, column=2, sticky=tk.W)
        self.rx_port = ttk.Entry(conn_frame, width=15)
        self.rx_port.insert(0, "COM4")
        self.rx_port.grid(row=0, column=3, padx=5)
        
        self.connect_btn = ttk.Button(conn_frame, text="Connect", command=self.connect)
        self.connect_btn.grid(row=0, column=4, padx=5)
        
        self.status_label = ttk.Label(conn_frame, text="Disconnected", foreground="red")
        self.status_label.grid(row=0, column=5, padx=10)
        
        # Send Message Frame
        send_frame = ttk.LabelFrame(self.root, text="Send CAN Message", padding=10)
        send_frame.pack(fill=tk.X, padx=10, pady=5)
        
        ttk.Label(send_frame, text="CAN ID (hex):").grid(row=0, column=0, sticky=tk.W)
        self.can_id = ttk.Entry(send_frame, width=15)
        self.can_id.insert(0, "123")
        self.can_id.grid(row=0, column=1, padx=5)
        
        ttk.Label(send_frame, text="Data (hex bytes):").grid(row=1, column=0, sticky=tk.W)
        self.can_data = ttk.Entry(send_frame, width=40)
        self.can_data.insert(0, "FF,FF,FF,FF,00,00,00,00")
        self.can_data.grid(row=1, column=1, padx=5)
        
        #ttk.Label(send_frame, text="(comma-separated, e.g. FF,AA,BB)").grid(row=1, column=2, sticky=tk.W)
        
        self.send_btn = ttk.Button(send_frame, text="Send", command=self.send_message)
        self.send_btn.grid(row=2, column=1, sticky=tk.W, pady=10)
        
        # Messages Frame
        msg_frame = ttk.LabelFrame(self.root, text="CAN Bus Messages", padding=10)
        msg_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=5)
        
        # Message list with scrollbar
        scrollbar = ttk.Scrollbar(msg_frame)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        self.message_list = tk.Listbox(msg_frame, yscrollcommand=scrollbar.set, font=("Courier", 9))
        self.message_list.pack(fill=tk.BOTH, expand=True)
        scrollbar.config(command=self.message_list.yview)
        
        # Clear button
        ttk.Button(msg_frame, text="Clear Messages", command=self.clear_messages).pack(pady=5)
        
    def connect(self):
        if self.running:
            self.disconnect()
            return
        
        try:
            # Open serial connections
            self.tx_serial = serial.Serial(self.tx_port.get(), 115200, timeout=1)
            self.rx_serial = serial.Serial(self.rx_port.get(), 115200, timeout=1)
            
            self.running = True
            self.connect_btn.config(text="Disconnect")
            self.status_label.config(text="Connected", foreground="green")
            
            # Start receive thread
            self.rx_thread = threading.Thread(target=self.read_messages, daemon=True)
            self.rx_thread.start()
            
            self.log_message("STATUS", "Connected to CAN Bus")
            
            # Check for initial ready messages
            self.root.after(500, self.check_ready_messages)
            
        except Exception as e:
            messagebox.showerror("Connection Error", f"Failed to connect: {str(e)}")
            self.disconnect()
    
    def disconnect(self):
        self.running = False
        if self.tx_serial:
            self.tx_serial.close()
        if self.rx_serial:
            self.rx_serial.close()
        self.connect_btn.config(text="Connect")
        self.status_label.config(text="Disconnected", foreground="red")
        self.log_message("STATUS", "Disconnected")
    
    def send_message(self):
        if not self.running:
            messagebox.showwarning("Not Connected", "Please connect first")
            return
        
        try:
            can_id = self.can_id.get().strip()
            can_data = self.can_data.get().strip()
            
            # Validate ID
            id_int = int(can_id, 16)
            
            # Validate and parse data
            data_bytes = [b.strip() for b in can_data.split(",")]
            dlc = len(data_bytes)
            
            if dlc > 8:
                messagebox.showerror("Invalid Data", "Maximum 8 bytes allowed")
                return
            
            # Validate hex values
            for byte in data_bytes:
                int(byte, 16)
            
            # Build command
            command = f"SEND|ID:0x{can_id.upper()}|DLC:{dlc}|DATA:{','.join(data_bytes).upper()}\n"
            
            # Send to transmitter
            self.tx_serial.write(command.encode())
            self.log_message("TX_SENT", f"ID: 0x{can_id.upper()} | Data: {can_data.upper()}")
            
        except ValueError as e:
            messagebox.showerror("Invalid Input", f"Please enter valid hex values: {str(e)}")
    
    def read_messages(self):
        while self.running:
            try:
                if self.rx_serial.in_waiting:
                    line = self.rx_serial.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        self.message_queue.put(line)
            except Exception as e:
                pass
        
        # Process queue in main thread
        self.process_message_queue()
    
    def process_message_queue(self):
        try:
            while True:
                msg = self.message_queue.get_nowait()
                self.parse_and_display_message(msg)
        except queue.Empty:
            pass
        
        if self.running:
            self.root.after(100, self.process_message_queue)
    
    def check_ready_messages(self):
        """Check for initial READY messages from devices"""
        if not self.running:
            return
        
        try:
            # Check TX
            if self.tx_serial.in_waiting:
                line = self.tx_serial.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    self.parse_and_display_message(line)
            
            # Check RX
            if self.rx_serial.in_waiting:
                line = self.rx_serial.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    self.parse_and_display_message(line)
        except:
            pass
        
        self.root.after(500, self.check_ready_messages)
    
    def parse_and_display_message(self, msg):
        """Parse incoming serial messages"""
        if msg.startswith("RX|"):
            # Received message from CAN bus
            self.log_message("RX", msg[3:])
        elif msg.startswith("TX|"):
            # Transmitted message confirmation
            self.log_message("TX_ACK", msg[3:])
        elif msg.startswith("READY|"):
            # Device ready
            self.log_message("READY", msg[6:])
        elif msg.startswith("ERROR|"):
            # Error message
            self.log_message("ERROR", msg[6:])
        else:
            # Unknown message
            self.log_message("INFO", msg)
    
    def log_message(self, msg_type, content):
        timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
        
        # Color coding based on message type
        color_map = {
            "RX": "#00AA00",      # Green
            "TX_SENT": "#0000FF", # Blue
            "TX_ACK": "#00AAFF",  # Light Blue
            "STATUS": "#FF8800",  # Orange
            "READY": "#00AA00",   # Green
            "ERROR": "#FF0000",   # Red
            "INFO": "#000000"     # Black
        }
        
        formatted_msg = f"[{timestamp}] {msg_type}: {content}"
        self.message_list.insert(tk.END, formatted_msg)
        self.message_list.itemconfig(tk.END, foreground=color_map.get(msg_type, "#000000"))
        self.message_list.see(tk.END)  # Auto-scroll to bottom
    
    def clear_messages(self):
        self.message_list.delete(0, tk.END)
    
    def on_closing(self):
        if self.running:
            self.disconnect()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = CANUI(root)
    root.mainloop()
