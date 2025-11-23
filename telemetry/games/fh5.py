"""Forza-Horizon 5 telemtry listener
Created by QWEN AI"""
import socket
import struct
from typing import Optional, Dict, Any

class ForzaTelemetry:
    def __init__(self, ip: str = "192.168.1.221", port: int = 60555):
        self.ip = ip
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        print((ip, port))
        self.sock.bind((self.ip, self.port))
        # self.sock.setblocking(False)  # или False, если нужен неблокирующий режим

        # Формат пакета: 1 int32 + 1 uint32 + 79 float32 = 4 + 4 + 316 = 324 байта
        self.struct_format = '<iI79f'

    def start(self):
        print('ForzaTelemetry() start()')

    def receive_raw_data(self) -> Optional[bytes]:
        """Получает один UDP-пакет (324 байта)."""
        try:
            data, _ = self.sock.recvfrom(1500)
            if len(data) == 324:
                return data
            else:
                print(f"Получен пакет неверного размера: {len(data)} байт")
                return None
        except Exception as e:
            print(f"Ошибка при получении данных: {e}")
            return None

    def get_data(self) -> Optional[Dict[str, Any]]:
        """
        Получает пакет и возвращает словарь с именованными полями.
        Возвращает None, если пакет недействителен или гонка не запущена.
        """
        raw = self.receive_raw_data()

        if raw is None:
            return None

        unpacked = struct.unpack(self.struct_format, raw)

        # Проверка: гонка активна?
        is_race_on = unpacked[0]
        if is_race_on != 1:
            return None  # Гонка не идёт — данные неактуальны

        data_dict = {
            # Служебные поля
            "is_race_on": is_race_on,
            "timestamp_ms": unpacked[1],

            # Engine
            "engine_max_rpm": unpacked[2],
            "engine_current_rpm": unpacked[3],

            # Acceleration (в g)
            "acceleration_x": unpacked[4],
            "acceleration_y": unpacked[5],
            "acceleration_z": unpacked[6],

            # Velocity (в м/с)
            "velocity_x": unpacked[7],
            "velocity_y": unpacked[8],
            "velocity_z": unpacked[9],

            # Angular velocity (рад/с)
            "angular_velocity_x": unpacked[10],
            "angular_velocity_y": unpacked[11],
            "angular_velocity_z": unpacked[12],

            # Orientation (в радианах)
            "yaw": unpacked[13],
            "pitch": unpacked[14],
            "roll": unpacked[15],

            # Speed (в м/с)
            "speed": unpacked[16],

            # Gear
            "gear": unpacked[17],  # 0 = N, 1-8 = передачи, -1 = R

            # Controls (0.0–1.0)
            "throttle": unpacked[18],
            "brake": unpacked[19],
            "clutch": unpacked[20],
            "handbrake": unpacked[21],

            # Suspension travel (в метрах)
            "suspension_travel_fl": unpacked[22],
            "suspension_travel_fr": unpacked[23],
            "suspension_travel_rl": unpacked[24],
            "suspension_travel_rr": unpacked[25],

            # Wheel data
            "wheel_slip_fl": unpacked[26],
            "wheel_slip_fr": unpacked[27],
            "wheel_slip_rl": unpacked[28],
            "wheel_slip_rr": unpacked[29],

            "wheel_rotation_speed_fl": unpacked[30],
            "wheel_rotation_speed_fr": unpacked[31],
            "wheel_rotation_speed_rl": unpacked[32],
            "wheel_rotation_speed_rr": unpacked[33],

            "wheel_on_rumble_strip_fl": unpacked[34],
            "wheel_on_rumble_strip_fr": unpacked[35],
            "wheel_on_rumble_strip_rl": unpacked[36],
            "wheel_on_rumble_strip_rr": unpacked[37],

            "wheel_in_puddle_fl": unpacked[38],
            "wheel_in_puddle_fr": unpacked[39],
            "wheel_in_puddle_rl": unpacked[40],
            "wheel_in_puddle_rr": unpacked[41],

            "surface_rumble_fl": unpacked[42],
            "surface_rumble_fr": unpacked[43],
            "surface_rumble_rl": unpacked[44],
            "surface_rumble_rr": unpacked[45],

            "tire_slip_ratio_fl": unpacked[46],
            "tire_slip_ratio_fr": unpacked[47],
            "tire_slip_ratio_rl": unpacked[48],
            "tire_slip_ratio_rr": unpacked[49],

            "tire_slip_angle_fl": unpacked[50],
            "tire_slip_angle_fr": unpacked[51],
            "tire_slip_angle_rl": unpacked[52],
            "tire_slip_angle_rr": unpacked[53],

            "tire_combined_slip_fl": unpacked[54],
            "tire_combined_slip_fr": unpacked[55],
            "tire_combined_slip_rl": unpacked[56],
            "tire_combined_slip_rr": unpacked[57],

            "suspension_travel_meters_fl": unpacked[58],
            "suspension_travel_meters_fr": unpacked[59],
            "suspension_travel_meters_rl": unpacked[60],
            "suspension_travel_meters_rr": unpacked[61],

            # Car damage (0.0 = целый, 1.0 = полностью разрушен)
            "car_ordinal": unpacked[62],
            "car_class": unpacked[63],
            "car_performance_index": unpacked[64],
            "drivetrain_type": unpacked[65],
            "num_cylinders": unpacked[66],

            # Position (в метрах от центра мира)
            "position_x": unpacked[67],
            "position_y": unpacked[68],
            "position_z": unpacked[69],

            # Speed (альтернативно, в км/ч)
            "speed_kmh": unpacked[70],  # ⚠️ В FH5 это поле НЕ заполняется! Используй speed * 3.6

            # Additional telemetry
            "power": unpacked[71],          # в ваттах
            "torque": unpacked[72],         # в ньютон-метрах

            "tire_temp_fl": unpacked[73],
            "tire_temp_fr": unpacked[74],
            "tire_temp_rl": unpacked[75],
            "tire_temp_rr": unpacked[76],

            "boost": unpacked[77],
            "fuel": unpacked[78],
            "dist_traveled": unpacked[79],
            "best_lap_time": unpacked[80],
        }

        return data_dict

    def close(self):
        """Закрывает сокет."""
        self.sock.close()