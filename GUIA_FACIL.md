# GUÍA FÁCIL: conseguir el plugin sin instalar nada

No necesitas saber programar. GitHub (gratis) compila el plugin en sus servidores y tú descargas el resultado. Solo se hace una vez; si luego cambiamos algo, repites los pasos 4 a 6.

## 1. Crea una cuenta en GitHub
Entra en https://github.com/signup y crea una cuenta gratuita.

## 2. Crea un repositorio
1. Arriba a la derecha pulsa **+** y luego **New repository**.
2. Nombre: `DG-MasterBus`.
3. Déjalo en **Public** (así la compilación es ilimitada y gratis; además JUCE gratuito exige código abierto GPL).
4. Pulsa **Create repository**.

## 3. Descomprime el zip en tu PC
Haz clic derecho en `DGMasterBus.zip` y elige "Extraer todo".

## 4. Sube los archivos
1. En tu repositorio nuevo pulsa **uploading an existing file**.
2. Abre la carpeta descomprimida, selecciona TODO su contenido (incluida la carpeta `.github`) y arrástralo a la ventana del navegador.
3. Abajo pulsa **Commit changes**.

Si la carpeta `.github` no se sube (a veces Windows la oculta), haz esto:
1. En el repositorio pulsa **Add file > Create new file**.
2. En el nombre escribe exactamente: `.github/workflows/build.yml`
3. Abre el archivo `build.yml` que viene suelto en el zip con el Bloc de notas, copia todo y pégalo ahí.
4. Pulsa **Commit changes**.

## 5. Espera la compilación
1. Ve a la pestaña **Actions** del repositorio.
2. Verás una ejecución llamada "Compilar DG-MasterBus" con un círculo amarillo. Tarda unos 10 a 15 minutos.
3. Cuando salga una marca verde, ya está.

Si sale una cruz roja, entra en esa ejecución, copia el mensaje de error y pégamelo en el chat; lo corrijo.

## 6. Descarga el plugin
1. Entra en la ejecución con la marca verde.
2. Abajo, en **Artifacts**, pulsa `DG-MasterBus-VST3-Windows`. Se descarga un zip.
3. Descomprímelo: dentro hay `DG-MasterBus.vst3`.

## 7. Instálalo
1. Copia `DG-MasterBus.vst3` a `C:\Program Files\Common Files\VST3\`
2. En Reaper: Options > Preferences > Plug-ins > VST > **Re-scan** (o "Clear cache and re-scan").
3. Reaper > Add FX en el track Master > busca "DG-MasterBus".

## 8. Ruteo en Reaper (5 buses al Master)
1. Track Master: botón **Route** > **Track channels = 10**.
2. En cada bus, en su envío al Master, elige el canal de destino: Drums 1/2, Guitars 3/4, Keys 5/6, Vocals 7/8, FX 9/10.
3. En el plugin del Master abre **Pin Connector** (botón "2 in 2 out" de la ventana del FX): conecta las 10 entradas del plugin a los canales 1 a 10 del track y las salidas a 1/2.

## Prueba rápida
Pon Tolerance = 0, Crosstalk = 0, Enhance apagado y compresor apagado. Con Gain Recovery = 13.98 dB la suma debe sonar igual que sin plugin, salvo el color del Drive. Si no, algún bus no está entrando por su par.

## Aviso de Windows
Al ser un plugin sin firmar, Windows puede mostrar un aviso (SmartScreen) al descomprimir el zip. Es normal: "Más información" > "Ejecutar de todas formas". Si no aparece el plugin en Reaper, clic derecho sobre el zip descargado > Propiedades > marca **Desbloquear** antes de extraerlo.
