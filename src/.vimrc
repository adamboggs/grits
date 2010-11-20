set makeprg=make\ test
set tw=100

au FileType valgrind syn match gritsFile "\v<(grits-\w+|roam|elev|env|map|sat|test|radar)\.c>" containedin=valgrindSrc
au FileType valgrind hi link gritsFile Error
