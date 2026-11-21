# Guía: grabar tu Bus Driver real para calibrar el modelo

Para que el plugin suene como tu hardware y no como una curva genérica, necesito medir cómo se comporta el tuyo. Se hace en Reaper, con tu cadena de siempre, y lleva unos 30 minutos.

## Antes de empezar (seguridad)
- **Baja el volumen de los monitores y los auriculares al mínimo.** La señal de prueba incluye tonos graves fuertes y saturación.
- Deja el **Output Pad del Bus Driver activado (-18 dB)** en todas las grabaciones. Es un atenuador limpio y protege la entrada de tu interfaz. Yo lo tengo en cuenta.
- El **Input Pad** del Bus Driver, apagado.

## Preparación en Reaper
1. Pon el proyecto a **48 kHz** (Project settings, abajo a la izquierda).
2. Quita todos los plugins: ninguno en los tracks, ni en el Master (nada de DG-MasterBus ni DG-Sender).
3. Crea un track llamado **TEST OUT** e importa en él `DG_senal_de_prueba.wav` (arrástralo al track). Dura unos 47 segundos. Deja el fader a 0 dB y el pan al centro.
4. Envía **TEST OUT** a la salida de tu interfaz que va al **Mini-Bus**: elige un par de entradas del Mini-Bus (por ejemplo, el 1-2) y usa siempre el mismo. Deja las demás entradas del Mini-Bus **desconectadas**.
5. Crea otro track llamado **TEST IN**. Como entrada elige **solo el canal izquierdo** de la entrada que recibe lo que sale del Bus Driver (modo mono). Pon el modo de grabación en **Record: input**.
6. **No toques las ganancias de entrada ni de salida de tu interfaz RME ni del Ferrofish** durante todas las tomas. Es importante que sean iguales en todas.

## Las 6 tomas
Antes de cada toma, pon los dos conmutadores de Drive (izquierdo y derecho) en la **misma** posición y ajusta Boost como indica la tabla. Después activa la grabación en TEST IN, reproduce desde el principio y para al terminar el archivo. Renombra el audio grabado con el nombre de la tabla.

| Toma | Drive del Bus Driver | Boost | Nombre |
|------|----------------------|-------|--------|
| 1 | Mínimo | Apagado | T1_drive0_boostOFF |
| 2 | +16 dB (4 pasos sobre el mínimo) | Apagado | T2_drive16_boostOFF |
| 3 | +32 dB (8 pasos sobre el mínimo) | Apagado | T3_drive32_boostOFF |
| 4 | Mínimo | Encendido | T4_drive0_boostON |
| 5 | +16 dB | Encendido | T5_drive16_boostON |
| 6 | +32 dB | Encendido | T6_drive32_boostON |

Es normal que se enciendan los LEDs de clip en las tomas con Boost. Apunta en cuáles se encienden.

## Lo que me tienes que mandar
1. **Los 6 audios grabados** (clic derecho sobre el audio en Reaper y "Open in explorer" te lleva al archivo). Arrástralos al chat. Si pesan mucho, envíame primero T1, T3 y T4 y después los demás, o comprímelos en un zip.
2. Los **modelos y ajustes de referencia** de tu equipo: modelo de tu RME y del Ferrofish, y el nivel de referencia que tengan puesto en entradas y salidas (por ejemplo, +4 dBu).
3. **Qué par de entradas** del Mini-Bus usaste y cómo está cableado (cuántas entradas tiene y cómo van a la salida estéreo).
4. Las **tomas donde se encendieron los LEDs de clip** del Bus Driver.

## Qué hago yo con eso
Mido la respuesta en frecuencia, la ganancia, cuánto se comprime y qué armónicos aparecen según el nivel, y ajusto el modelo del Bus Driver para que reproduzca esas curvas. Sin esos datos, lo que hay ahora es una aproximación fundamentada, no una copia fiel.

## Opcional: Kazrog True Iron
Si tienes True Iron, pasa el mismo archivo de prueba por el plugin (en un track, con cada una de sus 6 transformadores y los ajustes por defecto) y expórtalo. Así puedo comparar lo que hace con mi modelo del transformador.
