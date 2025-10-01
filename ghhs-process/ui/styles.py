COLOR_SCHEME = {
    # Основные цвета
    "bg_darkest": "#0a0a0a",
    "bg_dark": "#111111",
    "bg_medium": "#1a1a1a",
    "bg_light": "#252525",
    "bg_lighter": "#2d2d2d",
    
    # Акцентные цвета
    "accent_blue": "#00d4ff",
    "accent_blue_dark": "#0099cc",
    "accent_blue_light": "#33e0ff",
    
    # Градиенты
    "gradient_start": "#00a2ff",
    "gradient_end": "#00f2ff",
    
    # Текст
    "text_white": "#ffffff",
    "text_gray": "#e0e0e0",
    "text_dark": "#a0a0a0",
    "text_darker": "#666666",
    
    # Статусные цвета
    "warning_red": "#ff4444",
    "warning_red_dark": "#cc3333",
    "success_green": "#44ff44",
    "highlight": "#00394d",
    "highlight_light": "#005580",
    
    # Границы
    "border_dark": "#333333",
    "border_light": "#444444"
}

FONT_SCHEME = {
    "title": ("Segoe UI Variable Display", 18, "bold"),
    "heading": ("Segoe UI Variable Text", 12, "bold"),
    "normal": ("Segoe UI Variable Text", 10),
    "small": ("Segoe UI Variable Text", 9),
    "monospace": ("Cascadia Code", 10),
    "metric": ("Segoe UI Variable Text", 11, "bold")
}

class DarkTheme:
    def __init__(self, root):
        self.root = root
        self._configure_styles()
    
    def _configure_styles(self):
        """Настройка современных стилей"""
        self.root.option_add("*Font", FONT_SCHEME["normal"])
        self.root.option_add("*Background", COLOR_SCHEME["bg_dark"])
        self.root.option_add("*Foreground", COLOR_SCHEME["text_white"])
        self.root.option_add("*selectBackground", COLOR_SCHEME["highlight"])
        self.root.option_add("*selectForeground", COLOR_SCHEME["text_white"])
        
        # Стиль для кнопок
        self.root.option_add("*Button.Background", COLOR_SCHEME["bg_light"])
        self.root.option_add("*Button.Foreground", COLOR_SCHEME["text_white"])
        self.root.option_add("*Button.Relief", "flat")
        self.root.option_add("*Button.BorderWidth", 0)
        self.root.option_add("*Button.PadX", 12)
        self.root.option_add("*Button.PadY", 6)
        
        # Стиль для Entry
        self.root.option_add("*Entry.Background", COLOR_SCHEME["bg_light"])
        self.root.option_add("*Entry.Foreground", COLOR_SCHEME["text_white"])
        self.root.option_add("*Entry.BorderWidth", 1)
        self.root.option_add("*Entry.Relief", "flat")
        
        # Стиль для Listbox
        self.root.option_add("*Listbox.Background", COLOR_SCHEME["bg_light"])
        self.root.option_add("*Listbox.Foreground", COLOR_SCHEME["text_white"])
        self.root.option_add("*Listbox.selectBackground", COLOR_SCHEME["highlight"])