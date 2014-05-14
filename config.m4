PHP_ARG_ENABLE(evalhook, whether to enable evalhook support,
[  --enable-evalhook           Enable evalhook support])

if test "$PHP_EVALHOOK" != "no"; then
  PHP_NEW_EXTENSION(evalhook, evalhook.c, $ext_shared)
fi
