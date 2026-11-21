# DG-MasterBus (Fase 1)

Plugin Master: suma pasiva de 5 buses estéreo -> Drive (3 modos, oversampling 4x) -> Enhance paralelo con Wet/Dry -> compresor VCA estilo SSL G.
GUI provisional genérica de JUCE (la GUI de aluminio y el vúmetro son la Fase 3).

## Compilar (Windows 11)

Requisitos: Visual Studio 2022 Community (carga de trabajo "Desarrollo para el escritorio con C++"), CMake 3.22+ y Git.

```
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

La primera configuración descarga JUCE 8.0.4 (necesita internet). El VST3 queda en:
`build\DGMasterBus_artefacts\Release\VST3\DG-MasterBus.vst3`
Cópialo a `C:\Program Files\Common Files\VST3\` (requiere permisos de administrador).

## Ruteo en Reaper

1. Track Master: pon "Track channels" (botón Route/I/O) en **10**.
2. En cada bus (Drums, Guitars, Keys, Vocals, FX): en el envío al Master/Parent, elige el canal de destino: Drums 1/2, Guitars 3/4, Keys 5/6, Vocals 7/8, FX 9/10.
3. Inserta DG-MasterBus en el Master y abre su Pin Connector: entradas 1-10 conectadas a los 10 pines de entrada del plugin; salidas 1/2 a las salidas 1/2.
4. Verifica con una prueba: silencia buses de uno en uno y comprueba que cada uno entra por su par.

## Prueba de nulo

Con Tolerance = 0, Crosstalk = 0, Drive al mínimo con el modo que menos coloree, Enhance off, compresor off y Gain Recovery = 13.98 dB, la suma debe coincidir con la suma normal del DAW (salvo la saturación del Drive). Sirve para validar el ruteo.

## Valores de partida (a ajustar de oído)

- Enhance: ganancia 4 y bias 0.30 (graves, 150 Hz) / 0.15 (agudos, 5 kHz) en Enhance.h.
- Compresor: detector de pico enlazado, rodilla 2 dB, release Auto de 1.2 s a 0.15 s según la reducción. Es una aproximación del SSL G, no un modelo del circuito.
- Cambiar de modo de Drive con audio sonando puede dar un click leve (pendiente: crossfade).

## Pendiente

- Plugin Sender (coloración por canal en cada bus, sin ruteo).
- Enhance dentro del camino con oversampling (ahora corre a la frecuencia normal).
- GUI final y vúmetro (usa `grMeterDb`).
