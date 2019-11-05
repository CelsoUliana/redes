/*
    Celso Antonio Uliana Junior - Nov 2019
*/

/*
    Constantes para comparação.
*/
const cp = 'cp'
const ls = 'ls'
const cd = 'cd'
const home = 'home'
const mkdir = 'mkdir'
const rmdir = 'rmdir'
const slash = process.platform === 'win32' ? '\\' : '/'


module.exports = {
    cp : cp,
    ls : ls,
    cd : cd,
    home : home,
    mkdir : mkdir,
    rmdir : rmdir,
    slash : slash
}