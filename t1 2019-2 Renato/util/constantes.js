/*
    Celso Antonio Uliana Junior - Nov 2019
*/

/*
    Constantes para comparação.
*/
const cp = 'cp'
const rm = 'rm'
const ls = 'ls'
const mkdir = 'mkdir'
const rmdir = 'rmdir'
const slash = process.platform === 'win32' ? '\\' : '/'


module.exports = {
    cp : cp,
    ls : ls,
    rm : rm,
    mkdir : mkdir,
    rmdir : rmdir,
    slash : slash
}