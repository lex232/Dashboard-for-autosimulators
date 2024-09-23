## Адаптация приборной панели Porsche 970 рестайлинг для автосимуляторов 
## Porsche cluster (car dashboard) Panamera 970/ Cayenne for Assetto Corsa auto-simulator
Отображаем данные из игры на реальной приборной панели от Porsche Panamera 970 рестайлинг (от Cayenne также должна подойти)

## Поддержка игр сейчас
- Assetto Corsa<br></br>
![](files/game_assetto.jpg)  

## Материалы, которые очень помогли
- За основу для Arduino + Can shield взят код -  [Arduino Porsche Cluster](https://github.com/VintageCollector/cluster-dashboard-ets2-ats/tree/Porsche-Panamera-970)
- Для доработок использованы команды из списка - [CAN-Bus-ID Porsche Cluster 970](https://github.com/VintageCollector/Porsche_Panamera_970-CAN-Bus-ID)
- Образец телеметрии для Assetto Corsa - [Assetto Telemetry Example](https://github.com/ladothlak/self-driving-NASCaiR)

## Что сделано
- Отправка основных данных на панель с Can шилда через Arduino
- Телеметрия Assetto Corsa и взаимодействие с шилдом
- Схема отображения топлива на приборной панели

## Схема подключения и структура проекта

Минимальная схема подключения<br></br>
![](files/wiring_scheme-min.jpg)

Структура проекта
<pre>
├── assetto_corsa - интеграция с Assetto Corsa 
    ├── assetto_corsa_telemetry_reader.py - скрипт для телеметрии 
    └── data_example_assetto_corsa.txt - формат данных в памяти
├── porsche_cluster_arduino - код для Arduino
    └── porsche_cluster_arduino.ino
├── files - разные файлы, схемы и т.д.
├── requirements.txt - зависимости для старта python скрипта
└── README.md - описание проекта
</pre>
