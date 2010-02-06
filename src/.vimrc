set makeprg=make\ test
set tw=100
au FileType valgrind syn match gisFile "\v<(gis-\w+|elev|env|map|sat|test)\.c>" containedin=valgrindSrc
au FileType valgrind hi link gisFile Error
