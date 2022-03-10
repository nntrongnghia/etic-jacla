# Example esp_stream_jpg

Creates a http server and listen to `GET` requests at `http://[board-ip]/stream.jpg` with WIFI-STA mode. When the request is triggered, it streams QVGA JPEG image from the camera.

Configure your WiFi SSID and Password via `idf.py menuconfig`