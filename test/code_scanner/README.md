# Example output
Please make sure the code can be clearly captured. If a qrcode can be succesfully decoded, you will be able to see information displayed as below:

To correctly decrypt the QR code, change the AES decryption key in menuconfig:
```
idf.py menuconfig
```

```
I (11164) APP_CODE_SCANNER: Decode time in 70 ms.
I (11164) APP_CODE_SCANNER: Decoded QR-Code symbol "﻿测试"
```