import socket
import sys
import select
import tkinter as tk
from tkinter import filedialog, messagebox

SERVER_IP = "127.0.0.1"
PORT = 8080

class ClientApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Client Application")
        self.root.configure(bg='#2e2e2e')
        self.root.geometry("300x150")
        
        self.image_path = None
        self.option = tk.StringVar(value="-c")
        
        self.create_widgets()
        
        self.sock = None
        self.connect_to_server()
        
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
        self.root.after(1000, self.check_server_connection)

    def create_widgets(self):
        self.browse_button = tk.Button(self.root, text="Browse", command=self.browse_file, bg='#444444', fg='white')
        self.browse_button.grid(row=0, column=0, padx=10, pady=10, sticky='ew')

        
        self.option_frame = tk.Frame(self.root, bg='#2e2e2e')
        self.option_frame.grid(row=0, column=1, padx=10, pady=10)

        self.option_c = tk.Radiobutton(self.option_frame, text="-c", variable=self.option, value="-c", bg='#2e2e2e', fg='white', selectcolor='#444444')
        self.option_c.pack(side="left", padx=5)

        self.option_g = tk.Radiobutton(self.option_frame, text="-g", variable=self.option, value="-g", bg='#2e2e2e', fg='white', selectcolor='#444444')
        self.option_g.pack(side="left", padx=5)

        self.option_r = tk.Radiobutton(self.option_frame, text="-r", variable=self.option, value="-r", bg='#2e2e2e', fg='white', selectcolor='#444444')
        self.option_r.pack(side="left", padx=5)

        self.send_button = tk.Button(self.root, text="Send", command=self.send_data, bg='#444444', fg='white')
        self.send_button.grid(row=1, column=0, columnspan=2, padx=10, pady=10, sticky='ew')



    def browse_file(self):
        self.image_path = filedialog.askopenfilename()
        if self.image_path:
            messagebox.showinfo("Selected File", self.image_path)

    def send_data(self):
        if not self.image_path:
            messagebox.showerror("Error", "Please select an image file.")
            return

        try:
            message = f"{self.image_path} {self.option.get()}"
            self.sock.sendall(message.encode())
            print(f"Message sent: {message}")
            
            # Wait for server response
            ready = select.select([self.sock], [], [], 2)
            if ready[0]:
                response = self.sock.recv(1024)
                if not response:
                    raise ConnectionError("Admin closed the connection.")
                messagebox.showinfo("Server Response", response.decode())
            
        except Exception as e:
            messagebox.showerror("Connection Error", str(e))
            self.root.quit()

    def connect_to_server(self):
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((SERVER_IP, PORT))
            self.sock.setblocking(0)
            init_message = "Client connected"
            self.sock.sendall(init_message.encode())
        except Exception as e:
            messagebox.showerror("Connection Error", str(e))
            self.root.quit()

    def check_server_connection(self):
        try:
            ready = select.select([self.sock], [], [], 0)
            if ready[0]:
                response = self.sock.recv(1024)
                if not response:
                    raise ConnectionError("Server closed the connection.")
        except Exception as e:
            messagebox.showerror("Connection Error", str(e))
            self.root.quit()
        self.root.after(1000, self.check_server_connection)

    def on_closing(self):
        if self.sock:
            self.sock.close()
        self.root.destroy()

def handle_client_gui():
    root = tk.Tk()
    app = ClientApp(root)
    root.mainloop()

if __name__ == "__main__":
    handle_client_gui()
