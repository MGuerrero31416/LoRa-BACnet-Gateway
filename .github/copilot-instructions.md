# Repository Build Instructions

- This project requires ESP-IDF 6.0.2 from `C:\esp\v6.0.2\esp-idf`.
- Do not run bare `idf.py` from a normal PowerShell terminal; that terminal may not have ESP-IDF exported.
- For builds, run the repository wrapper from the workspace root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\build_idf60.ps1 build
```

- For clean rebuilds, run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\build_idf60.ps1 fullclean
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\build_idf60.ps1 build
```

- The wrapper activates and verifies the required ESP-IDF installation before invoking `idf.py`.

