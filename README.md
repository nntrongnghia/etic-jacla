# ETIC - Jacla: Smart trash container

# File structure
There is 5 principle folders:
- `components` for standalone libraries other than the default libraries in ESP-IDF framework
- `include` for C header files
- `src` for source files with the same names as headers in `include`
- `main` for `main.c` and compilator configuration
- `test` for simple fucntionality test (QR code scanner, camera, RFID, etc.)

# Usage
```
idf.py set-target <target>
idf.py flash monitor
```
Run `idf.py --list-targets` to see supported targets.
For the main application, run those command in the root directory.
For demo, run the commands in the corresponding directory.

