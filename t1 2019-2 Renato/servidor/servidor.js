/*
    Celso Antonio Uliana Junior - Nov 2019
*/

/*
    Modulos core, fs(escrita e leitura) e net(socket), funções e constantes.
*/
const fs = require('fs')
const net = require('net')
const { initServidor} = require('../util/funcoes')

/*
    Variavel que carrega o caminho do arquivo que será transferido.
*/
var path = {link : ''}

const server = net.createServer(socket => {

    console.log('Servidor rodando.\n')
    /*
        Variavel global imutavel que recebe o diretorio raiz de onde o servidor está rodando.
    */
    const globalPath = __dirname

    /*
        Array de dados caso o arquivo seja grande.
    */
    var pacotes = []
    
    initServidor(socket, path, pacotes, globalPath)
})

server.listen(1337, '127.0.0.1')