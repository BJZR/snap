# Snap

Control de versiones local ultraminimalista.

## Características

- Todo local, sin servidor remoto
- Snapshots completos del proyecto
- Sin complejidad de branches, merges o conflictos
- Super ligero (menos de 250 líneas de C)
- Perfecto para proyectos personales
- Restauración instantánea

## Instalación

```bash
gcc snap.c -o snap
```

## Uso básico

```bash
# Iniciar repositorio en tu proyecto
./snap init

# Guardar snapshot del estado actual
./snap save "primera versión"
./snap save "agregué login"
./snap save "bug fix en database"

# Ver historial de snapshots
./snap list

# Restaurar a una versión anterior
./snap restore 1

# Ver diferencias con snapshot anterior
./snap diff 2
```

## Comandos

- `snap init` - Inicializa repositorio en el directorio actual
- `snap save [mensaje]` - Guarda snapshot con mensaje opcional
- `snap list` - Muestra todos los snapshots con fecha y mensaje
- `snap restore <id>` - Restaura proyecto completo a ese snapshot
- `snap diff <id>` - Muestra diferencias entre snapshot y estado actual

## Ejemplo de flujo de trabajo

```bash
$ ./snap init
✓ repositorio iniciado

$ ./snap save "versión inicial"
✓ snapshot #1 guardado

# ... trabajas en el código ...

$ ./snap save "agregué API"
✓ snapshot #2 guardado

# ... haces más cambios ...

$ ./snap list
Snapshots:
  #1 - versión inicial (Mon Jan 05 10:30:00 2026)
  #2 - agregué API (Mon Jan 05 14:22:15 2026)

# Algo salió mal, volver atrás
$ ./snap restore 1
✓ restaurado a snapshot #1
```

## Cómo funciona

Snap crea una carpeta `.snap` en tu proyecto donde guarda copias completas del directorio en cada snapshot. Los snapshots están numerados secuencialmente (1, 2, 3...).

Cada snapshot incluye:
- Copia completa de todos los archivos
- Mensaje descriptivo
- Timestamp de cuándo se creó

## Limitaciones

- No tiene merge ni branches (es intencional)
- Los snapshots ocupan espacio (son copias completas)
- Solo local, no sincroniza con otros
- Máximo 512 caracteres por ruta de archivo

Estas limitaciones mantienen el software simple y fácil de entender.

## Ventajas sobre git

- Muchísimo más simple de entender
- Sin curva de aprendizaje
- No necesitas entender conceptos complejos
- Perfecto para backups rápidos durante desarrollo
- Código fuente incluido (puedes modificarlo)

## Cuándo usar snap

- Proyectos personales pequeños/medianos
- Cuando solo necesitas "guardar puntos" en tu desarrollo
- Experimentación rápida (guardar antes de cambios grandes)
- Como complemento a git para snapshots locales frecuentes

## Licencia

Dominio público. Úsalo como quieras.