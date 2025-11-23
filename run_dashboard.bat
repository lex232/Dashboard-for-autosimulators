echo "dashboard cluster CAN"
start /min cmd /k "cd / & cd PATH\Dashboard-for-autosimulators\ & venv\Scripts\activate & python telemetry/telemetry_reader.py"