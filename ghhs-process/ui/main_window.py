import tkinter as tk
from tkinter import ttk, messagebox
import psutil
import threading
import time
import math
from ui.styles import COLOR_SCHEME, FONT_SCHEME, DarkTheme
from core.process_manager import ProcessManager
from core.system_monitor import SystemMonitor

class ModernButton(tk.Canvas):
    def __init__(self, parent, text, command, width=100, height=30, 
                 bg_color=COLOR_SCHEME["bg_light"], fg_color=COLOR_SCHEME["text_white"],
                 accent_color=COLOR_SCHEME["accent_blue"]):
        super().__init__(parent, width=width, height=height, 
                        highlightthickness=0, bg=COLOR_SCHEME["bg_darkest"])
        self.command = command
        self.bg_color = bg_color
        self.fg_color = fg_color
        self.accent_color = accent_color
        self.width = width
        self.height = height
        
        self.bind("<Enter>", self._on_enter)
        self.bind("<Leave>", self._on_leave)
        self.bind("<Button-1>", self._on_click)
        self.bind("<ButtonRelease-1>", self._on_release)
        
        self.is_pressed = False
        self.is_hovered = False
        
        self._draw_button()
        self._draw_text(text)
    
    def _draw_button(self):
        self.delete("all")
        
        # Градиентный эффект
        if self.is_pressed:
            color = self._darken_color(self.bg_color, 20)
        elif self.is_hovered:
            color = self._lighten_color(self.bg_color, 10)
        else:
            color = self.bg_color
        
        # Скругленный прямоугольник
        radius = 6
        self.create_rectangle(
            radius, 0, self.width-radius, self.height,
            fill=color, outline="", tags="button"
        )
        self.create_rectangle(
            0, radius, self.width, self.height-radius,
            fill=color, outline="", tags="button"
        )
        self.create_oval(0, 0, radius*2, radius*2, fill=color, outline="", tags="button")
        self.create_oval(self.width-radius*2, 0, self.width, radius*2, fill=color, outline="", tags="button")
        self.create_oval(0, self.height-radius*2, radius*2, self.height, fill=color, outline="", tags="button")
        self.create_oval(self.width-radius*2, self.height-radius*2, self.width, self.height, 
                        fill=color, outline="", tags="button")
        
        # Акцентная полоса сверху
        self.create_rectangle(0, 0, self.width, 2, fill=self.accent_color, outline="", tags="accent")
    
    def _draw_text(self, text):
        self.create_text(self.width//2, self.height//2, text=text, 
                        fill=self.fg_color, font=FONT_SCHEME["normal"],
                        tags="text")
    
    def _on_enter(self, event):
        self.is_hovered = True
        self._draw_button()
    
    def _on_leave(self, event):
        self.is_hovered = False
        self.is_pressed = False
        self._draw_button()
    
    def _on_click(self, event):
        self.is_pressed = True
        self._draw_button()
    
    def _on_release(self, event):
        self.is_pressed = False
        self._draw_button()
        if self.command:
            self.command()
    
    def _lighten_color(self, color, percent):
        """Осветляет цвет"""
        # Упрощенная реализация для примера
        return COLOR_SCHEME["accent_blue_light"]
    
    def _darken_color(self, color, percent):
        """Затемняет цвет"""
        return COLOR_SCHEME["accent_blue_dark"]

class CircularProgress(tk.Canvas):
    def __init__(self, parent, size=60, bg_color=COLOR_SCHEME["bg_dark"]):
        super().__init__(parent, width=size, height=size, 
                        highlightthickness=0, bg=bg_color)
        self.size = size
        self.value = 0
        self._draw_progress()
    
    def set_value(self, value):
        self.value = max(0, min(100, value))
        self._draw_progress()
    
    def _draw_progress(self):
        self.delete("all")
        
        center = self.size // 2
        radius = self.size // 2 - 5
        
        # Фоновая окружность
        self.create_oval(center-radius, center-radius, center+radius, center+radius,
                        outline=COLOR_SCHEME["bg_light"], width=3, fill="")
        
        # Прогресс
        if self.value > 0:
            start_angle = 90
            end_angle = 90 - (360 * self.value / 100)
            
            self.create_arc(center-radius, center-radius, center+radius, center+radius,
                          start=start_angle, extent=end_angle-start_angle,
                          outline=COLOR_SCHEME["accent_blue"], width=3, style=tk.ARC)
        
        # Текст
        self.create_text(center, center, text=f"{int(self.value)}%", 
                        fill=COLOR_SCHEME["text_white"], font=FONT_SCHEME["metric"])

class MainWindow:
    def __init__(self, master):
        self.master = master
        self.theme = DarkTheme(master)
        
        self.process_manager = ProcessManager()
        self.system_monitor = SystemMonitor()
        self.selected_pid = None
        self.update_thread = None
        self.running = True
        self.process_cache = {}
        self.last_update = 0
        
        self._create_ui()
        self._start_updates()
        
    def _create_ui(self):
        """Создает современный интерфейс"""
        # Главный контейнер
        main_container = tk.Frame(self.master, bg=COLOR_SCHEME["bg_darkest"])
        main_container.pack(fill=tk.BOTH, expand=True, padx=2, pady=2)
        
        # Верхняя панель
        self._create_header(main_container)
        
        # Центральная область
        center_frame = tk.Frame(main_container, bg=COLOR_SCHEME["bg_darkest"])
        center_frame.pack(fill=tk.BOTH, expand=True, pady=5)
        
        # Левая панель - системная информация
        self._create_system_panel(center_frame)
        
        # Правая панель - процессы
        self._create_process_panel(center_frame)
        
        # Нижняя панель - детали
        self._create_details_panel(main_container)
    
    def _create_header(self, parent):
        """Создает заголовок приложения"""
        header = tk.Frame(parent, bg=COLOR_SCHEME["bg_dark"], height=70)
        header.pack(fill=tk.X, pady=(0, 5))
        header.pack_propagate(False)
        
        # Логотип и название
        logo_frame = tk.Frame(header, bg=COLOR_SCHEME["bg_dark"])
        logo_frame.pack(side=tk.LEFT, padx=20, pady=15)
        
        tk.Label(logo_frame, text="GHHS", 
                fg=COLOR_SCHEME["text_white"], 
                bg=COLOR_SCHEME["bg_dark"],
                font=FONT_SCHEME["title"]).pack(side=tk.LEFT)
        
        tk.Label(logo_frame, text="Process", 
                fg=COLOR_SCHEME["accent_blue"], 
                bg=COLOR_SCHEME["bg_dark"],
                font=FONT_SCHEME["title"]).pack(side=tk.LEFT, padx=(5, 0))
        
        # Поиск
        search_frame = tk.Frame(header, bg=COLOR_SCHEME["bg_dark"])
        search_frame.pack(side=tk.LEFT, padx=20, pady=20)
        
        self.search_var = tk.StringVar()
        search_entry = tk.Entry(search_frame, textvariable=self.search_var,
                              bg=COLOR_SCHEME["bg_light"], fg=COLOR_SCHEME["text_white"],
                              insertbackground=COLOR_SCHEME["text_white"],
                              width=30, relief="flat")
        search_entry.pack(side=tk.LEFT)
        search_entry.bind("<KeyRelease>", self._on_search_changed)
        
        # Кнопки управления
        control_frame = tk.Frame(header, bg=COLOR_SCHEME["bg_dark"])
        control_frame.pack(side=tk.RIGHT, padx=20, pady=20)
        
        ModernButton(control_frame, "Обновить", self._refresh_processes, 
                    width=100, height=32).pack(side=tk.LEFT, padx=5)
        
        ModernButton(control_frame, "Завершить", self._kill_selected_process,
                    width=100, height=32, accent_color=COLOR_SCHEME["warning_red"]).pack(side=tk.LEFT, padx=5)
    
    def _create_system_panel(self, parent):
        """Создает панель системной информации"""
        sys_frame = tk.LabelFrame(parent, text="Системные метрики", 
                                 bg=COLOR_SCHEME["bg_darkest"],
                                 fg=COLOR_SCHEME["accent_blue"],
                                 font=FONT_SCHEME["heading"],
                                 relief="flat", bd=1)
        sys_frame.pack(side=tk.LEFT, fill=tk.Y, padx=(0, 5))
        sys_frame.config(width=300)
        
        # CPU
        cpu_frame = tk.Frame(sys_frame, bg=COLOR_SCHEME["bg_darkest"])
        cpu_frame.pack(fill=tk.X, padx=10, pady=10)
        
        self.cpu_progress = CircularProgress(cpu_frame, size=80)
        self.cpu_progress.pack(side=tk.LEFT)
        
        cpu_info = tk.Frame(cpu_frame, bg=COLOR_SCHEME["bg_darkest"])
        cpu_info.pack(side=tk.LEFT, padx=10, fill=tk.Y)
        
        tk.Label(cpu_info, text="Процессор", fg=COLOR_SCHEME["text_gray"],
                bg=COLOR_SCHEME["bg_darkest"], font=FONT_SCHEME["small"]).pack(anchor="w")
        self.cpu_label = tk.Label(cpu_info, text="0%", fg=COLOR_SCHEME["accent_blue"],
                                 bg=COLOR_SCHEME["bg_darkest"], font=FONT_SCHEME["metric"])
        self.cpu_label.pack(anchor="w")
        
        # RAM
        ram_frame = tk.Frame(sys_frame, bg=COLOR_SCHEME["bg_darkest"])
        ram_frame.pack(fill=tk.X, padx=10, pady=10)
        
        self.ram_progress = CircularProgress(ram_frame, size=80)
        self.ram_progress.pack(side=tk.LEFT)
        
        ram_info = tk.Frame(ram_frame, bg=COLOR_SCHEME["bg_darkest"])
        ram_info.pack(side=tk.LEFT, padx=10, fill=tk.Y)
        
        tk.Label(ram_info, text="Память", fg=COLOR_SCHEME["text_gray"],
                bg=COLOR_SCHEME["bg_darkest"], font=FONT_SCHEME["small"]).pack(anchor="w")
        self.ram_label = tk.Label(ram_info, text="0%", fg=COLOR_SCHEME["warning_red"],
                                 bg=COLOR_SCHEME["bg_darkest"], font=FONT_SCHEME["metric"])
        self.ram_label.pack(anchor="w")
        
        # Диск
        disk_frame = tk.Frame(sys_frame, bg=COLOR_SCHEME["bg_darkest"])
        disk_frame.pack(fill=tk.X, padx=10, pady=10)
        
        self.disk_progress = CircularProgress(disk_frame, size=80)
        self.disk_progress.pack(side=tk.LEFT)
        
        disk_info = tk.Frame(disk_frame, bg=COLOR_SCHEME["bg_darkest"])
        disk_info.pack(side=tk.LEFT, padx=10, fill=tk.Y)
        
        tk.Label(disk_info, text="Диск", fg=COLOR_SCHEME["text_gray"],
                bg=COLOR_SCHEME["bg_darkest"], font=FONT_SCHEME["small"]).pack(anchor="w")
        self.disk_label = tk.Label(disk_info, text="0%", fg=COLOR_SCHEME["success_green"],
                                  bg=COLOR_SCHEME["bg_darkest"], font=FONT_SCHEME["metric"])
        self.disk_label.pack(anchor="w")
    
    def _create_process_panel(self, parent):
        """Создает панель списка процессов"""
        process_frame = tk.Frame(parent, bg=COLOR_SCHEME["bg_darkest"])
        process_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        # Заголовок таблицы
        header_frame = tk.Frame(process_frame, bg=COLOR_SCHEME["bg_light"], height=30)
        header_frame.pack(fill=tk.X)
        header_frame.pack_propagate(False)
        
        columns = [
            ("PID", 80), ("Процесс", 200), ("CPU%", 80), 
            ("Память", 100), ("Потоки", 80), ("Пользователь", 150)
        ]
        
        for i, (text, width) in enumerate(columns):
            label = tk.Label(header_frame, text=text, bg=COLOR_SCHEME["bg_light"],
                           fg=COLOR_SCHEME["accent_blue"], font=FONT_SCHEME["small"])
            label.place(x=sum(w for _, w in columns[:i]), y=0, width=width, height=30)
        
        # Список процессов (Canvas-based для лучшей производительности)
        self.process_canvas = tk.Canvas(process_frame, bg=COLOR_SCHEME["bg_darkest"],
                                      highlightthickness=0)
        self.scrollbar = tk.Scrollbar(process_frame, orient=tk.VERTICAL,
                                    command=self.process_canvas.yview)
        self.process_canvas.configure(yscrollcommand=self.scrollbar.set)
        
        self.scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.process_canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        # Фрейм для элементов процессов
        self.process_items_frame = tk.Frame(self.process_canvas, bg=COLOR_SCHEME["bg_darkest"])
        self.canvas_window = self.process_canvas.create_window((0, 0), 
                                                             window=self.process_items_frame, 
                                                             anchor="nw")
        
        self.process_canvas.bind("<Configure>", self._on_canvas_configure)
        self.process_items_frame.bind("<Configure>", self._on_frame_configure)
        
    def _create_details_panel(self, parent):
        """Создает панель деталей процесса"""
        details_frame = tk.LabelFrame(parent, text="Детали процесса", 
                                     bg=COLOR_SCHEME["bg_darkest"],
                                     fg=COLOR_SCHEME["accent_blue"],
                                     font=FONT_SCHEME["heading"],
                                     relief="flat", bd=1, height=150)
        details_frame.pack(fill=tk.X, pady=(5, 0))
        details_frame.pack_propagate(False)
        
        self.details_text = tk.Text(details_frame, bg=COLOR_SCHEME["bg_light"],
                                   fg=COLOR_SCHEME["text_white"], font=FONT_SCHEME["monospace"],
                                   relief="flat", padx=10, pady=10, wrap=tk.WORD)
        
        scrollbar = tk.Scrollbar(details_frame, command=self.details_text.yview)
        self.details_text.configure(yscrollcommand=scrollbar.set)
        
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.details_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
    
    def _on_canvas_configure(self, event):
        self.process_canvas.itemconfig(self.canvas_window, width=event.width)
    
    def _on_frame_configure(self, event):
        self.process_canvas.configure(scrollregion=self.process_canvas.bbox("all"))
    
    def _start_updates(self):
        """Запускает обновление данных"""
        self._update_data()
    
    def _update_data(self):
        """Обновляет данные (вызывается периодически)"""
        if not self.running:
            return
            
        try:
            # Системная информация
            cpu_usage = self.system_monitor.get_cpu_usage()
            ram_usage = self.system_monitor.get_ram_usage()
            disk_usage = self.system_monitor.get_disk_usage()
            
            self.cpu_progress.set_value(cpu_usage)
            self.ram_progress.set_value(ram_usage)
            self.disk_progress.set_value(disk_usage)
            
            self.cpu_label.config(text=f"{cpu_usage}%")
            self.ram_label.config(text=f"{ram_usage}%")
            self.disk_label.config(text=f"{disk_usage}%")
            
            # Процессы (обновляем реже для производительности)
            current_time = time.time()
            if current_time - self.last_update > 2.0:  # Раз в 2 секунды
                self._refresh_processes()
                self.last_update = current_time
                
        except Exception as e:
            print(f"Update error: {e}")
        
        # Следующее обновление через 500мс
        self.master.after(500, self._update_data)
    
    def _refresh_processes(self):
        """Обновляет список процессов (оптимизированно)"""
        search_term = self.search_var.get().lower()
        
        # Очищаем старые элементы
        for widget in self.process_items_frame.winfo_children():
            widget.destroy()
        
        # Получаем процессы
        processes = self.process_manager.get_processes_fast()
        displayed_count = 0
        
        for i, proc in enumerate(processes):
            if displayed_count >= 1000:  # Лимит для производительности
                break
                
            if not search_term or search_term in proc['name'].lower():
                self._create_process_row(i, proc)
                displayed_count += 1
        
        # Обновляем скролл регион
        self.process_items_frame.update_idletasks()
        self.process_canvas.configure(scrollregion=self.process_canvas.bbox("all"))
    
    def _create_process_row(self, index, proc):
        """Создает строку процесса"""
        row_frame = tk.Frame(self.process_items_frame, 
                           bg=COLOR_SCHEME["bg_light"] if index % 2 == 0 else COLOR_SCHEME["bg_medium"],
                           height=25)
        row_frame.pack(fill=tk.X)
        row_frame.pack_propagate(False)
        
        # Привязываем события
        row_frame.bind("<Enter>", lambda e, f=row_frame: self._on_row_enter(f))
        row_frame.bind("<Leave>", lambda e, f=row_frame: self._on_row_leave(f, index))
        row_frame.bind("<Button-1>", lambda e, p=proc: self._on_process_select(p))
        
        # Колонки
        columns = [
            (proc['pid'], 80),
            (proc['name'], 200),
            (f"{proc['cpu']:.1f}%", 80),
            (f"{proc['memory']:.1f} MB", 100),
            (proc['threads'], 80),
            (proc['user'][:15], 150)
        ]
        
        for i, (text, width) in enumerate(columns):
            label = tk.Label(row_frame, text=text, 
                           bg=row_frame.cget('bg'),
                           fg=COLOR_SCHEME["text_white"], 
                           font=FONT_SCHEME["small"],
                           anchor="w")
            label.place(x=sum(w for _, w in columns[:i]), y=0, width=width, height=25)
            label.bind("<Button-1>", lambda e, p=proc: self._on_process_select(p))
    
    def _on_row_enter(self, row):
        row.config(bg=COLOR_SCHEME["highlight_light"])
        for child in row.winfo_children():
            child.config(bg=COLOR_SCHEME["highlight_light"])
    
    def _on_row_leave(self, row, index):
        bg_color = COLOR_SCHEME["bg_light"] if index % 2 == 0 else COLOR_SCHEME["bg_medium"]
        row.config(bg=bg_color)
        for child in row.winfo_children():
            child.config(bg=bg_color)
    
    def _on_process_select(self, proc):
        self.selected_pid = proc['pid']
        self._show_process_details(proc['pid'])
    
    def _on_search_changed(self, event):
        self.master.after(300, self._refresh_processes)  # Дебаунсинг
    
    def _show_process_details(self, pid):
        """Показывает детали процесса"""
        try:
            details = self.process_manager.get_process_details_fast(pid)
            self.details_text.delete(1.0, tk.END)
            
            info_text = f"""PID: {details['pid']}
Имя: {details['name']}
Пользователь: {details['user']}
Путь: {details['exe']}
CPU: {details['cpu']}%
Память: {details['memory']:.1f} MB
Потоки: {details['threads']}
Статус: {details['status']}"""
            
            self.details_text.insert(1.0, info_text)
        except:
            self.details_text.delete(1.0, tk.END)
            self.details_text.insert(1.0, "Информация недоступна")
    
    def _kill_selected_process(self):
        if self.selected_pid:
            if messagebox.askyesno("Подтверждение", f"Завершить процесс {self.selected_pid}?"):
                try:
                    self.process_manager.kill_process(self.selected_pid)
                    self._refresh_processes()
                except Exception as e:
                    messagebox.showerror("Ошибка", str(e))
    
    def __del__(self):
        self.running = False