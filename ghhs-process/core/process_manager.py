import psutil
import ctypes
from datetime import datetime

class ProcessManager:
    def get_processes_fast(self):
        """Быстрое получение списка процессов"""
        processes = []
        for proc in psutil.process_iter(['pid', 'name', 'cpu_percent', 'memory_info', 
                                        'num_threads', 'username', 'status']):
            try:
                # Быстрая проверка без детальной информации
                memory_mb = proc.info['memory_info'].rss / 1024 / 1024 if proc.info['memory_info'] else 0
                processes.append({
                    'pid': proc.info['pid'],
                    'name': proc.info['name'],
                    'cpu': proc.info['cpu_percent'] or 0,
                    'memory': memory_mb,
                    'threads': proc.info['num_threads'],
                    'user': proc.info['username'] or 'SYSTEM',
                    'status': proc.info['status']
                })
            except (psutil.NoSuchProcess, psutil.AccessDenied):
                continue
        
        # Сортируем по использованию CPU
        return sorted(processes, key=lambda x: x['cpu'], reverse=True)[:1000]  # Лимит
    
    def get_process_details_fast(self, pid):
        """Быстрая детальная информация"""
        try:
            proc = psutil.Process(pid)
            with proc.oneshot():  # Оптимизация - получаем все сразу
                return {
                    'pid': pid,
                    'name': proc.name(),
                    'user': proc.username(),
                    'exe': proc.exe() or 'N/A',
                    'cpu': proc.cpu_percent(),
                    'memory': proc.memory_info().rss / 1024 / 1024,
                    'threads': proc.num_threads(),
                    'status': proc.status()
                }
        except:
            return {'pid': pid, 'name': 'N/A', 'user': 'N/A', 'exe': 'N/A', 
                   'cpu': 0, 'memory': 0, 'threads': 0, 'status': 'N/A'}
    
    def kill_process(self, pid):
        try:
            psutil.Process(pid).kill()
        except Exception as e:
            raise Exception(f"Не удалось завершить процесс: {str(e)}")