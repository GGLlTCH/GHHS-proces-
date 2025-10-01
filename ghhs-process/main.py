import tkinter as tk
from ui.main_window import MainWindow
import ctypes
import sys
import os

def is_admin():
    try:
        return ctypes.windll.shell32.IsUserAnAdmin()
    except:
        return False

if __name__ == "__main__":
    if not is_admin():
        ctypes.windll.shell32.ShellExecuteW(None, "runas", sys.executable, " ".join(sys.argv), None, 1)
        sys.exit(0)

    # Настройка DPI для правильного масштабирования
    ctypes.windll.shcore.SetProcessDpiAwareness(1)
    
    root = tk.Tk()
    root.title("GHHS Process Hacker")
    root.geometry("1400x900")
    root.configure(bg='#0a0a0a')
    
    # Убираем стандартное меню Windows
    root.overrideredirect(False)
    
    app = MainWindow(root)
    root.mainloop()