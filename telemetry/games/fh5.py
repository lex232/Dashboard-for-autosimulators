"""Forza-Horizon 5 telemtry listener
Created by QWEN AI"""
import socket
import struct
from typing import Optional, Dict, Any

class ForzaTelemetry:
    def __init__(self, ip: str = "127.0.0.1", port: int = 60555):
        self.ip = ip
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 3072)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 3072)
        self.sock.bind((self.ip, self.port))
        # self.sock.setblocking(False)  # или False, если нужен неблокирующий режим

        # Формат пакета
        self.struct_format = '<iIfffffffffffffffffffffffffffffffffffffffffffffffffffiiiiifffffffffffffffffHBBBBBBbbb'

        self.data_dict = {
            "speed": 0,
            "rpm": 0,
            "gear": 0
        }

    def start(self):
        print('ForzaTelemetry() start()')

    def reset_dict_values(self, data_to_clear):
        for key in data_to_clear:
            data_to_clear[key] = 0

    def receive_raw_data(self) -> Optional[bytes]:
        """Получает один UDP-пакет (324 байта)."""
        try:
            data, _ = self.sock.recvfrom(1024)
            if len(data) == 324:
                return data[:232] + data[244:323]
            else:
                print(f"Получен пакет неверного размера: {len(data)} байт")
                return None
        except Exception as e:
            print(f"Ошибка при получении данных: {e}")
            return None

    def get_data(self) -> Optional[Dict[str, Any]]:
        """
        Получает пакет и возвращает словарь с именованными полями.
        """
        raw = self.receive_raw_data()
        if raw is None:
            return self.reset_dict_values(self.data_dict)

        unpacked = struct.unpack(self.struct_format, raw)

        # Проверка: гонка активна?
        is_race_on = unpacked[0]
        if is_race_on != 1:
            return self.reset_dict_values(self.data_dict)

        self.data_dict = {
            "rpm":  unpacked[4],
            "speed":  unpacked[61] * 3.6,
            "gear":  unpacked[81]
        }
        return self.data_dict

    def close(self):
        """Закрывает сокет."""
        self.sock.close()
