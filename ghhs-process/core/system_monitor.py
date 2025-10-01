import psutil

class SystemMonitor:
    def get_cpu_usage(self):
        return psutil.cpu_percent(interval=0)
    
    def get_ram_usage(self):
        return psutil.virtual_memory().percent
    
    def get_disk_usage(self):
        try:
            return psutil.disk_usage('C:').percent
        except:
            return 0