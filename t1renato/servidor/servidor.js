/*
    Prof: Renato Ishii
    Celso Antonio Uliana Junior - October 2019
*/

/*
    Modulos core, fs(escrita e leitura) e net(socket)
*/
const net = require('net')
const fs = require('fs')

/*
    Constantes para comparação.
*/
const cp = 'cp'
const ls = 'ls'
const mkdir = 'mkdir'
const rmdir = 'rmdir'

/*
    Função para saber se é arquivo.
*/
const isArquivo = nomeArquivo => {
    return fs.lstatSync(nomeArquivo).isFile()
}

const getCommand = linha => {
    const com = linha.split(' ')
    return com[0]
}

const getParameter = linha => {
    const com = linha.split(' ')
    return com[1]
}

const server = net.createServer(socket => {
    socket.on('error', err => {
        console.log(err)
    })

    socket.on('data', data => {
        const linha = data.toString()
        const command = getCommand(linha)

        console.log(linha)
        console.log(command)

        if(command === mkdir){
            console.log('modo mkdir:\n')
            console.log(__dirname + '\\' + getParameter(linha))
            fs.mkdirSync(__dirname + '\\' + getParameter(linha), {recursive: true})
        }

        if(command === rmdir){
            console.log('modo rmdir:\n')
            console.log(__dirname + '\\' + getParameter(linha))
            fs.rmdirSync(__dirname + '\\' + getParameter(linha), {recursive: true})
        }

        if(command === ls){
            console.log('modo ls:\n')
            socket.write('Diretorios no host:\n')
            socket.write('-------------------\n')
        
            fs.readdirSync(__dirname).map(nomeArquivo => {
                if(!isArquivo(nomeArquivo))
                    socket.write('pasta: ' + nomeArquivo + '\n')
                else
                    socket.write(nomeArquivo + '\n')
            })        
        }

        if(command === cp){
            
        }
    })

})

server.listen(1337, '127.0.0.1')
