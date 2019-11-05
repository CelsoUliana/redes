/*
    Celso Antonio Uliana Junior - Nov 2019
*/

/*
    Modulo core fs.
*/
const fs = require('fs')
const net = require('net')
const pathResolver = require('path')
const {slash, mkdir, rmdir, ls, cp, cd, home} = require( pathResolver.resolve( __dirname, './constantes.js' ))

/*
    Função para saber se é arquivo.
*/
const isArquivo = nomeArquivo => {
    return fs.lstatSync(nomeArquivo).isFile()
}

/*
    Função para criar um diretorio dado um caminho.
*/
const funmkdir = string => {
    console.log('Modo mkdir:\n')
    console.log(string)
    fs.mkdirSync(string, {recursive: true})
}

/*
    Função para remover um diretorio dado um caminho.
*/
const funrmdir = string => {
    console.log('Modo rmdir: \n')
    console.log(string)
    fs.rmdirSync(string, {recursive: true})    
}

/*
    Função de pegar parametro de comando.
*/
const getParametro = linha => {
    const com = linha.split(' ')
    return com[1]
}

/*
    Função de pegar o comando.
*/
const getComando = linha => {
    const com = linha.split(' ')
    return com[0]
}


/*
    Função para configurar o Cliente.
*/
const initCliente = (socket, readline) => {

    /*
        Deixa a conexão principal aberta.
    */
    socket.setKeepAlive(true)

    /*
        Caso o evento erro seja disparado, irá ser escrito o log desse erro.
    */
    socket.on('error', err => console.log(err))

    /*
        Caso receba dados do servidor, escreva-os na tela.
    */
    socket.on('data', data => console.log(data.toString()))
    
    /*
        Quando o cliente.end() for chamado, fecha o readline também.
    */
    socket.on('close', () => {
        readline.close()
        console.log('Conexão fechada.')
    })
}

/*
    Executa o cliente.
*/
runClient = (cliente, rl) => {
    console.log('Conexão aberta.')

    rl.on('line', linha => {

        if(linha === 'close')
            cliente.end()
        
        const palavras = linha.split(' ')
        const comando = palavras[0]

        if(comando === mkdir || comando === rmdir || comando === ls)
            cliente.write(linha, () => {})

        else if(comando === cp){

            cliente.write(cp + ' ' + palavras[2], err => {

                if(err)
                    console.log(err)

                const clienteArquivo = new net.Socket()

                clienteArquivo.on('close', () => {
                    console.log('Transferencia Concluida.')
                })

                const arquivo = fs.readFile(palavras[1], (err, data) => {
                    if(!err){
                        console.log('Nome arquivo: ' + palavras[2])
                        console.log('tamanho arquivo: ' + data.length)
                        clienteArquivo.connect(1337, '127.0.0.1', () => {
                            clienteArquivo.write(data, () => {
                                clienteArquivo.end()
                            })
                            
                        })
                    }
                    else{
                        console.log(err)
                    }
                })
            })
        }
    })
}

/*
    Função para configurar o Servidor.
*/
const initServidor = (socket, path, pacotes, globalPath) => {

    socket.on('error', err => console.log(err))

    socket.on('data', data => {
        
        const linha = data.toString()
        const command = getComando(linha)

        if(command === mkdir){
            let temp = globalPath + slash + getParametro(linha)
            funmkdir(temp)
        }

        if(command === rmdir){
            let temp = globalPath + slash + getParametro(linha)
            funrmdir(temp)
        }

        if(command === ls){
            console.log('modo ls:\n')
            socket.write('Diretorios no host:\n')
            socket.write('-------------------\n')
        
            fs.readdirSync(globalPath).map(nomeArquivo => {
                if(!isArquivo(nomeArquivo))
                    socket.write('pasta: ' + nomeArquivo + '\n')
                else
                    socket.write(nomeArquivo + '\n')
            })        
        }

        else if(command === cp){
            console.log('Nome arquivo:' + getParametro(linha))
            path.link = getParametro(linha)
        }

        else if(path.link != ''){
            pacotes.push(data)
        }
    })

    socket.on('end', () => {
        if(path.link){
            console.log(globalPath + slash + path.link)
            const arquivo = Buffer.concat(pacotes)
            fs.writeFile(globalPath + slash + path.link, arquivo, err => {
                if(err){
                    console.log(err)
                }
                else{
                    console.log('Arquivo recebido com sucesso.')
                }
            })
            /*
                Limpeza de variaveis de arquivo após a transferencia
            */
            path.link = ''
            pacotes = []
    }
    })
}


module.exports = {
    funmkdir : funmkdir,
    funrmdir : funrmdir,
    isArquivo : isArquivo,
    runClient : runClient,
    getComando : getComando,
    initCliente : initCliente,
    initServidor : initServidor,
    getParametro : getParametro
}