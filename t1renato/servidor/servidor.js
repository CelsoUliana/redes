/*
    Celso Antonio Uliana Junior - Nov 2019
*/

/*
    Modulos core, fs(escrita e leitura) e net(socket)
*/
const fs = require('fs')
const net = require('net')

const cp = 'cp'
const ls = 'ls'
const cd = 'cd'
const home = 'home'
const mkdir = 'mkdir'
const rmdir = 'rmdir'
const slash = process.platform === 'win32' ? '\\' : '/'

/*
    Variavel que carrega o caminho do arquivo que será transferido.
*/
var path = ''

/*
    Função para saber se é arquivo.
*/
const isArquivo = nomeArquivo => {
    return fs.lstatSync(nomeArquivo).isFile()
}

/*
    Funções de parsing de comandos.
*/
const getComando = linha => {
    const com = linha.split(' ')
    return com[0]
}

const getParametro = linha => {
    const com = linha.split(' ')
    return com[1]
}

const server = net.createServer(socket => {

    /*
        Variavel global imutavel que recebe o diretorio raiz de onde o servidor está rodando.
    */
    const globalPath = __dirname

    /*
        Array de dados caso o arquivo seja grande.
    */
    var pacotes = []

    socket.on('error', err => {
        console.log(err)
    })
    
    socket.on('data', data => {
        
        const linha = data.toString()
        const command = getComando(linha)

        //console.log(linha)
        //console.log(command)

        if(command === mkdir){
            console.log('modo mkdir:\n')
            console.log(__dirname + slash + getParametro(linha))
            fs.mkdirSync(__dirname + slash + getParametro(linha), {recursive: true})
        }

        if(command === rmdir){
            console.log('modo rmdir:\n')
            console.log(__dirname + slash + getParametro(linha))
            fs.rmdirSync(__dirname + slash + getParametro(linha), {recursive: true})
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

        else if(command === cp){
            console.log('Nome arquivo:' + getParametro(linha))
            path = getParametro(linha)
        }

        else if(path != null || path != ''){
            pacotes.push(data)
        }
    })

    socket.on('end', () => {
        console.log(__dirname + slash + path)
        const arquivo = Buffer.concat(pacotes)
        fs.writeFile(__dirname + slash + path, arquivo, err => {
            if(err){
                console.log(err)
            }
            else{
                console.log('Arquivo recebido com sucesso: ' + path)
            }
        })
        /*
            Limpeza de variaveis de arquivo após a transferencia
        */
        path = ''
        pacotes = []
    })
})

server.listen(1337, '127.0.0.1')